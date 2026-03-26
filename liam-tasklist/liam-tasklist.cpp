/* liam-tasklist.cpp
Creating some simple functions of tasklist.exe in c++ for DFOR740 Midterm
This supports: a default view (no arguments), /V for verbose, and /SVC for services

Liam Salusky
With assistance of Claude, as I am nowhere near well versed enough in c++ to fully recreate tasklist.exe 


Declaration of AI Usage. 
AI assistance, Claude (Anthropic) was used during the development of this assignment.

How AI Was Used
- Understanding Windows APIs: AI helped me build a better understanding of how certain Windows API functions work, including 'CreateToolhelp32Snapshot', 'Process32First'/'Process32Next', 'GetProcessMemoryInfo', 'GetProcessTimes', 'OpenSCManagerA', and 'EnumServicesStatusExA'.
- Code structure guidance: AI guided me in organizing the program into clear functions('printDefault', 'printVerbose', 'printServices') and helped with formatting output to match 'tasklist.exe'.
- Debugging: AI helped me resolve a Unicode / ANSI compatibility issue where Visual Studio's default Unicode mode caused 'PROCESSENTRY32::szExeFile' to be incompatible with 'std::string'. 
    - During some troubleshooting, the AI started to lead me down a halucination into 'PROCESSENTRY32A' (ansi) to resolve a variable conversion issue, which does not actually exist. 


The AI generated code was reviewed, tested, and understood before its inclusion.

Other resources used:
- Win32 API (https[:]//learn.microsoft.com/en-us/windows/win32/api/) - Lots of reading to understand what is really happening during each step of our program's execution.
- YouTube "RED TEAM Recipes: Process Listing API: CreateToolhelp32Snapshot" (https[:]//www.youtube.com/watch?v=hLLd6UWYFYw) - Very helpful video that aided my understanding and application of the 'CreateToolhelp32Snapshot' win32 api
- Stackoverflow "How to get the process name in C++" (https[:]//stackoverflow.com/questions/4570174/how-to-get-the-process-name-in-c) - This helped me understand usage of OpenProcess
*/




// Without setting default to ANSI, szExeFile will not work due to some wide string conversion problems
#undef UNICODE
#undef _UNICODE

#include <iostream>
#include <string>
#include <Windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <iomanip>


// Start of our program
//      Reading and checking for args
//      Validating passed flags
//      Error if invalid flag
// =====================
int main(int argc, char* argv[]) {
    // Declare flags
    bool showVerbose = false;
    bool showServices = false;

    // Take command line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        // Convert argument to uppercase so "/v" and "/V" are treated the same
        for (char& c : arg) {
            c = toupper(c);
        }

        // Check for flags
        // Verbose 
        if (arg == "/V") {
            showVerbose = true;
        }
        // Services
        else if (arg == "/SVC") {
            showServices = true;
        }
        // Display the help menu
        else if (arg == "/?") {
            std::cout << "\nliam-tasklist [ /SVC /V ]\n\n";
            std::cout << "Description:\n";
            std::cout << "      This tool aims to recreate some of the functionality present in tasklist.exe\n\n";

            std::cout << "Parameter List:\n";
            std::cout << "      Running liam-tasklist with no flags will present the standard process list.\n\n";
            std::cout << "      /SVC                    Displays services hosted in each process.\n";
            std::cout << "      /V                      Displays verbose task information.\n";
            std::cout << "      /?                      Displays this help message.\n\n";
			
			std::cout << "Examples:\n";
            std::cout << "      liam-tasklist\n";
			std::cout << "      liam-tasklist /V\n";
			std::cout << "      liam-tasklist /SVC\n";


            return 0; // Exit without error
        }
        // Catch unknown arguments
        else {
            std::cout << "ERROR: Invalid argument/option - '" << argv[i] << "'\n";
            std::cout << "Type \"liam-tasklist /? \" for usage.";
            return 1;
        }
    } // End taking command line args

    // Routing based on the flags
    if (showVerbose && showServices) {
        std::cerr << "ERROR: The /V and /SVC switch cannot be used together.\n";
        return 1; // Exit with an error code
    }


    // Verbose output
    //      Uses CreateToolhelp32Snapshot to take a snapshot of running processes,
    //      Prints header for output,
    //      Walk through the snapshot using Process32First/Process32Next, iterate until no more processes
    //      Collect all of the information for each process using OpenProcess, GetProcessMemoryInfo, OpenProcessToken, GetTokenInformation, GetProcessTimes, GetWindowThreadProcessId, and LookupAccountSidA
    //      So we can collect the image name, PID, session name + number, memory usage, process status, name of user (by querying with the Security Identifier), cpu time, and then the title of the window for a verbose output
    //      Iterate with a do while loop to get info on each process, then
    //      Trims and prints output for the user
    // =====================
    // 
    // If the user intends to get a verbose listing by passing /V
    // Heavy lifting here helped by Claude, this was the most challenging part of tasklist for me
    else if (showVerbose) {
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE) {
            std::cerr << "Error: Could not get process snapshot." << std::endl;
            return 1; // Exit with error if we could not get a snapshot of running processes
        }

        // Per documentation, this is required otherwise calls will fail
        PROCESSENTRY32 entry;
        entry.dwSize = sizeof(PROCESSENTRY32);

        // Printing header for verbose output
        std::cout << std::left << std::setw(26) << "Image Name"
            << std::right << std::setw(8) << "PID" << " "
            << std::left << std::setw(16) << "Session Name"
            << std::right << std::setw(12) << "Session#"
            << std::right << std::setw(14) << "Mem Usage" << " "
            << std::left << std::setw(16) << "Status"
            << std::left << std::setw(30) << "User Name"
            << std::right << std::setw(12) << "CPU Time" << " "
            << std::left << "Window Title" << std::endl;
        // Printing underline for verbose output
        std::cout << std::string(25, '=') << " "
            << std::string(8, '=') << " "
            << std::string(15, '=') << "  "
            << std::string(11, '=') << " "
            << std::string(13, '=') << " "
            << std::string(15, '=') << " "
            << std::string(29, '=') << " "
            << std::string(12, '=') << " "
            << std::string(30, '=') << std::endl;



        // Process32First returns a boolean value, so we want to check and verify that there is atleast one process in our snapshot
        // The following do while loop iterates through all processes until the buffer has no process references
        if (Process32First(snapshot, &entry)) {
            do {
                DWORD sessionId = 0; // Declaring sessionId to use in the next line
                ProcessIdToSessionId(entry.th32ProcessID, 
                    &sessionId);
                std::string sessionName = (sessionId == 0) ? "Services" : "Console";

                // Initialization and default declarations of process information
                std::string memString = "N/A";
                std::string cpuTime = "0:00:00";
                std::string status = "Unknown";
                std::string userName = "N/A";
                std::string windowTitle = "N/A";

                // Declare a handle to interface with the OpenProcess output
                HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                    FALSE, entry.th32ProcessID);
                if (hProc != NULL) {
                    // Using GetProcessMemoryInfo to check how much memory the process is using, then convert it to KB from Bytes
                    PROCESS_MEMORY_COUNTERS memInfo;
                    if (GetProcessMemoryInfo(hProc, 
                            &memInfo, 
                            sizeof(memInfo))) 
                    {
                        SIZE_T kb = memInfo.WorkingSetSize / 1024;
                        memString = std::to_string(kb) + " K";
                    }


                    
                    HANDLE hToken = NULL;
                    // Open the process token, get the Security Identifier (SID), then look up the account name.
                    // This tells us which user account is running the process.
                    if (OpenProcessToken(hProc, 
                            TOKEN_QUERY, 
                            &hToken)) {
                        // First ask how big the buffer needs to be
                        DWORD tokenSize = 0;
                        GetTokenInformation(hToken, 
                            TokenUser, 
                            NULL, 
                            0, 
                            &tokenSize);

                        // Allocate and get the token info
                        BYTE* tokenBuffer = new BYTE[tokenSize];
                        if (GetTokenInformation(hToken, 
                            TokenUser, 
                            tokenBuffer,
                            tokenSize, 
                            &tokenSize)) {
                            
                            // The token info contains a SID 
                            // which we can look up to get DOMAIN\Username
                            TOKEN_USER* tokenUser = (TOKEN_USER*)tokenBuffer;
                            char name[256] = { 0 };
                            char domain[256] = { 0 };
                            DWORD nameLen = 256;
                            DWORD domainLen = 256;
                            SID_NAME_USE sidType;

                            if (LookupAccountSidA(NULL, 
                                tokenUser->User.Sid,
                                name, 
                                &nameLen,
                                domain, 
                                &domainLen, 
                                &sidType)) {
                                userName = std::string(domain) + "\\" + std::string(name);
                            }
                        }
                        delete[] tokenBuffer;
                        CloseHandle(hToken);
                    }

                    // GetProcessTimes gives us how much CPU time the process has used.
                    // It returns kernel time and user time
                    // We add them together to get total CPU time.
                    FILETIME createTime, exitTime, kernelTime, userTime;
                    if (GetProcessTimes(hProc, 
                        &createTime, 
                        &exitTime,
                        &kernelTime, 
                        &userTime)) {
                        
                        // FILETIME is in 100-nanosecond chunks, so we convert to seconds.
                        // We use ULARGE_INTEGER to combine the two 32-bit halves.
                        ULARGE_INTEGER kernel, user;
                        kernel.LowPart = kernelTime.dwLowDateTime;
                        kernel.HighPart = kernelTime.dwHighDateTime;
                        user.LowPart = userTime.dwLowDateTime;
                        user.HighPart = userTime.dwHighDateTime;

                        // Add kernel + user, divide by 10 million to get seconds
                        unsigned long long totalSeconds =
                            (kernel.QuadPart + user.QuadPart) / 10000000ULL;

                        unsigned long long hrs = totalSeconds / 3600;
                        unsigned long long mins = (totalSeconds % 3600) / 60;
                        unsigned long long secs = totalSeconds % 60;

                        // Build a text string to show CPU time, like how tasklist.exe shows (00:00:00)
                        cpuTime = std::to_string(hrs) + ":"
                            + (mins < 10 ? "0" : "") + std::to_string(mins) + ":"
                            + (secs < 10 ? "0" : "") + std::to_string(secs);
                    }

                    status = "Running";
                    CloseHandle(hProc);
                }

                // Getting the Window title 
                // Instead we look for any visible window owned by this process.
                struct FindData { // Declare a FindData struct, to get the Window Title for a verbose output
                    DWORD pid;
                    std::string title;
                };
                FindData findData;
                findData.pid = entry.th32ProcessID; 
                findData.title = "N/A";

                // EnumWindows calls our lambda once for every window on screen
                // We check if each window belongs to our PID until we get one
                EnumWindows([](HWND hwnd, LPARAM param) -> BOOL {
                    FindData* data = reinterpret_cast<FindData*>(param);
                    DWORD windowPid = 0;
                    GetWindowThreadProcessId(hwnd, &windowPid);

                    if (windowPid == data->pid && IsWindowVisible(hwnd)) {
                        char title[256];
                        GetWindowTextA(hwnd, title, 256);
                        if (strlen(title) > 0) {
                            data->title = title;
                            return FALSE; // Stop looking, we found a Window Title
                        }
                    }
                    return TRUE; // Keep looking
                    }, reinterpret_cast<LPARAM>(&findData));

                windowTitle = findData.title;

                // Trim the process name to 25 chars max as tasklist does the same in testing
                std::string imageName = entry.szExeFile;
                if (imageName.length() > 25) {
                    imageName = imageName.substr(0, 25);
                }

                // Print the verbose row
                std::cout << std::left << std::setw(26) << imageName
                    << std::right << std::setw(8) << entry.th32ProcessID << " "
                    << std::left << std::setw(16) << sessionName
                    << std::right << std::setw(12) << sessionId
                    << std::right << std::setw(14) << memString << " "
                    << std::left << std::setw(16) << status
                    << std::left << std::setw(30) << userName
                    << std::right << std::setw(12) << cpuTime << " "
                    << std::left << windowTitle << std::endl;

            } while (Process32Next(snapshot, &entry)); // Continue until we run out of processes
        }

        CloseHandle(snapshot); //clean up

    } // END verbose output handling

    // Now we progress to collecting service information
    //      We use OpenSCManagerA, EnumServicesStatusExA, CreateToolhelp32Snapshot, Process32First/Process32Next, ProcessIdToSessionId, and OpenProcess to:
    //      Interface with the Service Control Manager (services.exe)
    //      Create a pointer to reference the list of all services
    //      Take a snapshot to compare the services list to pids to gather service names
    // =====================
    // SERVICES: If the user intends to get a list of services by passing /SVC
    else if (showServices) {
        
        // Open a connection to the Service Control Manager, which keeps track of all services
        SC_HANDLE scManager = OpenSCManagerA(NULL, // Open a handle to the service control manager
            NULL, 
            SC_MANAGER_ENUMERATE_SERVICE);

        if (scManager == NULL) { // If we cant open a handle to Service Control Manager, exit
            std::cout << "Error: Could not open Service Control Manager." << std::endl;
            return 1; // exit with error
        }

        // Windows makes us call EnumServicesStatusEx twice:
        //   1st call with no buffer - tells us how big the buffer needs to be
        //   2nd call with the right sized buffer - actually gives us the data
        DWORD bytesNeeded = 0;
        DWORD serviceCount = 0;
        DWORD resumeHandle = 0;

        // 1st call: just asking "how much space do I need?"
        EnumServicesStatusExA(scManager, // Ask Service Control Manager
            SC_ENUM_PROCESS_INFO,
            SERVICE_WIN32, 
            SERVICE_ACTIVE,
            NULL, 
            0,
            &bytesNeeded, 
            &serviceCount, 
            &resumeHandle, 
            NULL);

        // Allocate a buffer big enough to hold all the service info from our 1st call
        BYTE* buffer = new BYTE[bytesNeeded];

        // 2nd call: actually get the service data
        if (!EnumServicesStatusExA(scManager, // Get data back from Service Control Manager, passing in the buffer we had to get in 1st call to account for memory needed
            SC_ENUM_PROCESS_INFO,
            SERVICE_WIN32, 
            SERVICE_ACTIVE,
            buffer, 
            bytesNeeded,
            &bytesNeeded, 
            &serviceCount,
            &resumeHandle, 
            NULL)) {
            
            // If we get some sort of error in getting data on services, send an error, and exit
            std::cerr << "Error: Could not list services." << std::endl;
            // Clear buffer and clean up before exiting
            delete[] buffer;
            CloseServiceHandle(scManager);
            return 1; // exit with error
        }

        // The buffer holds a pointer to where all the status information on processes can be found at
        ENUM_SERVICE_STATUS_PROCESSA* services = (ENUM_SERVICE_STATUS_PROCESSA*)buffer;

        // We need to take a process snapshot to look up process names by PID
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

        // Print header
        std::cout << std::left << std::setw(26) << "Image Name"
            << std::right << std::setw(8) << "PID"
            << "  "
            << std::left << "Services" << std::endl;

        std::cout << "========================= ========  ============================================\n";

        // Loop through each service
        for (DWORD i = 0; i < serviceCount; i++) {
            DWORD pid = services[i].ServiceStatusProcess.dwProcessId;
            if (pid == 0) continue; // Skip PID 0, that's the idle process

            std::string processName = "Unknown";
            PROCESSENTRY32 pe;
            pe.dwSize = sizeof(PROCESSENTRY32);
            // Look up the process name by walking through the snapshot of the process list until we reach it 
            if (Process32First(snapshot, &pe)) {
                do {
                    if (pe.th32ProcessID == pid) { // Once we have a match on the PID,  
                        processName = pe.szExeFile; // set the processName to the name of the process identifier by the PID,
                        break; // then exit the current do loop, and proceed to print our output
                    } 
                } while (Process32Next(snapshot, &pe)); // Keep walking down the snapshot until we hit the end of the snapshot
            }

            // Trim process name to 25 chars max
            if (processName.length() > 25) {
                processName = processName.substr(0, 25);
            }

            // Print the row: process name, PID, service name
            std::cout << std::left << std::setw(26) << processName
                << std::right << std::setw(8) << pid
                << "  "
                << std::left << services[i].lpServiceName << std::endl;
        } // Now repeat until we are out of services to iterate through.

        // Clean up
        delete[] buffer;
        CloseHandle(snapshot);
        CloseServiceHandle(scManager);
        return 0;
    } // END of /SVC output



   // Default output
   //      Uses CreateToolhelp32Snapshot to take a snapshot of running processes,
   //      Prints header for output,
   //      Walk through the snapshot using Process32First/Process32Next, iterate until no more processes
   //      Collect all of the information for each process using OpenProcess, GetProcessMemoryInfo, OpenProcessToken, GetTokenInformation, GetProcessTimes
   //      We can get the image name, pid, seesion name and number, as well as the memory being used by the process in KB
   // =====================
    // If no errors happen, and no flags are passed, we get to the default 'tasklist' execution
    else {
        // Take a snapshot
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

        // If we don't set the structure size, it breaks! (thanks Win32 api documentation)
        PROCESSENTRY32 entry;
        entry.dwSize = sizeof(PROCESSENTRY32);

        // Print the column headers, nice column formatting with setw :)
        std::cout << std::left << std::setw(26) << "Image Name"
            << std::right << std::setw(8) << "PID"
            << "  "
            << std::left << std::setw(16) << "Session Name"
            << std::right << std::setw(10) << "Session#"
            << std::right << std::setw(14) << "Mem Usage" << std::endl;

        // Print a line of ='s under headers for formatting
        std::cout << "=========================  =======  =============== ========= =============\n";

        // Get first entry, then repeat!
        if (Process32First(snapshot, &entry)) {
            do {
                DWORD sessionId = 0;
                ProcessIdToSessionId(entry.th32ProcessID, // Use ProcessIdToSessionId feeding the PID of the process, storing the result in sessionId
                    &sessionId);

                // 0 is for a system service and anything else (1) is a user service
                std::string sessionName = (sessionId == 0) ? "Services" : "Console";

                // Declare the default value for an amount of memory used by a process
                std::string memString = "N/A";

                // Create handle for OpenProcess to read memory information about a process, feed in the PID to query memory use
                HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 
                    FALSE, 
                    entry.th32ProcessID);

                // See how much memory the process is using
                PROCESS_MEMORY_COUNTERS memInfo;
                if (GetProcessMemoryInfo(hProc, 
                    &memInfo,
                    sizeof(memInfo))) {

                    // Convert from bytes to KB
                    SIZE_T kb = memInfo.WorkingSetSize / 1024;
                    memString = std::to_string(kb) + " K";
                }
                CloseHandle(hProc);

                // Trim the process name to 25 characters max, like how tasklist.exe does
                std::string image_name = entry.szExeFile;
                if (image_name.length() > 25) {
                    image_name = image_name.substr(0, 25);
                }

                // Print row for this process
                std::cout << std::left << std::setw(25) << image_name
                    << std::right << std::setw(8) << entry.th32ProcessID
                    << "  "
                    << std::left << std::setw(16) << sessionName
                    << std::right << std::setw(10) << sessionId
                    << std::right << std::setw(14) << memString << std::endl;

            } while (Process32Next(snapshot, &entry)); // Continue to iterate through until we hit the end of the processes
        }
        // Cleaning things up
        CloseHandle(snapshot);
        
    } // END of no flag operation.
    return 0; // Exit, program executed successfully
}
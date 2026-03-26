/* liam-taskkill.cpp
Creating some simple functions of taskkill.exe in c++ for DFOR740 Midterm
This supports: /PID <pid>, /IM <image name>, /F, and /T
    Unfortunately I did not give myself ample time to spend getting the /U or /S flags to work, so they are not included

Liam Salusky
With assistance of Claude, as I am nowhere near well versed enough in c++ to fully recreate taskkill.exe


Declaration of AI Usage.
AI assistance, Claude (Anthropic) was used during the development of this assignment.

How AI Was Used
- Understanding Windows APIs: AI helped me build a better understanding of how certain Windows API functions work, including CreateToolhelp32Snapshot, Process32First/Process32Next, OpenProcess, TerminateProcess, & GetWindowThreadProcessId
- Code structure guidance: AI guided me in organizing the program into a clearer to understand program
- Debugging: AI helped me resolve a Unicode / ANSI compatibility issue where Visual Studio's default Unicode mode caused 'PROCESSENTRY32::szExeFile' to be incompatible with 'std::string'.
- Building BFS search for the /T flag
- Applying wildcard search function for /IM
- Cleaning up variable names as mine were all over the place to begin with

AI generated code was reviewed, tested, and understood before its acceptance.

Other resources used:
- Win32 API (https[:]//learn.microsoft.com/en-us/windows/win32/api/) - Lots of reading to understand what is really happening during each step of the program's execution.
- GeeksforGeeks, Wildcard Pattern Matching (https[:]//www.geeksforgeeks.org/dsa/wildcard-pattern-matching/) 
- GeeksforGeeks, Breadth First Search (https[:]//www.geeksforgeeks.org/dsa/breadth-first-search-or-bfs-for-a-graph/)
*/

// Without setting default to ANSI, by undefining UNICODE, szExeFile will not work due to some wide string conversion problems
#undef UNICODE
#undef _UNICODE

#include <windows.h>   // Core Win32 API
#include <tlhelp32.h>  // Toolhelp32: snapshot and iteration of running processes
#include <stdio.h>     // printf, fprintf for console output
#include <stdlib.h>    // strtoul for converting PID strings to numbers
#include <string.h>    // String utilities



// Setting limits to be used later
#define MAX_PIDS    256
#define MAX_IMAGES  32
#define MAX_PROCS   4096
#define MAX_NAME    260
#define MAX_STR     512


// Setting a struct and helper function for later

typedef struct {
    DWORD pid;
    BOOL posted;
} EnumCtx;

// EnumWindowsProc
//      Callback for to enumerate through windows, used during graceful termination.
//      Windows calls this once for every window on the desktop and we check if it belongs to the process we're trying to close.
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    EnumCtx* ctx = (EnumCtx*)lParam;

    DWORD winPid = 0;
    // Check which process owns this window, store in winPid
    GetWindowThreadProcessId(hwnd, &winPid); 

    if (winPid == ctx->pid) { // If the window matches the PID we are looking for:
        // Send WM_CLOSE, the nice way to ask a program to close
        PostMessageA(hwnd, WM_CLOSE, 0, 0);
        ctx->posted = TRUE; // Lets us know this executed properly
    }

    return TRUE; // Keep looking, process might have multiple windows, we want to close them all
}


// Here we start the main function
// =========================
int main(int argc, char* argv[]) {

    // Processing command line args through if statements (and a for loop for case sensitivity)
    //      make flags case-insensitive,
    //      set flags as TRUE if applicable,
    //      and verifying that flags are valid and have enough arguments
    //      
    // =========================
    // If less than 2 arguments, show help and exit
    if (argc < 2) {
        printf("\nliam-taskkill [/F] [/T] [/PID pid | /IM imagename]\n\n");
        printf("Description:\n");
        printf("      This tool aims to recreate some of the functionality present in taskkill.exe\n\n");
        printf("Parameter List:\n");
        printf("      /PID <pid>        Kill process by process ID.\n");
        printf("      /IM <imagename>   Kill process by image name. Wildcards (* ?) accepted.\n");
        printf("      /F              Force-terminate the process (no graceful close).\n");
        printf("      /T              Terminate the process and all its child processes.\n");
        printf("      /?              Displays this help message.\n");
        return 1;
    }

    // Declare flags
    BOOL useForce = FALSE;
    BOOL useTree = FALSE;

    // Setting up storage for PIDs specified with /PID flag
    DWORD targetPids[MAX_PIDS];
    int targetPidCount = 0;

    // Setting up storage for image name patterns specified with /IM
    char imageNames[MAX_IMAGES][MAX_NAME];
    int imageCount = 0;

    // Process command line arguments
    for (int i = 1; i < argc; i++) {
        // Copy the argument so we can uppercase it without modifying the original
        char arg[MAX_STR];
        lstrcpynA(arg, argv[i], MAX_STR);

        // Convert argument to uppercase so "/f" and "/F" are treated the same - case insensitivity 
        for (int c = 0; arg[c]; c++) {
            arg[c] = (char)(unsigned char)CharUpperA((LPSTR)(unsigned char)arg[c]);
        }

        // Check for flags
        
        // Force kill
        if (lstrcmpA(arg, "/F") == 0) {
            useForce = TRUE;
        }
        // Tree kill
        else if (lstrcmpA(arg, "/T") == 0) {
            useTree = TRUE;
        }
        // Process ID — the next argument after /PID should be the actual number
        else if (lstrcmpA(arg, "/PID") == 0) {
            // Make sure there's actually a value after /PID
            if (++i >= argc) {
                fprintf(stderr, "ERROR: Invalid syntax. Value expected for '/PID'.\n");
                return 1;
            }

            // Convert the PID string to a number
            // strtoul sets 'end' to point past the last character it read
            char* end = NULL;
            DWORD pid = (DWORD)strtoul(argv[i], &end, 10);
            if (*end != '\0') {
                fprintf(stderr, "ERROR: Invalid PID value \"%s\".\n", argv[i]);
                return 1;
            }
            if (targetPidCount < MAX_PIDS) {
                targetPids[targetPidCount++] = pid;
            }
        }
        // Image name — the next argument after /IM should be the name or pattern
        else if (lstrcmpA(arg, "/IM") == 0) {
            // Make sure there's actually a value after /IM
            if (++i >= argc) {
                fprintf(stderr, "ERROR: Missing value for /IM.\n");
                return 1;
            }
            if (imageCount >= MAX_IMAGES) {
                fprintf(stderr, "ERROR: Too many /IM arguments.\n");
                return 1;
            }

            // Store the pattern in lowercase for case-insensitive matching later
            int ci = 0;
            for (; argv[i][ci] && ci < MAX_NAME - 1; ci++) {
                imageNames[imageCount][ci] = (char)(unsigned char)CharLowerA((LPSTR)(unsigned char)argv[i][ci]);
            }
            imageNames[imageCount][ci] = '\0';
            imageCount++;
        }
        // Display the help menu if user passes /?
        else if (lstrcmpA(arg, "/?") == 0) {
            printf("\nliam-taskkill [/F] [/T] [/PID pid | /IM imagename]\n\n");
            printf("Description:\n");
            printf("      This tool aims to recreate some of the functionality present in taskkill.exe\n\n");
            printf("Parameter List:\n");
            printf("      /PID <pid>        Kill process by process ID.\n");
            printf("      /IM <imagename>   Kill process by image name. Wildcards (* ?) accepted.\n");
            printf("      /F              Force-terminate the process (no graceful close).\n");
            printf("      /T              Terminate the process and all its child processes.\n");
            printf("      /?              Displays this help message.\n");
            return 0; // Exit without error
        }
        // Catch unknown arguments, show how to get the help menu
        else {
            fprintf(stderr, "ERROR: Invalid argument/option - '%s'\n", argv[i]);
            fprintf(stderr, "Type \"liam-taskkill /?\" for usage.\n");
            return 1;
        }
    }

    // After parsing, make sure the user actually specified something to kill
    if (targetPidCount == 0 && imageCount == 0) {
        fprintf(stderr, "ERROR: No process specified. Use /PID or /IM.\n");
        fprintf(stderr, "Type \"liam-taskkill /?\" for usage.\n");
        return 1;
    }



    // Take a snapshot of all running processes with CreateToolhelp32Snapshot
    //      Create arrays to hold process info from the process list snapshot later
    //      We need this to resolve /IM names to PIDs and to display process names in output
    //      Walk the snapshot using Process32First/Process32Next and save each to the procNames/procPids lists
    // =========================
    // 'static' for performance and to avoid crashing
    static DWORD procPids[MAX_PROCS];
    static char procNames[MAX_PROCS][MAX_NAME];
    int procCount = 0;

    // Gets us a snapshot of all running processes
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "ERROR: Could not create process snapshot.\n");
        return 1;
    }

    // If we don't set the structure size, it breaks!
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    // Walk through the snapshot, grabbing each process's PID and name, storing in procNames for later access
    if (Process32First(snapshot, &entry)) {
        do {
            if (procCount < MAX_PROCS) {
                procPids[procCount] = entry.th32ProcessID;
                lstrcpynA(procNames[procCount], entry.szExeFile, MAX_NAME);
                procCount++;
            }
        } while (Process32Next(snapshot, &entry)); // Continue to iterate through each PID and name until we reach the end
    }
    CloseHandle(snapshot);

    // This for loop will iterate through image names to make sure wildcard matching works
    //      
    // =========================
    // Resolve /IM imagename patterns to actual PIDs
    // For each pattern, we scan all processes and check if the name matches
    for (int im = 0; im < imageCount; im++) {
        BOOL matched = FALSE;

        for (int p = 0; p < procCount; p++) {
            // Lowercase the process name so comparison is case-insensitive
            char nameLow[MAX_NAME];
            int ci = 0;
            for (; procNames[p][ci] && ci < MAX_NAME - 1; ci++) {
                nameLow[ci] = (char)(unsigned char)CharLowerA((LPSTR)(unsigned char)procNames[p][ci]);
            }
            nameLow[ci] = '\0';

            // Declaring variables for wildcard matching
            const char* pattern = imageNames[im];
            const char* text = nameLow;
            const char* starP = NULL;
            const char* starT = NULL;
            BOOL isMatch = TRUE;

            // Wildcard matching, adapted from a string matching algorithm from GeeksforGeeks, made usable with Claude
            // 
            // =========================
            while (*text) {
                if (*pattern == '?' || *pattern == *text) {
                    // Characters match, or ? matches anything — move forward
                    pattern++;
                    text++;
                }
                else if (*pattern == '*') {
                    // Save where the star was so we can come back
                    starP = pattern++;
                    starT = text;
                }
                else if (starP) {
                    // Didn't match, but we had a * earlier
                    // Go back and let the * eat one more character
                    pattern = starP + 1;
                    text = ++starT;
                }
                else {
                    // No match and no * to fall back to
                    isMatch = FALSE;
                    break;
                }
            }
            // Any leftover *'s at the end match nothing, that's fine
            while (*pattern == '*') pattern++;
            if (*pattern != '\0') isMatch = FALSE;

            if (isMatch) {
                if (targetPidCount < MAX_PIDS) {
                    targetPids[targetPidCount++] = procPids[p];
                }
                matched = TRUE;
            } 
        }
        // If the wildcard search resulted in no matches, print the following error
        if (!matched) { 
            fprintf(stderr, "ERROR: The process \"%s\" not found.\n", imageNames[im]);
        }// END Wildcare matching
    }

    int failures = 0; // Variable to track failed attempts, to return at completion of program which will result in either an exit of code 0 or 1

    // Verify that PIDs that we are about to kill actually exist in the snapshot taken earlier
    //      For loop to iterate through to check that the target PID(s) we requested to terminate are in our list of pids from earlier
    // =========================
    {
        DWORD validPids[MAX_PIDS];
        int validCount = 0;
        
        for (int i = 0; i < targetPidCount; i++) {
            BOOL found = FALSE;
            for (int p = 0; p < procCount; p++) {
                if (procPids[p] == targetPids[i]) {
                    found = TRUE;
                    break;
                }
            }
            if (found) {
                validPids[validCount++] = targetPids[i];
            }
            else {
                fprintf(stderr, "ERROR: The process with PID \"%lu\" not found.\n",
                    (unsigned long)targetPids[i]);
                failures++;
            }
        } // END verifying PID existance

        // Replace targetPids with only the valid ones
        for (int i = 0; i < validCount; i++) {
            targetPids[i] = validPids[i];
        }
        targetPidCount = validCount;
    }

    // If nothing matched at all, nothing to kill, exit
    if (targetPidCount == 0) {
        return 1;
    }


    // Declare counter for child processes
    DWORD childPids[MAX_PIDS];
    int childCount = 0;
    
    // If /T was specified, find all child processes
    //      Use CreateToolhelp32Snapshot to snapshot, iterate through all the processes, 
    //      
    // =========================
    if (useTree) { 
        HANDLE treeSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (treeSnap != INVALID_HANDLE_VALUE) { // So long as there is no problem getting the snapshot, we will continue

            // Declaring variables to store child/parent PIDs
            static DWORD treePids[MAX_PROCS];
            static DWORD treeParents[MAX_PROCS];
            int treeCount = 0;
            // Grab every process PID and parent PID
            PROCESSENTRY32 pe;
            pe.dwSize = sizeof(PROCESSENTRY32);
            if (Process32First(treeSnap, &pe)) {
                do {
                    if (treeCount < MAX_PROCS) {
                        treePids[treeCount] = pe.th32ProcessID;
                        treeParents[treeCount] = pe.th32ParentProcessID;
                        treeCount++;
                    }
                } while (Process32Next(treeSnap, &pe));
            }
            CloseHandle(treeSnap);

            // Initialize counting variables for child processes, to verify counts
            DWORD visited[MAX_PIDS];
            int visitedCount = 0;
            DWORD queue[MAX_PIDS];
            int queueCount = 0;

            // Count up and verify we have seen each child pid
            for (int i = 0; i < targetPidCount; i++) {
                if (visitedCount < MAX_PIDS) {
                    visited[visitedCount++] = targetPids[i];
                    queue[queueCount++] = targetPids[i];
                }
            }

            // Keep going until we run out of children to find
            int head = 0;
            while (head < queueCount) {
                DWORD cur = queue[head++];


                // Now we will iterate through to check if a process is the child of a parent process
                // 
                // =========================
                // Check every process to see if its parent is 'cur'
                for (int j = 0; j < treeCount; j++) {
                    if (treeParents[j] == cur) {
                        // Make sure we haven't already seen this PID
                        BOOL alreadySeen = FALSE;
                        // We do a Breadth First Search (BFS) — start with our target PIDs,
                        // then find their children, then their children's children, etc.
                        
                        // Conceptually, we are searching for all adjacent processes, then delving deeper, finding more adjacent processes, until we hit the end and have seen everything
                        for (int v = 0; v < visitedCount; v++) {
                            if (visited[v] == treeParents[j] || visited[v] == treePids[j]) {
                                if (visited[v] == treePids[j]) { alreadySeen = TRUE; break; }
                            }
                        }
                        if (!alreadySeen && visitedCount < MAX_PIDS) {
                            visited[visitedCount++] = treePids[j];
                            queue[queueCount++] = treePids[j];
                        }
                    }
                }
            }

            // Pull out just the children processes (not the original parent, we already have that), store into childPids
            // =========================
            for (int i = 0; i < visitedCount; i++) {
                BOOL isRoot = FALSE;
                // See if we have already seen the PID, confirm it is not parent PID
                for (int t = 0; t < targetPidCount; t++) {
                    if (visited[i] == targetPids[t]) { isRoot = TRUE; break; }
                }
                if (!isRoot && childCount < MAX_PIDS) {
                    childPids[childCount++] = visited[i];
                }
            }
        } 
    }

    // Iterate through in reverse order, so deepest child processes are terminated before their parent process
    //      Terminate processes in the next code block 
    // =========================
    for (int i = childCount - 1; i >= 0; i--) { // If there are any counts of child processes, we will run this iteration to terminate the deepest one(s) first.
        const char* name = "<unknown>";
        // Find the and set the name for this PID so we can display it later
        for (int p = 0; p < procCount; p++) {
            if (procPids[p] == childPids[i]) {
                name = procNames[p];
                break;
            }
        }

        // Use OpenProcess and start terminating child PIDs, in the reverse order specified above
        //      Error if errors happen as needed
        // =========================
        HANDLE hProc = OpenProcess(PROCESS_TERMINATE, FALSE, childPids[i]);
        if (!hProc) {
            DWORD err = GetLastError();
            if (err == ERROR_ACCESS_DENIED) {
                fprintf(stderr, "ERROR: Access denied terminating PID %lu. Run as administrator.\n",
                    (unsigned long)childPids[i]);
            }
            else {
                fprintf(stderr, "ERROR: Could not open process %lu (error %lu).\n",
                    (unsigned long)childPids[i], (unsigned long)err);
            }
            failures++;
            continue;
        }

        if (!TerminateProcess(hProc, 1)) { // If we are not able to terminate a process, notify the user of the error
            fprintf(stderr, "ERROR: Failed to terminate PID %lu (error %lu).\n",
                (unsigned long)childPids[i], (unsigned long)GetLastError());
            CloseHandle(hProc);
            failures++;
            continue;
        }

        CloseHandle(hProc);
        printf("SUCCESS: The process \"%s\" with PID %lu has been terminated.\n",
            name, (unsigned long)childPids[i]);
    } // END identifying child PIDs


    // This next loop will kill the target processes, 
    //      iterating through our count of targetPIDs for how many to terminate
    //      this does not handle child processes, those have already been terminated
    //      
    // =========================
    for (int i = 0; i < targetPidCount; i++) { // Iterate until we have gone through the count of all target PIDs
        // Declare a name variable for processes as we iterate through our list
        const char* name = "<unknown>";
        // Find the name for display
        for (int p = 0; p < procCount; p++) {
            if (procPids[p] == targetPids[i]) {
                name = procNames[p];
                break;
            }
        }

        // If /F was NOT specified, graceful close
        //      Send WM_CLOSE to all the process's windows (like clicking the X button)
        //      Callback to our EnumWindows function defined toward the top, to send shutdown signals and receieve confirmation of closing
        // =========================
        if (!useForce) {
            EnumCtx ctx;
            ctx.pid = targetPids[i];
            ctx.posted = FALSE;

            // EnumWindows calls our callback for every window on the desktop, as a process might have multiple windows
            EnumWindows(EnumWindowsProc, (LPARAM)&ctx); // Send the current PID we are at over to our callback function, 

            if (ctx.posted) { //if the callback function ctx.postedreturns TRUE, then the process was ended (non-forcefully) 
                // We found windows and sent WM_CLOSE, now wait for it to exit
                HANDLE hWait = OpenProcess(SYNCHRONIZE, FALSE, targetPids[i]); // Open the process to see if the process will close
                if (hWait) {
                    // give it 3 seconds to close on its own
                    DWORD wait = WaitForSingleObject(hWait, 3000);
                    CloseHandle(hWait);
                    // WAIT_OBJECT_0 means the process actually exited
                    if (wait == WAIT_OBJECT_0) {
                        printf("SUCCESS: Sent termination signal to the process \"%s\" with PID %lu.\n",
                            name, (unsigned long)targetPids[i]);
                        continue; // Done with this process, continue back up at our for loop to continue terminating processes
                    }
                }
            }

            // If we get here, graceful close didn't work, let the user know of an error
            fprintf(stderr, "ERROR: The process \"%s\" with PID \"%lu\" could not be terminated.\nReason: This process can only be terminated forcefully (with /F option).\n",
                name,
                (unsigned long)targetPids[i]);
            failures++;
            continue;
        }

        // Forced path (/F) -- if we want to FORCE close an application
        HANDLE hProc = OpenProcess(PROCESS_TERMINATE, FALSE, targetPids[i]);
        if (!hProc) {
            DWORD err = GetLastError();
            if (err == ERROR_ACCESS_DENIED) {
                fprintf(stderr, "ERROR: The process with PID %lu could not be terminated.\nReason: Access is denied.\n",
                    (unsigned long)targetPids[i]);
            }
            else {
                fprintf(stderr, "ERROR: Could not open process %lu (error %lu).\n",
                    (unsigned long)targetPids[i], (unsigned long)err);
            }
            failures++;
            continue;
        }

        // If we get an error terminating a process, log to user
        if (!TerminateProcess(hProc, 1)) {
            fprintf(stderr, "ERROR: Failed to terminate PID %lu (error %lu).\n",
                (unsigned long)targetPids[i], (unsigned long)GetLastError());
            CloseHandle(hProc);
            failures++;
            continue;
        }

        CloseHandle(hProc); //Cleanup!
        printf("SUCCESS: The process \"%s\" with PID %lu has been terminated.\n",
            name, (unsigned long)targetPids[i]);
    }

    return failures > 0 ? 1 : 0; // If we have more than 1 failure, return an error, otherwise return no error.
}
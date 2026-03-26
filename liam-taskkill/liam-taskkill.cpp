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
- Applying wildcard search function
- Cleaning up variable names as mine were all over the place to begin with

AI generated code was reviewed, tested, and understood before its acceptance.

Other resources used:
- Win32 API (https[:]//learn.microsoft.com/en-us/windows/win32/api/) - Lots of reading to understand what is really happening during each step of our program's execution.
- GeeksforGeeks, Wildcard Pattern Matching (https[:]//www.geeksforgeeks.org/dsa/wildcard-pattern-matching/) 
- GeeksforGeeks, Breadth First Search (https[:]//www.geeksforgeeks.org/dsa/breadth-first-search-or-bfs-for-a-graph/)
*/


// Without setting default to ANSI, by undefining UNICODE, szExeFile will not work due to some wide string conversion problems
#undef UNICODE
#undef _UNICODE

#include <windows.h>   // Core Win32 API (process management, window messaging, etc.)
#include <tlhelp32.h>  // Toolhelp32: snapshot and iteration of running processes
#include <stdio.h>     // printf, fprintf for console output
#include <stdlib.h>    // strtoul for converting PID strings to numbers
#include <string.h>    // String utilities



// Limits for our arrays
#define MAX_PIDS    256
#define MAX_IMAGES  32
#define MAX_PROCS   4096
#define MAX_NAME    260
#define MAX_STR     512



// EnumWindowsProc — Callback for EnumWindows, used during graceful termination.
// Windows calls this once for every window on the desktop and we check if it belongs to the process we're trying to close.
typedef struct {
    DWORD pid;
    BOOL posted;
} EnumCtx;

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    EnumCtx* ctx = (EnumCtx*)lParam;

    // Check which process owns this window
    DWORD winPid = 0;
    GetWindowThreadProcessId(hwnd, &winPid);

    if (winPid == ctx->pid) {
        // Send WM_CLOSE, the nice way to ask a program to close
        PostMessageA(hwnd, WM_CLOSE, 0, 0);
        ctx->posted = TRUE;
    }

    return TRUE; // Keep looking, process might have multiple windows
}



int main(int argc, char* argv[]) {
    // If no arguments at all, show help and exit
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

    // Storage for PIDs specified with /PID flag
    DWORD targetPids[MAX_PIDS];
    int targetPidCount = 0;

    // Storage for image name patterns specified with /IM
    char imageNames[MAX_IMAGES][MAX_NAME];
    int imageCount = 0;

    // Take command line arguments
    for (int i = 1; i < argc; i++) {
        // Copy the argument so we can uppercase it without modifying argv
        char arg[MAX_STR];
        lstrcpynA(arg, argv[i], MAX_STR);

        // Convert argument to uppercase so "/f" and "/F" are treated the same
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
        // Display the help menu
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
        // Catch unknown arguments
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



    // Take a snapshot of all running processes
    // We need this to resolve /IM names to PIDs and to display process names in output

    // Arrays to hold process info from the process list snapshot down a few lines
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

    // Walk through the snapshot, grabbing each process's PID and name, storing in procNames
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

            // Wildcard matching, adapted from a common string matching algorithm as seen on GeeksforGeeks
            const char* pattern = imageNames[im];
            const char* text = nameLow;
            const char* starP = NULL;
            const char* starT = NULL;
            BOOL isMatch = TRUE;

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
        }
    }

    int failures = 0;

    // Verify that PIDs that we are about to kill actually exist in the snapshot
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
        }

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


    // If /T was specified, find all child processes
    // We do a Breadth First Search (BFS) — start with our target PIDs,
    // then find their children, then their children's children, etc.
    DWORD childPids[MAX_PIDS];
    int childCount = 0;

    if (useTree) {
        HANDLE treeSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (treeSnap != INVALID_HANDLE_VALUE) {

            // Grab every process PID and parent PID
            static DWORD treePids[MAX_PROCS];
            static DWORD treeParents[MAX_PROCS];
            int treeCount = 0;

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

            // BFS queue — we start with target PIDs and keep finding children
            DWORD visited[MAX_PIDS];
            int visitedCount = 0;
            DWORD queue[MAX_PIDS];
            int queueCount = 0;

            // Seed with our target PIDs
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

                // Check every process to see if its parent is 'cur'
                for (int j = 0; j < treeCount; j++) {
                    if (treeParents[j] == cur) {
                        // Make sure we haven't already seen this PID
                        BOOL alreadySeen = FALSE;
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

            // Pull out just the children (not the original targets, we already have those)
            for (int i = 0; i < visitedCount; i++) {
                BOOL isRoot = FALSE;
                for (int t = 0; t < targetPidCount; t++) {
                    if (visited[i] == targetPids[t]) { isRoot = TRUE; break; }
                }
                if (!isRoot && childCount < MAX_PIDS) {
                    childPids[childCount++] = visited[i];
                }
            }
        }
    }

    // /T Kill child processes first (bottom-up)
    // Go in reverse so deepest children die before their parents
    for (int i = childCount - 1; i >= 0; i--) {
        // Find the name for this PID so we can display it
        const char* name = "<unknown>";
        for (int p = 0; p < procCount; p++) {
            if (procPids[p] == childPids[i]) {
                name = procNames[p];
                break;
            }
        }

        // Open and terminate
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

        if (!TerminateProcess(hProc, 1)) {
            fprintf(stderr, "ERROR: Failed to terminate PID %lu (error %lu).\n",
                (unsigned long)childPids[i], (unsigned long)GetLastError());
            CloseHandle(hProc);
            failures++;
            continue;
        }

        CloseHandle(hProc);
        printf("SUCCESS: The process \"%s\" with PID %lu has been terminated.\n",
            name, (unsigned long)childPids[i]);
    }


    // Kill the target processes
    for (int i = 0; i < targetPidCount; i++) {
        // Find the name for display
        const char* name = "<unknown>";
        for (int p = 0; p < procCount; p++) {
            if (procPids[p] == targetPids[i]) {
                name = procNames[p];
                break;
            }
        }

        // If /F was NOT specified, try graceful close first
        // Send WM_CLOSE to all the process's windows (like clicking the X button)
        if (!useForce) {
            EnumCtx ctx;
            ctx.pid = targetPids[i];
            ctx.posted = FALSE;

            // EnumWindows calls our callback for every window on the desktop
            EnumWindows(EnumWindowsProc, (LPARAM)&ctx);

            if (ctx.posted) {
                // We found windows and sent WM_CLOSE, now wait for it to exit
                HANDLE hWait = OpenProcess(SYNCHRONIZE, FALSE, targetPids[i]);
                if (hWait) {
                    // WAIT_OBJECT_0 means the process actually exited
                    // give it 3 seconds to close on its own
                    DWORD wait = WaitForSingleObject(hWait, 3000);
                    CloseHandle(hWait);
                    if (wait == WAIT_OBJECT_0) {
                        printf("SUCCESS: Sent termination signal to the process \"%s\" with PID %lu.\n",
                            name, (unsigned long)targetPids[i]);
                        continue; // Done with this one
                    }
                }
            }

            // If we get here, graceful close didn't work
            fprintf(stderr, "ERROR: The process \"%s\" with PID \"%lu\" could not be terminated.\nReason: This process can only be terminated forcefully (with /F option).\n",
                name,
                (unsigned long)targetPids[i]);
            failures++;
            continue;
        }

        // Forced path (/F)
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

        // TerminateProcess immediately kills it, same as ending it in Task Manager
        // The '1' is the exit code
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

    return failures > 0 ? 1 : 0;
}

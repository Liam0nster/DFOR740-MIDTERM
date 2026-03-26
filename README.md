# DFOR740-MIDTERM, Spring 2026
DFOR740 Midterm assignment for the tasklist/taskkill C++ binaries.

Liam Salusky

This project consists of two parts: Tasklist and Taskkill. C++ code and built binaries can be found in their corresponding folders "[liam-tasklist]([url](https://github.com/Liam0nster/DFOR740-MIDTERM/tree/main/liam-tasklist))" and "[liam-taskkill]([url](https://github.com/Liam0nster/DFOR740-MIDTERM/tree/main/liam-taskkill))".

## liam-tasklist.exe
This binary supports:
- Running with no flags for a standard "tasklist.exe" output
- /V flag for verbose output
- /SVC flag for services output
- /? flag for help

### liam-tasklist.exe (no flags)
<img width="711" height="596" alt="image" src="https://github.com/user-attachments/assets/bfeb1a0b-ba97-45e6-8f3c-54c64f32a8c3" /></br>
- Above, we can see the beginning of output from running liam-tasklist.exe, piped into more, to catch the headers.
- We are running this from the "C:\\Users...liam-tasklist\x64\Release" folder
- To run from any directory, you would need to add liam-tasklist.exe to your system path

### liam-tasklist.exe /V (verbose output)

### liam-tasklist.exe /SVC (service output)

### liam-tasklist.exe /? (service output)


## liam-taskkill.exe

### liam-taskkill.exe (no flags)

### liam-taskkill.exe /F [/PID processid | /IM imagename]

### liam-taskkill.exe /T [/PID processid | /IM imagename]

### liam-taskkill.exe /PID processid

### liam-taskkill.exe /IM imagename


## Building Binaries
Visual Studio was used to compile the binaries through separate projects. 

To compile yourself:
- Create a new blank project,
- Copy the associated .cpp file into the Source Files in the Solution Explorer,
- Provide a name for the file,
- Add, then copy the code into the file
<img width="613" height="517" alt="image" src="https://github.com/user-attachments/assets/efedc611-fd07-440e-87a7-333cfc0f9d33" />
<img width="350" height="287" alt="image" src="https://github.com/user-attachments/assets/be317907-feb2-45bd-928e-83459d0660e4" />
<img width="450" height="151" alt="image" src="https://github.com/user-attachments/assets/a4140890-4816-4d6a-aca4-badf833303aa" />

 - Select "Release"
 - Shortcut: <Ctrl + b> will build the binary (Windows 11).
 - <img width="762" height="154" alt="image" src="https://github.com/user-attachments/assets/28e05e11-cc5a-491e-8159-7c4b81702260" />
 - You will see output of where the binary was saved in the console at the bottom of Visual Studio.

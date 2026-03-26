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
<img width="724" height="803" alt="image" src="https://github.com/user-attachments/assets/b44fb97c-efad-402d-b690-64329fded73b" /></br>
- Above, we can see the beginning of output from running liam-tasklist.exe
- In the directory with the executable, run `liam-tasklist.exe`
- To run from any directory, you would need to add liam-tasklist.exe to your system path, or move the .exe into the folder you want to run it from.

### liam-tasklist.exe /V (verbose output)
<img width="1526" height="514" alt="image" src="https://github.com/user-attachments/assets/2eb5cc48-ceed-4027-9bf7-2c82ab7583de" /></br>
- Pass the /V flag for a verbose output
- In the directory with the executable, run `liam-tasklist.exe /V`


### liam-tasklist.exe /SVC (service output)
<img width="753" height="483" alt="image" src="https://github.com/user-attachments/assets/2a26021d-e61d-480a-b7b2-1472c982016d" /></br>
- Pass the /SVC flag for a listing of services
- In the directory with the executable, run `liam-tasklist.exe /SVC`

### liam-tasklist.exe /? (service output)
<img width="767" height="352" alt="image" src="https://github.com/user-attachments/assets/5f1b4c0b-d799-4131-98ae-48ce9c98ee74" /></br>
- Pass the /? flag for the help menu
- In the directory with the executable, run `liam-tasklist.exe /?`

<img width="1587" height="17" alt="image" src="https://github.com/user-attachments/assets/658e1e52-a512-497a-8e88-8cb4c3cc6727" /></br>


## liam-taskkill.exe
This binary supports:
- /PID pid flag to kill a process by the process id
- /IM imagename flag to kill a process by its image name
- /F flag to forcefully terminate a process
- /T flag to terminate the process and each of its child processes
- /? flag to bring up the help menu

### liam-taskkill.exe (no flags)
<img width="770" height="262" alt="image" src="https://github.com/user-attachments/assets/8a71d703-b570-4c31-80d4-1f0a2d26ad54" /></br>
- Passing no flags or arguments will result in the help page coming up
- In the directory with the executable, run `liam-taskkill.exe`

### liam-taskkill.exe /F [/PID processid | /IM imagename]
<img width="747" height="53" alt="image" src="https://github.com/user-attachments/assets/01e003d1-45fa-4cdf-8e67-ac8e4b5449c4" /></br>
- In the directory with the executable, run `liam-taskkill.exe /F /PID pid` to forcefully close a process by its PID

<img width="814" height="59" alt="image" src="https://github.com/user-attachments/assets/c07e1c4c-406b-45a2-9585-59bc22714607" /></br>
- In the directory with the executable, run `liam-taskkill.exe /F /IM imagename` to forcefully close a process by name




### liam-taskkill.exe /T [/PID processid | /IM imagename]
<img width="830" height="161" alt="image" src="https://github.com/user-attachments/assets/b4a4abe4-7978-439b-ba87-af960bca4be7" /></br>
- In the directory with the executable, run `liam-taskkill.exe /F /T /IM imagename` to forcefully close a process by name and its child processes
- /F was used to verify they would close.


### liam-taskkill.exe /PID processid
<img width="744" height="56" alt="image" src="https://github.com/user-attachments/assets/682efa8a-6a9c-4e0a-bf46-b6e07794e5bb" /></br>
- In the directory with the executable, run `liam-taskkill.exe /PID pid` to close a process by its PID


### liam-taskkill.exe /IM imagename
<img width="775" height="50" alt="image" src="https://github.com/user-attachments/assets/e34928c5-57bb-4eec-b6a7-f4ba85bd2ec5" /></br>
- In the directory with the executable, run `liam-taskkill.exe /IM imagename` to close a process by name

#### Wildcard Support for /IM
<img width="794" height="73" alt="image" src="https://github.com/user-attachments/assets/f8cc53bb-4c52-47d7-b994-5ec39a889108" />
- In the directory with the executable, run `liam-taskkill.exe /F /IM partial-imagename*` to forcefully close one or more processes by a partial or full process name by wildcard matching

<img width="1587" height="17" alt="image" src="https://github.com/user-attachments/assets/658e1e52-a512-497a-8e88-8cb4c3cc6727" /></br>


## Building Binaries
Visual Studio was used to compile the binaries through separate projects. 

### Creating a project in Visual Studio
- Create a new blank project,
- Copy the associated .cpp file into the Source Files in the Solution Explorer,
- Provide a name for the file,
- Then copy the code into the .cpp file
</br><img width="613" height="517" alt="image" src="https://github.com/user-attachments/assets/efedc611-fd07-440e-87a7-333cfc0f9d33" /></br>
<img width="450" height="151" alt="image" src="https://github.com/user-attachments/assets/a4140890-4816-4d6a-aca4-badf833303aa" />

### Compiling
<img width="871" height="134" alt="image" src="https://github.com/user-attachments/assets/f92d8592-79d4-4312-8387-6e16f51eab8b" /></br>
 - At the top of Visual Studio, select "Release"
 - Shortcut: <Ctrl + b> will build the binary (Windows 11).
 <img width="762" height="154" alt="image" src="https://github.com/user-attachments/assets/28e05e11-cc5a-491e-8159-7c4b81702260" /></br>
 - You will see the output of where the binary was saved in the console at the bottom of Visual Studio.

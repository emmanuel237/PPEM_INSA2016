# Guideline for Libraries Installation
## Content 

This file contains instructions to setup the libraries in order to compile the
given project files. 

Instructions have been tested for:

* Windows 7 
  * Visual Studio  
  * Code::Blocks (MinGW)
  
The project compilation requires the following libraries:

* pthread 
* SDL2

## pthread 
1. Download the pthread library:   
   [pthread-w32-2-8-0-release.exe](ftp://sourceware.org/pub/pthreads-win32/pthreads-w32-2-8-0-release.exe)
2. Execute te downloaded executable to decompress its content in a temporary directory.
3. Copy the content of the decompressed /Pre-built.2/ directory into a folder named exactly as follows:  
  ```/<project-path>/lib/pthread-2.8.0/```  
  (where <project-path> is replaced with your project path).
   
## SDL2
1. Download the SDL2 Development libraries corresponding to your IDE.  
   [SDL2 Download Webpage](https://www.libsdl.org/download-2.0.php)  
   Among the different proposed libraries, make sure you use the "Development 
   libraries". 
3. This step differs depending on the used IDE.
   * **For MinGW based IDEs (Codeblocks, Makefile, Eclipse CDT, ...)**
     1. Decompress the dowloaded file in a temporary location. 
     2. In the decompressed file, copy the content of the following directory  
        ```i686-w64-mingw32```  
        into a folder named exactly as follows:  
        ```/<project-path>/lib/SDL-2.0.<xx>/```  
        where <xx> is replaced with your version number).
     3. Copy the content of  
        ```/<project-path>/lib/SDL-2.0.<xx>/include/SDL2/```  
        into its parent directory  
        ```/<project-path>/lib/SDL-2.0.<xx>/include/```
   * **For Visual Studio Users Only**  
     1. Decompress the dowloaded file in
	    ```/<project-path>/lib/SDL-2.0.<xx>/```
	 2. Copy the content of 
	    ```/<project-path>/lib/SDL-2.0.<xx>/lib/x86```  
		into its parent directory  
		```/<project-path>/lib/SDL-2.0.<xx>/lib```  
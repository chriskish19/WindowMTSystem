# WindowMTSystem
Uses Microsoft's Win32 API to create multiple windows each with their own threads.

## Getting Started:
You will need installed on your system:
1. [Git](https://git-scm.com/download/win)
2. [CMake](https://cmake.org/)
3. IDE(Integrated Development Enviroment such as [Visual Studio 2022](https://visualstudio.microsoft.com/vs/community/))

In your terminal:
```powershell
# clone the repository
git clone https://github.com/chriskish19/WindowMTSystem.git

# navigate to the directory
cd WindowMTSystem

# Make a build folder
mkdir build
cd build

# To build your IDE solution
cmake ../

# Compile the projects into executables
cmake --build .
```


## How to:
### Create a Custom Window Class:
In a created header file CustomWindow.h
```cpp
#include "iWindow.h"
class MyCustomWindow: public WMTS::MTPlainWin32Window{
public:
	MyCustomWindow(){
		WindowInit();
	}

private:
	// override base class WindowInit()
	void WindowInit() override{
		// Change these to your preference
		mWindowClassName = L"Win32Window";
		mWindowTitle = L"PlainWin32Window";

		mWindowWidthINIT = GetSystemMetrics(SM_CXSCREEN) / 2;
		mWindowHeightINIT = GetSystemMetrics(SM_CYSCREEN) / 2;

		// must call these functions in this order
		SetWindowClass();
		RegisterWindowClass();
		CreateAWindow();
	}

	// override the base class
	void SetWindowClass() override {
		// initialize the mWCEX structure to your prefered settings	
	}

	// override the base class implementation
	LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) override{
		// custom window procedure
	}
}
```

Make sure to remove WindowInit() from PlainWin32Window constructor or the progam will fail
```cpp
class PlainWin32Window :public iWindowClass{
public:
	PlainWin32Window() {
		WindowInit(); 
	}
```
To use the custom window class:
```cpp
#include "CustomWindow.h"
// custom window definition here

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
	MyCustomWindow Window;
	Window.ExecuteThreads(2);
	return 0;
}
```
  

## Future Goals:

#### Small Improvements:
1. Change the project to use CMAKE instead of relying on a visual studio solution.
2. Remove class FilePaths and use C++17 filesystem instead.
3. Add exception handling. Mark functions noexcept if they dont throw.
4. Logger class should use const references instead of creating copies of strings, in the constructor.
5. The way threads are kept track of could be cleaned up. The UpdateMapsAndResources() function is messy.
6. Mark functions that don't modify the object as const.

#### Large Additions:
Note: These additions will eventually be implemented, but it may take a couple years.
1. Write a DX12 Renderer that uses WMTS to handle window creation and rendering to, as an Example project.
2. Write a cross-platform version of WMTS for Linux and MacOS.
3. Write a cross-platform renderer Vulkan/OpenGL that uses the cross-platform version of WMTS, as an Example project.


## Info:
#### See Example1 on how to implement dynamically creating windows on separate threads at run time.


This is a work in progress, I am still learning how to use C++, OpenGL, Vulkan and DirectX.

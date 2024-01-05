# WindowMTSystem
Uses Microsoft's Win32 API to create multiple windows each with their own threads.

# Build From Source Code:
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

# Getting Started
## Download and Run Binaries
Go to releases page and download v1.0-d.exe and example1-d.exe. Double click to run.
Or In your terminal:
```powershell
# Example: navigate to where you saved the files
cd C:/users/downloads

# run v1.0-d.exe
.\v1.0-d

# run example1-d.exe
.\example1-d

```

When running example1-d, use your mouse and navigate to the top menu bar click on "File" then "New" and "Window" and a new window will appear. It is running on a seperate thread, notice when you move the window the title keeps changing and does not freeze. You can create as many windows as your system memory allows for.



# How to:
### Create a Custom Window Class:
In a created header file CustomWindow.hpp
```cpp
#include "iWindow.hpp"
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
#include "CustomWindow.hpp"
// custom window definition here

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
	MyCustomWindow Window;
	Window.ExecuteThreads(2);
	return 0;
}
```
  

# Future Goals:
#### Large Additions:
1. Write a DX12 Renderer that uses WMTS to handle window creation and rendering to, as an Example project.
2. Write a cross-platform version of WMTS for Linux and MacOS.
3. Write a cross-platform renderer Vulkan/OpenGL that uses the cross-platform version of WMTS, as an Example project.

# Info:
#### See Example1 on how to implement dynamically creating windows on separate threads at run time.

This is a work in progress, I am still learning how to use C++, OpenGL, Vulkan and DirectX.

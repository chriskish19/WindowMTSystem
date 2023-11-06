# WindowMTSystem

Uses win32 API to create multiple windows each with their own threads.
All of the code is defined in iWindow.h. To use include iWindow.h in a cpp file like main.cpp.
Or Clone the repository and use Visual Studio 2022 to compile and run.


### Example 1:
This code creates two windows, each window has its own logic thread and message loop thread. 

```cpp
#include "iWindow.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
	WMTS::MTPlainWin32Window Window;
	Window.ExecuteThreads(2);
	return 0;
}
```

### Example 2:
Create a custom window class with polymorphism:
```cpp
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
#include "iWindow.h"
// custom window definition here

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
	MyCustomWindow Window;
	Window.ExecuteThreads(2);
	return 0;
}
```
This is a work in progress, I am still learning how to use C++.  
For myself I'll to be using it in my game engine with Direct X12 API.

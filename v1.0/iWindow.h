#pragma once
#include <memory>
#include <Windows.h>
#include <string>
#include <iostream>
#include <chrono>
#include <time.h>
#include <fstream>
#include <direct.h>
#include <algorithm>
#include <unordered_map>
#include <thread>
#include <vector>
#include <format>
#include <chrono>
#include <atomic>


namespace WMTS {
	// handles any file paths needed throughout this header
	class FilePaths {
	public:
		static std::wstring GetExeFilePathW() {
			std::string path;
			char pBuf[1024];

			// _getcwd():
			/*
			The current working directory (CWD) is the directory in which a user is currently operating while using a command-line interface or a particular process is executing in. For many tasks performed at the command line, the CWD is the default directory that the system will use for reading or writing files when a full path is not specified.

			When a program is launched, it inherits its current working directory from the process that started it. This could be a command-line shell, a script, or another application. Over the course of its execution, a program can change its own current working directory, but this won't affect its parent process or any of its sibling processes.

			For instance, if you open a command-line interface (like the Command Prompt on Windows or Terminal on Unix-like systems) and you navigate to the `Documents` directory, then `Documents` becomes your current working directory. If you were to run a program from this directory without specifying a full path, the system would look for that program in the `Documents` directory by default.

			The `_getcwd` function in C and C++ is used to retrieve the current working directory of the process. The arguments it takes are:

			- `buffer`: A pointer to the destination buffer, which will hold the CWD string after the function is executed.
			- `maxlen`: The maximum length of the buffer, ensuring that the function doesn't overwrite past the end of the allocated space.

			After the function is called, the `buffer` will contain the path to the current working directory, and this path will be null-terminated.

			It's worth noting that if you're developing a program that depends on the current working directory, it can be a source of subtle bugs or unexpected behavior since the CWD is mutable and can change during the execution of the program or depending on how the user or another process starts the program.
			*/
			_getcwd(pBuf, 1024);

			path = pBuf;
			path += "\\";


			std::wstring wpath{ path.begin(),path.end() };

			return wpath;
		}
		static std::string GetExeFilePath() {
			std::string path;
			char pBuf[1024];

			// _getcwd():
			/*
			The current working directory (CWD) is the directory in which a user is currently operating while using a command-line interface or a particular process is executing in. For many tasks performed at the command line, the CWD is the default directory that the system will use for reading or writing files when a full path is not specified.

			When a program is launched, it inherits its current working directory from the process that started it. This could be a command-line shell, a script, or another application. Over the course of its execution, a program can change its own current working directory, but this won't affect its parent process or any of its sibling processes.

			For instance, if you open a command-line interface (like the Command Prompt on Windows or Terminal on Unix-like systems) and you navigate to the `Documents` directory, then `Documents` becomes your current working directory. If you were to run a program from this directory without specifying a full path, the system would look for that program in the `Documents` directory by default.

			The `_getcwd` function in C and C++ is used to retrieve the current working directory of the process. The arguments it takes are:

			- `buffer`: A pointer to the destination buffer, which will hold the CWD string after the function is executed.
			- `maxlen`: The maximum length of the buffer, ensuring that the function doesn't overwrite past the end of the allocated space.

			After the function is called, the `buffer` will contain the path to the current working directory, and this path will be null-terminated.

			It's worth noting that if you're developing a program that depends on the current working directory, it can be a source of subtle bugs or unexpected behavior since the CWD is mutable and can change during the execution of the program or depending on how the user or another process starts the program.
			*/
			_getcwd(pBuf, 1024);

			path = pBuf;
			path += "\\";

			return path;
		}
		inline static const std::string exePath{ GetExeFilePath() };
		inline static const std::wstring exePathW{ exePath.begin(),exePath.end()};
	};
	
	
	
// these macros are for the logger class
#define WSTRINGIFY(x) L#x
#define TOWSTRING(x) WSTRINGIFY(x)
#define LOCATION std::wstring(L"Line: " TOWSTRING(__LINE__) L" File: " __FILE__)


	// used in the logger class to classify errors
	enum class Error {
		FATAL,
		DEBUG,
		INFO,
		WARNING
	};
	
	class logger {
	public:
		logger(std::wstring s, Error type, std::wstring location) {
			initLogger();
			initErrorType(type);

			// mMessage is timestamped and has error type now add location to the end
			mMessage += location;
		}

		// log windows errors with this constructor
		logger(Error type, std::wstring location, DWORD Win32error = GetLastError()) {
			initLogger();
			initErrorType(type);

			LPWSTR errorMsgBuffer = nullptr;
			FormatMessageW(
				FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				Win32error,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPWSTR)&errorMsgBuffer,
				0,
				NULL
			);

			std::wstring win32error_str; // this is initialized to "" by the string class so if errorMsgBuffer is nullptr nothing is added to mMessage
			if (errorMsgBuffer) {
				win32error_str = std::wstring{ errorMsgBuffer };
				LocalFree(errorMsgBuffer);
			}
			else {
				logger log(L"Format message failed", Error::WARNING, LOCATION);
				log.to_console();
				log.to_output();
				log.to_log_file();
			}
			
			// mMessage is timestamped and has error type now add win32error and location to the end
			mMessage += win32error_str + location;
		}


		// output mMessage to console
		void to_console() {
			std::wcout << mMessage << std::endl;
		}

		// output mMessage to output window in visual studio
		void to_output() {
			OutputDebugStringW(mMessage.c_str());
		}

		// output mMessage to a log file
		void to_log_file() {
			// if logFile is not open send info to console and output window
			if (!logFile.is_open()) {
				logger log(L"failed to open logFile", Error::WARNING, LOCATION);
				log.to_console();
				log.to_output();
			}
			
			// write mMessage to the log.txt file
			logFile << mMessage << std::endl;
			
			// if it fails to write mMessage to log.txt, log the fail to the console and output window
			if (logFile.fail()) {
				logger log(L"failed to write to log file", Error::WARNING, LOCATION);
				log.to_console();
				log.to_output();
			}

			// important for log files where the latest information should be preserved in case the program crashes or is terminated before exiting normally.
			logFile.flush();
		}



	private:
		// default initialization code for logger class
		void initLogger() {
			if (!Logger_init && SubSysWindows) {
				AllocConsole();

				// Redirect the CRT standard input, output, and error handles to the console
				FILE* stream;

				freopen_s(&stream, "CONIN$", "r", stdin);
				freopen_s(&stream, "CONOUT$", "w", stdout);
				freopen_s(&stream, "CONOUT$", "w", stderr);

				std::cout << "Outputting to the new console!" << std::endl;
			}

			// set to true since logger class has been initialized
			Logger_init = true;

			// add time to mMessage
			timeStamp();
		}

		// adds time stamp to the begining of mMessage
		void timeStamp() {
			//Geting Current time
			auto clock = std::chrono::system_clock::now();

			// convert to a std::time_t object which _wctime_s accepts
			std::time_t CurrentTime = std::chrono::system_clock::to_time_t(clock);

			// buffer for _wctime_s
			wchar_t TimeBuff[30];

			// Converts the time_t object CurrentTime into a wchar_t 
			_wctime_s(TimeBuff, sizeof(TimeBuff) / sizeof(wchar_t), &CurrentTime);

			// put the TimeBuff into a wide string to make it easy to modify
			std::wstring CurrentTime_wstr{ TimeBuff };

			// _wctime_s puts a newline char at the end of the time stamp
			// I dont want a newline
			if (CurrentTime_wstr.ends_with(L'\n')) {
				CurrentTime_wstr.erase(std::prev(CurrentTime_wstr.end()));
			}

			// add brackets for easy reading
			CurrentTime_wstr.insert(CurrentTime_wstr.begin(), '[');
			CurrentTime_wstr.push_back(']');

			// mMessage now has the time stamp
			mMessage = CurrentTime_wstr + mMessage;
		}


		// adds the type of error to the begining of mMessage 
		void initErrorType(Error type = Error::INFO) {
			switch (type) {
			case Error::FATAL: { mMessage = L"[FATAL ERROR]" + mMessage; } break;
			case Error::DEBUG: { mMessage = L"[DEBUG ERROR]" + mMessage; } break;
			case Error::INFO: { mMessage = L"[INFO]" + mMessage; } break;
			case Error::WARNING: { mMessage = L"[WARNING]" + mMessage; } break;
			default: { mMessage = L"[UNKNOWN]" + mMessage; } break;
			}
		}


		// true for Console subsystem and false for not
		static bool IsConsoleSubsystem() {
			// Get the HMODULE of the current process.
			HMODULE hModule = GetModuleHandle(NULL);

			// Get the DOS header.
			PIMAGE_DOS_HEADER pDOSHeader = (PIMAGE_DOS_HEADER)hModule;

			// From the DOS header, get the NT (PE) header.
			PIMAGE_NT_HEADERS pNTHeaders = (PIMAGE_NT_HEADERS)((DWORD_PTR)hModule + pDOSHeader->e_lfanew);

			// Check the Subsystem value in the Optional Header.
			return pNTHeaders->OptionalHeader.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_CUI;
		}

		// true for windows subsystem and false for not
		static bool IsWindowsSubSystem() {
			// Get the HMODULE of the current process.
			HMODULE hModule = GetModuleHandle(NULL);

			// Get the DOS header.
			PIMAGE_DOS_HEADER pDOSHeader = (PIMAGE_DOS_HEADER)hModule;

			// From the DOS header, get the NT (PE) header.
			PIMAGE_NT_HEADERS pNTHeaders = (PIMAGE_NT_HEADERS)((DWORD_PTR)hModule + pDOSHeader->e_lfanew);

			// Check the Subsystem value in the Optional Header.
			return pNTHeaders->OptionalHeader.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI;
		}

		// is the /SUBSYSTEM under linker settings set to console or Windows
		inline static bool SubSysConsole{ IsConsoleSubsystem() };
		inline static bool SubSysWindows{ IsWindowsSubSystem() };

		// true for logger class initialized
		inline static bool Logger_init{ false };

		// the main log message
		std::wstring mMessage;

		// code for outputting to a log file
		inline static std::wstring logfilepath{ FilePaths::exePathW + L"log.txt" };
		inline static std::wofstream logFile{logfilepath,std::ios::out};
	};
	
	
	
	struct WindowDimensions {
		// This constructor gives memory to the ptrs and 0 as a value
		WindowDimensions() {
			mWidth = std::make_shared<UINT>(0);
			mHeight = std::make_shared<UINT>(0);
			mClientWidth = std::make_shared<UINT>(0);
			mClientHeight = std::make_shared<UINT>(0);
		}

		WindowDimensions(HWND WindowHandle) {
			RECT windowRect;
			if (GetWindowRect(WindowHandle, &windowRect)) {
				UINT width = windowRect.right - windowRect.left;
				UINT height = windowRect.bottom - windowRect.top;

				mWidth = std::make_shared<UINT>(width);
				mHeight = std::make_shared<UINT>(height);
			}
			else {
				logger log(Error::WARNING, LOCATION);
				log.to_console();
				log.to_output();
				log.to_log_file();
			}
			
			RECT clientRect;
			if (GetClientRect(WindowHandle, &clientRect)) {
				UINT clientWidth = clientRect.right - clientRect.left;
				UINT clientHeight = clientRect.bottom - clientRect.top;

				mClientWidth = std::make_shared<UINT>(clientWidth);
				mClientHeight = std::make_shared<UINT>(clientHeight);
			}
			else {
				logger log(Error::WARNING, LOCATION);
				log.to_console();
				log.to_output();
				log.to_log_file();
			}
		}

		// call this on a resize event
		void UpdateWindowDimensions(HWND WindowHandle) {
			RECT windowRect;
			if (GetWindowRect(WindowHandle, &windowRect)) {
				UINT width = windowRect.right - windowRect.left;
				UINT height = windowRect.bottom - windowRect.top;

				*mWidth = width;
				*mHeight = height;
			}
			else {
				logger log(Error::WARNING, LOCATION);
				log.to_console();
				log.to_output();
				log.to_log_file();
			}

			RECT clientRect;
			if (GetClientRect(WindowHandle, &clientRect)) {
				UINT clientWidth = clientRect.right - clientRect.left;
				UINT clientHeight = clientRect.bottom - clientRect.top;

				*mClientWidth = clientWidth;
				*mClientHeight = clientHeight;
			}
			else {
				logger log(Error::WARNING, LOCATION);
				log.to_console();
				log.to_output();
				log.to_log_file();
			}
		}

		std::shared_ptr<UINT> GetWidth() { return mWidth; }
		std::shared_ptr<UINT> GetHeight() { return mHeight; }
		std::shared_ptr<UINT> GetClientWidth() { return mClientWidth; }
		std::shared_ptr<UINT> GetClientHeight() { return mClientHeight; }
	private:
		// entire window dimensions
		std::shared_ptr<UINT> mWidth;
		std::shared_ptr<UINT> mHeight;

		// drawable area inside window borders
		std::shared_ptr<UINT> mClientWidth;
		std::shared_ptr<UINT> mClientHeight;
	};

	class iWindow {
	protected:
		iWindow() {

		}
		virtual ~iWindow(){}
		virtual WindowDimensions GetWindowSize(HWND WindowHandle) = 0;
		virtual HWND GetHandle(size_t index=0) = 0;
		virtual LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) = 0;

		static LRESULT CALLBACK window_proc_proxy(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
			iWindow* window = nullptr;
			if (message == WM_NCCREATE) {
				CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
				window = reinterpret_cast<iWindow*>(pCreate->lpCreateParams);
				SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)window);
			}
			else {
				window = reinterpret_cast<iWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
			}

			if (window) {
				return window->WindowProcedure(hwnd, message, wParam, lParam);
			}

			return DefWindowProc(hwnd, message, wParam, lParam);
		}

		std::vector<HWND> mWindowHandles;
		std::unordered_map<HWND, WindowDimensions> mWindow_mp;
		std::wstring mWindowTitle;
		HINSTANCE mHinstance{ GetModuleHandle(NULL) };

		virtual int ProcessMessage() = 0;
	};

	class iWindowClass:public iWindow {
	protected:
		virtual ~iWindowClass() {}

		// in this function you define a WNDCLASSEXW structure
		virtual void SetWindowClass() = 0;

		// registers mWCEX window class
		virtual void RegisterWindowClass() = 0;

		virtual void CreateAWindow() = 0;

		virtual void SetWindowTitle(std::wstring newTitle,HWND WindowHandle) = 0;

		WNDCLASSEXW mWCEX{};
		std::wstring mWindowClassName;
	};


	class PlainWin32Window :public iWindowClass{
	public:
		PlainWin32Window() {
			// defaults
			mWindowClassName = L"Win32Window";
			mWindowTitle = L"PlainWin32Window";
			
			mWindowWidthINIT = GetSystemMetrics(SM_CXSCREEN) / 2;
			mWindowHeightINIT = GetSystemMetrics(SM_CYSCREEN) / 2;

			SetWindowClass();
			RegisterWindowClass();
			CreateAWindow();
		}
		
		PlainWin32Window(int width,int height,std::wstring title) {
			mWindowClassName = L"Win32Window";
			mWindowTitle = title;

			mWindowWidthINIT = width;
			mWindowHeightINIT = height;

			SetWindowClass();
			RegisterWindowClass();
			CreateAWindow();
		}

		int ProcessMessage() override {
			MSG msg{};

			// Main message loop:
			while (GetMessage(&msg, nullptr, 0, 0))
			{
				if (!TranslateAccelerator(msg.hwnd, NULL, &msg))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
			return (int)msg.wParam;
		}
	protected:
		void SetWindowTitle(std::wstring newTitle,HWND WindowHandle) override {
			SetWindowText(WindowHandle, newTitle.c_str());
		}


		UINT mWindowWidthINIT;
		UINT mWindowHeightINIT;

		void SetWindowClass() override {
			mWCEX.cbSize = sizeof(WNDCLASSEXW);
			mWCEX.style = CS_HREDRAW | CS_VREDRAW;
			mWCEX.lpfnWndProc = window_proc_proxy;
			mWCEX.cbClsExtra = 0;
			mWCEX.cbWndExtra = 0;
			mWCEX.hInstance = mHinstance;
			mWCEX.hIcon = NULL;
			mWCEX.hCursor = LoadCursor(nullptr, IDC_ARROW);
			mWCEX.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
			mWCEX.lpszMenuName = NULL;
			mWCEX.lpszClassName = mWindowClassName.c_str();
			mWCEX.hIconSm = NULL;
		}

		void RegisterWindowClass() override {
			if (!RegisterClassExW(&mWCEX)) {
				logger log(Error::FATAL, LOCATION);
				log.to_console();
				log.to_output();
				log.to_log_file();
			}
		}

		void CreateAWindow() override {
			HWND hwnd = nullptr;

			hwnd = CreateWindowW(
				mWindowClassName.c_str(),
				mWindowTitle.c_str(),
				WS_OVERLAPPEDWINDOW,
				CW_USEDEFAULT,
				0,
				mWindowWidthINIT,
				mWindowHeightINIT,
				nullptr,
				nullptr,
				mHinstance,
				this);

			if (!IsWindow(hwnd)) {
				logger log(Error::FATAL, LOCATION);
				log.to_console();
				log.to_output();
				log.to_log_file();
				return;
			}

			// add to the vector of handles
			mWindowHandles.push_back(hwnd);

			// Get the window dimensions and store it in WindowSize
			WindowDimensions WindowSize(hwnd);

			// add to the map of Handles to WindowDimensions
			mWindow_mp.emplace(hwnd, WindowSize);

			// show window, because it starts as hidden
			ShowWindow(hwnd, SW_SHOWDEFAULT);
		}

		LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) override{
			switch (message)
			{
			case WM_KEYDOWN: {
				break;
			}
			case WM_MBUTTONDOWN:
			{
				break;
			}
			case WM_MBUTTONUP:
			{
				break;
			}
			case WM_MOUSEMOVE: {
				break;
			}
			case WM_DISPLAYCHANGE:
			{
				break;
			}
			case WM_SIZE:
			case WM_SIZING:
			{
				// update WindowDimensions here
				auto found = mWindow_mp.find(hwnd);
				if (found != mWindow_mp.end()) {
					found->second.UpdateWindowDimensions(found->first);
				}
				break;
			}
			case WM_COMMAND:
			{
				int wmId = LOWORD(wParam);
				// Parse the menu selections:
				switch (wmId)
				{
				default:
					return DefWindowProc(hwnd, message, wParam, lParam);
				}
				break;
			}
			case WM_PAINT:
			{
				PAINTSTRUCT ps{};
				HDC hdc = BeginPaint(hwnd, &ps);

				EndPaint(hwnd, &ps);
				break;
			}
			
			case WM_DESTROY:
				PostQuitMessage(0);
				break;
			default:
				return DefWindowProc(hwnd, message, wParam, lParam);


			}
			return DefWindowProc(hwnd, message, wParam, lParam);
		}

		HWND GetHandle(size_t index = 0) override {
			index = std::clamp(index, (size_t)0, mWindowHandles.size() - 1);
			return mWindowHandles[index];
		}

		WindowDimensions GetWindowSize(HWND WindowHandle) override {
			auto found = mWindow_mp.find(WindowHandle);
			if (found != mWindow_mp.end()) {
				return found->second;
			}
			// if its not found return the first element in the map
			return mWindow_mp.begin()->second;
		}
	};

	// multi thread win32 window system
	class MTPlainWin32Window :public PlainWin32Window {
	public:
		void ExecuteThreads(size_t NumberOfWindows) {
			UINT total_threads = std::thread::hardware_concurrency();

			// The number of created windows will be +1 because of Main Window being created 
			// in the PlainWin32Window class
			if(NumberOfWindows>0) --NumberOfWindows;

			// minus one for main thread
			NumberOfWindows = std::clamp(NumberOfWindows, (size_t)0, (size_t)total_threads - 1);

			// get main thread id first
			auto mainThreadID = std::this_thread::get_id();

			// add main thread ID to the map first and main handle at 0 index
			thread_mp.emplace(mainThreadID, GetHandle());

			// build thread pool
			for (size_t i{}; i < NumberOfWindows; i++) {
				thread_pool.emplace_back(new std::thread(&WMTS::MTPlainWin32Window::Run, this));
			}

			// for the main thread window
			ProcessMessage();

			// causes main thread to wait for thread pool to finish executing
			for (auto thread : thread_pool) {
				if (thread->joinable()) {
					thread->join();
				}
			}

			// clean up
			for (auto thread : thread_pool) {
				delete thread;
			}

			
		}

		
		void RunLogic(std::thread::id CurrentThreadID,std::shared_ptr<std::atomic<bool>> run) {
			// Example code for showing functionality:
			// Put any logic code here: 
			

			// for std::format printing in the window title
			int x = 0;
			
			// search the thread map for the hwnd
			auto found = thread_mp.find(CurrentThreadID);
			if (found != thread_mp.end()) {
				while (*run) {
					SetWindowTitle(std::format(L"Happy Window [{:*<{}}]", L'*', x + 1), found->second);
					(++x) %= 20;

					std::this_thread::sleep_for(std::chrono::milliseconds(50));
				}
			}
		}
	private:
		// thread to HWND map
		std::unordered_map<std::thread::id, HWND> thread_mp;

		void Run() {
			CreateAWindow();
			ProcessMessage();
		}

		std::vector<std::thread*> thread_pool;

		int ProcessMessage() override {
			// messages
			MSG msg{};
			
			// keep program executing
			bool running{ true };
			std::shared_ptr<std::atomic<bool>> run_logic{ std::make_shared<std::atomic<bool>> (true) };

			// get current thread id to use the correct window handle
			auto CurrentThread = std::this_thread::get_id();

			// put logic on a separete thread
			std::thread* logic_thread = new std::thread(&WMTS::MTPlainWin32Window::RunLogic, this, CurrentThread, run_logic);

			// Main program loop
			while (running) {
				// Windows message loop:
				while (PeekMessage(&msg, nullptr, 0, 0,PM_REMOVE))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
					
					if (msg.message == WM_QUIT) {
						running = false;
						*run_logic = false;
					}
				}
			}

			// join logic thread
			if(logic_thread->joinable()) 
				logic_thread->join();

			// clean up
			delete logic_thread;

			return (int)msg.wParam;
		}

		void CreateAWindow() override {
			HWND hwnd = nullptr;

			hwnd = CreateWindowW(
				mWindowClassName.c_str(),
				mWindowTitle.c_str(),
				WS_OVERLAPPEDWINDOW,
				CW_USEDEFAULT,
				0,
				mWindowWidthINIT,
				mWindowHeightINIT,
				nullptr,
				nullptr,
				mHinstance,
				this);

			if (!IsWindow(hwnd)) {
				logger log(Error::FATAL, LOCATION);
				log.to_console();
				log.to_output();
				log.to_log_file();
				return;
			}

			// add to the vector of handles
			mWindowHandles.push_back(hwnd);

			// Get the window dimensions and store it in WindowSize
			WindowDimensions WindowSize(hwnd);

			// add to the map of Handles to WindowDimensions
			mWindow_mp.emplace(hwnd, WindowSize);

			// get this thread id
			auto CurrentThread = std::this_thread::get_id();

			// add to the thread map
			thread_mp.emplace(CurrentThread, hwnd);

			// show window, because it starts as hidden
			ShowWindow(hwnd, SW_SHOWDEFAULT);
		}
	};
}
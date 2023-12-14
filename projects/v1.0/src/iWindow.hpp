#pragma once
#include <memory>
#include <Windows.h>
#include <string>
#include <iostream>
#include <chrono>
#include <time.h>
#include <fstream>
#include <algorithm>
#include <unordered_map>
#include <thread>
#include <vector>
#include <format>
#include <chrono>
#include <atomic>
#include <mutex>
#include <filesystem>
#include <stdexcept>
#include <optional>

namespace WMTS {
// these macros are for the logger class
#define WMTS_WSTRINGIFY(x) L#x
#define WMTS_LOCATION std::wstring(L"Line: " WMTS_WSTRINGIFY(__LINE__) L" File: " __FILE__)


	// used in the logger class to classify errors
	enum class Error {
		// if its fatal it will affect the programs execution and most likley an exception will be thrown and the program may exit
		FATAL,

		// small errors that have no effect on execution flow and no exceptions are thrown
		DEBUG,

		// for information to the user
		INFO,

		// more significant errors than a debug message but does not exit the program or throw an exception
		WARNING
	};
	
	class logger {
	public:
		logger(const std::wstring& s, Error type, const std::wstring& location) {
			initLogger();
			initErrorType(type);

			// mMessage is timestamped and has error type now add location and the message(std::wstring s) to the end
			mMessage += location + L" Message: " + s;
		}

		// log windows errors with this constructor
		logger(Error type, const std::wstring& location, DWORD Win32error = GetLastError()) {
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
				logger log(L"Format message failed", Error::WARNING, WMTS_LOCATION);
				log.to_console();
				log.to_output();
				log.to_log_file();
			}
			
			// mMessage is timestamped and has error type now add win32error and location to the end
			mMessage += win32error_str + location;
		}


		// output mMessage to console
		void to_console() const{
			std::wcout << mMessage << std::endl;
		}

		// output mMessage to output window in visual studio
		void to_output() const{
			OutputDebugStringW(mMessage.c_str());
		}

		// output mMessage to a log file
		void to_log_file(){
			std::lock_guard<std::mutex> local_lock(mLogfileWrite_mtx);
			
			// if logFile is not open send info to console and output window
			if (!logFile.is_open()) {
				logger log(L"failed to open logFile", Error::WARNING, WMTS_LOCATION);
				log.to_console();
				log.to_output();
			}
			
			// write mMessage to the log.txt file
			logFile << mMessage << std::endl;
			
			// if it fails to write mMessage to log.txt, log the fail to the console and output window
			if (logFile.fail()) {
				logger log(L"failed to write to log file", Error::WARNING, WMTS_LOCATION);
				log.to_console();
				log.to_output();
			}

			// important for log files where the latest information should be preserved in case the program crashes or is terminated before exiting normally.
			logFile.flush();
		}



	private:
		// default initialization code for logger class
		void initLogger() {
			// since any header could use a similar logger we need to guard agianst reintializing the console
			// each time a logger is constructed
			if (!Logger_init && SubSysWindows && GetConsoleWindow()==nullptr) {
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
		inline static std::filesystem::path logfilepath{ std::filesystem::current_path()/"WMTSlog.txt" };
		inline static std::wofstream logFile{logfilepath,std::ios::out};

		// prevent multiple threads from writing to the log file at the same time
		// the output would be messy and hard to read without this
		// the stream is thread safe by design but << is not syncronized
		std::mutex mLogfileWrite_mtx;
	};
	
	
	
	struct WindowDimensions {
		// This constructor gives memory to the ptrs and 0 as a value
		WindowDimensions() {
			mWidth = std::make_shared<UINT>(0);
			mHeight = std::make_shared<UINT>(0);
			mClientWidth = std::make_shared<UINT>(0);
			mClientHeight = std::make_shared<UINT>(0);
		}

		// This constructor uses the window handle and gets the window rect dimensions
		// (client and full) and allocates memory for the ptrs with the current rect values
		// if the GetWindowRect function fails the error is logged
		WindowDimensions(const HWND WindowHandle) {
			RECT windowRect;
			if (GetWindowRect(WindowHandle, &windowRect)) {
				UINT width = windowRect.right - windowRect.left;
				UINT height = windowRect.bottom - windowRect.top;

				mWidth = std::make_shared<UINT>(width);
				mHeight = std::make_shared<UINT>(height);
			}
			else {
				logger log(Error::WARNING, WMTS_LOCATION);
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
				logger log(Error::WARNING, WMTS_LOCATION);
				log.to_console();
				log.to_output();
				log.to_log_file();
			}
		}

		// call this on a resize event to update the shared_ptrs memory with the latest values
		// This is useful for keeping other parts of the program updated with the latest window dimensions
		void UpdateWindowDimensions(const HWND WindowHandle) {
			// prevents multiple threads from dereferencing the shared_ptrs and modiying
			// the memory they point to at the same time which would lead to problems
			std::lock_guard<std::mutex> local_lock(mUpdateWindowDimensions_mtx);
			

			RECT windowRect;
			if (GetWindowRect(WindowHandle, &windowRect)) {
				UINT width = windowRect.right - windowRect.left;
				UINT height = windowRect.bottom - windowRect.top;

				*mWidth = width;
				*mHeight = height;
			}
			else {
				logger log(Error::WARNING, WMTS_LOCATION);
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
				logger log(Error::WARNING, WMTS_LOCATION);
				log.to_console();
				log.to_output();
				log.to_log_file();
			}
		}

		// custom copy constructor
		WindowDimensions(const WindowDimensions& other)
			: mWidth(other.mWidth),
			mHeight(other.mHeight),
			mClientWidth(other.mClientWidth),
			mClientHeight(other.mClientHeight) {
			// Note: We don't copy the mutex, as each object should have its own mutex.
			// and we dont allocate new memory instead the shared_ptrs point to the original memory
			// in the copied object which reference counts the original shared_ptrs
		}

		// const overload so much const
		// this ensures the returned ptrs are read-only
		// cant change the memory the shared_ptr points to and cant change the value it points to
		// this makes them thread safe
		const std::shared_ptr<const UINT> GetWidth() const { return std::const_pointer_cast<const UINT>(mWidth); }
		const std::shared_ptr<const UINT> GetHeight() const { return std::const_pointer_cast<const UINT>(mHeight); }
		const std::shared_ptr<const UINT> GetClientWidth() const { return std::const_pointer_cast<const UINT>(mClientWidth); }
		const std::shared_ptr<const UINT> GetClientHeight() const { return std::const_pointer_cast<const UINT>(mClientHeight); }
	private:
		// entire window dimensions
		std::shared_ptr<UINT> mWidth;
		std::shared_ptr<UINT> mHeight;

		// drawable area inside window borders
		std::shared_ptr<UINT> mClientWidth;
		std::shared_ptr<UINT> mClientHeight;

		// mutex used in the function UpdateWindowDimensions()
		std::mutex mUpdateWindowDimensions_mtx;
	};

	// a thread safe class that has the maps and resources needed to keep track of the multiple windows created
	class WindowResources{
	public:
		// Constructor
    	WindowResources() = default;

		// Delete the copy constructor
		WindowResources(const WindowResources&) = delete;

		// Delete the copy assignment operator
		WindowResources& operator=(const WindowResources&) = delete;

		// add an entry to mThread_mp
		void AddToThreadmp(const std::thread::id t_id,const HWND WindowHandle){
			std::lock_guard<std::mutex> local_lock(mThreadmp_mtx);
			mThread_mp.emplace(t_id,WindowHandle);
		}

		// search mThread_mp for a window handle(HWND) given a thread id
		std::optional<HWND> SearchThreadmp(const std::thread::id t_id){
			std::lock_guard<std::mutex> local_lock(mThreadmp_mtx);
			auto found = mThread_mp.find(t_id);
			if(found != mThread_mp.end()){
				return found->second;
			}
			return std::nullopt;
		}
		
		// adds a handle to mWindowHandles
		void AddToWindowHandles(const HWND WindowHandle){
			std::lock_guard<std::mutex> local_lock(mWindowHandles_mtx);
			mWindowHandles.push_back(WindowHandle);
		}

		// search mWindowHandles for a matching window handle
		// returns std::nullopt if a handle cannot be found
		std::optional<HWND> SearchWindowHandles(const HWND WindowHandle){
			std::lock_guard<std::mutex> local_lock(mWindowHandles_mtx);
			auto found = std::find(mWindowHandles.begin(),mWindowHandles.end(),WindowHandle);
			if(found != mWindowHandles.end()){
				return *found;
			}
			return std::nullopt;
		}

		// get a windoow handle from mWindowHandles using an index
		// if mWindowHandles is empty nullptr is returned
		HWND GetWindowHandle(size_t index=0){
			std::lock_guard<std::mutex> local_lock(mWindowHandles_mtx);
			if(mWindowHandles.empty()) return nullptr;
			index = std::clamp(index, (size_t)0, mWindowHandles.size() - 1);
			return mWindowHandles[index];
		}

		// add an entry to mWindow_mp
		// must make a copy of WindowDimensions, expensive yes but thread safe
		void AddToWindowmp(const HWND WindowHandle,const WindowDimensions size){
			std::lock_guard<std::mutex> local_lock(mWindowmp_mtx);
			mWindow_mp.emplace(WindowHandle,size);
		}

		// search mWindow_mp for a window handle
		std::optional<WindowDimensions> SearchWindowmp(const HWND WindowHandle){
			std::lock_guard<std::mutex> local_lock(mWindowmp_mtx);
			auto found = mWindow_mp.find(WindowHandle);
			if(found != mWindow_mp.end()){
				return found->second;
			}
			return std::nullopt;
		}

	private:
		std::unordered_map<std::thread::id, HWND> mThread_mp;
		std::vector<HWND> mWindowHandles;
		std::unordered_map<HWND,WindowDimensions> mWindow_mp;

		std::mutex mThreadmp_mtx;
		std::mutex mWindowHandles_mtx;
		std::mutex mWindowmp_mtx;
	};

	class iWindow {
	protected:
		iWindow() {}
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

		virtual bool CreateAWindow() = 0;

		// I cannot pass newTitle as a reference since multiple threads will be accessing it
		// they each need their own copy
		virtual void SetWindowTitle(const std::wstring newTitle,const HWND WindowHandle) const = 0;

		// used in constructor to initialize class
		virtual void WindowInit() = 0;

		WNDCLASSEXW mWCEX{};
		std::wstring mWindowClassName;
		WindowResources mResources;
	};

	class PlainWin32Window :public iWindowClass{
	public:
		PlainWin32Window() {
			WindowInit();
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
		void WindowInit() override {
			// defaults
			mWindowClassName = L"Win32Window";
			mWindowTitle = L"PlainWin32Window";

			// Gets the system resolution and divides it by two 
			// for both width and height, gives a nice sized window
			mWindowWidthINIT = GetSystemMetrics(SM_CXSCREEN) / 2;
			mWindowHeightINIT = GetSystemMetrics(SM_CYSCREEN) / 2;

			SetWindowClass();
			RegisterWindowClass();
			CreateAWindow();
		}


		void SetWindowTitle(const std::wstring newTitle,const HWND WindowHandle) const override {
			SetWindowText(WindowHandle, newTitle.c_str());
		}

		// initial size for the window when it is first created
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
				logger log(Error::FATAL, WMTS_LOCATION);
				log.to_console();
				log.to_output();
				log.to_log_file();
				throw std::runtime_error("Failed to Register Windows Class mWCEX in the PlainWin32Window Class");
			}
		}

		bool CreateAWindow() override {
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
				logger log(Error::FATAL, WMTS_LOCATION);
				log.to_console();
				log.to_output();
				log.to_log_file();
				throw std::runtime_error("Win32 API CreateWindow(args..) function failure in PlainWin32Window Class");
			}

			// add to the vector mWindowHandles
			mResources.AddToWindowHandles(hwnd);

			// Get the window dimensions and store it in WindowSize
			WindowDimensions WindowSize(hwnd);

			// add to the map of Handles to WindowDimensions
			mResources.AddToWindowmp(hwnd,WindowSize);

			// show window, because it starts as hidden
			ShowWindow(hwnd, SW_SHOWDEFAULT);

			return true;
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
				auto size = mResources.SearchWindowmp(hwnd);
				if(size.has_value()){
					size.value().UpdateWindowDimensions(hwnd);
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
			return mResources.GetWindowHandle(index);
		}

		WindowDimensions GetWindowSize(HWND WindowHandle) override {
			auto size = mResources.SearchWindowmp(WindowHandle);
			if(size.has_value()){
				return size.value();
			}
			// return the main window dimensions
			// if it is not set then nullptr is returned by GetHandle()
			// if nullptr is used to initialize WindowDimemsions an error is logged
			return WindowDimensions(GetHandle());	
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
			mResources.AddToThreadmp(mainThreadID,GetHandle());

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
			
			auto found = mResources.SearchThreadmp(CurrentThreadID);

			if (found.has_value()) {
				while (*run) {
					SetWindowTitle(std::format(L"Happy Window [{:*<{}}]", L'*', x + 1), found.value());
					(++x) %= 20;

					std::this_thread::sleep_for(std::chrono::milliseconds(50));
				}
			}
		}
	private:
		// thread guard to prevent concurrent access to:
		// CreateAWindow()
		std::mutex thread_guard1;

		void Run() {
			// if CreateAWindow fails we dont want the thread to continue
			// it would cause problems in ProcessMessage()
			if (!CreateAWindow()){
				// Note: this will cause the thread to enter a wait state
				// waiting to be joined
				return;
			}
				
			ProcessMessage();
		}

		std::vector<std::thread*> thread_pool;

		int ProcessMessage() override {
			// messages
			MSG msg{};
			
			// RunLogic function loop 
			std::shared_ptr<std::atomic<bool>> run_logic{ std::make_shared<std::atomic<bool>> (true) };

			// get current thread id to use the correct window handle
			auto CurrentThread = std::this_thread::get_id();

			// put logic on a separete thread
			std::thread* logic_thread = new std::thread(&WMTS::MTPlainWin32Window::RunLogic, this, CurrentThread, run_logic);

			// Windows message loop:
			while (GetMessage(&msg, nullptr, 0, 0))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			// exit RunLogic() loop
			*run_logic = false;

			// join logic thread
			if(logic_thread->joinable()) 
				logic_thread->join();

			// clean up
			delete logic_thread;

			return (int)msg.wParam;
		}

		bool CreateAWindow() override {
			// No need to unlock, as std::lock_guard will unlock automatically
			std::lock_guard<std::mutex> lock(thread_guard1);

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
				logger log(Error::FATAL, WMTS_LOCATION);
				log.to_console();
				log.to_output();
				log.to_log_file();
				return false;
			}

			// add to the vector of handles
			mResources.AddToWindowHandles(hwnd);

			// Get the window dimensions and store it in WindowSize
			WindowDimensions WindowSize(hwnd);

			// add to the map of Handles to WindowDimensions
			mResources.AddToWindowmp(hwnd,WindowSize);

			// get this thread id
			auto CurrentThread = std::this_thread::get_id();

			// add to the thread map
			mResources.AddToThreadmp(CurrentThread,hwnd);

			// show window, because it starts as hidden
			ShowWindow(hwnd, SW_SHOWDEFAULT);

			return true;
		}
	};
}
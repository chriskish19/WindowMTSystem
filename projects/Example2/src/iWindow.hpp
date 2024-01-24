#pragma once
#include <Windows.h>
#include "resource.h"
#include "WindowDimensions.hpp"
#include "WindowResources.hpp"
#include "logger.hpp"


namespace WMTS {	
	class iWindow {
	protected:
		iWindow() {}
		virtual ~iWindow(){}
		virtual WindowDimensions GetWindowSize(HWND WindowHandle) = 0;
		virtual HWND GetHandle(size_t index=0) = 0;
		virtual LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) = 0;
		static LRESULT CALLBACK window_proc_proxy(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
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
		PlainWin32Window();
		int ProcessMessage() override;
	protected:
		void WindowInit() override;
		void SetWindowTitle(const std::wstring newTitle,const HWND WindowHandle) const override;

		// initial size for the window when it is first created
		UINT mWindowWidthINIT;
		UINT mWindowHeightINIT;

		void SetWindowClass() override;
		void RegisterWindowClass() override;
		bool CreateAWindow() override;
		LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) override;
		HWND GetHandle(size_t index = 0) override;
		WindowDimensions GetWindowSize(HWND WindowHandle) override;
	};

	// multi thread win32 window system
	class MTPlainWin32Window :public PlainWin32Window {
	public:
		void ExecuteThreads();
		void RunLogic(std::thread::id CurrentThreadID,std::shared_ptr<std::atomic<bool>> run);
	private:
		// fp: function pointer
		// this_obj: this pointer
		void BuildThreadPool(size_t NumberOfThreads, auto fp, auto this_obj);

		std::mutex main_thread_guard;
		std::unique_lock<std::mutex> main_thread_lock;
		std::condition_variable main_thread_cv;

		// thread guard for CreateAWindow()
		std::mutex thread_guard1;

		LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) override;

		// total avaliable threads from the system
		UINT total_threads = std::thread::hardware_concurrency();

		void Run();
		int ProcessMessage() override;
		bool CreateAWindow() override;
	};
}
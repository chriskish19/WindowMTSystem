#pragma once
#include <mutex>
#include <map>
#include <optional>
#include <vector>
#include <unordered_map>
#include <Windows.h>
#include "WindowDimensions.hpp"
#include <algorithm>


namespace WMTS{
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
		void AddToThreadmp(const std::thread::id t_id,const HWND WindowHandle);

		// search mThread_mp for a window handle(HWND) given a thread id
		std::optional<HWND> SearchThreadmp(const std::thread::id& t_id);
		
		// remove an entry from mThreadmp using an iterator position
		void RemoveFromThreadmp(auto position);

		// adds a handle to mWindowHandles
		void AddToWindowHandles(const HWND WindowHandle);

		// search mWindowHandles for a matching window handle
		// returns std::nullopt if a handle cannot be found
		std::optional<HWND> SearchWindowHandles(const HWND WindowHandle);

		// remove an entry from mWindowHandles using an iterator position
		void RemoveFromWindowHandles(auto entry);

		// get a windoow handle from mWindowHandles using an index
		// if mWindowHandles is empty nullptr is returned
		HWND GetWindowHandle(size_t index=0);

		// add an entry to mWindow_mp
		// must make a copy of WindowDimensions, expensive yes but thread safe
		void AddToWindowmp(const HWND WindowHandle,const WindowDimensions size);

		// search mWindow_mp for a window handle
		std::optional<WindowDimensions> SearchWindowmp(const HWND WindowHandle);

		// removes an entry from mWindowmp using an iterator position
		void RemoveFromWindowmp(auto entry);

		// add an entry to mThread_pool_mp
		void AddToThreadpoolmp(std::thread::id t_id,std::thread* t_p);

		// search mThread_pool_mp for a corresponding std::thread*
		// returns nullopt if not found
		std::optional<std::thread*> SearchThreadpoolmp(const std::thread::id t_id);

		std::unordered_map<std::thread::id, std::thread*>::iterator ItSearchThreadpoolmp(const std::thread::id t_id);
		
		// removes an entry from mThread_pool_mp using an iterator position
		void RemoveFromThreadpoolmp(auto entry);

		bool GetThreadpoolmpEmptyState();

		std::unordered_map<std::thread::id, std::thread*>::iterator ItGetEndofThreadpoolmp();

		size_t GetThreadpoolmpSize();

		// call this when a thread is exiting
		void Update(const std::thread::id t_id);
	private:
		// thread id to window handle map
		std::unordered_map<std::thread::id, HWND> mThread_mp;
		
		// vector of window handles
		std::vector<HWND> mWindowHandles;
		
		// Window handle to WindowDimensions map
		std::unordered_map<HWND,WindowDimensions> mWindow_mp;
		
		// thread ID to thread pointer map
		std::unordered_map<std::thread::id,std::thread*> mThread_pool_mp;

		std::mutex mThreadpoolmp_mtx;
		std::mutex mThreadmp_mtx;
		std::mutex mWindowHandles_mtx;
		std::mutex mWindowmp_mtx;
	};
}
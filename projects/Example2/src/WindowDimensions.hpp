#pragma once
#include <memory>
#include <Windows.h>
#include "logger.hpp"
#include <mutex>


namespace WMTS{
    struct WindowDimensions {
        // This constructor gives memory to the ptrs and 0 as a value
        WindowDimensions();

        // This constructor uses the window handle and gets the window rect dimensions
        // (client and full) and allocates memory for the ptrs with the current rect values
        // if the GetWindowRect function fails the error is logged
        WindowDimensions(const HWND WindowHandle);

        // call this on a resize event to update the shared_ptrs memory with the latest values
        // This is useful for keeping other parts of the program updated with the latest window dimensions
        void UpdateWindowDimensions(const HWND WindowHandle);

        // custom copy constructor
        WindowDimensions(const WindowDimensions& other);
        // Note: We don't copy the mutex, as each object should have its own mutex.
        // and we dont allocate new memory instead the shared_ptrs point to the original memory
        // in the copied object which reference counts the original shared_ptrs
        

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

}
#include "WindowDimensions.hpp"

WMTS::WindowDimensions::WindowDimensions(){
    mWidth = std::make_shared<UINT>(0);
    mHeight = std::make_shared<UINT>(0);
    mClientWidth = std::make_shared<UINT>(0);
    mClientHeight = std::make_shared<UINT>(0);
}

WMTS::WindowDimensions::WindowDimensions(const HWND WindowHandle){
    RECT windowRect;
    if (GetWindowRect(WindowHandle, &windowRect)) {
        UINT width = windowRect.right - windowRect.left;
        UINT height = windowRect.bottom - windowRect.top;

        mWidth = std::make_shared<UINT>(width);
        mHeight = std::make_shared<UINT>(height);
    }
    else {
        logger log(Error::WARNING);
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
        logger log(Error::WARNING);
        log.to_console();
        log.to_output();
        log.to_log_file();
    }
}

void WMTS::WindowDimensions::UpdateWindowDimensions(const HWND WindowHandle){
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
        logger log(Error::WARNING);
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
        logger log(Error::WARNING);
        log.to_console();
        log.to_output();
        log.to_log_file();
    }
}

WMTS::WindowDimensions::WindowDimensions(const WindowDimensions& other)
    : mWidth(other.mWidth),
    mHeight(other.mHeight),
    mClientWidth(other.mClientWidth),
    mClientHeight(other.mClientHeight){}
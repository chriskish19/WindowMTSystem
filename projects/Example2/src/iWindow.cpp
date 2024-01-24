#include "iWindow.hpp"

LRESULT CALLBACK WMTS::iWindow::window_proc_proxy(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam){
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

WMTS::PlainWin32Window::PlainWin32Window(){
    WindowInit();
}

int WMTS::PlainWin32Window::ProcessMessage(){
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

void WMTS::PlainWin32Window::WindowInit(){
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

void WMTS::PlainWin32Window::SetWindowTitle(const std::wstring newTitle,const HWND WindowHandle) const{
    SetWindowText(WindowHandle, newTitle.c_str());
}

void WMTS::PlainWin32Window::SetWindowClass(){
    mWCEX.cbSize = sizeof(WNDCLASSEXW);
    mWCEX.style = CS_HREDRAW | CS_VREDRAW;
    mWCEX.lpfnWndProc = window_proc_proxy;
    mWCEX.cbClsExtra = 0;
    mWCEX.cbWndExtra = 0;
    mWCEX.hInstance = mHinstance;
    mWCEX.hIcon = NULL;
    mWCEX.hCursor = LoadCursor(nullptr, IDC_ARROW);
    mWCEX.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    mWCEX.lpszMenuName = MAKEINTRESOURCEW(IDR_MENU1);
    mWCEX.lpszClassName = mWindowClassName.c_str();
    mWCEX.hIconSm = NULL;
}

void WMTS::PlainWin32Window::RegisterWindowClass(){
    if (!RegisterClassExW(&mWCEX)) {
        logger log(Error::FATAL);
        log.to_console();
        log.to_output();
        log.to_log_file();
        throw std::runtime_error("Failed to Register Windows Class mWCEX in the PlainWin32Window Class");
    }
}

bool WMTS::PlainWin32Window::CreateAWindow(){
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
        logger log(Error::FATAL);
        log.to_console();
        log.to_output();
        log.to_log_file();
        throw std::runtime_error("Win32 API CreateWindow(args..) function failure in PlainWin32Window Class");
    }

    // add to the vector of handles
    mResources.AddToWindowHandles(hwnd);

    // Get the window dimensions and store it in WindowSize
    WindowDimensions WindowSize(hwnd);

    // add to the map of Handles to WindowDimensions
    mResources.AddToWindowmp(hwnd,WindowSize);

    // show window, because it starts as hidden
    ShowWindow(hwnd, SW_SHOWDEFAULT);

    return true;
}

LRESULT CALLBACK WMTS::PlainWin32Window::WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam){
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
        case ID_NEW_WINDOW: {
            
            break;
        }
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

HWND WMTS::PlainWin32Window::GetHandle(size_t index){
    return mResources.GetWindowHandle(index);
}

WMTS::WindowDimensions WMTS::PlainWin32Window::GetWindowSize(HWND WindowHandle){
    auto size = mResources.SearchWindowmp(WindowHandle);
    if(size.has_value()){
        return size.value();
    }
    // return the main window dimensions
    // if it is not set then nullptr is returned by GetHandle()
    // if nullptr is used to initialize WindowDimemsions an error is logged
    return WindowDimensions(GetHandle());
}

void WMTS::MTPlainWin32Window::ExecuteThreads(){
    // get main thread id first
    auto mainThreadID = std::this_thread::get_id();

    // add main thread ID to the map first and main handle at 0 index
    mResources.AddToThreadmp(mainThreadID,GetHandle());

    // for the main thread window
    ProcessMessage();

    // main thread window is closed now
    // wait for all threads to finish
    main_thread_lock = std::unique_lock<std::mutex>(main_thread_guard);
    main_thread_cv.wait(main_thread_lock, [this] {return mResources.GetThreadpoolmpEmptyState(); });
}

void WMTS::MTPlainWin32Window::RunLogic(std::thread::id CurrentThreadID,std::shared_ptr<std::atomic<bool>> run){
    // Example code for showing functionality:
    // Put any logic code here: 
    
    // for std::format printing in the window title
    int x = 0;
    
    // search the threadmp for the corresponding window handle
    auto found = mResources.SearchThreadmp(CurrentThreadID);

    if (found.has_value()) {
        while (*run) {
            SetWindowTitle(std::format(L"Happy Window [{:*<{}}]", L'*', x + 1), found.value());
            (++x) %= 20;

            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
}

void WMTS::MTPlainWin32Window::BuildThreadPool(size_t NumberOfThreads, auto fp, auto this_obj){
    NumberOfThreads = std::clamp(NumberOfThreads, (size_t)0, (size_t)total_threads - 1);

    // build thread pool
    for (size_t i{}; (i < NumberOfThreads) && (mResources.GetThreadpoolmpSize() < total_threads); i++) {
        // build map of thread_pool
        auto thread = new std::thread(fp, this_obj);
        mResources.AddToThreadpoolmp(thread->get_id(),thread);
    }
}

LRESULT CALLBACK WMTS::MTPlainWin32Window::WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam){
    switch(message){
        case WM_COMMAND:{
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId){
                case ID_NEW_WINDOW: {
                    BuildThreadPool(1, &WMTS::MTPlainWin32Window::Run, this);
                    break;
                }
            default:
                return DefWindowProc(hwnd, message, wParam, lParam);
            }
            break;
        }
        default:
            return PlainWin32Window::WindowProcedure(hwnd, message, wParam, lParam);
    }
    return PlainWin32Window::WindowProcedure(hwnd, message, wParam, lParam);
}

void WMTS::MTPlainWin32Window::Run(){
    // if CreateAWindow fails we dont want the thread to continue
    // it would cause problems in ProcessMessage()
    if (!CreateAWindow())
        return; // exit the run function
    
    ProcessMessage();

    auto ThreadID = std::this_thread::get_id();

    // Update maps and resources
    // much cleaner now and thread safe
    mResources.Update(ThreadID);

    // tell the waiting main thread to check if there is still threads in the thread pool map
    // if the map is empty its safe to exit
    main_thread_cv.notify_one();
}

int WMTS::MTPlainWin32Window::ProcessMessage(){
    // messages
    MSG msg{};
    
    // RunLogic function loop 
    std::shared_ptr<std::atomic<bool>> run_logic{ std::make_shared<std::atomic<bool>> (true) };

    // get current thread id to use the correct window handle
    auto CurrentThread = std::this_thread::get_id();

    // put logic on a separate thread
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

bool WMTS::MTPlainWin32Window::CreateAWindow(){
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
        logger log(Error::FATAL);
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
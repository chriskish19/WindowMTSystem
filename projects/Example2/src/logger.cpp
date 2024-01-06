#include "logger.hpp"


WMTS::logger::logger(const std::wstring& s, Error type, const std::source_location& location)
:m_location(location)
{
    initLogger();
    time_stamp();
    initErrorType(type);
    location_stamp();
    mMessage += L"Message: " + s;
}

WMTS::logger::logger(Error type, const std::source_location& location, DWORD Win32error)
:m_location(location)
{
    initLogger();
    time_stamp();
    initErrorType(type);
    location_stamp();

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
        logger log(L"Format message failed", Error::WARNING);
        log.to_console();
        log.to_output();
        log.to_log_file();
    }
    
    // mMessage is timestamped and has error type now add win32error and location to the end
    mMessage += win32error_str;
}


void WMTS::logger::time_stamp(){
    //Geting Current time
    auto clock = std::chrono::system_clock::now();

    // add the time to the message
    mMessage = std::format(L"[{:%F %T}] {}", clock, mMessage);
}

void WMTS::logger::initLogger(){
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
}

void WMTS::logger::initErrorType(Error type){
    switch (type) {
        case Error::FATAL: { mMessage = L"[FATAL ERROR]" + mMessage; } break;
        case Error::DEBUG: { mMessage = L"[DEBUG ERROR]" + mMessage; } break;
        case Error::INFO: { mMessage = L"[INFO]" + mMessage; } break;
        case Error::WARNING: { mMessage = L"[WARNING]" + mMessage; } break;
        default: { mMessage = L"[UNKNOWN]" + mMessage; } break;
    }
}

bool WMTS::logger::IsConsoleSubsystem(){
    // Get the HMODULE of the current process.
    HMODULE hModule = GetModuleHandle(NULL);

    // Get the DOS header.
    PIMAGE_DOS_HEADER pDOSHeader = (PIMAGE_DOS_HEADER)hModule;

    // From the DOS header, get the NT (PE) header.
    PIMAGE_NT_HEADERS pNTHeaders = (PIMAGE_NT_HEADERS)((DWORD_PTR)hModule + pDOSHeader->e_lfanew);

    // Check the Subsystem value in the Optional Header.
    return pNTHeaders->OptionalHeader.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_CUI;
}

bool WMTS::logger::IsWindowsSubSystem(){
    // Get the HMODULE of the current process.
    HMODULE hModule = GetModuleHandle(NULL);

    // Get the DOS header.
    PIMAGE_DOS_HEADER pDOSHeader = (PIMAGE_DOS_HEADER)hModule;

    // From the DOS header, get the NT (PE) header.
    PIMAGE_NT_HEADERS pNTHeaders = (PIMAGE_NT_HEADERS)((DWORD_PTR)hModule + pDOSHeader->e_lfanew);

    // Check the Subsystem value in the Optional Header.
    return pNTHeaders->OptionalHeader.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI;
}

void WMTS::logger::to_log_file(){
    std::lock_guard<std::mutex> local_lock(mLogfileWrite_mtx);
			
    // if logFile is not open send info to console and output window
    if (!logFile.is_open()) {
        logger log(L"failed to open logFile", Error::WARNING);
        log.to_console();
        log.to_output();
    }
    
    // write mMessage to the log.txt file
    logFile << mMessage << std::endl;
    
    // if it fails to write mMessage to log.txt, log the fail to the console and output window
    if (logFile.fail()) {
        logger log(L"failed to write to log file", Error::WARNING);
        log.to_console();
        log.to_output();
    }

    // important for log files where the latest information should be preserved in case the program crashes or is terminated before exiting normally.
    logFile.flush();
}

void WMTS::logger::to_output(){
    OutputDebugStringW(mMessage.c_str());
}

void WMTS::logger::to_console() const{
    std::wcout << mMessage << std::endl;
}

void WMTS::logger::location_stamp(){
    std::string file_name(m_location.file_name());
    std::string function_name(m_location.function_name());
    std::string line(std::to_string(m_location.line()));

    std::wstring w_file_name(file_name.begin(),file_name.end());
    std::wstring w_function_name(function_name.begin(),function_name.end());
    std::wstring w_line(line.begin(),line.end());

    mMessage += std::format(L"File: {} Line: {} Function: {}",w_file_name,w_line,w_function_name);
}
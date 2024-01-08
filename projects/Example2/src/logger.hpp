#pragma once
#include <string>
#include <format>
#include <source_location>
#include <filesystem>
#include <Windows.h>
#include <iostream>
#include <mutex>
#include <fstream>
#include "D3DCommon.h"
#include <comdef.h>



namespace WMTS{
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
		logger(const std::wstring& s, Error type, const std::source_location& location = std::source_location::current());

		// log windows errors with this constructor
		logger(Error type, const std::source_location& location = std::source_location::current(), DWORD Win32error = GetLastError());

		// log dx12 errors with this constructor
		logger(Error type, HRESULT hr, ID3DBlob* dx_error, const std::source_location& location = std::source_location::current());

		// log dx12 HRESULT errors with this constructor
		logger(Error type, HRESULT hr, const std::source_location& location = std::source_location::current());

		// output mMessage to console
		void to_console() const;

		// output mMessage to output window in visual studio
		void to_output();

		// output mMessage to a log file
		void to_log_file();
	private:
        // adds the location to mMessage
        void location_stamp();

		// default initialization code for logger class
		void initLogger();

		// adds the type of error to the begining of mMessage 
		void initErrorType(Error type = Error::INFO);

        // adds the time to mMessage
        void time_stamp();

		// true for Console subsystem and false for not
		static bool IsConsoleSubsystem();

		// true for windows subsystem and false for not
		static bool IsWindowsSubSystem();

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

        const std::source_location m_location;
	};
}
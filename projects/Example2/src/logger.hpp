#pragma once
#include <string>



namespace WMTS{
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
}
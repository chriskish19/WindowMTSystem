#include "iWindow.hpp"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
	try{
		// might be heavy on the stack I dont know, to be on the safe side it's allocated on the heap
		// TODO: Learn about stack limits in C++ and when to allocate on the stack vs heap
		std::unique_ptr<WMTS::MTPlainWin32Window> Window{std::make_unique<WMTS::MTPlainWin32Window>()};
		Window->ExecuteThreads();
	}
	catch(const std::runtime_error& e){
		// the error message
		std::cerr << e.what() << std::endl;
		
		// exit the program there is no recovering
		return 1;
	}
	catch(const std::bad_alloc& e){
		// the error message
		std::cerr << e.what() << std::endl;

		// exit the program there is no recovering
		return 2;
	}
	return 0;
}
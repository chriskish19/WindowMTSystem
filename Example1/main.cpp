#include "iWindow.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
	WMTS::MTPlainWin32Window Window;
	Window.ExecuteThreads();
	return 0;
}
#pragma once
#include "logger.hpp"
#include "iDevice.hpp"


// load direct x library files
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3dcompiler.lib")


namespace WMTS{

	class dx12:public idx12 {
	public:
		dx12(HWND WindowHandle,UINT WindowWidth, UINT WindowHeight);

	protected:
		void LoadDebugInterface() override;
		void InitializeDevices() override;
		void InitializeCommandQueue() override;
		void Resize(UINT width, UINT height) override;
		void DestroyFrameBuffer() override;
		void SetupSwapChain(UINT width, UINT height) override;
	};
}
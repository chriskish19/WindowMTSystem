#pragma once
#include "logger.hpp"
#include "iDevice.hpp"
#include <chrono>
#include "Camera.hpp"


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
		

		Vertex mVertexBufferData[3] = {{{1.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
                                      {{-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
                                      {{0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}};

		uint32_t mIndexBufferData[3] = {0, 1, 2};

		std::chrono::time_point<std::chrono::steady_clock> m_tStart, m_tEnd;
		float mElapsedTime = 0.0f;
		Camera m_MainView;
		HWND m_WindowHandle;
	};
}
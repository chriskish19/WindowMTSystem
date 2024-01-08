#pragma once
#include <wrl/client.h>
#include <d3d12.h>
#include <DirectXMath.h>
#include <vector>
#include "d3dx12.h"
#include <dxgi1_4.h>


namespace WMTS{
    struct Vertex {
		float position[3];
		float color[3];
	};
	
	struct UniformBufferObjectVS {
		DirectX::XMMATRIX projectionMatrix;
		DirectX::XMMATRIX viewMatrix;
		DirectX::XMMATRIX modelMatrix;
	};

	class iDevices {
	protected:
		virtual ~iDevices() {}

		static const UINT backbufferCount{ 2 };

		// matrices
		UniformBufferObjectVS uboVS{};

		Microsoft::WRL::ComPtr<IDXGIFactory4> mFactory{ nullptr };

		Microsoft::WRL::ComPtr<IDXGIAdapter1> mAdapter{ nullptr };
#if defined(_DEBUG)
		Microsoft::WRL::ComPtr<ID3D12Debug1> mDebugController1{ nullptr };

		Microsoft::WRL::ComPtr<ID3D12DebugDevice> mDebugDevice{ nullptr };

		Microsoft::WRL::ComPtr<ID3D12Debug> mDebugController{ nullptr };
#endif
		Microsoft::WRL::ComPtr<ID3D12Device> mDevice{ nullptr };

		Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue{ nullptr };

		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mCommandAllocator{ nullptr };

		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList{ nullptr };

		UINT mCurrentBuffer{};

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvHeap{ nullptr };

		std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> mRenderTargets{ std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>>(backbufferCount,nullptr) };

		Microsoft::WRL::ComPtr<IDXGISwapChain3> mSwapChain{ nullptr };

		D3D12_VIEWPORT mViewport{};

		D3D12_RECT mSurfaceSize{};

		Microsoft::WRL::ComPtr<ID3D12Resource> mVertexBuffer{ nullptr };

		Microsoft::WRL::ComPtr<ID3D12Resource> mIndexBuffer{ nullptr };

		Microsoft::WRL::ComPtr<ID3D12Resource> mUniformBuffer{ nullptr };

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mUniformBufferHeap{ nullptr };

		UINT8* mMappedUniformBuffer{ nullptr };

		D3D12_VERTEX_BUFFER_VIEW mVertexBufferView{};

		D3D12_INDEX_BUFFER_VIEW mIndexBufferView{};

		UINT mRtvDescriptorSize{};

		Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature{ nullptr };

		Microsoft::WRL::ComPtr<ID3D12PipelineState> mPipelineState{ nullptr };

		UINT mFrameIndex{};

		HANDLE mFenceEvent{ nullptr };

		Microsoft::WRL::ComPtr<ID3D12Fence> mFence{ nullptr };

		UINT64 mFenceValue{};

		UINT dxgiFactoryFlags = 0;

		UINT mWidth{}, mHeight{};

		virtual void InitializeDevices() = 0;

		virtual void InitializeCommandQueue() = 0;
	};

	class idx12 :public iDevices{
	protected:
		virtual ~idx12(){}

		virtual void Render() = 0;

		virtual void CreateCommands() = 0;

		virtual void DestroyCommands() = 0;

		virtual void SetupCommands() = 0;

		virtual void InitializeResources() = 0;

		virtual void InitFrameBuffer() = 0;

		virtual void SetupSwapChain(UINT width, UINT height) = 0;

		virtual void DestroyFrameBuffer() = 0;

		virtual void Resize(UINT width, UINT height) = 0;

		virtual void LoadVertices() = 0;

		virtual void LoadDebugInterface() = 0;
	};
}
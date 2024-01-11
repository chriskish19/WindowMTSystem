#include "Renderer.hpp"

WMTS::dx12::dx12(HWND WindowHandle, UINT WindowWidth, UINT WindowHeight):m_WindowHandle(WindowHandle)
{
#if defined(_DEBUG)
			LoadDebugInterface();
#endif
			InitializeDevices();
			InitializeCommandQueue();
			Resize(WindowWidth,WindowHeight);
}

void WMTS::dx12::LoadDebugInterface()
{
    {
        HRESULT debug = D3D12GetDebugInterface(IID_PPV_ARGS(&mDebugController));
        if (FAILED(debug)) {
            logger log(Error::DEBUG, debug);
            log.to_console();
            log.to_output();
            log.to_log_file();
        }
    }

    {
        HRESULT debug = mDebugController->QueryInterface(IID_PPV_ARGS(&mDebugController1));
        if (FAILED(debug)) {
            logger log(Error::DEBUG, debug);
            log.to_console();
            log.to_output();
            log.to_log_file();
        }
    }

    mDebugController1->EnableDebugLayer();

    mDebugController1->SetEnableGPUBasedValidation(true);

    dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
}

void WMTS::dx12::InitializeDevices()
{
    {
        HRESULT debug = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&mFactory));
        if (FAILED(debug)) {
            logger log(Error::FATAL, debug);
            log.to_console();
            log.to_output();
            log.to_log_file();
        }
    }

    for (UINT adapterIndex = 0;
        DXGI_ERROR_NOT_FOUND !=
        mFactory->EnumAdapters1(adapterIndex, &mAdapter);
        ++adapterIndex)
    {
        DXGI_ADAPTER_DESC1 desc;
        mAdapter->GetDesc1(&desc);

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            // Don't select the Basic Render Driver adapter.
            continue;
        }

        // Check to see if the adapter supports Direct3D 12, but don't create
        // the actual device yet.
        if (SUCCEEDED(D3D12CreateDevice(mAdapter.Get(), D3D_FEATURE_LEVEL_12_0,
            _uuidof(ID3D12Device), nullptr)))
        {
            break;
        }

        // We won't use this adapter, so release it
        mAdapter->Release();
    }

    {
        HRESULT debug = D3D12CreateDevice(mAdapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&mDevice));
        if (FAILED(debug)) {
            logger log(Error::FATAL, debug);
            log.to_console();
            log.to_output();
            log.to_log_file();
        }
    }

    mDevice->SetName(L"Hello D3D12 Device");

#if defined(_DEBUG)
    {
        HRESULT debug = mDevice->QueryInterface(IID_PPV_ARGS(&mDebugDevice));
        if (FAILED(debug)) {
            logger log(Error::DEBUG, debug);
            log.to_console();
            log.to_output();
            log.to_log_file();
        }
    }
#endif
}

void WMTS::dx12::InitializeCommandQueue()
{
    D3D12_COMMAND_QUEUE_DESC queueDesc{};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    {
        HRESULT debug = mDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mCommandQueue));
        if (FAILED(debug)) {
            logger log(Error::FATAL, debug);
            log.to_console();
            log.to_output();
            log.to_log_file();
        }
    }

    {
        HRESULT debug = mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mCommandAllocator));
        if (FAILED(debug)) {
            logger log(Error::FATAL, debug);
            log.to_console();
            log.to_output();
            log.to_log_file();
        }
    }

    {
        HRESULT debug = mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence));
        if (FAILED(debug)) {
            logger log(Error::FATAL, debug);
            log.to_console();
            log.to_output();
            log.to_log_file();
        }
    }
}

void WMTS::dx12::Resize(UINT width, UINT height)
{
    mWidth = std::clamp(width, 1u, 0xffffu);
    mHeight = std::clamp(height, 1u, 0xffffu);

    // Signal and increment the fence value.
    const UINT64 fence = mFenceValue;

    {
        HRESULT debug = mCommandQueue->Signal(mFence.Get(), fence);
        if (FAILED(debug)) {
            logger log(Error::FATAL, debug);
            log.to_console();
            log.to_output();
            log.to_log_file();
        }
    }

    mFenceValue++;

    // Wait until the previous frame is finished.
    if (mFence->GetCompletedValue() < fence) {
        {
            HRESULT debug = mFence->SetEventOnCompletion(fence, mFenceEvent);
            if (FAILED(debug)) {
                logger log(Error::FATAL, debug);
                log.to_console();
                log.to_output();
                log.to_log_file();
            }
        }

        WaitForSingleObjectEx(mFenceEvent, INFINITE, false);
    }

    DestroyFrameBuffer();
    SetupSwapChain(mWidth, mHeight);
    InitFrameBuffer();
}

void WMTS::dx12::DestroyFrameBuffer()
{
    for (size_t i{}; i < mRenderTargets.size(); i++) {
        if (mRenderTargets[i]) {
            mRenderTargets[i]->Release();
            mRenderTargets[i] = nullptr;
        }
    }

    if (mRtvHeap) {
        mRtvHeap->Release();
        mRtvHeap = nullptr;
    }
}

void WMTS::dx12::SetupSwapChain(UINT width, UINT height)
{
    mSurfaceSize.left = 0;
    mSurfaceSize.top = 0;
    mSurfaceSize.right = static_cast<LONG>(mWidth);
    mSurfaceSize.bottom = static_cast<LONG>(mHeight);
    mViewport.TopLeftX = 0.0f;
    mViewport.TopLeftY = 0.0f;
    mViewport.Width = static_cast<float>(mWidth);
    mViewport.Height = static_cast<float>(mHeight);
    mViewport.MinDepth = .1f;
    mViewport.MaxDepth = 1000.f;

    uboVS.projectionMatrix = m_MainView.GetProjectionMatrix();

	// View Matrix (Translate)
	uboVS.viewMatrix = m_MainView.GetViewMatrix();

	// Model Matrix (Identity)
	uboVS.modelMatrix = DirectX::XMMatrixIdentity();

    if (mSwapChain != nullptr)
	{
		mSwapChain->ResizeBuffers(backbufferCount, mWidth, mHeight,
			DXGI_FORMAT_R8G8B8A8_UNORM, 0);
	}
	else {
		DXGI_SWAP_CHAIN_DESC1 swapchainDesc{};
		swapchainDesc.BufferCount = backbufferCount;
		swapchainDesc.Width = width;
		swapchainDesc.Height = height;
		swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapchainDesc.SampleDesc.Count = 1;

		IDXGISwapChain1* swapchain = nullptr;
		{
			HRESULT debug = mFactory->CreateSwapChainForHwnd(mCommandQueue.Get(), m_WindowHandle, &swapchainDesc, nullptr, nullptr, &swapchain);
			if (FAILED(debug)) {
				logger log(Error::FATAL, debug);
				log.to_console();
				log.to_output();
			}
			else {
				mSwapChain = static_cast<IDXGISwapChain3*>(swapchain);
			}
		}
	}

	mFrameIndex = mSwapChain->GetCurrentBackBufferIndex();
}

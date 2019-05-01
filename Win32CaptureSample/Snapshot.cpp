#include "pch.h"
#include "Snapshot.h"

using namespace winrt;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::UI;
using namespace Windows::UI::Composition;
using namespace Windows::Graphics::DirectX;
using namespace Windows::Graphics::DirectX::Direct3D11;
using namespace Windows::Graphics::Capture;

CompositionDrawingSurface Snapshot::CreateThumbnailSurface(
	CompositionGraphicsDevice const& graphicsDevice,
	IDirect3DDevice const& device,
	GraphicsCaptureItem const& item)
{
	auto dispatcherQueue = DispatcherQueue::GetForCurrentThread();

	auto surface = graphicsDevice.CreateDrawingSurface(
		{ (float)item.Size().Width, (float)item.Size().Height },
		DirectXPixelFormat::B8G8R8A8UIntNormalized,
		DirectXAlphaMode::Premultiplied);
	
	auto success = dispatcherQueue.TryEnqueue([surface, device, item]() -> void
	{
		auto framePool = Direct3D11CaptureFramePool::Create(
			device,
			DirectXPixelFormat::B8G8R8A8UIntNormalized,
			1,
			item.Size());
		auto session = framePool.CreateCaptureSession(item);

		framePool.FrameArrived([framePool, session, device, surface](auto && ...)
		{
			auto d3dDevice = GetDXGIInterfaceFromObject<ID3D11Device>(device);
			com_ptr<ID3D11DeviceContext> d3dContext;
			d3dDevice->GetImmediateContext(d3dContext.put());
			auto multithread = d3dContext.as<ID3D11Multithread>();

			auto frame = framePool.TryGetNextFrame();
			auto frameSurface = GetDXGIInterfaceFromObject<ID3D11Texture2D>(frame.Surface());

			ResizeSurface(surface, { (float)frame.ContentSize().Width , (float)frame.ContentSize().Height });

			POINT point = {};
			auto dxgiSurface = SurfaceBeginDraw(surface, &point);
			auto buffer = dxgiSurface.as<ID3D11Texture2D>();

			{
				auto lock = D3D11DeviceLock(multithread.get());
				d3dContext->CopySubresourceRegion(buffer.get(), 0, point.x, point.y, 0, frameSurface.get(), 0, NULL);
			}

			SurfaceEndDraw(surface);

			framePool.Close();
			session.Close();
		});
		session.StartCapture();
	});
	WINRT_VERIFY(success);
	

	return surface;
}

#include "pch.h"
#include "SimpleCapture.h"
#include "composition.interop.h"

using namespace winrt;

using namespace Windows;
using namespace Windows::Foundation;
using namespace Windows::System;
using namespace Windows::Graphics;
using namespace Windows::Graphics::Capture;
using namespace Windows::Graphics::DirectX;
using namespace Windows::Graphics::DirectX::Direct3D11;
using namespace Windows::Foundation::Numerics;
using namespace Windows::UI;
using namespace Windows::UI::Composition;

SimpleCapture::SimpleCapture(
    Compositor const& compositor,
    IDirect3DDevice const& device,
    GraphicsCaptureItem const& item)
{
    m_compositor = compositor;
    m_item = item;
    m_device = device;

    auto d3dDevice = GetDXGIInterfaceFromObject<ID3D11Device>(m_device);
    d3dDevice->GetImmediateContext(m_d3dContext.put());
    m_multithread = m_d3dContext.as<ID3D11Multithread>();

    m_compositionGraphics = CreateCompositionGraphicsDevice(m_compositor, d3dDevice.get());

    m_surface = m_compositionGraphics.CreateDrawingSurface2(
        m_item.Size(),
        DirectXPixelFormat::B8G8R8A8UIntNormalized,
        DirectXAlphaMode::Premultiplied);

    m_framePool = Direct3D11CaptureFramePool::Create(
        m_device,
        DirectXPixelFormat::B8G8R8A8UIntNormalized,
        2,
        m_item.Size());
    m_session = m_framePool.CreateCaptureSession(m_item);
    m_lastSize = m_item.Size();
    m_framePool.FrameArrived({ this, &SimpleCapture::OnFrameArrived });

    WINRT_ASSERT(m_session != nullptr);
}

void SimpleCapture::StartCapture()
{
    CheckClosed();
    m_session.StartCapture();
}

ICompositionSurface SimpleCapture::GetSurface()
{
    CheckClosed();
    return m_surface;
}

void SimpleCapture::Close()
{
    auto expected = false;
    if (m_closed.compare_exchange_strong(expected, true))
    {
        m_session.Close();
        m_framePool.Close();

        m_surface = nullptr;
        m_framePool = nullptr;
        m_session = nullptr;
        m_item = nullptr;
    }
}

void SimpleCapture::OnFrameArrived(
    Direct3D11CaptureFramePool const& sender,
    winrt::Windows::Foundation::IInspectable const&)
{
    auto newSize = false;

    {
        auto frame = sender.TryGetNextFrame();

        if (frame.ContentSize().Width != m_lastSize.Width ||
            frame.ContentSize().Height != m_lastSize.Height)
        {
            // The thing we have been capturing has changed size.
            // We need to resize our swap chain first, then blit the pixels.
            // After we do that, retire the frame and then recreate our frame pool.
            newSize = true;
            m_lastSize = frame.ContentSize();
            ResizeSurface(m_surface, m_lastSize);
        }

        {
            auto frameSurface = GetDXGIInterfaceFromObject<ID3D11Texture2D>(frame.Surface());
            
            POINT point = {};
            auto dxgiSurface = SurfaceBeginDraw(m_surface, &point);
            auto buffer = dxgiSurface.as<ID3D11Texture2D>();

            {
                auto lock = D3D11DeviceLock(m_multithread.get());
                m_d3dContext->CopySubresourceRegion(buffer.get(), 0, point.x, point.y, 0, frameSurface.get(), 0, NULL);
            }        

            SurfaceEndDraw(m_surface);
        }
    }

    if (newSize)
    {
        m_framePool.Recreate(
            m_device,
            DirectXPixelFormat::B8G8R8A8UIntNormalized,
            2,
            m_lastSize);
    }
}


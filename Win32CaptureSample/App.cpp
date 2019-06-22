#include "pch.h"
#include "App.h"
#include "SimpleCapture.h"
#include "Window.h"
#include "Snapshot.h"

using namespace winrt;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Numerics;
using namespace Windows::UI;
using namespace Windows::UI::Composition;
using namespace Windows::Graphics::DirectX::Direct3D11;
using namespace Windows::Graphics::Capture;

void App::Initialize(
    const std::vector<Window> const& windows,
    ContainerVisual root)
{
    auto queue = DispatcherQueue::GetForCurrentThread();

    m_compositor = root.Compositor();
    m_root = m_compositor.CreateSpriteVisual();

    auto d3dDevice = CreateD3DDevice();
    auto dxgiDevice = d3dDevice.as<IDXGIDevice>();
    m_device = CreateDirect3DDevice(dxgiDevice.get());

    m_compositionGraphics = CreateCompositionGraphicsDevice(m_compositor, d3dDevice.get());

    m_root.RelativeSizeAdjustment({ 1, 1 });
    m_root.Brush(m_compositor.CreateColorBrush(Colors::White()));
    root.Children().InsertAtTop(m_root);

    auto square = (int)ceil(sqrt(windows.size()));
    auto visualSize = 1.0f / square;
    auto count = 0;
    for (auto& window : windows)
    {
        auto col = count % square;
        auto row = count / square;

        auto item = CreateCaptureItemForWindow(window.Hwnd());
        m_items.push_back(item);
        auto surface = Snapshot::CreateThumbnailSurface(m_compositionGraphics, m_device, item);

        auto visual = m_compositor.CreateSpriteVisual();
        visual.RelativeSizeAdjustment({ visualSize, visualSize });
        visual.RelativeOffsetAdjustment({ visualSize * col, visualSize * row, 0 });
        visual.Brush(m_compositor.CreateSurfaceBrush(surface));
        m_root.Children().InsertAtTop(visual);

        count++;
    }
}

void App::OnClick(
    float2 normalizedPosition)
{
    if (m_liveCapture.get() == nullptr)
    {
        auto square = (int)ceil(sqrt(m_items.size()));
        auto visualSize = 1.0f / square;

        auto row = (int)floor(normalizedPosition.y * square);
        auto col = (int)floor(normalizedPosition.x * square);

        auto index = (square * row) + col;

        if (index < m_items.size())
        {
            auto item = m_items[index];
            m_liveCapture = std::make_unique<LiveCapture>(m_compositor, m_device, item);
            m_root.Children().RemoveAll();
            m_root.Children().InsertAtTop(m_liveCapture->Visual());
        }
    }
}

LiveCapture::LiveCapture(
    Compositor const& compositor,
    IDirect3DDevice const& device,
    GraphicsCaptureItem const& item)
{
    m_compositor = compositor;
    m_content = m_compositor.CreateSpriteVisual();
    m_brush = m_compositor.CreateSurfaceBrush();

    m_content.AnchorPoint({ 0.5f, 0.5f });
    m_content.RelativeOffsetAdjustment({ 0.5f, 0.5f, 0 });
    m_content.RelativeSizeAdjustment({ 1, 1 });
    m_content.Size({ -80, -80 });
    m_content.Brush(m_brush);
    m_brush.HorizontalAlignmentRatio(0.5f);
    m_brush.VerticalAlignmentRatio(0.5f);
    m_brush.Stretch(CompositionStretch::Uniform);
    auto shadow = m_compositor.CreateDropShadow();
    shadow.Mask(m_brush);
    m_content.Shadow(shadow);
    
    StartCapture(device, item);
}

void LiveCapture::StartCapture(
    IDirect3DDevice const& device,
    GraphicsCaptureItem const& item)
{
    if (m_capture.get() != nullptr)
    {
        m_capture->Close();
        m_capture = nullptr;
    }

    m_capture = std::make_unique<SimpleCapture>(device, item);

    auto surface = m_capture->CreateSurface(m_compositor);
    m_brush.Surface(surface);

    m_capture->StartCapture();
}
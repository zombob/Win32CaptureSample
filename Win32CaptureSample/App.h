#pragma once

class SimpleCapture;
class LiveCapture;
struct Window;

class App
{
public:
    App() {}
    ~App() {}

    void Initialize(
		const std::vector<Window> const& windows,
        winrt::Windows::UI::Composition::ContainerVisual root);

	void OnClick(winrt::Windows::Foundation::Numerics::float2 normalizedPosition);

private:
    winrt::Windows::UI::Composition::Compositor m_compositor{ nullptr };
	winrt::Windows::UI::Composition::CompositionGraphicsDevice m_compositionGraphics{ nullptr };
    winrt::Windows::UI::Composition::SpriteVisual m_root{ nullptr };

    winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice m_device{ nullptr };
	std::vector<winrt::Windows::Graphics::Capture::GraphicsCaptureItem> m_items;
	std::unique_ptr<LiveCapture> m_liveCapture{ nullptr };
};

class LiveCapture
{
public:
	LiveCapture(
		winrt::Windows::UI::Composition::Compositor const& compositor,
		winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice const& device,
		winrt::Windows::Graphics::Capture::GraphicsCaptureItem const& item);
	~LiveCapture() {}

	winrt::Windows::UI::Composition::Visual Visual() { return m_content; }

private:
	void StartCapture(
		winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice const& device, 
		winrt::Windows::Graphics::Capture::GraphicsCaptureItem const& item);

private:
	winrt::Windows::UI::Composition::Compositor m_compositor{ nullptr };
	winrt::Windows::UI::Composition::SpriteVisual m_content{ nullptr };
	winrt::Windows::UI::Composition::CompositionSurfaceBrush m_brush{ nullptr };

	std::unique_ptr<SimpleCapture> m_capture{ nullptr };
};
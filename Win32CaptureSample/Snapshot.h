#pragma once

class Snapshot
{
public:
	static winrt::Windows::UI::Composition::CompositionDrawingSurface CreateThumbnailSurface(
		winrt::Windows::UI::Composition::CompositionGraphicsDevice const& graphicsDevice,
		winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice const& device,
		winrt::Windows::Graphics::Capture::GraphicsCaptureItem const& item);
private:
	Snapshot() {}
};
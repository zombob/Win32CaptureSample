#pragma once

class SimpleCapture
{
public:
    SimpleCapture(
        winrt::Windows::UI::Composition::Compositor const& compositor,
        winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice const& device,
        winrt::Windows::Graphics::Capture::GraphicsCaptureItem const& item);
    ~SimpleCapture() { Close(); }

    void StartCapture();
    winrt::Windows::UI::Composition::ICompositionSurface GetSurface();

    void Close();

private:
    void OnFrameArrived(
        winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool const& sender,
        winrt::Windows::Foundation::IInspectable const& args);

    inline void CheckClosed()
    {
        if (m_closed.load() == true)
        {
            throw winrt::hresult_error(RO_E_CLOSED);
        }
    }

private:
    winrt::Windows::Graphics::Capture::GraphicsCaptureItem m_item{ nullptr };
    winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool m_framePool{ nullptr };
    winrt::Windows::Graphics::Capture::GraphicsCaptureSession m_session{ nullptr };
    winrt::Windows::Graphics::SizeInt32 m_lastSize;

    winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice m_device{ nullptr };
    winrt::com_ptr<ID3D11Multithread> m_multithread{ nullptr };
    winrt::com_ptr<ID3D11DeviceContext> m_d3dContext{ nullptr };

    winrt::Windows::UI::Composition::Compositor m_compositor{ nullptr };
    winrt::Windows::UI::Composition::CompositionGraphicsDevice m_compositionGraphics{ nullptr };
    winrt::Windows::UI::Composition::CompositionDrawingSurface m_surface{ nullptr };

    std::atomic<bool> m_closed = false;
};
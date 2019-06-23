#pragma once
#include "DesktopWindow.h"
#include "EnumerationWindow.h"

class App;

struct SampleWindow : DesktopWindow<SampleWindow>
{
    SampleWindow(HINSTANCE instance, int cmdShow, std::shared_ptr<App> app);

    winrt::Windows::UI::Composition::Desktop::DesktopWindowTarget CreateWindowTarget(winrt::Windows::UI::Composition::Compositor const& compositor)
    {
        return CreateDesktopWindowTarget(compositor, m_window, true);
    }

    void Initialize(winrt::Windows::Foundation::IUnknown const& object)
    {
        auto initializer = object.as<IInitializeWithWindow>();
        winrt::check_hresult(initializer->Initialize(m_window));
    }

    LRESULT MessageHandler(UINT const message, WPARAM const wparam, LPARAM const lparam);

private:
    void CreateControls(HINSTANCE instance);

private:
    HWND m_comboBoxHwnd = NULL;
    HWND m_buttonHwnd = NULL;
    std::vector<EnumerationWindow> m_windows;
    std::shared_ptr<App> m_app;
};
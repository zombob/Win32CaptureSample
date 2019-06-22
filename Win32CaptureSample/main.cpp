#include "pch.h"
#include "App.h"
#include "SimpleCapture.h"
#include "Win32WindowEnumeration.h"
#include <Windowsx.h>

using namespace winrt;
using namespace Windows::UI;
using namespace Windows::UI::Composition;

int CALLBACK WinMain(
    HINSTANCE instance,
    HINSTANCE previousInstance,
    LPSTR     cmdLine,
    int       cmdShow);

LRESULT CALLBACK WndProc(
    HWND   hwnd,
    UINT   msg,
    WPARAM wParam,
    LPARAM lParam);

auto g_app = std::make_shared<App>();

int CALLBACK WinMain(
    HINSTANCE instance,
    HINSTANCE previousInstance,
    LPSTR     cmdLine,
    int       cmdShow)
{
    // Initialize COM
	init_apartment(apartment_type::single_threaded);

    // Check to see that capture is supported
    auto isCaptureSupported = winrt::Windows::Graphics::Capture::GraphicsCaptureSession::IsSupported();
    if (!isCaptureSupported)
    {
        MessageBox(
            NULL,
            L"Screen capture is not supported on this device for this release of Windows!",
            L"Win32CaptureSample",
            MB_OK | MB_ICONERROR);

        return 1;
    }

    // Create the window
    WNDCLASSEX wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = instance;
    wcex.hIcon = LoadIcon(instance, MAKEINTRESOURCE(IDI_APPLICATION));
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"Win32CaptureSample";
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
    WINRT_VERIFY(RegisterClassEx(&wcex));

    HWND hwnd = CreateWindow(
        L"Win32CaptureSample",
        L"Win32CaptureSample",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        800,
        600,
        NULL,
        NULL,
        instance,
        NULL);
    WINRT_VERIFY(hwnd);

    ShowWindow(hwnd, cmdShow);
    UpdateWindow(hwnd);

    // Get all the windows
	auto windows = EnumerateWindows();

    // Create a DispatcherQueue for our thread
    auto controller = CreateDispatcherQueueControllerForCurrentThread();

    // Initialize Composition
    auto compositor = Compositor();
    auto target = CreateDesktopWindowTarget(compositor, hwnd, true);
    auto root = compositor.CreateContainerVisual();
    root.RelativeSizeAdjustment({ 1.0f, 1.0f });
    target.Root(root);

    auto picker = CreateCapturePickerForHwnd(hwnd);

    // Enqueue our capture work on the dispatcher
    auto queue = controller.DispatcherQueue();
    auto success = queue.TryEnqueue([=]() -> void
    {
		g_app->Initialize(windows, root);
    });
    WINRT_VERIFY(success);

    // Message pump
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}

LRESULT CALLBACK WndProc(
    HWND   hwnd,
    UINT   msg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
	case WM_LBUTTONUP:
		{
			auto rawX = GET_X_LPARAM(lParam);
			auto rawY = GET_Y_LPARAM(lParam);

			RECT rect = {};
			WINRT_VERIFY(GetClientRect(hwnd, &rect));
			auto windowWidth = (float)(rect.right - rect.left);
			auto windowHeight = (float)(rect.bottom - rect.top);

			g_app->OnClick({ rawX / windowWidth, rawY / windowHeight });
		}
		break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
        break;
    }

    return 0;
}
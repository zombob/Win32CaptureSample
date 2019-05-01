#pragma once

struct Window
{
public:
	Window(nullptr_t) {}
	Window(HWND hwnd, std::wstring& title, std::wstring& className)
	{
		m_hwnd = hwnd;
		m_title = title;
		m_className = className;
	}

	HWND Hwnd() const noexcept { return m_hwnd; }
	std::wstring Title() const noexcept { return m_title; }
	std::wstring ClassName() const noexcept { return m_className; }

private:
	HWND m_hwnd;
	std::wstring m_title;
	std::wstring m_className;
};
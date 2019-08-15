#include "Window.h"

////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not
// use this file except in compliance with the License.  You may obtain a copy
// of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
// License for the specific language governing permissions and limitations
// under the License.
//
// https://github.com/GameTechDev/IntroductionToVulkan/blob/master/Project/Common/OperatingSystem.cpp
// This file has been modified for the purposes of this project.
////////////////////////////////////////////////////////////////////////////////


namespace MelonRenderer {

	Window::Window() : m_windowHandle() {
	}

	WindowHandle Window::GetParameters() const {
		return m_windowHandle;
	}

#if defined(VK_USE_PLATFORM_WIN32_KHR)

	constexpr char windowName[] = "MelonRenderer" ;

	LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
		switch (message) {
		case WM_SIZE:
		case WM_EXITSIZEMOVE:
			PostMessage(hWnd, WM_USER + 1, wParam, lParam);
			break;
		case WM_KEYDOWN:
		case WM_CLOSE:
			PostMessage(hWnd, WM_USER + 2, wParam, lParam);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		return 0;
	}

	Window::~Window() {
		if (m_windowHandle.m_HWnd) {
			DestroyWindow(m_windowHandle.m_HWnd);
		}

		if (m_windowHandle.m_hInstance) {
			UnregisterClass(windowName, m_windowHandle.m_hInstance);
		}
	}

	bool Window::Init(const char* name) {
		m_windowHandle.m_hInstance = GetModuleHandle(nullptr);

		// Register window class in windows
		WNDCLASSEX wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = m_windowHandle.m_hInstance;
		wcex.hIcon = NULL;
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName = NULL;
		wcex.lpszClassName = windowName;
		wcex.hIconSm = NULL;

		if (!RegisterClassEx(&wcex)) {
			return false;
		}

		m_windowHandle.m_HWnd = CreateWindow(windowName, name, WS_OVERLAPPEDWINDOW, windowX, windowY, windowWidth, windowHeight, 
			nullptr, nullptr, m_windowHandle.m_hInstance, nullptr);
		if (!m_windowHandle.m_HWnd) {
			return false;
		}

		return true;
	}

	bool Window::Tick(WindowEventHandler& windowEventhandler) const {
		// Display window
		ShowWindow(m_windowHandle.m_HWnd, SW_SHOWNORMAL);
		UpdateWindow(m_windowHandle.m_HWnd);

		// Main message loop
		MSG message;
		bool loop = true;
		bool resize = false;
		bool result = true;

		while (loop) {
			if (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
				// Process events
				switch (message.message) {
					// Resize
				case WM_USER + 1:
					resize = true;
					break;
					// Close
				case WM_USER + 2:
					loop = false;
					break;
				}
				TranslateMessage(&message);
				DispatchMessage(&message);
			}
			else {
				// Resize
				if (resize) {
					resize = false;
					if (!windowEventhandler.OnWindowSizeChanged()) {
						result = false;
						break;
					}
				}
				// Draw
				if (windowEventhandler.ReadyToDraw()) {
					if (!windowEventhandler.Draw()) {
						result = false;
						break;
					}
				}
				else {
					std::this_thread::sleep_for(std::chrono::milliseconds(50));
				}
			}
		}

		return result;
	}

#elif defined(VK_USE_PLATFORM_XCB_KHR)

	Window::~Window() {
		xcb_destroy_window(m_windowHandle.m_connection, m_windowHandle.m_window);
		xcb_disconnect(m_windowHandle.m_connection);
	}

	bool Window::Init(const char* name) {
		int screen_index;
		m_windowHandle.m_connection = xcb_connect(nullptr, &screen_index);

		if (!m_windowHandle.m_connection) {
			return false;
		}

		const xcb_setup_t* setup = xcb_get_setup(m_windowHandle.m_connection);
		xcb_screen_iterator_t screen_iterator = xcb_setup_roots_iterator(setup);

		while (screen_index-- > 0) {
			xcb_screen_next(&screen_iterator);
		}

		xcb_screen_t* screen = screen_iterator.data;

		m_windowHandle.m_window = xcb_generate_id(m_windowHandle.m_connection);

		uint32_t value_list[] = {
		  screen->white_pixel,
		  XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_STRUCTURE_NOTIFY
		};

		xcb_create_window(
			m_windowHandle.m_connection,
			XCB_COPY_FROM_PARENT,
			m_windowHandle.m_window,
			screen->root,
			windowX,
			windowY,
			windowWidth,
			windowHeight,
			0,
			XCB_WINDOW_CLASS_INPUT_OUTPUT,
			screen->root_visual,
			XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK,
			value_list);

		xcb_flush(m_windowHandle.m_connection);

		xcb_change_property(
			m_windowHandle.m_connection,
			XCB_PROP_MODE_REPLACE,
			m_windowHandle.m_window,
			XCB_ATOM_WM_NAME,
			XCB_ATOM_STRING,
			8,
			strlen(name),
			name);

		return true;
	}

	bool Window::Tick(WindowEventHandler& windowEventhandler) const {
		// Prepare notification for window destruction
		xcb_intern_atom_cookie_t  protocols_cookie = xcb_intern_atom(m_windowHandle.m_connection, 1, 12, "WM_PROTOCOLS");
		xcb_intern_atom_reply_t* protocols_reply = xcb_intern_atom_reply(m_windowHandle.m_connection, protocols_cookie, 0);
		xcb_intern_atom_cookie_t  delete_cookie = xcb_intern_atom(m_windowHandle.m_connection, 0, 16, "WM_DELETE_WINDOW");
		xcb_intern_atom_reply_t* delete_reply = xcb_intern_atom_reply(m_windowHandle.m_connection, delete_cookie, 0);
		xcb_change_property(m_windowHandle.m_connection, XCB_PROP_MODE_REPLACE, m_windowHandle.m_window, (*protocols_reply).atom, 4, 32, 1, &(*delete_reply).atom);
		free(protocols_reply);

		// Display window
		xcb_map_window(m_windowHandle.m_connection, m_windowHandle.m_window);
		xcb_flush(m_windowHandle.m_connection);

		// Main message loop
		xcb_generic_event_t* event;
		bool loop = true;
		bool resize = false;
		bool result = true;

		while (loop) {
			event = xcb_poll_for_event(m_windowHandle.m_connection);

			if (event) {
				// Process events
				switch (event->response_type & 0x7f) {
					// Resize
				case XCB_CONFIGURE_NOTIFY: {
					xcb_configure_notify_event_t* configure_event = (xcb_configure_notify_event_t*)event;
					static uint16_t width = configure_event->width;
					static uint16_t height = configure_event->height;

					if (((configure_event->width > 0) && (width != configure_event->width)) ||
						((configure_event->height > 0) && (height != configure_event->height))) {
						resize = true;
						width = configure_event->width;
						height = configure_event->height;
					}
				}
										   break;
										   // Close
				case XCB_CLIENT_MESSAGE:
					if ((*(xcb_client_message_event_t*)event).data.data32[0] == (*delete_reply).atom) {
						loop = false;
						free(delete_reply);
					}
					break;
				case XCB_KEY_PRESS:
					loop = false;
					break;
				}
				free(event);
			}
			else {
				// Draw
				if (resize) {
					resize = false;
					if (!windowEventhandler.OnWindowSizeChanged()) {
						result = false;
						break;
					}
				}
				if (windowEventhandler.ReadyToDraw()) {
					if (!windowEventhandler.Draw()) {
						result = false;
						break;
					}
				}
				else {
					std::this_thread::sleep_for(std::chrono::milliseconds(50));
				}
			}
		}

		return result;
	}

#elif defined(VK_USE_PLATFORM_XLIB_KHR)

	Window::~Window() {
		XDestroyWindow(m_windowHandle.m_dpy, m_windowHandle.m_window);
		XCloseDisplay(m_windowHandle.m_dpy);
	}

	bool Window::Init(const char* name) {
		m_windowHandle.m_dpy = XOpenDisplay(nullptr);
		if (!m_windowHandle.m_dpy) {
			return false;
		}

		int default_screen = DefaultScreen(m_windowHandle.m_dpy);

		m_windowHandle.m_window = XCreateSimpleWindow(
			m_windowHandle.m_dpy,
			DefaultRootWindow(m_windowHandle.m_dpy),
			windowX,
			windowY,
			windowWidth,
			windowHeight,
			1,
			BlackPixel(m_windowHandle.m_dpy, default_screen),
			WhitePixel(m_windowHandle.m_dpy, default_screen));

		// XSync( m_windowHandle.m_dpy, false );
		XSetStandardProperties(m_windowHandle.m_dpy, m_windowHandle.m_window, name, name, None, nullptr, 0, nullptr);
		XSelectInput(m_windowHandle.m_dpy, m_windowHandle.m_window, ExposureMask | KeyPressMask | StructureNotifyMask);

		return true;
	}

	bool Window::Tick(WindowEventHandler& windowEventhandler) const {
		// Prepare notification for window destruction
		Atom delete_window_atom;
		delete_window_atom = XInternAtom(m_windowHandle.m_dpy, "WM_DELETE_WINDOW", false);
		XSetWMProtocols(m_windowHandle.m_dpy, m_windowHandle.m_window, &delete_window_atom, 1);

		// Display window
		XClearWindow(m_windowHandle.m_dpy, m_windowHandle.m_window);
		XMapWindow(m_windowHandle.m_dpy, m_windowHandle.m_window);

		// Main message loop
		XEvent event;
		bool loop = true;
		bool resize = false;
		bool result = true;

		while (loop) {
			if (XPending(m_windowHandle.m_dpy)) {
				XNextEvent(m_windowHandle.m_dpy, &event);
				switch (event.type) {
					//Process events
				case ConfigureNotify: {
					static int width = event.xconfigure.width;
					static int height = event.xconfigure.height;

					if (((event.xconfigure.width > 0) && (event.xconfigure.width != width)) ||
						((event.xconfigure.height > 0) && (event.xconfigure.width != height))) {
						width = event.xconfigure.width;
						height = event.xconfigure.height;
						resize = true;
					}
				}
									  break;
				case KeyPress:
					loop = false;
					break;
				case DestroyNotify:
					loop = false;
					break;
				case ClientMessage:
					if (static_cast<unsigned int>(event.xclient.data.l[0]) == delete_window_atom) {
						loop = false;
					}
					break;
				}
			}
			else {
				// Draw
				if (resize) {
					resize = false;
					if (!windowEventhandler.OnWindowSizeChanged()) {
						result = false;
						break;
					}
				}
				if (windowEventhandler.ReadyToDraw()) {
					if (!windowEventhandler.Draw()) {
						result = false;
						break;
					}
				}
				else {
					std::this_thread::sleep_for(std::chrono::milliseconds(50));
				}
			}
		}

		return result;
	}

#endif

}


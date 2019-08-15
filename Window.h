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
// https://github.com/GameTechDev/IntroductionToVulkan/blob/master/Project/Common/OperatingSystem.h
// This file has been modified for the purposes of this project.
////////////////////////////////////////////////////////////////////////////////


#if defined(VK_USE_PLATFORM_WIN32_KHR)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#elif defined(VK_USE_PLATFORM_XCB_KHR)
#include <xcb/xcb.h>
#include <dlfcn.h>
#include <cstdlib>
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <dlfcn.h>
#include <cstdlib>
#endif

#if defined _WIN32
#define VULKAN_LIBRARY_TYPE HMODULE
#elif defined __linux
#include <dlfcn.h>
#define VULKAN_LIBRARY_TYPE void*
#endif

#include <string>
#include <iostream>
#include <chrono>
#include <thread>

namespace MelonRenderer {

	struct WindowHandle
	{
#ifdef VK_USE_PLATFORM_WIN32_KHR
		HINSTANCE m_hInstance;
		HWND m_HWnd;
#elif defined VK_USE_PLATFORM_XLIB_KHR
		Display* m_dpy;
		Window m_window;
#elif defined VK_USE_PLATFORM_XCB_KHR
		xcb_connection_t* m_connection;
		xcb_window_t m_window;
#endif
	};

	constexpr uint32_t windowWidth = 500;
	constexpr uint32_t windowHeight = 500;
	constexpr uint32_t windowX = 20;
	constexpr uint32_t windowY = 20;

	class WindowEventHandler {
		public:
			virtual bool OnWindowSizeChanged() = 0;
			virtual bool Draw() = 0;

			virtual bool ReadyToDraw() const {
				return m_drawReady;
			}

			virtual ~WindowEventHandler() {}

		protected:
			bool m_drawReady = false;
	};

	class Window {
		public:
			Window();
			~Window();

			bool Init(const char* name);
			bool Tick(WindowEventHandler& project) const;
			WindowHandle  GetParameters() const;

		private:
			WindowHandle  m_windowHandle;
	};

	} 

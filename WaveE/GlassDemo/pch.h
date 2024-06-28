//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently.

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif

#include <windows.h>
#include <atlbase.h>
#include <atlwin.h>
#include "hidusage.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>

#include <string>
#include <vector>
#include <iostream>
#include <wrl.h>
#include <shellapi.h>

#include "WMaths.h"
#include "DX12Helper.h"
#include <strsafe.h>

namespace WaveE
{
	// Useful Functions
	template <typename T>
	T align_value(T valueToAlign, int alignment)
	{
		// Calculate how much `valueToAlign` needs to be adjusted to be aligned
		T remainder = valueToAlign % alignment;
		T adjustment = (remainder == 0) ? 0 : (alignment - remainder);

		// Align `valueToAlign` based on the adjustment calculated
		return valueToAlign + adjustment;
	}

	// Useful Usings
	template <class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	// Using for device and others in case the version changes
	using WaveEDevice = ID3D12Device1;
	using WaveECommandList = ID3D12GraphicsCommandList;
	using WaveECommandQueue = ID3D12CommandQueue;
	using WaveESwapChain = IDXGISwapChain3;

	// Useful Macros

	#define WaveInstance WaveManager::Instance()

	#define WAVEE_NO_COPY(classname) \
		classname(const classname&) = delete; \
		classname& operator=(const classname&) = delete;

	#define WAVEE_SINGLETON(classname) \
		WAVEE_NO_COPY(classname) \
		public: \
			static classname* Instance() \
			{ \
				return ms_pInstance; \
			} \
			static void Init() \
			{ \
				if (!ms_pInstance) \
				{ \
					ms_pInstance = new classname; \
				} \
			} \
			static void Uninit() \
			{ \
				delete ms_pInstance; \
				ms_pInstance = nullptr; \
			} \
		private: \
			static classname* ms_pInstance;

	#define WAVEE_SINGLETON_CPP(classname) \
		classname* classname::ms_pInstance = nullptr;

	#define WAVEE_ASSERT(condition) \
		{\
		bool WAVEE_ASSERT_RESULT = condition;\
		assert(WAVEE_ASSERT_RESULT);\
		}

	#define WAVEE_ASSERT_MESSAGE(condition, message) \
		{\
		bool WAVEE_ASSERT_RESULT = condition;\
		assert(WAVEE_ASSERT_RESULT);\
		}

	#define WAVEE_ERROR_DWORD(errorCode)						\
	{															\
		LPCTSTR strErrorMessage = NULL;\
		FormatMessage(\
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ARGUMENT_ARRAY | FORMAT_MESSAGE_ALLOCATE_BUFFER,\
		NULL,\
		errorCode,\
		0,\
		(LPWSTR)&strErrorMessage,\
		0,\
		NULL);\
		OutputDebugString(strErrorMessage);\
		WAVEE_ASSERT(false);\
	}															
}
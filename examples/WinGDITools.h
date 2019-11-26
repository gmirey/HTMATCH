/* -----------------------------------
 * HTMATCH
 * WinGDITools.h
 * -----------------------------------
 * Defines a few display tools for testing VanillaHTM within a MS Windows environment.
 * Those may be handy to quickly build test applications
 *
 * Copyright 2019, Guillaume Mirey
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef _HTMATCH_WIN_GDI_TOOLS_H
#define _HTMATCH_WIN_GDI_TOOLS_H

#include "tools/system.h"
#include "vanillaHTM/VanillaHTMConfig.h"

#pragma comment( lib, "Gdiplus.lib" )
#include <windows.h>
#include "gdiplus.h"
#include "gdiplustypes.h"
#include "gdipluspixelformats.h"
#include "gdiplusimaging.h"

namespace HTMATCH { namespace GDI {

    // Quick-and-dirty "static" helper...
    // ... another name for 'global' uglyness. Nevermind...
    // NON thread-safe, lads.
    class GDITools {
    public:

        static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
        {
            switch(msg)
            {
                case WM_CLOSE:
                    DestroyWindow(hwnd);
                break;
                case WM_DESTROY:
                    PostQuitMessage(0);
                    _bContinue = false;
                break;
                case WM_SYSKEYDOWN:
                    if (wParam == VK_MENU) {
                        _bCancelInput = true;
                        _uLastCharTicks = 0u;
                    }
                    break;
                case WM_KEYDOWN:
                    _cLastChar = wchar_t(wParam);
                    _bCancelInput = false;
                    _uLastCharTicks = 0u;
                    break;
                case WM_SIZE:
                    {
                        delete _pGraphics;
                        HDC hdc = GetDC(_hwnd);
                        _pGraphics = Gdiplus::Graphics::FromHDC(hdc);
                    }
                    break;
                default:
                    return DefWindowProc(hwnd, msg, wParam, lParam);
            }
            return 0;
        }

        static void _toDisplayBitmapFromBinaryOnSheet(Gdiplus::Bitmap* pBitmap, const uint64* pBinaryData,
            uint8 uRed, uint8 uGreen, uint8 uBlue)
        {
            static const Gdiplus::Rect rectFullSheet(0, 0, VANILLA_HTM_SHEET_WIDTH, VANILLA_HTM_SHEET_HEIGHT);
            if (Gdiplus::Status::Ok == pBitmap->LockBits(&rectFullSheet,
                    Gdiplus::ImageLockModeWrite|Gdiplus::ImageLockModeUserInputBuf, PixelFormat24bppRGB, &_BmpData)) {
                uint8* pCurrentFromScan0 = (uint8*)(_BmpData.Scan0);
                for (uint8 uY = 0u; uY < VANILLA_HTM_SHEET_HEIGHT; uY++) {
                    size_t uIndex = size_t(uY);
                    for (uint8 uX = 0u; uX < VANILLA_HTM_SHEET_WIDTH; uX++, uIndex += VANILLA_HTM_SHEET_HEIGHT) {
                        size_t uQword = uIndex >> 6u;
                        size_t uBit = uIndex & 0x003Fu;
                        uint8 uValue = uint8(pBinaryData[uQword] >> uBit & 1uLL);
                        *pCurrentFromScan0 = uBlue * uValue;
                        pCurrentFromScan0++;
                        *pCurrentFromScan0 = uGreen * uValue;
                        pCurrentFromScan0++;
                        *pCurrentFromScan0 = uRed * uValue;
                        pCurrentFromScan0++;
                    }
                }
                pBitmap->UnlockBits(&_BmpData);
            }
        }

        template<typename DataType, typename FuncType>
        static void _toDisplayBitmapFromTypeOnSheet(Gdiplus::Bitmap* pBitmap, const DataType* pData, FuncType valueToColor)
        {
            static const Gdiplus::Rect rectFullSheet(0, 0, VANILLA_HTM_SHEET_WIDTH, VANILLA_HTM_SHEET_HEIGHT);
            uint8 uRed, uGreen, uBlue;
            if (Gdiplus::Status::Ok == pBitmap->LockBits(&rectFullSheet,
                    Gdiplus::ImageLockModeWrite|Gdiplus::ImageLockModeUserInputBuf, PixelFormat24bppRGB, &_BmpData)) {
                uint8* pCurrentFromScan0 = (uint8*)(_BmpData.Scan0);
                for (uint8 uY = 0u; uY < VANILLA_HTM_SHEET_HEIGHT; uY++) {
                    size_t uIndex = size_t(uY);
                    for (uint8 uX = 0u; uX < VANILLA_HTM_SHEET_WIDTH; uX++, uIndex += VANILLA_HTM_SHEET_HEIGHT) {
                        valueToColor(pData[uIndex], uRed, uGreen, uBlue);
                        *pCurrentFromScan0 = uBlue;
                        pCurrentFromScan0++;
                        *pCurrentFromScan0 = uGreen;
                        pCurrentFromScan0++;
                        *pCurrentFromScan0 = uRed;
                        pCurrentFromScan0++;
                    }
                }
                pBitmap->UnlockBits(&_BmpData);
            }
        }
        ; // template termination

        static uint8* getZeroedByteBufferForSheetBitmap(Gdiplus::Bitmap* pBitmap) {
            static const Gdiplus::Rect rectFullSheet(0, 0, VANILLA_HTM_SHEET_WIDTH, VANILLA_HTM_SHEET_HEIGHT);
            if (Gdiplus::Status::Ok == pBitmap->LockBits(&rectFullSheet,
                    Gdiplus::ImageLockModeWrite|Gdiplus::ImageLockModeUserInputBuf, PixelFormat24bppRGB, &_BmpData)) {
                std::memset(_BmpData.Scan0, 0, VANILLA_HTM_SHEET_2DSIZE * 3u);
                return (uint8*)(_BmpData.Scan0);
            } else {
                return 0;
            }
        }

        static void whenDoneWithByteBufferOver(Gdiplus::Bitmap* pBitmap) {
            pBitmap->UnlockBits(&_BmpData);
        }

        static bool Init()
        {
            _uLastCharTicks = 0;
            _cLastChar = L'A';
            _bContinue = true;
            _bCancelInput = false;
            _BmpData.Width = VANILLA_HTM_SHEET_WIDTH;
            _BmpData.Height = VANILLA_HTM_SHEET_HEIGHT;
            _BmpData.PixelFormat = PixelFormat24bppRGB;
            _BmpData.Stride = VANILLA_HTM_SHEET_WIDTH * 3;
            _BmpData.Scan0 = calloc(VANILLA_HTM_SHEET_HEIGHT, _BmpData.Stride);

            Gdiplus::GdiplusStartupInput gdiplusStartupInput;
            if (Gdiplus::Status::Ok != Gdiplus::GdiplusStartup(&_gdiplusToken, &gdiplusStartupInput, NULL)) {
	            free(_BmpData.Scan0);
                return false;
            }

            WNDCLASSEX wc;
            static const wchar_t* szClassName = L"HTMATCH_GDI_WINDOW";

            wc.cbSize        = sizeof(WNDCLASSEX);
            wc.style         = 0;
            wc.lpfnWndProc   = GDITools::WndProc;
            wc.cbClsExtra    = 0;
            wc.cbWndExtra    = 0;
            wc.hInstance     = GetModuleHandle(NULL);
            wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
            wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
            wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
            wc.lpszMenuName  = NULL;
            wc.lpszClassName = szClassName;
            wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

            if(!RegisterClassEx(&wc)) {
                MessageBox(NULL, L"Window Registration Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
	            free(_BmpData.Scan0);
	            Gdiplus::GdiplusShutdown(_gdiplusToken);
                return false;
            }

            _hwnd = CreateWindowEx(
                WS_EX_CLIENTEDGE,
                szClassName,
                L"Cell Activations Display",
                WS_OVERLAPPEDWINDOW,
                CW_USEDEFAULT, CW_USEDEFAULT, 240, 120,
                NULL, NULL, wc.hInstance, NULL);

            if(_hwnd == NULL) {
                MessageBox(NULL, L"Window Creation Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
	            free(_BmpData.Scan0);
	            Gdiplus::GdiplusShutdown(_gdiplusToken);
                return false;
            }

            ShowWindow(_hwnd, SW_SHOWNORMAL);
            UpdateWindow(_hwnd);
            SetWindowPos(_hwnd, 0, 10, 10, 700, 700, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);

            HDC hdc = GetDC(_hwnd);
            _pGraphics = Gdiplus::Graphics::FromHDC(hdc);

            return true;
        }

        static void Shutdown()
        {
            delete _pGraphics;
            free(_BmpData.Scan0);
            Gdiplus::GdiplusShutdown(_gdiplusToken);
        }

        // This header does not come with a .cpp
        // => need to declare the following statics in your main application...

        static HWND _hwnd;
        static uint32 _uLastCharTicks;
        static bool _bCancelInput;
        static bool _bContinue;
        static wchar_t _cLastChar;
        static Gdiplus::BitmapData _BmpData;
        static ULONG_PTR _gdiplusToken;
        static Gdiplus::Graphics* _pGraphics;
    };
    

} } // namespace GDI namespace HTMATCH

#endif // _HTMATCH_WIN_GDI_TOOLS_H


#pragma once

#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>

#pragma comment(lib, "gdiplus.lib")

// Lightweight native Windows GDI+ image wrapper replacing legacy 32-bit CxImage.
// Decodes JPG/PNG/BMP resources once into a native HBITMAP for fast, flicker-free
// BitBlt/StretchBlt rendering on both x64 and Win32 builds.
class CLauncherImage
{
public:
	CLauncherImage() : m_hBitmap(NULL), m_nWidth(0), m_nHeight(0) {}
	~CLauncherImage() { Destroy(); }

	CLauncherImage(const CLauncherImage&) = delete;
	CLauncherImage& operator=(const CLauncherImage&) = delete;

	void Destroy()
	{
		if (m_hBitmap != NULL)
		{
			::DeleteObject(m_hBitmap);
			m_hBitmap = NULL;
		}
		m_nWidth = 0;
		m_nHeight = 0;
	}

	bool IsEnabled() const { return m_hBitmap != NULL; }
	int GetWidth() const { return m_nWidth; }
	int GetHeight() const { return m_nHeight; }

	bool LoadResource(HRSRC hRes, LPCWSTR /*unusedFormat*/ = NULL)
	{
		Destroy();
		if (hRes == NULL)
			return false;

		HMODULE hModule = ::GetModuleHandle(NULL);
		DWORD dwSize = ::SizeofResource(hModule, hRes);
		if (dwSize == 0)
			return false;

		HGLOBAL hResData = ::LoadResource(hModule, hRes);
		if (hResData == NULL)
			return false;

		const void* pResBytes = ::LockResource(hResData);
		if (pResBytes == NULL)
			return false;

		HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE, dwSize);
		if (hMem == NULL)
			return false;

		void* pMem = ::GlobalLock(hMem);
		if (pMem == NULL)
		{
			::GlobalFree(hMem);
			return false;
		}
		memcpy(pMem, pResBytes, dwSize);
		::GlobalUnlock(hMem);

		IStream* pStream = NULL;
		if (FAILED(::CreateStreamOnHGlobal(hMem, TRUE, &pStream)) || pStream == NULL)
		{
			::GlobalFree(hMem);
			return false;
		}

		Gdiplus::Bitmap* pBitmap = Gdiplus::Bitmap::FromStream(pStream, FALSE);
		pStream->Release();
		if (pBitmap == NULL || pBitmap->GetLastStatus() != Gdiplus::Ok)
		{
			delete pBitmap;
			return false;
		}

		HBITMAP hBmp = NULL;
		Gdiplus::Status st = pBitmap->GetHBITMAP(Gdiplus::Color(255, 0, 255), &hBmp);
		if (st == Gdiplus::Ok && hBmp != NULL)
		{
			m_hBitmap = hBmp;
			m_nWidth = static_cast<int>(pBitmap->GetWidth());
			m_nHeight = static_cast<int>(pBitmap->GetHeight());
		}
		delete pBitmap;
		return m_hBitmap != NULL;
	}

	bool LoadFromResource(UINT uiResourceId, LPCWSTR lpType = L"JPG")
	{
		HRSRC hRes = ::FindResourceW(NULL, MAKEINTRESOURCEW(uiResourceId), lpType);
		return LoadResource(hRes);
	}

	// Transfers ownership of the underlying HBITMAP to the caller.
	HBITMAP DetachBitmap()
	{
		HBITMAP hBmp = m_hBitmap;
		m_hBitmap = NULL;
		m_nWidth = 0;
		m_nHeight = 0;
		return hBmp;
	}

	// Creates a duplicate HBITMAP (CxImage::MakeBitmap compatibility).
	HBITMAP MakeBitmap() const
	{
		if (m_hBitmap == NULL)
			return NULL;
		return static_cast<HBITMAP>(::CopyImage(m_hBitmap, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION));
	}

	void Draw(HDC hdc, const RECT& r) const
	{
		if (hdc == NULL || m_hBitmap == NULL)
			return;

		int dstW = r.right - r.left;
		int dstH = r.bottom - r.top;
		if (dstW <= 0 || dstH <= 0)
			return;

		HDC hMemDC = ::CreateCompatibleDC(hdc);
		if (hMemDC == NULL)
			return;

		HGDIOBJ hOld = ::SelectObject(hMemDC, m_hBitmap);
		if (dstW == m_nWidth && dstH == m_nHeight)
		{
			::BitBlt(hdc, r.left, r.top, dstW, dstH, hMemDC, 0, 0, SRCCOPY);
		}
		else
		{
			::SetStretchBltMode(hdc, HALFTONE);
			::StretchBlt(hdc, r.left, r.top, dstW, dstH, hMemDC, 0, 0, m_nWidth, m_nHeight, SRCCOPY);
		}
		::SelectObject(hMemDC, hOld);
		::DeleteDC(hMemDC);
	}

private:
	HBITMAP m_hBitmap;
	int m_nWidth;
	int m_nHeight;
};

#include "Stdafx.h"
#include "DnCustomControlCommon.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

void CalcHotKeyUV( int nVKHotKey, EtTextureHandle hTexture, SUICoord &UVCoord, int nIconWidth, int nIconHeight )
{
	if( !hTexture ) return;

	// Handle consecutive cases separately and others per case.
	int nIndex = 0;
	if( '0' <= nVKHotKey && nVKHotKey <= '9' )
		nIndex = nVKHotKey - '0';
	else if( 'A' <= nVKHotKey && nVKHotKey <= 'Z' )
		nIndex = 10 + nVKHotKey - 'A';
	else
	{
		switch( nVKHotKey )
		{
		case VK_BACK:		nIndex = 66; break;
		case VK_OEM_1:		nIndex = 38; break;	// DIK_SEMICOLON
		case VK_OEM_7:		nIndex = 39; break;	// DIK_APOSTROPHE
		case VK_OEM_3:		nIndex = 36; break;	// DIK_GRAVE
		case VK_OEM_5:		nIndex = 37; break;	// DIK_BACKSLASH
		case VK_OEM_COMMA:	nIndex = 40; break;
		case VK_OEM_PERIOD:	nIndex = 41; break;
		case VK_OEM_2:		nIndex = 42; break;	// DIK_SLASH
		case VK_MULTIPLY:	nIndex = 54; break;
		case VK_SPACE:		nIndex = 67; break;
		case VK_NUMPAD7:	nIndex = 49; break;
		case VK_NUMPAD8:	nIndex = 50; break;
		case VK_NUMPAD9:	nIndex = 51; break;
		case VK_SUBTRACT:	nIndex = 56; break;
		case VK_NUMPAD4:	nIndex = 46; break;
		case VK_NUMPAD5:	nIndex = 47; break;
		case VK_NUMPAD6:	nIndex = 48; break;
		case VK_ADD:		nIndex = 55; break;
		case VK_NUMPAD1:	nIndex = 43; break;
		case VK_NUMPAD2:	nIndex = 44; break;
		case VK_NUMPAD3:	nIndex = 45; break;
		case VK_NUMPAD0:	nIndex = 52; break;
		case VK_DECIMAL:	nIndex = 57; break;
		case VK_DIVIDE:		nIndex = 53; break;
		case VK_HOME:		nIndex = 62; break;
		case VK_UP:			nIndex = 58; break;
		case VK_PRIOR:		nIndex = 64; break;
		case VK_LEFT:		nIndex = 69; break;
		case VK_RIGHT:		nIndex = 68; break;
		case VK_END:		nIndex = 63; break;
		case VK_DOWN:		nIndex = 59; break;
		case VK_NEXT:		nIndex = 65; break;
		case VK_INSERT:		nIndex = 60; break;
		case VK_DELETE:		nIndex = 61; break;
		case VK_LCONTROL:	nIndex = 94; break;
		case VK_F1:			nIndex = 95; break;
		case VK_F2:			nIndex = 96; break;
		case VK_F3:			nIndex = 97; break;
		case VK_F4:			nIndex = 98; break;
		case VK_F5:			nIndex = 99; break;
		case VK_F6:			nIndex = 100; break;
		case VK_F7:			nIndex = 101; break;
		case VK_F8:			nIndex = 102; break;
		case VK_OEM_PLUS:	nIndex = 103; break;
		case VK_OEM_MINUS:	nIndex = 104; break;
		}
	}
	CalcButtonUV( nIndex, hTexture, UVCoord, nIconWidth, nIconHeight );
}
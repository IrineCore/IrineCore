#pragma once
#include "EtType.h"
#include "EtUIType.h"

struct SUIDialogInfo_Old
{
	UIAllignHoriType AllignHori;
	UIAllignVertType AllignVert;
	SUICoord DlgCoord;
	DWORD dwDlgColor;
	char szDlgTextureName[ _MAX_PATH ];
	char szUITexturename[ _MAX_PATH ];
	int nFrameWidth;
	int nFrameHeight;
	int nFrameLeft;
	int nFrameTop;
	int nFrameRight;
	int nFrameBottom;
	char szReserved[ 1024 - (_MAX_PATH * 2) - 52 ];
};

struct SUIDialogInfo_01
{
	enum { VERSION = 0x101 };

	UIAllignHoriType AllignHori;
	UIAllignVertType AllignVert;
	SUICoord DlgCoord;
	DWORD dwDlgColor;
	char szDlgTextureName[ _MAX_PATH ];
	char szUITexturename[ _MAX_PATH ];
	int nFrameWidth;
	int nFrameHeight;
	int nFrameLeft;
	int nFrameTop;
	int nFrameRight;
	int nFrameBottom;
};

struct SUIDialogInfo_02
{
	enum { VERSION = 0x102 };

	UIAllignHoriType AllignHori;
	UIAllignVertType AllignVert;
	SUICoord DlgCoord;
	DWORD dwDlgColor;
	char szDlgTextureName[ _MAX_PATH ];
	char szUITexturename[ _MAX_PATH ];
	int nFrameWidth;
	int nFrameHeight;
	int nFrameLeft;
	int nFrameTop;
	int nFrameRight;
	int nFrameBottom;
	char szOpenSoundFileName[_MAX_PATH];
	char szCloseSoundFileName[_MAX_PATH];
};

struct SUIDialogInfo_03
{
	enum { VERSION = 0x103 };

	UIAllignHoriType AllignHori;
	UIAllignVertType AllignVert;
	SUICoord DlgCoord;
	DWORD dwDlgColor;
	char szDlgTextureName[ _MAX_PATH ];
	char szUITexturename[ _MAX_PATH ];
	int nFrameWidth;
	int nFrameHeight;
	int nFrameLeft;
	int nFrameTop;
	int nFrameRight;
	int nFrameBottom;
	char szOpenSoundFileName[_MAX_PATH];
	char szCloseSoundFileName[_MAX_PATH];
	bool bModalDialog;
};

struct SUIDialogInfo_04
{
	enum { VERSION = 0x104 };

	UIAllignHoriType AllignHori;
	UIAllignVertType AllignVert;
	SUICoord DlgCoord;
	DWORD dwDlgColor;
	char szDlgTextureName[ _MAX_PATH ];
	char szUITexturename[ _MAX_PATH ];
	int nFrameWidth;
	int nFrameHeight;
	int nFrameLeft;
	int nFrameTop;
	int nFrameRight;
	int nFrameBottom;
	bool bModalDialog;
	bool bSound;
};

struct SUIDialogInfo_05
{
	enum { VERSION = 0x105 };

	UIAllignHoriType AllignHori;
	UIAllignVertType AllignVert;
	SUICoord DlgCoord;
	DWORD dwDlgColor;
	char szDlgTextureName[ _MAX_PATH ];
	char szUITexturename[ _MAX_PATH ];
	int nFrameLeft;
	int nFrameTop;
	int nFrameRight;
	int nFrameBottom;
	bool bModalDialog;
	bool bSound;
};

struct SUIDialogInfo
{
	enum { VERSION = 0x106 };

	UIAllignHoriType AllignHori;
	UIAllignVertType AllignVert;
	SUICoord DlgCoord;
	DWORD dwDlgColor;
	char szDlgTextureName[ _MAX_PATH ];
	char szUITexturename[ _MAX_PATH ];
	int nFrameLeft;
	int nFrameTop;
	int nFrameRight;
	int nFrameBottom;
	bool bModalDialog;
	bool bSound;
	bool bLockScalingByResolution;	// Prevent scale changes based on resolution. Default is False

	SUIDialogInfo()
	{
		ZeroMemory( this, sizeof( SUIDialogInfo ) );
		dwDlgColor = 0xffffffff;
		AllignHori = AT_HORI_NONE;
		AllignVert = AT_VERT_NONE;
	}

public:
	bool Load_01( CStream &stream );
	bool Load_02( CStream &stream );
	bool Load_03( CStream &stream );
	bool Load_04( CStream &stream );
	bool Load_05( CStream &stream );

	bool Load( CStream &stream );
	bool Save( CStream &stream );
};
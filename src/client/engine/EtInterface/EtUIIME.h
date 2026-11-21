#pragma once
#include <dimm.h>

#define IMM32_DLLNAME	"\\imm32.dll"
#define VER_DLLNAME		"\\version.dll"

#define MAX_CANDLIST		10
#define MAX_COMPSTRING_SIZE 256

#define CHT_IMEFILENAME1    "TINTLGNT.IME" // New Phonetic
#define CHT_IMEFILENAME2    "CINTLGNT.IME" // New Chang Jie
#define CHT_IMEFILENAME3    "MSTCIPHA.IME" // Phonetic 5.1
#define CHS_IMEFILENAME1    "PINTLGNT.IME" // MSPY1.5/2/3
#define CHS_IMEFILENAME2    "MSSCIPYA.IME" // MSPY3 for OfficeXP

#define LANG_CHT            MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_TRADITIONAL)
#define LANG_CHS            MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED)
#define _CHT_HKL            ( ( HKL )( INT_PTR )0xE0080404 ) // New Phonetic
#define _CHT_HKL2           ( ( HKL )( INT_PTR )0xE0090404 ) // New Chang Jie
#define _CHS_HKL            ( ( HKL )( INT_PTR )0xE00E0804 ) // MSPY
#define MAKEIMEVERSION( major, minor )      ( ( DWORD )( ( ( BYTE )( major ) << 24 ) | ( ( BYTE )( minor ) << 16 ) ) )

#define IMEID_CHT_VER42 ( LANG_CHT | MAKEIMEVERSION( 4, 2 ) )   // New(Phonetic/ChanJie)IME98  : 4.2.x.x // Win98
#define IMEID_CHT_VER43 ( LANG_CHT | MAKEIMEVERSION( 4, 3 ) )   // New(Phonetic/ChanJie)IME98a : 4.3.x.x // Win2k
#define IMEID_CHT_VER44 ( LANG_CHT | MAKEIMEVERSION( 4, 4 ) )   // New ChanJie IME98b          : 4.4.x.x // WinXP
#define IMEID_CHT_VER50 ( LANG_CHT | MAKEIMEVERSION( 5, 0 ) )   // New(Phonetic/ChanJie)IME5.0 : 5.0.x.x // WinME
#define IMEID_CHT_VER51 ( LANG_CHT | MAKEIMEVERSION( 5, 1 ) )   // New(Phonetic/ChanJie)IME5.1 : 5.1.x.x // IME2002(w/OfficeXP)
#define IMEID_CHT_VER52 ( LANG_CHT | MAKEIMEVERSION( 5, 2 ) )   // New(Phonetic/ChanJie)IME5.2 : 5.2.x.x // IME2002a(w/Whistler)
#define IMEID_CHT_VER60 ( LANG_CHT | MAKEIMEVERSION( 6, 0 ) )   // New(Phonetic/ChanJie)IME6.0 : 6.0.x.x // IME XP(w/WinXP SP1)
#define IMEID_CHS_VER41 ( LANG_CHS | MAKEIMEVERSION( 4, 1 ) )   // MSPY1.5  // SCIME97 or MSPY1.5 (w/Win98, Office97)
#define IMEID_CHS_VER42 ( LANG_CHS | MAKEIMEVERSION( 4, 2 ) )   // MSPY2    // Win2k/WinME
#define IMEID_CHS_VER53 ( LANG_CHS | MAKEIMEVERSION( 5, 3 ) )   // MSPY3    // WinXP

struct SInputLocale
{
	HKL   m_hKL;            // Keyboard layout
	WCHAR m_wszLangAbb[3];  // Language abbreviation
	WCHAR m_wszLang[64];    // Localized language name
};

struct SCandList
{
	WCHAR awszCandidate[ MAX_CANDLIST ][ 256 ];
	CEtUIUniBuffer HoriCand; // Candidate list string (for horizontal candidate window)
	int   nFirstSelected; // First character position of the selected string in HoriCand
	int   nHoriSelectedLen; // Length of the selected string in HoriCand
	DWORD dwCount;       // Number of valid entries in the candidate list
	DWORD dwSelection;   // Currently selected candidate entry relative to page top
	DWORD dwPageSize;
	int   nReadingError; // Index of the error character
	bool  bShowWindow;   // Whether the candidate list window is visible
	SUICoord CandidateCoord;
};

class CEtUIIME
{
public:
	CEtUIIME(void);
	virtual ~CEtUIIME(void);

protected:
	enum 
	{ 
		INDICATOR_NON_IME, 
		INDICATOR_CHS, 
		INDICATOR_CHT, 
		INDICATOR_KOREAN, 
		INDICATOR_JAPANESE 
	};
	enum IMESTATE 
	{ 
		IMEUI_STATE_OFF, 
		IMEUI_STATE_ON, 
		IMEUI_STATE_ENGLISH 
	};

	static int s_nRefCount;

	static HINSTANCE s_hDllImm32;
	static HINSTANCE s_hDllIme;
	static HINSTANCE s_hDllVer;
	static HIMC s_hImcDef;

	static bool s_bEnableImeSystem;
	static IMESTATE s_ImeState;

	static int s_nCompCaret;
	static int s_nFirstTargetConv;
	static float s_fCompStringX;
	static float s_fCompStringY;
	static BYTE s_abyCompStringAttr[ MAX_COMPSTRING_SIZE ];
	static DWORD s_adwCompStringClause[ MAX_COMPSTRING_SIZE ];
	static WCHAR s_wszReadingString[ 32 ];
	static HKL s_hklCurrent;
	static SCandList s_CandList;
	static bool s_bVerticalCand;
	static bool s_bShowReadingWindow;
	static bool s_bHorizontalReading;

	static CEtUIUniBuffer s_CompString;
	static std::vector< SInputLocale > s_vecLocale;
	static LPWSTR s_pwszCurrIndicator;
	static WCHAR s_aszIndicator[ 5 ][ 3 ];
	static bool s_bChineseIME;
	static bool s_bInsertOnType;

#if defined(DEBUG) || defined(_DEBUG)
	static bool m_bIMEStaticMsgProcCalled;
#endif

public:
	static bool s_bHideCaret;

	// For some Chinese IMEs, returning 1 for WM_IME_COMPOSITION in mainframe prevents the composition or candidate window from appearing at the configured position.
	// So it must be intercepted via procedure call, at which point SendCompString causes characters to be entered twice when composition ends (when GCS_RESULTSTR arrives).
	// Control variable to prevent calling SendCompString in this case.
	static bool s_bRESULTSTR_NotSendComp;

	// Determines whether to disable IME when losing focus. Default is false.
	// So far, false for domestic builds and true for all overseas versions.
	static bool s_bDisableIMEonKillFocus;

	// Reportedly crashes on the Chinese client when switching from souguIME to Intelligent ABC IME.
	// Turned out it crashes the moment IME API is called to process GCS_COMPCLAUSE.
	// Cause unknown, so skip handling this message on the Chinese client. Defaults to true, false only for _CH.
	//
	// Another unknown issue is that Intelligent ABC IME does not work at all on the Korean client. What could be the problem..
	// This isn't a Dragon Nest client bug; it doesn't work in MS Word either.
	static bool s_bProcess_GCS_COMPCLAUSE;

	// The US client requested not to use IMEEditBox or LineIMEEditBox.
	// However, searching through the entire UI to replace and manage controls feels too wrong, so add a flag instead.
	// When this is enabled, IME does not work even though it is an IME control.
	static bool s_bDisableIMEonIMEControl;

protected:
	static void SetIMEMode( DWORD dwNewConvMode, DWORD dwNewSentMode, BOOL fFlag );

public:
	static bool IsNativeMode();
	static void ConverseNativeToEng()		{ SetIMEMode ( IME_CMODE_NATIVE, 0, FALSE ); }
	static void ConverseEngToNative()		{ SetIMEMode ( IME_CMODE_NATIVE, 0, TRUE ); }
	static void ConverseFullToHalf()		{ SetIMEMode ( IME_CMODE_FULLSHAPE, 0, FALSE ); }

public:
	static void Initialize();
	static void Uninitialize();

	static void StaticOnCreateDevice();
	static bool StaticMsgProc( UINT uMsg, WPARAM wParam, LPARAM lParam );

	static void EnableImeSystem( bool bEnable ) { s_bEnableImeSystem = bEnable; }

	static int GetCompStringLength() { return s_CompString.GetTextSize(); }
	static std::wstring GetCompString() { return s_CompString.GetBuffer(); }

	// Exposed as public because it also needs to be called from MainFrame.
	static void CheckToggleState();

protected:
	static WORD GetLanguage() { return LOWORD( s_hklCurrent ); }
	static WORD GetPrimaryLanguage() { return PRIMARYLANGID( LOWORD( s_hklCurrent ) ); }
	static WORD GetSubLanguage() { return SUBLANGID( LOWORD( s_hklCurrent ) ); }
	static void SendKey( BYTE nVirtKey );
	static DWORD GetImeId( UINT uIndex = 0 );
	static void CheckInputLocale();
	static void SetupImeApi();
	static void ResetCompositionString();
	static void GetReadingWindowOrientation( DWORD dwId );
	static void GetPrivateReadingString();

public:
	// Empty implementation of the IMM32 API
	static INPUTCONTEXT* WINAPI Dummy_ImmLockIMC( HIMC ) { return NULL; }
	static BOOL WINAPI Dummy_ImmUnlockIMC( HIMC ) { return FALSE; }
	static LPVOID WINAPI Dummy_ImmLockIMCC( HIMCC ) { return NULL; }
	static BOOL WINAPI Dummy_ImmUnlockIMCC( HIMCC ) { return FALSE; }
	static BOOL WINAPI Dummy_ImmDisableTextFrameService( DWORD ) { return TRUE; }
	static LONG WINAPI Dummy_ImmGetCompositionStringW( HIMC, DWORD, LPVOID, DWORD ) { return IMM_ERROR_GENERAL; }
	static DWORD WINAPI Dummy_ImmGetCandidateListW( HIMC, DWORD, LPCANDIDATELIST, DWORD ) { return 0; }
	static HIMC WINAPI Dummy_ImmGetContext( HWND ) { return NULL; }
	static BOOL WINAPI Dummy_ImmReleaseContext( HWND, HIMC ) { return FALSE; }
	static HIMC WINAPI Dummy_ImmAssociateContext( HWND, HIMC ) { return NULL; }
	static BOOL WINAPI Dummy_ImmGetOpenStatus( HIMC ) { return 0; }
	static BOOL WINAPI Dummy_ImmSetOpenStatus( HIMC, BOOL ) { return 0; }
	static BOOL WINAPI Dummy_ImmGetConversionStatus( HIMC, LPDWORD, LPDWORD ) { return 0; }
	static HWND WINAPI Dummy_ImmGetDefaultIMEWnd( HWND ) { return NULL; }
	static UINT WINAPI Dummy_ImmGetIMEFileNameA( HKL, LPSTR, UINT ) { return 0; }
	static UINT WINAPI Dummy_ImmGetVirtualKey( HWND ) { return 0; }
	static BOOL WINAPI Dummy_ImmNotifyIME( HIMC, DWORD, DWORD, DWORD ) { return FALSE; }
	static BOOL WINAPI Dummy_ImmSetConversionStatus( HIMC, DWORD, DWORD ) { return FALSE; }
	static BOOL WINAPI Dummy_ImmSimulateHotKey( HWND, DWORD ) { return FALSE; }
	static BOOL WINAPI Dummy_ImmIsIME( HKL ) { return FALSE; }

	// Traditional Chinese IME
	static UINT WINAPI Dummy_GetReadingString( HIMC, UINT, LPWSTR, PINT, BOOL*, PUINT ) { return 0; }
	static BOOL WINAPI Dummy_ShowReadingWindow( HIMC, BOOL ) { return FALSE; }

	// Verion library imports
	static BOOL APIENTRY Dummy_VerQueryValueA( const LPVOID, LPSTR, LPVOID *, PUINT ) { return 0; }
	static BOOL APIENTRY Dummy_GetFileVersionInfoA( LPSTR, DWORD, DWORD, LPVOID ) { return 0; }
	static DWORD APIENTRY Dummy_GetFileVersionInfoSizeA( LPSTR, LPDWORD ) { return 0; }

	// Function pointers: IMM32
	static INPUTCONTEXT* (WINAPI * _ImmLockIMC)( HIMC );
	static BOOL (WINAPI * _ImmUnlockIMC)( HIMC );
	static LPVOID (WINAPI * _ImmLockIMCC)( HIMCC );
	static BOOL (WINAPI * _ImmUnlockIMCC)( HIMCC );
	static BOOL (WINAPI * _ImmDisableTextFrameService)( DWORD );
	static LONG (WINAPI * _ImmGetCompositionStringW)( HIMC, DWORD, LPVOID, DWORD );
	static DWORD (WINAPI * _ImmGetCandidateListW)( HIMC, DWORD, LPCANDIDATELIST, DWORD );
	static HIMC (WINAPI * _ImmGetContext)( HWND );
	static BOOL (WINAPI * _ImmReleaseContext)( HWND, HIMC );
	static HIMC (WINAPI * _ImmAssociateContext)( HWND, HIMC );
	static BOOL (WINAPI * _ImmGetOpenStatus)( HIMC );
	static BOOL (WINAPI * _ImmSetOpenStatus)( HIMC, BOOL );
	static BOOL (WINAPI * _ImmGetConversionStatus)( HIMC, LPDWORD, LPDWORD );
	static HWND (WINAPI * _ImmGetDefaultIMEWnd)( HWND );
	static UINT (WINAPI * _ImmGetIMEFileNameA)( HKL, LPSTR, UINT );
	static UINT (WINAPI * _ImmGetVirtualKey)( HWND );
	static BOOL (WINAPI * _ImmNotifyIME)( HIMC, DWORD, DWORD, DWORD );
	static BOOL (WINAPI * _ImmSetConversionStatus)( HIMC, DWORD, DWORD );
	static BOOL (WINAPI * _ImmSimulateHotKey)( HWND, DWORD );
	static BOOL (WINAPI * _ImmIsIME)( HKL );

	// Function pointers: Traditional Chinese IME
	static UINT (WINAPI * _GetReadingString)( HIMC, UINT, LPWSTR, PINT, BOOL*, PUINT );
	static BOOL (WINAPI * _ShowReadingWindow)( HIMC, BOOL );

	// Function pointers: Verion library imports
	static BOOL (APIENTRY * _VerQueryValueA)( const LPVOID, LPSTR, LPVOID *, PUINT );
	static BOOL (APIENTRY * _GetFileVersionInfoA)( LPSTR, DWORD, DWORD, LPVOID );
	static DWORD (APIENTRY * _GetFileVersionInfoSizeA)( LPSTR, LPDWORD );
};
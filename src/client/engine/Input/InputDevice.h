#pragma once

#include "Singleton.h"
#include "SundriesFunc.h"
#include "Timer.h"

// With gamepad support added, mapping pad values everywhere an event occurs is...
// Seems hard to maintain, so to map gamepad values directly to keyboard and mouse
// Moved wrapping values to the Et side.
enum WrappingKeyIndex {
	IW_MOVEFRONT,
	IW_MOVEBACK,
	IW_MOVELEFT,
	IW_MOVERIGHT,
	IW_TOGGLEAUTORUN,
	IW_JUMP,
	IW_PICKUP,
	IW_REBIRTH,
	IW_TURN,
	IW_TOGGLEMOUSE,
	IW_TOGGLEDROPITEM,
	IW_DECREASEMOUSE,
	IW_INCREASEMOUSE,
	IW_TOGGLEBATTLE,
	IW_AIMASSIST,
	IW_NORMALATTK,
	IW_SPECIALATTK,
	IW_AVOID,
	IW_LOOKLEFT,
	IW_LOOKRIGHT,
	IW_LOOKUP,
	IW_LOOKDOWN,
	IW_ZOOMIN,
	IW_ZOOMOUT,

	WrappingKeyIndex_Amount,
};

enum UIWrappingKeyIndex {
	IW_UI_QUICKSLOT1,
	IW_UI_QUICKSLOT2,
	IW_UI_QUICKSLOT3,
	IW_UI_QUICKSLOT4,
	IW_UI_QUICKSLOT5,
	IW_UI_QUICKSLOT6,
	IW_UI_QUICKSLOT7,
	IW_UI_QUICKSLOT8,
	IW_UI_QUICKSLOT9,
	IW_UI_QUICKSLOT10,
	IW_UI_CHAR,
	IW_UI_COMMUNITY,
	IW_UI_INVEN,
	IW_UI_QUEST,
	IW_UI_SKILL,
	IW_UI_MISSION,
	IW_UI_MAP,
	IW_UI_HELP,
	IW_UI_CHAT,
	IW_UI_REPUTE,
	IW_UI_QUICKSLOTCHANGE,
	IW_CHATREPLY,
	IW_PVPTAB,
	IW_SYSTEM,
	IW_UI_EVENT,
	IW_UI_USERHELP,
	IW_UI_SITUATION,
	IW_UI_PVP_VILLAGE_ACCESS,
	IW_UI_TOGGLEMIC,
	IW_UI_MAILBOX_OPEN,
	IW_UI_PVPRANK_OPEN,
#if defined( PRE_ADD_SKILLSLOT_EXTEND )
	IW_UI_QUICKSLOT11,
	IW_UI_QUICKSLOT12,
	IW_UI_QUICKSLOT13,
	IW_UI_QUICKSLOT14,
	IW_UI_QUICKSLOT15,
	IW_UI_QUICKSLOT16,
	IW_UI_QUICKSLOT17,
	IW_UI_QUICKSLOT18,
	IW_UI_QUICKSLOT19,
	IW_UI_QUICKSLOT20,
#endif	// #if defined( PRE_ADD_SKILLSLOT_EXTEND )
	IW_UI_MENU,
	UIWrappingKeyIndex_Amount,
};

struct SWrappingData
{
	BYTE cDIK;
	BYTE cVK;
};
extern SWrappingData g_EnableWrappingData[];

BYTE _ToDIK( BYTE cVK );
BYTE _ToVK( BYTE cDIK );
bool _IsEnableWrappingDIK( BYTE cDIK );
BYTE _NoNumLockVKToNumLockVK( BYTE cVK );	// Why is the function name like this.
BYTE _GetCurrentPushKey( bool bTransNumLock = false );


class CInputReceiver;
class CInputDeviceBase;
class CInputDevice : public CSingleton<CInputDevice>
{
public:
	CInputDevice();
	virtual ~CInputDevice();

	enum DEVICE_TYPE {
		KEYBOARD = 0x01,
		MOUSE = 0x02,
		JOYPAD = 0x04,
	};
	enum eAbsenceCheckTimerType
	{
		eABSENCE_MIN,
		eABSENCE_PARTY = eABSENCE_MIN,
		eABSENCE_IDLEMOTION,
		eABSENCE_MAX
	};

	struct SAbsenceCheckInfo
	{
		float	fAbsenceCheckTimer;
		bool	bHoldAbsenceCheckTimer;
	};

	bool Initialize( HWND hWnd, int nDeviceType, bool bCheckFocus = false );
	void Finalize();
	void Process( LOCAL_TIME LocalTime, float fDelta );


	LPDIRECTINPUT8 GetDI() { return m_pDI; }
	HWND GetWndHandle() { return m_hWnd; }
	CInputDeviceBase *GetDeviceObject( DEVICE_TYPE DeviceType );
	void AddReceiverObject( CInputReceiver *pObject ) { m_pVecReceiverList.push_back( pObject ); }
	void RemoveReceiverObject( CInputReceiver *pObject ) { ERASE_VEC( m_pVecReceiverList, pObject ); }

	void SetForceProcessPass( bool bPass = true ) { m_bForcePassProcess = bPass; }
	void ResetAllInput();

	bool IsValidAbsenceChecktype(eAbsenceCheckTimerType type) const;
	float GetAbsenceCheckTimer(eAbsenceCheckTimerType type) const;
	void ResetAbsenceCheckTimer(eAbsenceCheckTimerType type);
	void HoldAbsenceCheckTimer(eAbsenceCheckTimerType type, bool bHold);

	bool CreateDevice( DEVICE_TYPE Type );

	static int ms_InverseModeCount;
	static int ms_InverseMode;		//Input invert mode (1:Keyboard&Mouse, 2:Keyboard, 3:Mouse)
	static void SetInverseMode(int nMode);
	static int GetInverseMode() { return ms_InverseMode; }

	static bool IsInverseKeyboard();
	static bool IsInverseMouse();

	static void ResetInverseMode();

protected:
	HWND m_hWnd;
	LPDIRECTINPUT8 m_pDI;
	bool m_bCheckWndFocus;
	std::vector<CInputDeviceBase *> m_pVecDevice;
	std::vector<CInputReceiver *> m_pVecReceiverList;
	bool m_bForcePassProcess;

	SAbsenceCheckInfo m_AbsenceCheckTimers[eABSENCE_MAX];
};

class CInputDeviceBase {
public:
	CInputDeviceBase( CInputDevice::DEVICE_TYPE DeviceType ) { m_DeviceType = DeviceType; m_nEvent = 0; }
	virtual ~CInputDeviceBase() {}

	virtual bool Initialize() = 0;
	virtual bool Process( LOCAL_TIME LocalTime ) = 0;
	virtual void ResetDeviceState() = 0;

	int GetEvent() { return m_nEvent; }
	CInputDevice::DEVICE_TYPE GetDeviceType() { return m_DeviceType; }

protected:
	CInputDevice::DEVICE_TYPE m_DeviceType;
	int m_nEvent;

};

class CInputKeyboard : public CInputDeviceBase
{
public:
	CInputKeyboard();
	virtual ~CInputKeyboard();

	virtual bool Initialize();
	virtual bool Process( LOCAL_TIME LocalTime );

	bool IsPushKey( BYTE cKey ) { return m_bKeyState[cKey]; }
	void ReleasePushKey( BYTE cKey ) { m_bKeyState[cKey] = false; }
	void ResetDeviceState() { 
		memset( m_bKeyState, 0, sizeof(m_bKeyState) ); 
		memset( m_PushKeyTime, 0, sizeof(m_PushKeyTime) ); 
		memset( m_ReleaseKeyTime, 0, sizeof(m_ReleaseKeyTime) ); 
		m_VecKeyList.clear(); 
		m_VecKeyEventTime.clear(); 
		m_nEvent = 0; 
	}
	std::vector<BYTE> *GetEventKeyList() { return &m_VecKeyList; }

	LOCAL_TIME	GetEventTime( DWORD dwIndex ) { return m_VecKeyEventTime[dwIndex]; }
	void		SetEventTime( DWORD dwIndex, LOCAL_TIME LocalTime ) { m_VecKeyEventTime[dwIndex] = LocalTime; }

	LOCAL_TIME	GetPushKeyTime( BYTE cKey ) { return m_PushKeyTime[cKey]; }
	LOCAL_TIME	GetReleaseKeyTime( BYTE cKey ) { return m_ReleaseKeyTime[cKey]; }

	HRESULT		SetCooperativeLevel(DWORD flag);

	void AllowAccessibilityShortcutKeys( bool bAllowKeys );

	void SetKey( int index, bool bPush, LOCAL_TIME LocalTime, DWORD pushTime );

protected:
	LPDIRECTINPUTDEVICE8 m_pDIDevice;
	DWORD m_dwBufferCount;
	DWORD m_dwApplyBufferCount;
	DIDEVICEOBJECTDATA *m_pDidod;

	bool m_bKeyState[256];
	LOCAL_TIME m_PushKeyTime[256];
	LOCAL_TIME m_ReleaseKeyTime[256];

	// Kept for Event information.
	std::vector<BYTE> m_VecKeyList;
	std::vector<LOCAL_TIME> m_VecKeyEventTime;

	STICKYKEYS m_StartupStickyKeys;
	TOGGLEKEYS m_StartupToggleKeys;
	FILTERKEYS m_StartupFilterKeys;

protected:
	HRESULT GetDeviceData();
	void Release();
};

class CInputMouse : public CInputDeviceBase
{
public:

	enum emMOUSE_BUTTON
	{
		buttonLeft = 0,
		buttonRight = 1,
		buttonMiddle = 2,
		buttonMax = 3,
	};

	CInputMouse();
	virtual ~CInputMouse();

	virtual bool Initialize();
	virtual bool Process( LOCAL_TIME LocalTime );

	D3DXVECTOR2 *GetMousePos() { return &m_MousePos; }
	D3DXVECTOR3 *GetMouseVariatioin() { return &m_MouseVariation; }
	bool IsPushButton( BYTE cButtonIndex ) { return m_bButtonState[cButtonIndex]; }
	void ReleasePushButton( BYTE cButtonIndex ) { m_bButtonState[cButtonIndex] = false; }
	void ResetDeviceState() { 
		memset( m_bButtonState, 0, sizeof(m_bButtonState) );
		memset( m_ButtonLastTime, 0, sizeof(m_ButtonLastTime) );
		memset( m_MouseEventTime, 0, sizeof(m_MouseEventTime) ); 
		m_MousePos = EtVector2( 0.f, 0.f );
		m_MouseVariation = EtVector3( 0.f, 0.f, 0.f );
		m_nEvent = 0; 
		m_VecEventList.clear(); 
	}

	LOCAL_TIME GetEventTime( DWORD dwIndex ) { return m_MouseEventTime[dwIndex]; }
	void SetEventTime( DWORD dwIndex, LOCAL_TIME LocalTime ) { m_MouseEventTime[dwIndex] = LocalTime; }

	std::vector<int> *GetEventList() { return &m_VecEventList; }

	HRESULT		SetCooperativeLevel(DWORD flag);

	void SetLeftButton( bool bPush, LOCAL_TIME LocalTime, DWORD pushTime );
	void SetRightButton( bool bPush, LOCAL_TIME LocalTime, DWORD pushTime );
	void SetPos( int delta, int index );

protected:
	LPDIRECTINPUTDEVICE8 m_pDIDevice;
	DWORD m_dwBufferCount;
	DWORD m_dwApplyBufferCount;
	DIDEVICEOBJECTDATA *m_pDidod;

	D3DXVECTOR2 m_MousePos;
	D3DXVECTOR3 m_MouseVariation;
	bool m_bButtonState[buttonMax];
	LOCAL_TIME m_ButtonLastTime[buttonMax];
	LOCAL_TIME m_MouseEventTime[11];

	std::vector<int> m_VecEventList;

protected:
	HRESULT GetDeviceData();
	void Release();
};

class CInputJoyPad : public CInputDeviceBase
{
public:

	enum {
		LS_UP,
		LS_DOWN,
		LS_LEFT,
		LS_RIGHT,
		LS_ZOOMIN,
		LS_ZOOMOUT,
		RS_UP,
		RS_DOWN,
		RS_LEFT,
		RS_RIGHT,
		RS_ZOOMIN,
		RS_ZOOMOUT,
		MS_UP,
		MS_DOWN,
		MS_LEFT,
		MS_RIGHT,
		B0,
		B1,
		B2,
		B3,
		B4,
		B5,
		B6,
		B7,
		B8,
		B9,
		B10,
		B11,
		B12,
		B13,
		B14,
		B15,
		B16,
		B17,
		B18,
		B19,
		B20,
		B21,
		B22,
		B23,
		B24,
		B25,
		B26,
		B27,
		B28,
		B29,
		B30,
		B31,

		MAX_BUTTON,
		
		VIEW_DELTA = 10,

		NULL_VALUE = 255
	};

	CInputJoyPad();
	virtual ~CInputJoyPad();

	virtual bool Initialize();
	virtual bool Process( LOCAL_TIME LocalTime );

	bool IsPushEnable( std::pair<BYTE, BYTE> & cKeys );
	bool IsPushButton( BYTE cButtonIndex ) { return m_bButtonState[cButtonIndex]; }
	bool IsPushJoypadButton( std::pair<BYTE, BYTE> & cKeys );
	void ReleasePushButton( BYTE cButtonIndex ) { m_bButtonState[cButtonIndex] = false; }
	void ResetDeviceState() { 
		memset( m_bButtonState, 0, sizeof(m_bButtonState) );
		memset( m_ReleaseJoypadTime, 0, sizeof(m_ReleaseJoypadTime) );
		memset( m_PushJoypadTime, 0, sizeof(m_PushJoypadTime) );
		m_nEvent = 0; 
		m_VecEventList.clear(); 
	}

	void SetPushButtonTime( BYTE cKey, LOCAL_TIME LocalTime )	{ m_PushJoypadTime[cKey] = LocalTime; }
	void SetReleaseButtonTime( BYTE cKye, LOCAL_TIME LocalTime )	{m_ReleaseJoypadTime[cKye] = LocalTime; }
	LOCAL_TIME	GetPushButtonTime( BYTE cKey ) { return m_PushJoypadTime[cKey]; }
	LOCAL_TIME	GetReleaseButtonTime( BYTE cKey ) { return m_ReleaseJoypadTime[cKey]; }

	LOCAL_TIME GetEventTime( DWORD dwIndex ) { return m_VecEventTime[dwIndex]; }
	void SetEventTime( DWORD dwIndex, LOCAL_TIME LocalTime ) { m_VecEventTime[dwIndex] = LocalTime; }

	void SetWrappingKey( BYTE * pWrappingKey, BYTE * pUIWrappingKey );
	void SetWrappingButton(BYTE * pWrappingButton, BYTE * pWrappingAssistButton, BYTE * pUIWrappingButton, BYTE * pUIWrappingAssistButton, int wrappingSize, int uiWrappingSize);

	std::vector<BYTE> *GetEventList() { return &m_VecEventList; }

	HRESULT		SetCooperativeLevel(DWORD flag);

	void SetKeyboard( CInputKeyboard * pKeyboard );
	void SetMouse( CInputMouse * pMouse );
	void SetHWND( HWND hWnd ) { m_hWnd = hWnd; }

	float GetCursorSpeed()	{ return m_fCursorSpeed; }
	float GetCameraSpeed()	{ return m_fCameraSpeed; }
	void  SetCursorSpeed(float fSpeed)	{ m_fCursorSpeed = fSpeed; }
	void  SetCameraSpeed(float fSpeed)	{ m_fCameraSpeed = fSpeed; }

protected:
	LPDIRECTINPUTDEVICE8 m_pDIDevice;
	DWORD m_dwBufferCount;
	DWORD m_dwApplyBufferCount;
	DIDEVICEOBJECTDATA *m_pDidod;
	HWND m_hWnd;

	bool m_bButtonState[MAX_BUTTON];
	LOCAL_TIME m_ReleaseJoypadTime[MAX_BUTTON];
	LOCAL_TIME m_PushJoypadTime[MAX_BUTTON];
	DWORD m_JoypadTimeStamp[MAX_BUTTON];

	bool m_ButtonPush[WrappingKeyIndex_Amount];
	bool m_UIButtonPush[UIWrappingKeyIndex_Amount];

	std::vector<BYTE> m_VecEventList;
	std::vector<LOCAL_TIME> m_VecEventTime;

	BYTE * m_pWrappingKey;
	BYTE * m_pUIWrappingKey;
	BYTE * m_pWrappingButton;
	BYTE * m_pWrappingAssistButton;
	BYTE * m_pUIWrappingButton;
	BYTE * m_pUIWrappingAssistButton;

	CInputMouse *		m_pMouse;
	CInputKeyboard *	m_pKeyboard;

	int m_iWrappingSize;
	int m_iUIWrappingSize;

	float m_fCursorSpeed;
	float m_fCameraSpeed;

protected:
	HRESULT GetDeviceData();
	
	void Release();
};
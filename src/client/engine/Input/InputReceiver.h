#pragma once

#include "InputDevice.h"

class CInputReceiver {
public:
	CInputReceiver( bool bUseEventMsg = false );
	virtual ~CInputReceiver();

	enum IR_STATE {
		IR_KEY_DOWN = 0x0001,
		IR_KEY_UP = 0x0002,

		IR_MOUSE_MOVE = 0x0004,
		IR_MOUSE_WHEEL = 0x0008,
		IR_MOUSE_LB_DOWN = 0x0010,
		IR_MOUSE_LB_UP = 0x0020,
		IR_MOUSE_RB_DOWN = 0x0040,
		IR_MOUSE_RB_UP = 0x0080,
		IR_MOUSE_WB_DOWN = 0x0100,
		IR_MOUSE_WB_UP = 0x0200,

		IR_MOUSE_LB_DBCLK = 0x0400,
		IR_MOUSE_RB_DBCLK = 0x0800,
		IR_MOUSE_WB_DBCLK = 0x1000,

		IR_JOYPAD_DOWN = 0x2000,
		IR_JOYPAD_UP = 0x4000,
	};

	virtual void OnInputReceive( int nReceiverState, LOCAL_TIME LocalTime = -1 ) {}

	// Screen coordinate-based value ( 0~ScrrenX, 0~ScreenY )
	D3DXVECTOR2 GetMousePosByScreen();
	// Last responded coordinates (Z is wheel)
	D3DXVECTOR3 GetMouseVariation();

protected:
	bool m_bUseEventMsg;
	unsigned short m_State;
	CInputKeyboard *m_pDeviceKeyboard;
	CInputMouse *m_pDeviceMouse;
	CInputJoyPad * m_pDeviceJoypad;

	bool m_bPrevKeyState[256];
	LOCAL_TIME m_PrevKeyTime[256];

	bool m_bPrevJoypadState[CInputJoyPad::MAX_BUTTON];
	LOCAL_TIME m_PrevJoypadTime[CInputJoyPad::MAX_BUTTON];

	LOCAL_TIME m_PrevButtonTime[3];

public: // Should be protected.. temporarily made public for tool shortcut.
	// Keyboard values accessible anytime.
	bool IsPushKey( BYTE cKey );
	LOCAL_TIME GetPushKeyTime( BYTE cKey );
	LOCAL_TIME GetReleaseKeyTime( BYTE cKey );
	float GetPushKeyDelta( BYTE cKey, LOCAL_TIME LocalTime );
	void ReleasePushKey( BYTE cKey );
	void ResetPushKeyDelta( BYTE cKey, LOCAL_TIME LocalTime );

	// Mouse values accessible anytime.
	bool IsPushMouseButton( BYTE cButtonIndex );
	void ReleasePushButton( BYTE cButtonIndex );
	void ReleaseAllButton();
	float GetPushButtonDelta( BYTE cButtonIndex, LOCAL_TIME LocalTime );
	void ResetPushButtonDelta( BYTE cButtonIndex, LOCAL_TIME LocalTime );

	//Joystick values accessible anytime
	bool IsPushJoypadButton( std::pair<BYTE, BYTE> & cKeys );
	void SetPushJoypadButtonTime( std::pair<BYTE, BYTE> cKeys, LOCAL_TIME LocalTime );
	void SetReleaseJoypadButtonTime( std::pair<BYTE, BYTE> cKeys, LOCAL_TIME LocalTime );
	LOCAL_TIME GetPushJoypadButtonTime( std::pair<BYTE, BYTE> cKeys );
	LOCAL_TIME GetReleaseJoypadButtonTime( std::pair<BYTE, BYTE> cKeys );
	float GetPushJoypadButtonDelta( std::pair<BYTE, BYTE> cKeys, LOCAL_TIME LocalTime );
	void ReleaseJoypadButton( std::pair<BYTE, BYTE> cKeys );
	void ResetPushJoypadButtonDelta( std::pair<BYTE, BYTE> cKeys, LOCAL_TIME LocalTime );
	bool IsJoypad()	{ return m_pDeviceJoypad ? true : false; }
	float GetCursorSpeed();
	float GetCameraSpeed();
	void  SetCursorSpeed(float fSpeed);
	void  SetCameraSpeed(float fSpeed);
	
	// Keyboard values obtainable at the moment state changes in OnInputReceive
	DWORD GetEventKeyCount();
	BYTE GetEventKey( DWORD dwIndex );
	bool GetEventKeyState( DWORD dwIndex );
	LOCAL_TIME GetEventKeyTime( DWORD dwIndex );
	void SetEventKeyTime( DWORD dwIndex, LOCAL_TIME LocalTime );

	// Mouse values obtainable at the moment state changes in OnInputReceive
	int GetEventMouse();
	DWORD GetEventMouseCount();
	int GetEventMouse( DWORD dwIndex );
	LOCAL_TIME GetEventMouseTime( IR_STATE State );
	void SetEventMouseTime( IR_STATE State, LOCAL_TIME LocalTime );

	// Joystick values obtainable at the moment state changes in OnInputReceive
	DWORD GetEventJoypadCount();
	BYTE GetEventJoypad( DWORD dwIndex );
	bool GetEventJoypadState( DWORD dwIndex );
	LOCAL_TIME GetEventJoypadTime( DWORD dwIndex );
	void SetEventJoypadTime( DWORD dwIndex, LOCAL_TIME LocalTime );

	void SetWrappingKey( BYTE * pWrappingKey, BYTE * pUIWrappingKey );
	void SetPadWrappingButton(BYTE * pWrappingButton, BYTE * pWrappingAssistButton, BYTE * pUIWrappingButton, BYTE * pUIWrappingAssistButton, int wrappingSize, int uiWrappingSize);

	bool ResetDevicePointer( int nDeviceType );

	void UseEventMessage( bool bUse );
};
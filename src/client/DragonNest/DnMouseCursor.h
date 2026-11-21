#pragma once

#include "Singleton.h"
#include "InputReceiver.h"

class CDnMouseCursor : public CSingleton<CDnMouseCursor>, public CInputReceiver, public CLostDeviceProcess
{
public:
	CDnMouseCursor();
	virtual ~CDnMouseCursor();

	enum CursorStateEnum
	{
		NOASSIGN = -1,
		NORMAL = 0,
		CLICK = 1,
		SIZE = 2,
		CursorStateEnum_Amount,
	};

private:
	CursorStateEnum	m_CursorState;
	CursorStateEnum	m_LastCursorState;
	HCURSOR	m_CursorArray[CursorStateEnum_Amount];
	EtTextureHandle m_CursorTexture[CursorStateEnum_Amount];
	float m_fMouseSensitivity;
	bool m_bShowCursor;
	int m_nShowRefCnt;

	// Each State has a different reference point. Default is 1, 1
	UINT m_nXHotSpotArray[CursorStateEnum_Amount];
	UINT m_nYHotSpotArray[CursorStateEnum_Amount];

	RECT m_prevClipCursorRect;
	POINT m_prevCursorPos;

	//blondy
	bool m_bStatic; //Added always-visible feature to prevent mouse interference from other tasks or UI in UI-only scenes
	//blondy end

	bool m_bForceClipCursorNull;

public:
	virtual void OnLostDevice();
	virtual void OnResetDevice();

	bool Create();
	void SetCursorState( CursorStateEnum State );
	CursorStateEnum GetCursorState() { return m_CursorState; }
	void SetHotSpot( UINT nStateIndex, UINT nXHotSpot = 1, UINT nYHotSpot = 1 );
	void UpdateCursor();
	
	float GetMouseSensitivity() { return m_fMouseSensitivity; }
	void SetMouseSensitivity( float fValue );

	void ShowCursor( bool bShow, bool bForce = false );
	bool IsShowCursor();

	int GetShowCursorRefCount() { return m_nShowRefCnt; }

	void AddShowRefCnt();
	void ReleaseShowRefCnt();

	// Function created to automatically move the UI mouse cursor,
	// Uses coordinates in the client area as arguments.
	void SetCursorPos( int nX, int nY );
	void GetCursorPos( int *pX, int *pY );

	void SetStaticMouse( bool isStatic ){m_bStatic = isStatic;};

	void ClipCursor( RECT *pRect );
	void SetCursorPosToDevice( int nX, int nY );
	void SetForceClipCursorNull( bool bForceClipCursorNull ) { m_bForceClipCursorNull = bForceClipCursorNull; }

	float GetCursorWidth();
	float GetCursorHeight();
};
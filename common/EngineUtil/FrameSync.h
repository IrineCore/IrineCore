#pragma once

#include "Timer.h"

class CFrameSync:public TBoostMemoryPool<CFrameSync>
{
public:
	CFrameSync();
	~CFrameSync();

	void Begin( int nSecureFrame );
	void End();
	void Pause();
	void Resume();
	void Reset();

	void UpdateTime( int nDebugSecureCount = 0 );
	bool CheckSync();
	void CheckSecureFrameLoad( int nIgnoreFrame );

	LOCAL_TIME GetMSTime();

	float GetFps();
	void SetTimeSpeed( float fSpeed, DWORD dwDelay = 0 );

protected:
	LOCAL_TIME m_SystemTime;
	LOCAL_TIME m_NextSyncTime;
	LOCAL_TIME m_PrevUpdateTime;
	/*
	LARGE_INTEGER m_qpFreq;
	LARGE_INTEGER m_qpBaseTime;
	LARGE_INTEGER m_qpTime;
	LARGE_INTEGER m_qpPrevTime;
	*/

	int m_nSecureFrame;
	LOCAL_TIME m_SecureCount;
	LOCAL_TIME m_PrevSecureCount;
	bool m_bActivate;
	bool m_bPause;

	float m_fFPS;

	float m_fTimeSpeed;
	LOCAL_TIME m_LastModifySpeedTime;
	LOCAL_TIME m_RevisionModifyTime;
	DWORD m_dwModifySpeedDelay;
	DWORD m_dwPovitTime;
};


class CFrameSkip
{
public:
	CFrameSkip()	{ Clear(); }
	virtual ~CFrameSkip() { }

	//! Initialization function.. 
	void		Clear()  
	{
		SetFramePerSec( 60.0f );  
		m_fTimer = 0.0f;	
	}

	//! Sets the frame rate.
	void		SetFramePerSec( float fFps )
	{
		m_fSecPerFrame = 1.0f/fFps;
		m_fTimer = 0.0f;
	}

	/*! When Update(float dt) is called inside a loop,
	    This function returns true only at the configured fixed frame rate. */
	bool		Update( float fDt )
	{
		m_fTimer += fDt;		
		if( m_fTimer < 0 )
			return false;		

		// Subtract the time corresponding to one frame.
		m_fTimer -= m_fSecPerFrame;
		return true;
	}

	//! Returns false if it should be skipped depending on the case.	
	bool		IsFrameSkip() { return (m_fTimer >= 0); }

protected:
	float		m_fTimer; 
	float		m_fSecPerFrame;
};
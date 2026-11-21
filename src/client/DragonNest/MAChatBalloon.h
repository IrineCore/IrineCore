#pragma once

#include "DnChatBalloonDlg.h"

/*
The old method loads and renders base textures, which has extensibility and various usability issues.
So added the UI format.
However, the reason we keep the existing method is that too much prior work depends on it, making it hard to modify.
Later, speech bubble related work will be handled in UI.
*/
class MAChatBalloon
{
public:
	MAChatBalloon();
	virtual ~MAChatBalloon();

	static int m_siTotalBalloon;
private:

	enum{
		DEFAULT_TIME = 10000
	};

	enum{
		NUM_CHATBALLOON_TEX = 6,
	};
	EtTextureHandle	m_hChatBalloonTexture[NUM_CHATBALLOON_TEX];
	EtTextureHandle	m_hChatBalloonTailTexture[NUM_CHATBALLOON_TEX];

	std::vector< std::wstring > m_wszTalkText;
	DWORD					m_dwTalkStartTime;
	DWORD					m_dwDuringTime;

	float					m_fTalkWidth;
	float					m_fTalkTailPos;
	float					m_fTalkTailPosPrev;
	int						m_nChatType;
	bool					m_bConutBalloon;
	bool                    m_bEnableCustomChatBalloon;

	float					m_fAdditionalYOffset;

	float					m_fTalkWidthForWordBreak;	// Since TalkWidth above is designed in a way that sometimes does not hold the desired value, create a separate variable for proper wordbreak.

	CDnChatBalloonDlg *		m_Dialog;

	std::vector< std::wstring > m_vecCustomChatBalloonText;
	bool m_bForceShow;

	void ProcessCustomChatBalloonText( float fElapsedTime );
	int m_nCustomTextIndex;
	float m_fCustomTextDelayTime;

	bool CheckDistance( EtVector3 vPos, bool bNormalChatBalloon );

public:

	void RenderChatBalloon( EtVector3 vPos, EtVector3 vHeadPos, float fElapsedTime );
	void SetChatBalloonCustomText( const char * wszFileName, LPCWSTR wszMessage, DWORD dwTextColor, DWORD dwStartTime, int nChatType, DWORD dwTalkTime = DEFAULT_TIME ); // Use a custom speech bubble as if a normal user is chatting.
	void SetChatBalloonText( LPCWSTR wszMessage, DWORD dwStartTime, int nChatType, DWORD dwTalkTime = DEFAULT_TIME );
	void SetChatBalloonCustom( const char * wszFileName, LPCWSTR wszMessage, DWORD dwTextColor, DWORD dwStartTime, int nChatType, DWORD dwTalkTime = DEFAULT_TIME );
	
	bool IsEnableCustomChatBallon(){return m_bEnableCustomChatBalloon;}
	bool IsRenderChatBalloon();
	void StopRenderChatBalloon();

	void StopChatBalloonCustom();
	void AddCustomChatBalloonText( LPCWSTR wszMessage );
	void ClearCustomChatBalloonText() { m_vecCustomChatBalloonText.clear(); }
	void ForceShowState( bool bForceShow ) { m_bForceShow = bForceShow; }

	// Cannot adjust in pixels since units are in projection space; intended only for special cases like issue #20162.
	void SetAdditionalYOffset( float fValue ) { m_fAdditionalYOffset = fValue; }

	void IncreaseBalloon()	{ ++m_siTotalBalloon; m_bConutBalloon = true; }
	void DecreaseBalloon()	{ --m_siTotalBalloon; m_bConutBalloon = false; }
	bool IsCountBalloon()	{ return m_bConutBalloon; }
	static void ClearCount()	{ m_siTotalBalloon = 0; }
};

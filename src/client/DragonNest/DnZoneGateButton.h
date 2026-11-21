#pragma once

class CDnZoneGateButton : public CEtUIButton
{
public:
	enum emDungeonType
	{
		DT_RECOMMEND,
		DT_NOTRECOMMEND,
		DT_NOTENTER,
	};
public:
	CDnZoneGateButton( CEtUIDialog *pParent );
	virtual ~CDnZoneGateButton(void);

protected:
	bool m_bPushPin;
	EtVector3 m_vWorldPos;
	
public:
	// Note : Dungeon gate display info
	//
	bool m_bDungeonGate;
	std::wstring m_strGateName;
	std::wstring m_strDungeonName[5];

#ifdef PRE_ADD_QUESTCOUNT_IN_STAGELIST
	// If additional map gate info is needed, add and use it below.
	struct SZoneGateInfo
	{
		emDungeonType m_DungeonType[5];
		int	m_nMapIndex[5];
	};

	SZoneGateInfo m_GateInfo;
#else
	emDungeonType m_DungeonType[5];
#endif
public:
	bool IsPushPin() { return m_bPushPin; }
	void SetPushPin( bool bPin ) { m_bPushPin = bPin; }

	void SetWorldPos( EtVector3 &vPos ) { m_vWorldPos = vPos; }
	EtVector3 *GetWorldPos() { return &m_vWorldPos; }
public:
	virtual bool HandleMouse( UINT uMsg, float fX, float fY, WPARAM wParam, LPARAM lParam );
	virtual void Render( float fElapsedTime );
};

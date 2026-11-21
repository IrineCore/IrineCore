#pragma once
#include "DnCustomDlg.h"
#include "DnWorld.h"
#include "DnBlindDlg.h"
#include "DnDarklairEnterLevelDlg.h"

class CDnDungeonExpectDlg;

class CDnDarklairDungeonEnterDlg : public CDnCustomDlg, public CDnBlindCallBack
{
public:
	CDnDarklairDungeonEnterDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnDarklairDungeonEnterDlg();

public:
	// CDnCustomDlg
	virtual void InitialUpdate();
	virtual void Initialize( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void Show( bool bShow );
	virtual void Process( float fElapsedTime );

	// CDnBlindCallBack
	virtual void OnBlindOpen();
	virtual void OnBlindOpened();
	virtual void OnBlindClose();
	virtual void OnBlindClosed();

	void Refresh( CDnWorld::DungeonGateStruct *pGateStruct );
	int GetFloorIndex() { return m_nIndexFloor; }
	void SetSelectDungeonInfo( int nMapIndex, int nDifficult );

protected:
	void SetDungeonImage( const std::string &DungeonImageName );
	void UpdateControlPos();
	void UpdateButtonState();

#if defined(PRE_ADD_CHALLENGE_DARKLAIR)
	bool IsCloseGateByTime( const int nMapIndex );
#endif	// #if defined(PRE_ADD_CHALLENGE_DARKLAIR)

protected:
	EtTextureHandle				m_hIntroImage;
	EtTextureHandle				m_hIntroMap;
	SUICoord					m_uiTextureImageBack;
	SUICoord					m_uiBottomBlackBack;

	CDnDarklairEnterLevelDlg*	m_pDarklairEnterLevelDlg;
	CDnDungeonExpectDlg*		m_pDungeonExpectDlg;
	CEtUIStatic*				m_pStaticName;
	CEtUIStatic*				m_pStaticBottomBack;
	CEtUITextureControl*		m_pTextureDungeonImage;
	CEtUITextureControl*		m_pTextureDungeonMap;

	struct stDarklairFloorInfo
	{
		CDnStageEnterButton*	m_pButtonFloor;
		int						m_nMapIndex;
		int						m_nMinLevel;
		char					m_cPermitFlag;
		stDarklairFloorInfo() : m_pButtonFloor( NULL ), m_nMapIndex( -1 ), m_nMinLevel( -1 ), m_cPermitFlag( 0 ) {}

		void SetInfo( const wchar_t *wszDungeonName, const wchar_t *wszDungeonDesc, int nMinPartyCount, int nMaxPartyCount, 
						int nMapIndex, int nMapType, int nMinLevel, int nMaxLevel, bool bAchieve, char cPermitFlag )
		{
#if defined(PRE_ADD_CHALLENGE_DARKLAIR)
			m_pButtonFloor->SetInfo( wszDungeonName, wszDungeonDesc, nMinPartyCount, nMaxPartyCount, nMapType, nMinLevel, nMaxLevel, nMapIndex, cPermitFlag );
#else
			m_pButtonFloor->SetInfo( wszDungeonName, wszDungeonDesc, nMinPartyCount, nMaxPartyCount, nMapType, nMinLevel, nMaxLevel );
#endif	// #if defined(PRE_ADD_CHALLENGE_DARKLAIR)
			m_pButtonFloor->SetAchieve( bAchieve );
			m_nMapIndex = nMapIndex;
			m_cPermitFlag = cPermitFlag;
		}

		void SetMinLevel( int nMinLevel ) { m_nMinLevel = nMinLevel; }
		void Show( bool bShow )		{ m_pButtonFloor->Show( bShow ); }
		bool IsShow()				{ return m_pButtonFloor->IsShow(); }
		void Enable( bool bEnable )	{ m_pButtonFloor->Enable( bEnable ); }
		bool IsEnable()				{ return m_pButtonFloor->IsEnable(); }
	};

	enum { MAX_DARKLAIR_FLOOR = 5 };
	std::vector<stDarklairFloorInfo>	m_vecDarklairFloorInfo;
	int									m_nIndexFloor;
};

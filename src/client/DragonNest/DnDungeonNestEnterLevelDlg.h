#pragma once
#include "DnCustomDlg.h"
#include "DnDungeonEnterDlg.h"


class CDnItem;

class CDnDungeonNestEnterLevelDlg : public CDnCustomDlg
{
public:
	CDnDungeonNestEnterLevelDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnDungeonNestEnterLevelDlg(void);

protected:
	CEtUITextureControl *m_pTextureDungeonArea;
	EtTextureHandle m_hIntroArea;

	CEtUIStatic *m_pStaticTimer;
	CEtUIButton *m_pButtonEnter;
	CEtUIButton *m_pButtonCancel;

	CEtUIStatic *m_pSelectName;
	CEtUIStatic *m_pRecommLevel;
	CEtUIStatic *m_pRecommCount;
	CEtUIStatic *m_pDungeonFatigueDegree;

	std::vector<int> m_vecRecommandLevel;
	std::vector<int> m_vecRecommandPartyCount;

	CDnItem *m_pNeedItem;
	CDnItemSlotButton *m_pItemSlotButton;
	CEtUIStatic *m_pNeedItemStatic;
	CEtUIStatic *m_pNeedItemCount;
	bool m_bEnoughNeedItem;			// Client-side determination flag. Checks only items needed by self.
	char m_cDungeonEnterPermit;		// Whether server validates. Checks required items for all party members.
	int m_nMaxUsableCoinCount;
	std::vector<DnActorHandle> m_hVecPermitActorList;

	float m_fElapsedTime;
	int m_nCount;
	int m_nSelectedLevelIndex;
	int m_nMapIndex;

#ifdef PRE_FIX_DUNGEONENTER_CLOSE_BY_QUEST
	bool m_bCloseProcessingByUser;
#endif

protected:
	void SetTime( int nTime );
	void SetControlState();

public:
	void SetImage( int nMapIndex );
	virtual void SetSelectDungeonInfo( int nMapIndex, int nDifficult );
	virtual void SetDungeonName( const wchar_t *wszDungeonName );
	virtual void SetDungeonDesc( const wchar_t *wszStr ) {}
	virtual void UpdateRecommandLevel( std::vector<int> &vecRecommandLevel );
	virtual void UpdateRecommandPartyCount( std::vector<int> &vecRecommandPartyCount );
	virtual void UpdateNeedItem( int nItemID, int nCount, char cPermitFlag, std::vector<DnActorHandle> *pVecList );
	virtual void UpdateDungeonInfo( CDnDungeonEnterDlg::SDungeonGateInfo *pInfo );
	virtual int GetDungeonLevel() { return m_nSelectedLevelIndex; }
#ifdef PRE_FIX_DUNGEONENTER_CLOSE_BY_QUEST
	bool IsCloseProcessingByUser() const;
#endif

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Process( float fElapsedTime );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void Show( bool bShow );
	void SetFatigueDegree( int nMapIndex, int nDifficult );
};

#pragma once

#include "DnCustomDlg.h"
#include "DnBlindDlg.h"


#define MAX_PARTY_MEMEBER		4		// Max 4 party members
#define MAX_JOB					4		// Number of displayed classes: 4 (Warrior, Sorceress, Cleric, Archer)

struct stStaticRankInfo
{
	CEtUIStatic* m_pStaticTitle;
	CEtUIStatic* m_pStaticRank;
	CEtUIStatic* m_pStaticRound;
	CEtUIStatic* m_pStaticTime;
	CDnJobIconStatic* m_pJobIcon[MAX_PARTY_MEMEBER];
	CEtUIStatic* m_pStaticMember[MAX_PARTY_MEMEBER];
#if defined( PRE_ADD_CHALLENGE_DARKLAIR )
	CDnItemSlotButton * m_pItemSlotButton;
#endif	// #if defined( PRE_ADD_CHALLENGE_DARKLAIR )

	stStaticRankInfo() : m_pStaticTitle( NULL ), m_pStaticRound( NULL ), m_pStaticRank( NULL ), m_pStaticTime( NULL )
	{
		memset( m_pJobIcon, 0, sizeof(m_pJobIcon) );
		memset( m_pStaticMember, 0, sizeof(m_pStaticMember) );

#if defined( PRE_ADD_CHALLENGE_DARKLAIR )
		m_pItemSlotButton = NULL;
#endif	// #if defined( PRE_ADD_CHALLENGE_DARKLAIR )
	}

	~stStaticRankInfo()
	{
#if defined( PRE_ADD_CHALLENGE_DARKLAIR )
		if( NULL != m_pItemSlotButton )
		{
			MIInventoryItem * pItem = m_pItemSlotButton->GetItem();
			SAFE_DELETE( pItem );
		}
#endif	// #if defined( PRE_ADD_CHALLENGE_DARKLAIR )
	}

	void Reset()
	{
		m_pStaticTitle->Show( false );
		m_pStaticRound->Show( false );
		m_pStaticRank->Show( false );
		m_pStaticTime->Show( false );

		for( int j=0; j<MAX_PARTY_MEMEBER; j++ )
		{
			m_pStaticMember[j]->Show( false );
			m_pJobIcon[j]->SetIconID( -1 );
		}

#if defined( PRE_ADD_CHALLENGE_DARKLAIR )
		MIInventoryItem * pItem = m_pItemSlotButton->GetItem();
		SAFE_DELETE( pItem );
		m_pItemSlotButton->Show( false );
#endif	// #if defined( PRE_ADD_CHALLENGE_DARKLAIR )
	}
};

class CDnDarkLairRankBoardDlg : public CDnCustomDlg, public CDnBlindCallBack
{
public:
	CDnDarkLairRankBoardDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL  );
	virtual ~CDnDarkLairRankBoardDlg();

protected:
	bool m_bRankInfoReqeust;

	CEtUIListBoxEx* m_pDarkLairRankListBox;

	CEtUIStatic* m_pStaticPlayerInfo;		// Character info
	CEtUIStatic* m_pStaticHistoryTitle;		// Rank board info
	stStaticRankInfo m_StaticBestInfo;		// My best record
	stStaticRankInfo m_StaticTop3Info[3];	// Rank info up to 3rd place

#if defined( PRE_ADD_CHALLENGE_DARKLAIR )
	enum{
		E_RANK_REWARD_ITEM_COUNT = 10,
	};

	int m_nRankRewardItem[E_RANK_REWARD_ITEM_COUNT];
	bool m_bRankReward;
#endif	// #if defined( PRE_ADD_CHALLENGE_DARKLAIR )

public:
	virtual void InitialUpdate();
	virtual void Initialize( bool bShow );
	virtual void Show( bool bShow );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

public:
	void SetRankInfoRequest( bool bRequest ) { m_bRankInfoReqeust = bRequest; }
	bool IsRankInfoRequest() { return m_bRankInfoReqeust; }
	void SetDarkLairRankInfo( TDLRankHistoryPartyInfo *pBestInfo, TDLRankHistoryPartyInfo *pHistoryInfo );
#if defined(PRE_ADD_CHALLENGE_DARKLAIR)
	void SetDarkLairRankMapIndex( int nMapIndex );
#endif	// #if defined(PRE_ADD_CHALLENGE_DARKLAIR)

private:
	void ResetStatic();	// Initialize static controls (party member and job labels hidden by default, clear list)
};

class CDnDarkLairRankBoardListItemDlg : public CDnCustomDlg
{
public:
	CDnDarkLairRankBoardListItemDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnDarkLairRankBoardListItemDlg();

protected:
	stStaticRankInfo m_StaticRankInfo;
#if defined(PRE_ADD_CHALLENGE_DARKLAIR)
	int m_nRankRewardItem;
#endif	// #if defined(PRE_ADD_CHALLENGE_DARKLAIR)

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();

	void SetDarkLairRankInfo( TDLRankHistoryPartyInfo *pHistoryInfo );
#if defined(PRE_ADD_CHALLENGE_DARKLAIR)
	void SetDarLairRankRewardItem( const int nRankRewardItem ) { m_nRankRewardItem = nRankRewardItem; }
#endif	// #if defined(PRE_ADD_CHALLENGE_DARKLAIR)

private:
	void ResetStatic();	// Initialize static controls (party member and job labels hidden by default)
};

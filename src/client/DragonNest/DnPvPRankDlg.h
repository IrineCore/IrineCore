#pragma once

#ifdef PRE_ADD_PVP_RANKING
#include "DnCustomDlg.h"

class CDnPvPRankDlg : public CDnCustomDlg
{
private:
	enum ERankType
	{
		ERankType_None = -1,
		ERankType_Colosseum = 0,
		ERankType_Ladder,
#if defined( PRE_ADD_DWC )
		ERankType_DWC,
#endif
	};
	ERankType m_RankType;

	// Controls //
	CEtUIListBoxEx * m_pListRank;
	CEtUITreeCtl * m_pTreeJob;
	CEtUIComboBox * m_pComboCondition;
	CEtUIButton * m_pBtnSearch;
	CEtUIRadioButton * m_pBtnColosseum;
	CEtUIRadioButton * m_pBtnLadder;
#if defined( PRE_ADD_DWC )
	CEtUIRadioButton * m_pBtnDWC;
	CEtUIStatic * m_pStaticDWCTab[3];
	CEtUIStatic * m_pTextDWCTab[3];
#endif
	CEtUIIMEEditBox * m_pIMEEditBox;
#ifdef PRE_ADD_PVPRANK_INFORM
	CEtUIButton* m_pBtnRankInform;
#endif

	CEtUIButton * m_pStaticBack;  // <
	CEtUIButton * m_pStaticNext;  // >
	CEtUIButton * m_pStaticFirst; // <<
	CEtUIButton * m_pStaticLast;  // >>

	CEtUIStatic * m_pStaticMyPoint;  // Rating.
	CEtUIStatic * m_pStaticMyResult; // Result.


	CEtUIStatic * m_pStaticTotRank;		// Overall rank.
	CEtUIStatic * m_pStaticFirstRank;	// 1st job class ranking.
	CEtUIStatic * m_pStaticSecondRank;	// 2nd job rank.
	CEtUIStatic * m_pStaticGuild;		// Guild name.
	CEtUIStatic * m_pStaticName;		// Character name.
	CEtUIStatic * m_pStaticPoint;		// Points.
	CEtUIStatic * m_pStaticResult;		// Result.
	CEtUIStatic * m_pStaticSubTitle;
	CEtUITextureControl * m_pTextureControlGuild; // Guild icon.
	CEtUITextureControl * m_pTextureControl; // Colosseum rank icon.

	CDnJobIconStatic * m_pJobIcon; // Job icon. 

	struct SPvPTypeControl
	{
		CEtUIStatic * pResultTab;
		CEtUIStatic * pPointTab;
		CEtUIStatic * pResultText;
		CEtUIStatic * pPointText;
		SPvPTypeControl() : pResultTab( NULL ), pPointTab( NULL ), pResultText( NULL ), pPointText( NULL ) {}
	};
#define PVPTYPESIZE (ERankType::ERankType_Ladder + 1)
	SPvPTypeControl m_arrControls[ PVPTYPESIZE ];	
	
	std::map< int, CTreeItem * > m_mapParentJob; // Major category TreeItem.
	std::map< int, int > m_mapParentJobBySub;    // Subclass job type.
	
	std::vector< CEtUIStatic * > m_vecPages;

	enum EComboCondition
	{
		EComboCondition_NONE = 0,
		EComboCondition_Character = 1,
		EComboCondition_Guild,
	};

	int m_CrrPage;	   // Currently displayed page.
	int m_RequestPage; // Request Page - in units of RANKINGMAX.
	int m_totScrollPage; // Maximum number of pages skippable with the '>>' button.

	int m_Job;
	int m_SubClass;
	wchar_t * m_StrSearch;

	// Searched list info.
	struct SCPvPRankList * m_pListColosseum;
	struct SCPvPLadderRankList * m_pListLadder;

	int m_crrMouseOverPage;
	int m_crrSelectPage; // Currently selected page number 1~5.

	bool m_bScroll; // Scroll mode - (<,>) buttons: false, (<<,>>) buttons: true.

	LadderSystem::MatchType::eCode m_eCurrentLadderType;

public:
	CDnPvPRankDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL, bool bAutoCursor = false );
	virtual ~CDnPvPRankDlg(){
		ReleaseDlg();
	}
	void ReleaseDlg();

	// Override - CEtUIDialog //
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl * pControl, UINT uMsg = 0 );		
	virtual void Show( bool bShow );


private:
	// Return parent job of job.
	int GetParentJob( int job );

	// Build TreeControl per job.
	void BuildTreeJob();

	// Job selection.
	void SelectionTreeJob();

	// Request my info.
	void RequestMyInfo( ERankType rank , LadderSystem::MatchType::eCode eMatchType = LadderSystem::MatchType::None );
	

	void PageChange(); // Change by 1 page & set user list.
	void PageScroll( int n ); // Change RANKINGPAGECOUNT unit.
	
	void RefreshPageControls();

	void Reset(); // Initialize my info.

	void ResetList(); // Reset list.
	
	void ResetPvPList();


	std::wstring GetSubString( ERankType type );

	// Guild mark.
	void SetGuildIcon();

	// Set ranking per job.
	void SetJobRank( int job, INT64 classRank, INT64 subClassRank );


public:

	// My info.
	void SetInfoMyRankColosseum( struct TPvPRankingDetail * pInfo );    // Colosseum.
	void SetInfoMyRankLadder( struct TPvPLadderRankingDetail * pInfo ); // Ladder.

	// Character info.
	void SetInfoColosseum( struct TPvPRankingDetail * pInfo ); // Colosseum.
	void SetInfoLadder( struct TPvPLadderRankingDetail * pInfo ); // Ladder.
	
	// List.
	void SetListColosseum( struct SCPvPRankList * pInfo ); // Colosseum.
	void SetListLadder( SCPvPLadderRankList * pInfo ); // Ladder.
#if defined( PRE_ADD_DWC )
	void RefreshDWCRankList();
#endif

};

#endif // PRE_ADD_PVP_RANKING
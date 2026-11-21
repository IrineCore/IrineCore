#pragma once

#if defined( PRE_ADD_DWC )

#include "DnCustomDlg.h"
#include "DNPacket.h"

class CDnJobIconStatic;

class CDnPVPDWCTabDlg : public CEtUIDialog
{
public:
	enum eTabType {
		TAB_COLOSSEUM = 0,
		TAB_DWC = 1,
		TAB_DWC_EX = 2,
		TAB_MAX = 3,
	};

public:
	CDnPVPDWCTabDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnPVPDWCTabDlg();

protected:
	CEtUIRadioButton *m_pTabButtons[TAB_MAX];
	int m_nSelectedTab;

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );

	int GetSelectedTab() const { return m_nSelectedTab; }
	void SelectTab( int nTab );
};

class CDnPvPRankListItemDwcDlg : public CDnCustomDlg
{
public:
	CDnPvPRankListItemDwcDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnPvPRankListItemDwcDlg();

protected:
	CEtUIStatic *m_pStaticRank;
	CEtUIStatic *m_pStaticTeamName;
	CEtUIStatic *m_pStaticTeamCaptain;
	CEtUIStatic *m_pStaticResult;
	CEtUIStatic *m_pStaticCount;
	CDnJobIconStatic *m_pStaticClass;

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();

	void SetRankInfo( const TDWCRankInfo &info );
};

#endif // PRE_ADD_DWC

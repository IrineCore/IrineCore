#pragma once

#include "DnCustomDlg.h"
#include "DnBlindDlg.h"

class CDnGuildWarPreliminaryStatusDlg : public CDnCustomDlg, public CDnBlindCallBack
{
public:
	CDnGuildWarPreliminaryStatusDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL  );
	virtual ~CDnGuildWarPreliminaryStatusDlg();

public:
	virtual void InitialUpdate();
	virtual void Initialize( bool bShow );
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg );

public:
	void SetPresentCondition( int nBlueTeamSocre, int nRedTeamScore, int nMySocre, int nMyGuildScore );

protected:
	CEtUIStatic*	m_pStaticInterimResult[3];	// Interim result (0: Blue team leads, 1: Red team leads, 2: Tie)
	CEtUIStatic*	m_pStaticBlueTeamScore;		// Blue team score
	CEtUIStatic*	m_pStaticRedTeamScore;		// Red team score
	CEtUIStatic*	m_pStaticMyScore;			// My score
	CEtUIStatic*	m_pStaticMyGuildScore;		// My guild score
};

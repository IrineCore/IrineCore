#pragma once
#include "DnCustomDlg.h"
#include "DnJobIconStatic.h"


class CDnPVPInfoDlg : public CDnCustomDlg
{
	enum eInfoValue
	{
		PVPMedalTableID = 374341682, // If medal types change later we should move them to a table, but since the planner says there will be no future changes, use them directly without a table.
		PVPWinUIString = 126039,
	};
public:
	CDnPVPInfoDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnPVPInfoDlg(void);

protected:

	CDnJobIconStatic *pUIJobIcon; // Job name 
	CEtUIStatic *pUIGuildName; // Guild name
	CEtUIStatic *pUILevel;     // Level
	CEtUIStatic *pUIName;      // ID
	CEtUITextureControl *pUIRankIcon;  // Rank icon 
	CEtUIStatic *pUIRankName;  // Rank name
	CEtUIStatic *pUIXP;        // Current Colosseum XP
	CEtUIStatic *pUINeedXP;    // XP required for next level
	CEtUIStatic *pUIScore;
	CEtUIStatic *pUIMedal;

	
public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void Process( float fElapsedTime );

	void SetPVPInfo();

};

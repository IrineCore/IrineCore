#pragma once

#include "DnCustomDlg.h"
#include "DnMissionTask.h"

class CDnItem;
class CDnMissionAppellationTooltipDlg;
class CDnMissionRewardCoinTooltipDlg;
class CDnMissionDailyQuestDlg;

class CDnNestInfoListItem : public CDnCustomDlg
{
public:
	CDnNestInfoListItem( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnNestInfoListItem(void);

protected:
	CEtUIStatic *m_pNestName;			//Nest name
	CEtUIStatic *m_pNestClearInfo;		//Whether Nest is cleared
	CEtUIStatic *m_pNestEventIcon;		//Nest event status icon
	CEtUIStatic *m_pTooltipControl;		//Added dummy for tooltip display
	
	TCustomEventInfo m_NestInfo;		//Nest info

public:
	void SetInfo( TCustomEventInfo &pInfo, int nActivate, int uiStringID );
	

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void Process( float fElapsedTime );

	virtual void SetElementDialogShowState( bool bShow );
};

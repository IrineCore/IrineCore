#pragma once

#include "DnCustomDlg.h"

class CDnStageShortCutRepairDlg : public CDnCustomDlg
{
public:
	enum {
		COST_LINE_MAX = 2,
	};

public:
	CDnStageShortCutRepairDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnStageShortCutRepairDlg();

protected:
	CEtUIStatic *m_pStaticGold[COST_LINE_MAX];
	CEtUIStatic *m_pStaticSilver[COST_LINE_MAX];
	CEtUIStatic *m_pStaticBronze[COST_LINE_MAX];
	CEtUIStatic *m_pIconRepute;

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );

	void SetRepairCost( INT64 nEquipCost, INT64 nAllCost );
};

#pragma once

#ifdef PRE_ADD_CHAOSCUBE

#include "DnCustomDlg.h"
#include "DnSmartMoveCursor.h"

class CDnItem;

class CDnChaosGaugeCubeStuffConfirmDlg : public CDnCustomDlg
{
public:
	CDnChaosGaugeCubeStuffConfirmDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnChaosGaugeCubeStuffConfirmDlg();

protected:
	CEtUIStatic *m_pStaticName;
	CEtUIStatic *m_pStaticCount;
	CEtUIStatic *m_pStaticGauge;
	CDnSmartMoveCursor m_SmartMove;

	char m_cCubeInvenType;
	short m_sCubeSlotIndex;
	INT64 m_biCubeSerial;

	char m_cStuffInvenType;
	short m_sStuffSlotIndex;
	INT64 m_biStuffSerial;
	int m_nStuffCount;

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );

	void SetStuffConfirmInfo(
		char cCubeInvenType, short sCubeSlotIndex, INT64 biCubeSerial,
		CDnItem *pStuffItem, int nCount, int nGaugePerItem );
};

#endif // PRE_ADD_CHAOSCUBE

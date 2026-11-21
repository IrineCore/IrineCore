#pragma once
#include "EtUIDialog.h"

class CDnBossAlertDlg : public CEtUIDialog
{
public:
	CDnBossAlertDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnBossAlertDlg();

protected:
	CEtUIStatic *m_pStaticName;
	CEtUIStatic *m_pStaticBar;
	CEtUIStatic *m_pStaticBoard;
	CEtUITextureControl *m_pTexture;

	SUICoord m_CoordName;
	SUICoord m_CoordBar;
	SUICoord m_CoordTexture;
	SUICoord m_CoordBoard;

	SUICoord m_CoordDlg;

	EtTextureHandle m_hTexture;
	float m_fDelta;
	float m_fMaxDelta;

protected:
	float GetWeightValue( float fStartDelta, float fEndDelta, float fCurDelta );

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Process( float fElapsedTime );
	virtual void Show( bool bShow );

	void SetBoss( WCHAR *wszName, const char *szImageFileName );
};

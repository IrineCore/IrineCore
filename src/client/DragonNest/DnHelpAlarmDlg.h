#pragma once
#include "DnCustomDlg.h"
#include "DnMissionTask.h"

class CDnItem;

class CDnHelpAlarmDlg : public CDnCustomDlg
{
public:
	CDnHelpAlarmDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL  );
	virtual ~CDnHelpAlarmDlg(void);

protected:
	CEtUIStatic *m_pStaticDesc;
	CDnItemSlotButton *m_pSlotItem;
	CEtUITextureControl *m_pTextureNpc;
	CDnItem *m_pItem;
	EtTextureHandle m_hNpcFaceTexture;

	float m_fShowTime;
	float m_fShowRatio;
	float m_fElapsedTime;
	float m_fConst;

protected:
	void SetFadeTime( float fFadeTime );

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void Process( float fElapsedTime );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

	void SetAlarm( int nHelpTableID );
};

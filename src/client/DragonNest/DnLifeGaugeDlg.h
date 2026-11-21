#pragma once

#include "DnCustomDlg.h"


class CDnWorldOperationProp;

class CDnLifeGaugeDlg : public CEtUIDialog
{
public:
	CDnLifeGaugeDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_TOP, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnLifeGaugeDlg();

private :

	CEtUIProgressBar * m_pTimeBar;
	CEtUIProgressBar * m_pWaterBar;
	CEtUIStatic * m_pStaticName;
	CEtUIStatic * m_pStaticGuildName;
	CEtUITextureControl * m_pMarkTexture;

	CDnWorldOperationProp * m_pProp;

	int m_nItemID;
	int m_eGrowLevel;

	bool m_bGuild;

public :
	void SetTimeBar( float fProgress );
	void SetWaterBar( float fProgress );
	void SetProp( CDnWorldOperationProp * pProp )	{ m_pProp = pProp; }

	float GetTimeBar()	{ return m_pTimeBar->GetProgress(); }
	float GetWaterBar()	{ return m_pWaterBar->GetProgress(); }

	void UpdateGauge( int nItemID, int eGrowLevel );
	void SetGuildInfo( bool bGuild, const TGuildView &GuildView, WCHAR * wszName );

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void Render( float fElapsedTime );
	//virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	//virtual void Process( float fElapsedTime );
	//virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0 );
	//virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
};

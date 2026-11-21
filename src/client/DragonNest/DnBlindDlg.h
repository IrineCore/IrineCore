#pragma once
#include "EtUIDialog.h"
#include "DnInterface.h"

class CDnBlindCallBack
{
public:
	CDnBlindCallBack() {}
	virtual ~CDnBlindCallBack();

public:
	virtual void OnBlindOpen() {}
	virtual void OnBlindOpened() {}
	virtual void OnBlindClose() {}
	virtual void OnBlindClosed() {}
};

class CDnBlindCaptionDlg;

class CDnBlindDlg : public CEtUIDialog
{
public:
	enum emBLIND_MODE
	{
		modeOpen,
		modeOpened,
		modeClose,
		modeClosed,
		BLIND_HEIGHT = 100,
	};

public:
	CDnBlindDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnBlindDlg(void);

protected:
	SUICoord m_UpBlind;
	SUICoord m_DownBlind;
	SUICoord m_BillBoard;

	DWORD m_BillBoardColor;
	EtColor m_BlindColor;

	emBLIND_MODE m_BlindMode;

	EtVector2 Vector1;
	EtVector2 Vector2;

	EtColor Color1;
	EtColor Color2;

	//CDnBlindCallBack *m_pBlindCallBack;

	std::vector<CDnBlindCallBack*>	m_BlindCallBackList;

	CDnBlindCaptionDlg *m_pBlindCaptionDlg;
	float m_fCaptionDelta;

public:
	void UpdateBlind( int nBlindHeight, EtColor &blindColor );
	emBLIND_MODE GetBlindMode() { return m_BlindMode; }
	void SetBlindMode( emBLIND_MODE emBlindMode ) { m_BlindMode = emBlindMode; }
	void SetBillBoardColor( DWORD dwColor ) { m_BillBoardColor = dwColor; }

	void AddBlindCallBack( CDnBlindCallBack *pCallBack ) 
	{
		if( pCallBack == NULL ) return;
		std::vector<CDnBlindCallBack*>::iterator it = std::find( m_BlindCallBackList.begin(), m_BlindCallBackList.end(), pCallBack);
		if ( it == m_BlindCallBackList.end() )
		{
			m_BlindCallBackList.push_back(pCallBack); 
		}
	}

	void RemoveBlindCallback( CDnBlindCallBack *pCallback )
	{
		std::vector<CDnBlindCallBack*>::iterator it = std::find( m_BlindCallBackList.begin(), m_BlindCallBackList.end(), pCallback);
		if( it != m_BlindCallBackList.end() ) {
			m_BlindCallBackList.erase( it );
		}
	}

	void ClearBlindCallBack()
	{
		m_BlindCallBackList.clear();
	}

	void SetCaption( const wchar_t *wszCaption, int nDelay = 0 );
	void ClearCaption();

	void CallBackClosed(); // #54501 Party members are invisible after a cutscene plays during a quest.

public:
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void Render( float fElapsedTime );
	virtual void Process( float fElapsedTime );

	virtual void OnChangeResolution();
};
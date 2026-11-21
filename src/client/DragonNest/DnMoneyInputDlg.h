#pragma once
#include "EtUIDialog.h"

class CDnMoneyControl
{
public:
	CDnMoneyControl( CEtUIDialog *pDialog );
	virtual ~CDnMoneyControl(){}

	enum {
		GOLD_TYPE,
		PETAL_TYPE
	};

protected:

	CEtUIDialog	*m_pDialog;

	CEtUIStatic *m_pCoinG;
	CEtUIStatic *m_pCoinS;
	CEtUIStatic *m_pCoinC;
	CEtUIStatic *m_pCoinP;

	CEtUIStatic *m_pCoinGOutline;
	CEtUIStatic *m_pCoinSOutline;
	CEtUIStatic *m_pCoinCOutline;
	CEtUIStatic *m_pCoinPOutline;

	CEtUIStatic *m_pImageG;
	CEtUIStatic *m_pImageS;
	CEtUIStatic *m_pImageC;
	CEtUIStatic *m_pImageP;

	CEtUIEditBox	*m_pEditBoxMoneyG;
	CEtUIEditBox	*m_pEditBoxMoneyS;
	CEtUIEditBox	*m_pEditBoxMoneyC;
	CEtUIEditBox	*m_pEditBoxMoneyP;

	CEtUIButton		*m_pButtonG;
	CEtUIButton		*m_pButtonS;
	CEtUIButton		*m_pButtonC;
	CEtUIButton		*m_pButtonP;

	bool m_bEnablePetal;

	wchar_t m_wszMoney[16];
	INT64 m_nMoney;
	INT64 m_nMaxMoney;
	INT64 m_nPetal;
	INT64 m_nMaxPetal;
	int m_eMoneyType;

	// Remember string length before this input; check for transition from 2 digits to 1 digit.
	// Currently unused.
	int m_nPrevLenMoneyG;
	int m_nPrevLenMoneyS;
	int m_nPrevLenMoneyC;		
	int m_nPrevLenMoneyP;
public:

	void InitialUpdate( bool bEnablePetal = false );
	bool ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg );
	void Show( bool bShow );
	bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

	void SetMax();
	void Init();

	template< int _Size> static void GetStrMoneyG( INT64 Money , OUT wchar_t (&pOutString)[_Size] , OUT INT64 *pMoney = NULL ) {		
		StaticAssert( _Size >= 16 );
		INT64 nMoney = Money / 10000;
		if( pMoney ) *pMoney = nMoney;
		swprintf_s(pOutString, L"%I64d", nMoney);	
	}
	template< int _Size> static void GetStrMoneyS( INT64 Money , OUT wchar_t (&pOutString)[_Size] , OUT INT64 *pMoney = NULL) {
		StaticAssert( _Size >= 16 );		
		INT64 nMoney = (Money / 100) % 100;
		if( pMoney ) *pMoney = nMoney;
		// If the number has only one digit (ones place) and gold has a value		
		if( nMoney < 10 && Money >= 10000 )
			swprintf_s(pOutString, L"0%I64d", nMoney);
		else
			swprintf_s(pOutString, L"%I64d", nMoney);
	}
	template< int _Size> static void GetStrMoneyC( INT64 Money , OUT wchar_t (&pOutString)[_Size] , OUT INT64 *pMoney = NULL) {
		StaticAssert( _Size >= 16 );
		INT64 nMoney = Money % 100;
		if( pMoney ) *pMoney = nMoney;
		// If the number has only one digit (ones place) and gold has a value		
		if( nMoney < 10 && Money >= 100 )
			swprintf_s(pOutString, L"0%I64d", nMoney);
		else
			swprintf_s(pOutString, L"%I64d", nMoney);
	}
	template< int _Size> static void GetStrMoneyP( INT64 Money , OUT wchar_t (&pOutString)[_Size] , OUT INT64 *pMoney = NULL) {
		StaticAssert( _Size >= 16 );
		if( pMoney ) *pMoney = Money;
		swprintf_s(pOutString, L"%I64d", Money);
	}

	// Get money display string.
	wchar_t *GetStrMoneyG() { GetStrMoneyG( m_nMoney, m_wszMoney); return m_wszMoney;} 
	wchar_t *GetStrMoneyS() { GetStrMoneyS( m_nMoney, m_wszMoney); return m_wszMoney;} 
	wchar_t *GetStrMoneyC() { GetStrMoneyC( m_nMoney, m_wszMoney); return m_wszMoney;}
	wchar_t *GetStrMoneyP() { GetStrMoneyP( m_nPetal, m_wszMoney); return m_wszMoney;}

	INT64 GetMoney() { return m_nMoney; }
	INT64 GetPetal() { return m_nPetal; }
	// Max value setting is only needed when the inventory item is not currency.
	// -1 uses inventory money amount.
	void SetMaxMoney( INT64 nMoney ) { m_nMaxMoney = nMoney; }
	void SetMaxPetal( INT64 nPetal ) { m_nMaxPetal = nPetal; }

	// Min/max clamping. Returns true if clamped.
	bool UpdateMoney();	
	bool UpdatePetal();

	bool IsEditBoxFocus();

	void SetMoney( INT64 nMoney );
	void SetMoneyType( int eMoneyType );
	int  GetMoneyType()	{ return m_eMoneyType; }
	void EnableFee( bool bEnable );
};

class CDnMoneyInputDlg : public CEtUIDialog//, public CDnMoneyControl
{
public:
	CDnMoneyInputDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL  );
	virtual ~CDnMoneyInputDlg(void);

protected:
	CEtUIStatic *m_pStaticTitle;	
	CEtUIButton *m_pButtonOK;
	CEtUIButton *m_pButtonCancel;	

	CEtUIButton *m_pButtonMax;
	CEtUIButton *m_pButtonInit;

	CDnMoneyControl	m_MoneyControl;

	
	

public:
	INT64 GetMoney() { return m_MoneyControl.GetMoney(); }

	// Call only when custom settings are needed. By default, strings are set for personal trade.
	void SetInfo( LPCWSTR wszTitle, LPCWSTR wszOK );

	// Max value setting is only needed when the inventory item is not currency.
	// -1 uses inventory money amount.
	void SetMaxMoney( INT64 nMoney = -1 ) { m_MoneyControl.SetMaxMoney( nMoney ); }

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
};

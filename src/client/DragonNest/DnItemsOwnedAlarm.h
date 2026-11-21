#pragma once

#include "DnCustomDlg.h"

class CDnAlarmMarkButton : public CEtUIButton
{
public:
	CDnAlarmMarkButton( CEtUIDialog *pParent );
	virtual ~CDnAlarmMarkButton();

protected:
	bool m_bBlink;
	float m_fBlinkTime;

public:
	virtual void Initialize( SUIControlProperty *pProperty );
	virtual void Process( float fElapsedTime );

	void SetBlink( bool bBlink ) { m_bBlink = bBlink; }
	bool IsBlink() const { return m_bBlink; }
};

class CDnItemsOwnedAlarm : public CDnCustomDlg
{
public:
	CDnItemsOwnedAlarm( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnItemsOwnedAlarm();

protected:
	CEtUIStatic *m_pStatic0;
	CEtUITextBox *m_pTextBoxTitle;
	CEtUITextBox *m_pTextBoxSubTitle;
	float m_fAutoHideTime;

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Process( float fElapsedTime );

	void ShowAlarm( const WCHAR *wszTitle, const WCHAR *wszSubTitle, float fDuration = 5.0f );
};

class CDnItemsOwnedAlarmMng
{
public:
	CDnItemsOwnedAlarmMng();
	virtual ~CDnItemsOwnedAlarmMng();

protected:
	CDnItemsOwnedAlarm *m_pAlarmDlg;

public:
	virtual void Initialize();
	virtual void Finalize();
	void CheckOwnedItemNotice( int nItemID );
	CDnItemsOwnedAlarm *GetAlarmDlg() const { return m_pAlarmDlg; }
};

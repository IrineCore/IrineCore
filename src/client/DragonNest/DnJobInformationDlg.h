#pragma once

#include "DnCustomDlg.h"

class CDnJobIconStatic;

class CDnJobInformationMessageBox : public CDnCustomDlg
{
public:
	CDnJobInformationMessageBox( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnJobInformationMessageBox();

protected:
	CDnJobIconStatic *m_pStaticClass;
	CEtUIStatic *m_pStaticAsk;

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );

	void SetJobMessage( int nJobID, const WCHAR *wszMessage );
};

class CDnJobInformationPopupDlg : public CDnCustomDlg
{
public:
	CDnJobInformationPopupDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnJobInformationPopupDlg();

protected:
	CEtUIProgressBar *m_pProgressExp;
	CEtUIStatic *m_pStaticLv;
	CEtUIStatic *m_pStaticNotice;

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );

	void SetPopupInfo( int nLevel, float fExpPercent, const WCHAR *wszNotice );
};

class CDnJobInformationMainDlg : public CDnCustomDlg
{
public:
	enum {
		MAX_JOB_RADIO = 6,
	};

public:
	CDnJobInformationMainDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnJobInformationMainDlg();

protected:
	CEtUIButton *m_pBtnAddClass;
	CEtUIButton *m_pBtnSkill;
	CDnJobIconStatic *m_pStaticClassIcon;
	CEtUITextureControl *m_pTextureBg;
	CEtUITextureControl *m_pTextureClass;
	CEtUIStatic *m_pStaticClassName;
	CEtUIStatic *m_pStaticInfo;
	CEtUIStatic *m_pStaticSkillInfo;
	CEtUIRadioButton *m_pRadioJobs[MAX_JOB_RADIO];

	CDnJobInformationMessageBox *m_pMessageBox;
	CDnJobInformationPopupDlg *m_pPopupDlg;

	int m_nSelectedJobID;

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );

	void SetJobInfo( int nJobID );
	CDnJobInformationMessageBox *GetMessageBox() const { return m_pMessageBox; }
	CDnJobInformationPopupDlg *GetPopupDlg() const { return m_pPopupDlg; }
};

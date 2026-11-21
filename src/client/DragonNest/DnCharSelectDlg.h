#pragma once
#include "EtUIDialog.h"

#ifdef PRE_MOD_SELECT_CHAR

class CDnCharSelectListDlg;
class CDnCharSelectCloseDlg;
class CDnCharLevelLimitInfoDlg;


class CDnCharSelectDlg : public CEtUIDialog, public CEtUICallback
{
public:
	CDnCharSelectDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL  );
	virtual ~CDnCharSelectDlg(void);

protected:
	CDnCharSelectListDlg*		m_pCharSelectListDlg;
	CDnCharSelectCloseDlg*		m_pCharSelectCloseDlg;
	CDnCharLevelLimitInfoDlg*	m_pCharLevelLimitInfoDlg;

	float						m_fCharSelectDescDelta;
	CEtUIStatic*				m_pStaticCharSelectDesc;

protected:

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void Process( float fElapsedTime );
	virtual void Show( bool bShow );
	virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg );

public:
	void SetCharacterList( std::vector<TCharListData>& vecCharListData, int nMaxCharCount );
	void SelectCharIndex( int nSelectIndex );
#if defined(PRE_ADD_23829)
	void Show2ndPassCreateButton(bool bShow);
#endif // PRE_ADD_23829

#ifdef PRE_MOD_SELECT_CHAR
	CDnCharSelectListDlg* GetCharSelectListDlg(){ return m_pCharSelectListDlg; }	
#endif // PRE_MOD_SELECT_CHAR
};


class CDnCharSelectCloseDlg : public CEtUIDialog
{
public:
	CDnCharSelectCloseDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL  );
	virtual ~CDnCharSelectCloseDlg();

public:
	virtual void Initialize( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
};


#else // PRE_MOD_SELECT_CHAR


class CDnCharSelectTitleDlg;
#include "DnCharSelectArrowLeftDlg.h"
#include "DnCharSelectArrowRightDlg.h"

class CDnCharSelectDlg : public CEtUIDialog, public CEtUICallback
{
public:
	CDnCharSelectDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL  );
	virtual ~CDnCharSelectDlg(void);

protected:
	CDnCharSelectTitleDlg *m_pCharSelectTitleDlg;
	CDnCharSelectArrowLeftDlg* m_pCharSelectArrowLeftDlg;
	CDnCharSelectArrowRightDlg* m_pCharSelectArrowRightDlg;
	float m_fCharSelectDescDelta;
	CEtUIStatic *m_pStaticCharSelectDesc;

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0 );
	virtual void Process( float fElapsedTime );
	virtual void Show( bool bShow );

	bool bIsDeleteChracter;
	void ShowCharSelectArrowLeft( bool bShow ) { if( m_pCharSelectArrowLeftDlg ) m_pCharSelectArrowLeftDlg->Show( bShow ); }
	void ShowCharSelectArrowRight( bool bShow ) { if( m_pCharSelectArrowRightDlg ) m_pCharSelectArrowRightDlg->Show( bShow ); }

#if defined(PRE_ADD_23829)
	void Show2ndPassCreateButton(bool bShow);
#endif
};

#endif // PRE_MOD_SELECT_CHAR
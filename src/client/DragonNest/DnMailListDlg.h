#pragma once

#include "EtUIDialog.h"

#include "DnTradeMail.h"
#include "DnTradeTask.h"

class CDnMailListDlg : public CEtUIDialog, public CEtUICallback
{
	struct SMailListRow
	{
		int				m_nMailID;
		CEtUIStatic*	m_pBase;
		CEtUIStatic*	m_pNoticeBase;
#ifdef PRE_ADD_CADGE_CASH
		CEtUIStatic*	m_CashBase;
#endif // PRE_ADD_CADGE_CASH
		CEtUICheckBox*	m_pCheckBox;		// Check
		CEtUIStatic*	m_pStaticTitle;		// Mail title
		CEtUIStatic*	m_pStaticName;		// Mail status (opened)
		CEtUIStatic*	m_pStaticAttach;	// Attachment
		CEtUIStatic*	m_pStaticQuick;		// Quick delivery
		CEtUIStatic*	m_pStaticTime;		// Remaining time

		SMailListRow()
			: m_nMailID(0)
			, m_pBase(NULL)
			, m_pNoticeBase(NULL)
#ifdef PRE_ADD_CADGE_CASH
			, m_CashBase(NULL)
#endif // PRE_ADD_CADGE_CASH
			, m_pCheckBox(NULL)
			, m_pStaticAttach(NULL)
			, m_pStaticQuick(NULL)
			, m_pStaticName(NULL)
			, m_pStaticTitle(NULL)
			, m_pStaticTime(NULL)
		{
		}

		void Clear()
		{
			m_nMailID = 0;
			m_pStaticName->ClearText();
			m_pStaticTitle->ClearText();
			m_pStaticTime->ClearText();
			m_pCheckBox->Enable(false);
			m_pCheckBox->SetChecked(false);
			m_pCheckBox->Show(false);
			m_pStaticAttach->Show(false);
			m_pStaticQuick->Show(false);
			m_pBase->Show(true);
			m_pNoticeBase->Show(false);
#ifdef PRE_ADD_CADGE_CASH
			m_CashBase->Show(false);
#endif // PRE_ADD_CADGE_CASH
		}

		bool Show(bool bShow, bool bAttach = false, bool bQuick = false)
		{
			m_pCheckBox->Show(bShow);
			m_pStaticTitle->Show(bShow);
			m_pStaticName->Show(bShow);
			m_pStaticAttach->Show(bAttach);
			m_pStaticQuick->Show(bQuick);
			m_pStaticTime->Show(bShow);

			return true;
		}

		void Init()
		{
			Clear();
		}

		void SetState( MailType::ReadState bOpen, bool bAttach )
		{
			DWORD titleColor = (bOpen == MailType::Read) ? textcolor::DARK : textcolor::WHITE;
			m_pStaticTitle->SetTextColor(titleColor);
			m_pStaticAttach->Show(bAttach);
		}

		bool IsInsideItem( float fX, float fY )
		{
			SUICoord columnCoord, titleCoord;
			m_pBase->GetUICoord(columnCoord);
			m_pStaticTitle->GetUICoord(titleCoord);
			titleCoord.fHeight = columnCoord.fHeight;
			return titleCoord.IsInside( fX, fY );
		}

		bool IsEmpty() const { return (m_nMailID <= 0); }

		void Set(const CDnTradeMail::SMailBoxInfo& info, bool bSetCheckBox);
	};

	enum
	{
		MAIL_LIST_WINDOW_SIZE	= 3,
		MESSAGEBOX_DEL_CONFIRM	= 0,
	};

public:
	CDnMailListDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnMailListDlg(void) {}

	void	Initialize(bool bShow);
	void	InitialUpdate();
	void	Show( bool bShow );
	void	RefreshList(const std::vector<CDnTradeMail::SMailBoxInfo>& mailList);
	void	Clear();

	void	Lock()				{ m_bLock = true;	EnableButtons(false); }
	void	UnLock()			{ m_bLock = false;	EnableButtons(true); }
	bool	IsLocked() const	{ return m_bLock; }

protected:
	void	Process( float fElapsedTime );
	void	ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg );
	bool	MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	void	OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg );

private:
	int		FindInsideItem(float fX, float fY);
	//void	UpdateSelectBar();
	void	UpdatePage();
	void	UpdatePageButton();
	void	UpdateList();

	void	PrevPage();
	void	NextPage();
	void	SetAllCheckBox(bool bSet, bool bCheckBox = false);

	void	RefreshButtons();
	bool	IsAnyChecked() const;

	void	EnableButtons(bool bEnable);

	CEtUIButton*				m_ButtonWrite;
	//CEtUIButton*				m_ButtonRead;
	CEtUIButton*				m_ButtonDel;
	CEtUIButton*				m_ButtonAttachGet;

	CEtUIStatic*				m_pSelectBar;

	CEtUIButton*				m_pButtonPagePrev;
	CEtUIButton*				m_pButtonPageNext;

	CEtUIStatic*				m_pStaticPage;
	CEtUICheckBox*				m_pButtonCheckAll;
	CEtUIStatic*				m_pStaticCheckAll;

	std::vector<SMailListRow>	m_MailList;

	int							m_nCurSelect;
	//int							m_nCurrentPage;
	int							m_nCurWindowPage;

	bool						m_bNewList;
	bool						m_bCheckAll;
	bool						m_bLock;
};
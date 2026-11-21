#pragma once

#include "EtUIDialog.h"

class CEtUITextBox;
class CDnChatOptDlg;

class CDnChatDlg : public CEtUIDialog, public CEtUINameLinkInterface
{
public:
	CDnChatDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnChatDlg(void);

protected:
	CEtUITextBox *m_pChatTextBox;
	CDnChatOptDlg *m_pChatOptDlg;
	CEtUIButton *m_pButtonDummy;
	int m_nChatType;

	// Currently text boxes must be completely re-registered on resolution change to display properly.
	// (AddText uses GetCaretPos to calculate the word region; redoing this produces a word region that matches the text area accurately.
	//  Just swapping the font resizes it in fixed steps, causing it to overflow the word area.
	//  Must put it into a new text box like this due to line-wrapping issues when exceeding the word area.)
	// Other textboxes are recreated when shown again so it is fine, but
	// For things like the chat window, keep separate recovery data and reset it upon recovery.
	struct SAddChatData
	{
		std::wstring wszName;
		std::wstring szMessage;
		int nChatType;
		DWORD dwFormat;
		bool isAppend;
		bool hasColor;
		DWORD colorValue;
		DWORD bgColorValue;
		bool bIgnoreCarrageReturn;
	};
	std::list<SAddChatData> m_listAddChatData;

	bool m_bIsShowScrollBar;

public:
	void InitOption( int nChatType );
	void AddChat( LPCWSTR wszName, LPCWSTR szMessage, int nChatType, DWORD dwFormat = UITEXT_NONE, bool isAppend = false, bool hasColor = false, DWORD colorValue = 0xffffffff, DWORD bgColorValue = 0, bool bIgnoreCarrageReturn = false, bool bOnResetDevice = false );
	bool IsTextBoxEmpty();

	CDnChatOptDlg &GetChatOptDlg()			{ return *m_pChatOptDlg; }
	const CDnChatOptDlg &GetChatOptDlg() const		{ return *m_pChatOptDlg; }
	int GetChatType()						{ return m_nChatType; }

	bool IsShowOptDlg();
	void ShowOptDlg( bool bShow );
	void ShowScrollBar( bool bShow );
	bool IsShowScrollBar(){return m_bIsShowScrollBar;}


public:
	virtual void InitialUpdate();
	virtual void Initialize( bool bShow );
	virtual void UpdateDlgCoord( float fX, float fY, float fWidth, float fHeight );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

	virtual void OnChangeResolution();

	virtual bool OnParseTextItemInfo(const std::wstring& argString);
	virtual bool OnParseTextUserName(const std::wstring& name);

private:
	void ShowChatContextMenuDlg(bool bShow, float fX, float fY, const std::wstring& name);
};

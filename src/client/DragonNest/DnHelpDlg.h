#pragma once
#include "EtUIDialog.h"

class CEtUIListBox;
class CEtUIStatic;
class CEtUIHtmlTextBox;

class CDnHelpDlg : public CEtUIDialog
{
public:
	CDnHelpDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL  );
	virtual ~CDnHelpDlg(void);

	struct HelpStruct {
		int nTableID;
		std::string szCategory;
		std::string szXmlFileName;
		std::vector<int> nVecKeyword;
	};

	struct KeywordStruct {
		int nKeywordStringID;
		int nLinkHelpTableID;
		int nOrder;
	};

protected:
	CEtUITreeCtl *m_pTreeCategory;
	CEtUIHtmlTextBox *m_pHtmlTextHelp;
	CEtUIListBoxEx *m_pListboxKeyword;
	CEtUIStatic *m_pStaticKeyword;
	CEtUIStatic *m_pStaticTitle;

	std::string m_szLastSelectCatogory;
	int m_nLastSelectTableID;

	std::vector<HelpStruct *> m_pVecCategoryList;
	std::map<int, HelpStruct *> m_pMapCategorySearch;

	std::vector<KeywordStruct *> m_pVecKeywordList;

protected:
	void LoadHelpTable();
	void LoadKeywordTable();

	void RefreshCategory();
	void RefreshKeyword();
	void RefreshHelpHtml( int nTableID );
	HelpStruct *FindHelpStructFromTableID( int nTableID );

	void ExpandRoot( CTreeItem *pItem );

public:
	virtual void InitialUpdate();
	virtual void Initialize( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void Show( bool bShow );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

	void ShowCategory( int nIndex );
};

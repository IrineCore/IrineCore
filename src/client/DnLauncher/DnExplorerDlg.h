#pragma once

#include "DnHtmlView.h"

#ifdef _USE_PARTITION_SELECT

// --- Base Class CDnHtmlViewEx / CWebpage / CDnExplorerDlg ---

// Extended HtmlView Class for Fit Size
class CDnHtmlViewEx : public CDHtmlViewSpec
{
protected: // create from serialization only
	CDnHtmlViewEx();
	DECLARE_DYNCREATE(CDnHtmlViewEx)

	//to support html's elements event
	DECLARE_DHTML_EVENT_MAP()

protected:
	virtual void OnInitialUpdate(); // called first time after construct
	void FitWindowSize();

	// Implementation
public:
	virtual ~CDnHtmlViewEx();
	virtual void OnDocumentComplete( LPCTSTR lpszURL );

protected:
	BOOL m_bDocumentComplete;

	// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};

// Webpage Class
class CWebPage  
{
public:
	CWebPage();
	virtual ~CWebPage();

	bool SetDocument(IDispatch* pDisp);
	inline LPDISPATCH GetHtmlDocument() const { return m_spDoc; }
	inline const CString GetLastError() const { return m_strError; }
	bool GetJScript(CComPtr<IDispatch>& spDisp);
	bool GetJScripts(CComPtr<IHTMLElementCollection>& spColl);

	CString ScanJScript(CString& strAText, CStringArray& args);

	bool CallJScript(const CString strFunc,CComVariant* pVarResult = NULL);
	bool CallJScript(const CString strFunc,const CString strArg1,CComVariant* pVarResult = NULL);
	bool CallJScript(const CString strFunc,const CString strArg1,const CString strArg2,CComVariant* pVarResult = NULL);
	bool CallJScript(const CString strFunc,const CString strArg1,const CString strArg2,const CString strArg3,CComVariant* pVarResult = NULL);
	bool CallJScript(const CString strFunc,const CStringArray& paramArray,CComVariant* pVarResult = NULL);

protected:
	inline void ShowError(CString lpszText) { m_strError.Format( L"JSCall Error:\n", lpszText ); }

protected:
	CComPtr<IHTMLDocument2>	m_spDoc;
	CString	m_strError;
};

CString GetNextToken( CString& strSrc, const CString strDelim, bool bTrim = false, bool bFindOneOf = true );

// Explorer Dialog Class
class CDnExplorerDlg : public CDialog
{
	DECLARE_DYNAMIC(CDnExplorerDlg)

public:
	CDnExplorerDlg( CWnd* pParent = NULL );   // standard constructor
	virtual ~CDnExplorerDlg();

	// Dialog Data
	enum { IDD = IDD_DLG_EXPLORER };

protected:
	CString			m_strURL;
	CDnHtmlViewEx*	m_pHtmlView;
	CCreateContext	m_pContext;
	CWnd*			m_pFrameWnd;
	RECT			m_Rect;
	BOOL			m_bShowFlag;

	virtual BOOL OnInitDialog();
	BOOL ShowWindowEx( int nCmdShow );

public:
	void SetURL( CString strURL ) { m_strURL = strURL; }
	virtual void OnDocumentComplete();

public:
	afx_msg void OnWindowPosChanging( WINDOWPOS* lpwndpos );
	afx_msg void OnBnClickedCancel();

	DECLARE_MESSAGE_MAP()
};


// --- Child Class ---

#define WM_SELECT_PARTITION					WM_APP+102	// Partition selection window message

// Partition selection Html View
class CDnSelectPartitionHtmlView : public CDnHtmlViewEx
{
protected: // create from serialization only
	CDnSelectPartitionHtmlView();
	DECLARE_DYNCREATE(CDnSelectPartitionHtmlView)

	//to support html's elements event
	DECLARE_DHTML_EVENT_MAP()

	HRESULT OnButtonSelectPartition( IHTMLElement *pElement );

	// Implementation
public:
	virtual ~CDnSelectPartitionHtmlView();
	virtual void OnDocumentComplete( LPCTSTR lpszURL );

	// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};

class CDnSelectPartitionDlg : public CDnExplorerDlg
{
	DECLARE_DYNAMIC(CDnSelectPartitionDlg)

public:
	CDnSelectPartitionDlg( CWnd* pParent = NULL );
	virtual ~CDnSelectPartitionDlg();

	enum { IDD = IDD_DLG_EXPLORER };

	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage( MSG* pMsg );

public:

	DECLARE_MESSAGE_MAP()
};


#endif // _USE_PARTITION_SELECT
#include "stdafx.h"
#include "DnExplorerDlg.h"
#include "DnLauncherDlg.h"

#ifdef _USE_PARTITION_SELECT

IMPLEMENT_DYNCREATE(CDnHtmlViewEx, CDHtmlViewSpec)

BEGIN_MESSAGE_MAP(CDnHtmlViewEx, CDHtmlViewSpec)
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(CDnHtmlViewEx)
END_DHTML_EVENT_MAP()

//////////////////////////////////////////////////////////////////////////
// CDnHtmlViewEx
//////////////////////////////////////////////////////////////////////////

CDnHtmlViewEx::CDnHtmlViewEx() : m_bDocumentComplete( FALSE )
{
}

CDnHtmlViewEx::~CDnHtmlViewEx()
{
}

void CDnHtmlViewEx::OnInitialUpdate()
{
	SetRegisterAsDropTarget( FALSE );
}

void CDnHtmlViewEx::OnDocumentComplete( LPCTSTR lpszURL )
{
	if( !m_bDocumentComplete )
		FitWindowSize();
	m_bDocumentComplete = TRUE;

	CDnExplorerDlg* pExplorerDlg = dynamic_cast<CDnExplorerDlg*>( GetParent() );
	if( pExplorerDlg )
		pExplorerDlg->OnDocumentComplete();

	CDHtmlViewSpec::OnDocumentComplete( lpszURL );
}

void CDnHtmlViewEx::FitWindowSize()
{
	IDispatch *pDisp = this->GetHtmlDocument();
	if( pDisp != NULL )
	{
		HRESULT hr;
		IHTMLDocument3* pHTMLDocument3 = NULL;
		hr = pDisp->QueryInterface( IID_IHTMLDocument3, (void**)&pHTMLDocument3 );
		if( pHTMLDocument3  )
		{
			IHTMLElement* pDivElement = NULL;
			pHTMLDocument3->getElementById( L"Divid", &pDivElement );	// Requires id=Divid inside the Html Div tag to work
			if( pDivElement )
			{
				// Get only the size of the Div area from Html.
				long lOffsetWidth, lOffsetHeight;
				pDivElement->get_offsetWidth( &lOffsetWidth );
				pDivElement->get_offsetHeight( &lOffsetHeight );

				// Resize the Dialog to match the size calculated above.
				CRect rectView( 0, 0, 0, 0 );
				rectView.right = lOffsetWidth + 4;
				rectView.bottom = lOffsetHeight + 4;
				MoveWindow( &rectView );

				CWnd* pWnd = GetParent();
				if( pWnd )
				{
					CWnd* pRootWnd = pWnd->GetParent();
					if( pRootWnd )
					{
						CRect rectClient, rectWindow, rectRootWindow;
						pWnd->GetClientRect( &rectClient );
						pWnd->GetWindowRect( &rectWindow );
						pRootWnd->GetWindowRect( &rectRootWindow );

						// Adjust position to appear centered in parent dialog
						int nWidth = rectWindow.Width() - rectClient.Width() + rectView.right;
						int nHeight = rectWindow.Height() - rectClient.Height() + rectView.bottom;
						rectWindow.left = rectRootWindow.left + (rectRootWindow.Width() / 2) - (nWidth / 2);
						rectWindow.top = rectRootWindow.top + (rectRootWindow.Height() / 2) - (nHeight / 2);

						rectWindow.right = rectWindow.left + nWidth;
						rectWindow.bottom = rectWindow.top + nHeight;

						pWnd->MoveWindow( &rectWindow );
					}
				}
				pDivElement->Release();
			}
			pHTMLDocument3->Release();
		}

		pDisp->Release();
	}
}


//////////////////////////////////////////////////////////////////////////
// CWebPage Class
//////////////////////////////////////////////////////////////////////////
#define CHECK_POINTER(p)\
	ATLASSERT(p != NULL);\
	if(p == NULL)\
{\
	ShowError(L"NULL pointer");\
	return false;\
}

const CString GetSystemErrorMessage( DWORD dwError )
{
	CString strError;
	LPTSTR lpBuffer;

	if(!FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,  dwError,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT),
		(LPTSTR) &lpBuffer, 0, NULL))

	{
		strError = "FormatMessage Netive Error" ;
	}
	else
	{
		strError = lpBuffer;
		LocalFree(lpBuffer);
	}
	return strError;
}

CString GetNextToken( CString& strSrc, const CString strDelim, bool bTrim, bool bFindOneOf )
{
	CString strToken;
	int idx = bFindOneOf? strSrc.FindOneOf(strDelim) : strSrc.Find(strDelim);
	if(idx != -1)
	{
		strToken  = strSrc.Left(idx);
		strSrc = strSrc.Right(strSrc.GetLength() - (idx + 1) );
	}
	else
	{
		strToken = strSrc;
		strSrc.Empty();
	}
	if(bTrim)
	{
		strToken.TrimLeft();
		strToken.TrimRight();
	}
	return strToken;
}

CWebPage::CWebPage()
{
}

CWebPage::~CWebPage()
{
}

bool CWebPage::SetDocument(IDispatch* pDisp)
{
	CHECK_POINTER(pDisp);

	m_spDoc = NULL;

	CComPtr<IDispatch> spDisp = pDisp;

	HRESULT hr = spDisp->QueryInterface(IID_IHTMLDocument2,(void**)&m_spDoc);
	if(FAILED(hr))
	{
		ShowError(L"Failed to get HTML document COM object");
		return false;
	}
	return true;
}

bool CWebPage::GetJScript(CComPtr<IDispatch>& spDisp)
{
	CHECK_POINTER(m_spDoc);
	HRESULT hr = m_spDoc->get_Script(&spDisp);
	ATLASSERT(SUCCEEDED(hr));
	return SUCCEEDED(hr);
}

bool CWebPage::GetJScripts(CComPtr<IHTMLElementCollection>& spColl)
{
	CHECK_POINTER(m_spDoc);
	HRESULT hr = m_spDoc->get_scripts(&spColl);
	ATLASSERT(SUCCEEDED(hr));
	return SUCCEEDED(hr);
}

bool CWebPage::CallJScript(const CString strFunc,CComVariant* pVarResult)
{
	CStringArray paramArray;
	return CallJScript(strFunc,paramArray,pVarResult);
}

bool CWebPage::CallJScript(const CString strFunc,const CString strArg1,CComVariant* pVarResult)
{
	CStringArray paramArray;
	paramArray.Add(strArg1);
	return CallJScript(strFunc,paramArray,pVarResult);
}

bool CWebPage::CallJScript(const CString strFunc,const CString strArg1,const CString strArg2,CComVariant* pVarResult)
{
	CStringArray paramArray;
	paramArray.Add(strArg1);
	paramArray.Add(strArg2);
	return CallJScript(strFunc,paramArray,pVarResult);
}

bool CWebPage::CallJScript(const CString strFunc,const CString strArg1,const CString strArg2,const CString strArg3,CComVariant* pVarResult)
{
	CStringArray paramArray;
	paramArray.Add(strArg1);
	paramArray.Add(strArg2);
	paramArray.Add(strArg3);
	return CallJScript(strFunc,paramArray,pVarResult);
}

bool CWebPage::CallJScript(const CString strFunc, const CStringArray& paramArray,CComVariant* pVarResult)
{
	CComPtr<IDispatch> spScript;
	if(!GetJScript(spScript))
	{
		ShowError(L"Cannot GetScript");
		return false;
	}
	CComBSTR bstrMember(strFunc);
	DISPID dispid = NULL;
	HRESULT hr = spScript->GetIDsOfNames(IID_NULL,&bstrMember,1,
		LOCALE_SYSTEM_DEFAULT,&dispid);
	if(FAILED(hr))
	{
		ShowError(GetSystemErrorMessage(hr));
		return false;
	}

	const int arraySize = paramArray.GetSize();

	DISPPARAMS dispparams;
	memset(&dispparams, 0, sizeof dispparams);
	dispparams.cArgs = arraySize;
	dispparams.rgvarg = new VARIANT[dispparams.cArgs];

	for( int i = 0; i < arraySize; i++)
	{
		CComBSTR bstr = paramArray.GetAt(arraySize - 1 - i); // back reading
		bstr.CopyTo(&dispparams.rgvarg[i].bstrVal);
		dispparams.rgvarg[i].vt = VT_BSTR;
	}
	dispparams.cNamedArgs = 0;

	EXCEPINFO excepInfo;
	memset(&excepInfo, 0, sizeof excepInfo);
	CComVariant vaResult;
	UINT nArgErr = (UINT)-1;  // initialize to invalid arg

	hr = spScript->Invoke(dispid,IID_NULL,0,
		DISPATCH_METHOD,&dispparams,&vaResult,&excepInfo,&nArgErr);

	delete [] dispparams.rgvarg;
	if(FAILED(hr))
	{
		ShowError(GetSystemErrorMessage(hr));
		return false;
	}

	if(pVarResult)
	{
		*pVarResult = vaResult;
	}
	return true;
}

// returned java script function name, input string is truncating
CString CWebPage::ScanJScript(CString& strAText, CStringArray& args)
{
	args.RemoveAll();
	CString strDelim(" \n\r\t"),strSrc(strAText);
	bool bFound = false;
	while(!strSrc.IsEmpty())
	{
		CString strStart = GetNextToken(strSrc,strDelim);
		if(strStart == "function")
		{
			bFound = true;
			break;
		}
		if(strStart == "/*")
		{
			// Skip comments
			while(!strSrc.IsEmpty())
			{
				CString strStop = GetNextToken(strSrc,strDelim);
				if(strStop == "*/")
				{
					break;
				}
			}
		}
	}

	if(!bFound)
		return L"";

	CString strFunc = GetNextToken(strSrc,L"(",true);
	CString strArgs = GetNextToken(strSrc,L")",true);

	// Parse arguments
	CString strArg;
	while(!(strArg = GetNextToken(strArgs,L",")).IsEmpty())
		args.Add(strArg);

	strAText= strSrc;
	return strFunc;
}

//////////////////////////////////////////////////////////////////////////
// CDnExplorerDlg
//////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CDnExplorerDlg, CDialog)

CDnExplorerDlg::CDnExplorerDlg( CWnd* pParent )
: CDialog( CDnExplorerDlg::IDD, pParent ), m_pHtmlView( NULL), m_pFrameWnd( NULL ), m_bShowFlag( TRUE )
{
	ShowWindowEx( SW_HIDE );	// Call FitSize and make visible when OnDocumentComplete is called
}

CDnExplorerDlg::~CDnExplorerDlg()
{
}

BEGIN_MESSAGE_MAP(CDnExplorerDlg, CDialog)
	ON_WM_WINDOWPOSCHANGING()
	ON_BN_CLICKED(IDCANCEL, &CDnExplorerDlg::OnBnClickedCancel)
END_MESSAGE_MAP()

BOOL CDnExplorerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	RECT rc;
	GetClientRect(&rc);

	m_Rect.left = 0;
	m_Rect.top = 0; 
	m_Rect.right = rc.right + 4; 
	m_Rect.bottom = rc.bottom + 4;

	m_pFrameWnd = this;
	m_pContext.m_pCurrentDoc = NULL;
	m_pContext.m_pNewViewClass = RUNTIME_CLASS( CDnView );
	CDnView *pView = (CDnView*)((CFrameWnd*)m_pFrameWnd)->CreateView( &m_pContext );
	pView->ShowWindow( SW_SHOW );

	m_pContext.m_pCurrentDoc = NULL;
	m_pContext.m_pNewViewClass = RUNTIME_CLASS( CDnHtmlViewEx );
	m_pHtmlView = (CDnHtmlViewEx*)((CFrameWnd*)m_pFrameWnd)->CreateView( &m_pContext );
	m_pHtmlView->ShowWindow( SW_SHOW );
	m_pHtmlView->MoveWindow( &m_Rect );
	m_pHtmlView->Navigate( m_strURL );

	return TRUE;
}

void CDnExplorerDlg::OnWindowPosChanging( WINDOWPOS* lpwndpos )
{
	CDialog::OnWindowPosChanging( lpwndpos );

	if( m_bShowFlag )
		lpwndpos->flags |= SWP_SHOWWINDOW;
	else
		lpwndpos->flags &= ~SWP_SHOWWINDOW;
}

BOOL CDnExplorerDlg::ShowWindowEx( int nCmdShow )
{
	m_bShowFlag = nCmdShow == SW_SHOW ? TRUE : FALSE;
	return GetSafeHwnd() ? ShowWindow( nCmdShow ) : TRUE;
}

void CDnExplorerDlg::OnBnClickedCancel()
{
	OnCancel();
}

void CDnExplorerDlg::OnDocumentComplete()
{
	ShowWindowEx( SW_SHOW );
}


//////////////////////////////////////////////////////////////////////////
// CDnSelectPartitionHtmlView
//////////////////////////////////////////////////////////////////////////


IMPLEMENT_DYNCREATE(CDnSelectPartitionHtmlView, CDnHtmlViewEx)

BEGIN_MESSAGE_MAP(CDnSelectPartitionHtmlView, CDnHtmlViewEx)
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(CDnSelectPartitionHtmlView)
	DHTML_EVENT_ONCLICK( _T("btnServer"), OnButtonSelectPartition )
END_DHTML_EVENT_MAP()

CDnSelectPartitionHtmlView::CDnSelectPartitionHtmlView()
{
}

CDnSelectPartitionHtmlView::~CDnSelectPartitionHtmlView()
{
}

void CDnSelectPartitionHtmlView::OnDocumentComplete( LPCTSTR lpszURL )
{
	if( !m_bDocumentComplete )
	{
		CComVariant vtResult;
		CComPtr<IDispatch> spDisp = NULL;
		CComPtr<IHTMLDocument2> spDocument = NULL;
		CComPtr<IHTMLWindow2> spWindow = NULL;

		// Get document interface
		spDisp = GetHtmlDocument();
		if(spDisp == NULL)
			return;

		CWebPage webpage;
		webpage.SetDocument( spDisp );

		int nChannelIndex = 1;
		std::vector<stChannelListSet>::iterator iter1 = DNPATCHINFO.GetChannelList().begin();
		for( ; iter1 != DNPATCHINFO.GetChannelList().end(); iter1++, nChannelIndex++ )
		{
			CString strChannelIndex;
			strChannelIndex.Format( L"%d", nChannelIndex );

			webpage.CallJScript( L"AddChannel", strChannelIndex, (*iter1).m_strChannelName );

			int nPartitionIndex = 1;
			std::vector<stPartitionListSet>::iterator iter2 = (*iter1).m_vecPartitionList.begin();
			for( ; iter2 != (*iter1).m_vecPartitionList.end(); iter2++, nPartitionIndex++ )
			{
				// Call JavaScript
				CString strPartitionIndex;
				strPartitionIndex.Format( L"%d", nPartitionIndex );
				CStringArray strArray;
				strArray.Add( strChannelIndex );
				strArray.Add( strPartitionIndex );
				strArray.Add( (*iter2).m_strPartitionName );
				strArray.Add( L"0" );
				CString strNew;
				strNew.Format( L"%d", (*iter2).m_wNew );
				strArray.Add( strNew );

				int nSelected = 0;
				if( DNOPTIONDATA.m_nSelectChannelNum == nChannelIndex && 
					DNOPTIONDATA.m_nSelectPartitionId == (*iter2).m_wPartitionId )
					nSelected = 1;

				CString strSelected;
				strSelected.Format( L"%d", nSelected );
				strArray.Add( strSelected );

				webpage.CallJScript(L"AddSever", strArray );
			}
		}
	}

	CDnHtmlViewEx::OnDocumentComplete( lpszURL );
}

HRESULT CDnSelectPartitionHtmlView::OnButtonSelectPartition( IHTMLElement *pElement )
{
	UpdateData();

	CComVariant varChannel;
	CComVariant varPartition;

	if( pElement->getAttribute( CComBSTR("channelID"), 0, &varChannel ) == S_OK )
	{
		CString strVar = varChannel;
		int nSelectChannelNum = _wtoi( strVar.GetBuffer(0) );

		if( pElement->getAttribute( CComBSTR("serverID"), 0, &varPartition ) == S_OK )
		{
			strVar = varPartition;
			int nSelectPartitionNum = _wtoi( strVar.GetBuffer(0) );
			GetParent()->PostMessage( WM_SELECT_PARTITION, (WPARAM)nSelectChannelNum, (LPARAM)nSelectPartitionNum );
		}
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
// CDnSelectPartitionDlg
//////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CDnSelectPartitionDlg, CDialog)
CDnSelectPartitionDlg::CDnSelectPartitionDlg( CWnd* pParent )
: CDnExplorerDlg( pParent )
{
}

CDnSelectPartitionDlg::~CDnSelectPartitionDlg()
{
}

BEGIN_MESSAGE_MAP(CDnSelectPartitionDlg, CDialog)
	ON_WM_WINDOWPOSCHANGING()
END_MESSAGE_MAP()

BOOL CDnSelectPartitionDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	RECT rc;
	GetClientRect(&rc);

	m_Rect.left = 0;
	m_Rect.top = 0; 
	m_Rect.right = rc.right + 4; 
	m_Rect.bottom = rc.bottom + 4;

	m_pFrameWnd = this;
	m_pContext.m_pCurrentDoc = NULL;
	m_pContext.m_pNewViewClass = RUNTIME_CLASS( CDnView );
	CDnView *pView = (CDnView*)((CFrameWnd*)m_pFrameWnd)->CreateView( &m_pContext );
	pView->ShowWindow( SW_SHOW );

	m_pContext.m_pCurrentDoc = NULL;
	m_pContext.m_pNewViewClass = RUNTIME_CLASS( CDnSelectPartitionHtmlView );
	m_pHtmlView = (CDnSelectPartitionHtmlView*)((CFrameWnd*)m_pFrameWnd)->CreateView( &m_pContext );
	m_pHtmlView->ShowWindow( SW_SHOW );
	m_pHtmlView->MoveWindow( &m_Rect );
	m_pHtmlView->Navigate( m_strURL );


	return TRUE;
}


BOOL CDnSelectPartitionDlg::PreTranslateMessage( MSG* pMsg )
{
	if( pMsg->message == WM_SELECT_PARTITION )
	{
		CDnLauncherDlg* pDnLauncherDlg = (CDnLauncherDlg*)GetParent();
		if( pDnLauncherDlg )
		{

#ifdef _FIRST_PATCH
			switch(DNFIRSTPATCHINFO.IsFirstPatchProcess())
			{
			case DnFirstPatchInfo::EM_FIRSTPATCH_ING: // Module patching in progress
				{
					pDnLauncherDlg->SetOnlySelectPartition( (int)pMsg->wParam, (int)pMsg->lParam );
				}
				break;

			case DnFirstPatchInfo::EM_FIRSTPATCH_NOT: // Module patch finished state
				{
					pDnLauncherDlg->SetSelectPartition( (int)pMsg->wParam, (int)pMsg->lParam );
				}
				break;
			}
#else
			pDnLauncherDlg->SetSelectPartition( (int)pMsg->wParam, (int)pMsg->lParam );
#endif
		}

		EndDialog( WM_SELECT_PARTITION );
	}

	return CDialog::PreTranslateMessage( pMsg );
}
#endif // _USE_PARTITION_SELECT

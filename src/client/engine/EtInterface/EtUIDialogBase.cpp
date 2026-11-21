#include "StdAfx.h"
#include "EtUIDialogBase.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

std::list<CEtUIDialog*> CEtUIDialogBase::s_plistDialogBottomMsg;
std::list<CEtUIDialog*> CEtUIDialogBase::s_plistDialogBottom;
std::list<CEtUIDialog*> CEtUIDialogBase::s_plistDialogTopMsg;
std::list<CEtUIDialog*> CEtUIDialogBase::s_plistDialogTop;
std::list<CEtUIDialog*> CEtUIDialogBase::s_plistDialogFocus;
std::list<CEtUIDialog*> CEtUIDialogBase::s_plistDialogModal;
std::list<CEtUIDialog*> CEtUIDialogBase::s_plistDialogMostTop;
std::list<CEtUIDialog*> CEtUIDialogBase::s_plistDialogSelf;

bool CEtUIDialogBase::s_bShowModal = false;

#if defined(PRE_FIX_57852)
CEtUIDialogBase::MAP_DIALOG_LIST CEtUIDialogBase::ms_AddDialogList;
#endif // PRE_FIX_57852

DECL_SMART_PTR_STATIC( CEtUIDialog, 500 )

CEtUIDialogBase::CEtUIDialogBase( UI_DIALOG_TYPE dialogType )
{
	m_emDialogType = dialogType;

	switch( m_emDialogType )
	{
	case UI_TYPE_BOTTOM_MSG:s_plistDialogBottomMsg.push_back( static_cast<CEtUIDialog*>(this) );break;
	case UI_TYPE_BOTTOM:	s_plistDialogBottom.push_back( static_cast<CEtUIDialog*>(this) );	break;
	case UI_TYPE_TOP_MSG:	s_plistDialogTopMsg.push_back( static_cast<CEtUIDialog*>(this) );	break;
	case UI_TYPE_TOP:		s_plistDialogTop.push_back( static_cast<CEtUIDialog*>(this) );		break;
	case UI_TYPE_FOCUS:		s_plistDialogFocus.push_back( static_cast<CEtUIDialog*>(this) );	break;
	case UI_TYPE_MODAL:		s_plistDialogModal.push_back( static_cast<CEtUIDialog*>(this) );	break;
	case UI_TYPE_MOST_TOP:	s_plistDialogMostTop.push_back( static_cast<CEtUIDialog*>(this) );	break;
	case UI_TYPE_SELF:		s_plistDialogSelf.push_back( static_cast<CEtUIDialog*>(this) );		break;	// Kept solely to hold a reference.
	}
}

CEtUIDialogBase::~CEtUIDialogBase()
{
	switch( m_emDialogType )
	{
	case UI_TYPE_BOTTOM_MSG:RemoveDialog( s_plistDialogBottomMsg );	break;
	case UI_TYPE_BOTTOM:	RemoveDialog( s_plistDialogBottom );	break;
	case UI_TYPE_TOP_MSG:	RemoveDialog( s_plistDialogTopMsg );	break;
	case UI_TYPE_TOP:		RemoveDialog( s_plistDialogTop );		break;
	case UI_TYPE_FOCUS:		RemoveDialog( s_plistDialogFocus );		break;
	case UI_TYPE_MODAL:		RemoveDialog( s_plistDialogModal );		break;
	case UI_TYPE_MOST_TOP:	RemoveDialog( s_plistDialogMostTop );	break;
	case UI_TYPE_SELF:		RemoveDialog( s_plistDialogSelf );		break;
	}
}

void CEtUIDialogBase::ShowDialogList( UI_DIALOG_TYPE dialogType, bool bShow )
{
	switch( dialogType )
	{
	case UI_TYPE_BOTTOM_MSG:ShowList( s_plistDialogBottomMsg, bShow );	break;
	case UI_TYPE_BOTTOM:	ShowList( s_plistDialogBottom, bShow );		break;
	case UI_TYPE_TOP_MSG:	ShowList( s_plistDialogTopMsg, bShow );		break;
	case UI_TYPE_TOP:		ShowList( s_plistDialogTop, bShow );		break;
	case UI_TYPE_FOCUS:		ShowList( s_plistDialogFocus, bShow );		break;
	case UI_TYPE_MODAL:		ShowList( s_plistDialogModal, bShow );		break;
	case UI_TYPE_MOST_TOP:	ShowList( s_plistDialogMostTop, bShow );	break;
	}
}

void CEtUIDialogBase::CloseAllDialog()
{
	ShowList( s_plistDialogMostTop, false );
	ShowList( s_plistDialogTop, false );
	ShowList( s_plistDialogTopMsg, false );
	ShowList( s_plistDialogModal, false );
	ShowList( s_plistDialogFocus, false );
	ShowList( s_plistDialogBottom, false );
	ShowList( s_plistDialogBottomMsg, false );
}

void CEtUIDialogBase::RenderAll( float fElapsedTime )
{
	RenderList( s_plistDialogBottom, fElapsedTime );
	RenderList( s_plistDialogBottomMsg, fElapsedTime );
	RenderList( s_plistDialogFocus, fElapsedTime );
	RenderList( s_plistDialogModal, fElapsedTime );
	RenderList( s_plistDialogTop, fElapsedTime );
	RenderList( s_plistDialogTopMsg, fElapsedTime );
	RenderList( s_plistDialogMostTop, fElapsedTime );
}

void CEtUIDialogBase::ProcessAll( float fElapsedTime )
{
	ProcessList( s_plistDialogMostTop, fElapsedTime );
	ProcessList( s_plistDialogTop, fElapsedTime );
	ProcessList( s_plistDialogTopMsg, fElapsedTime );
	ProcessList( s_plistDialogModal, fElapsedTime );
	ProcessList( s_plistDialogFocus, fElapsedTime );
	ProcessList( s_plistDialogBottom, fElapsedTime );
	ProcessList( s_plistDialogBottomMsg, fElapsedTime );

#if defined(PRE_FIX_57852)
	ProcessAddDialogList();
#endif // PRE_FIX_57852
}

bool CEtUIDialogBase::MsgProcModal( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( s_plistDialogModal.empty() )
		return false;

	std::list< CEtUIDialog* >::reverse_iterator iter = s_plistDialogModal.rbegin();

	if( iter == s_plistDialogModal.rend() ) 
		return false;

	for( ; iter != s_plistDialogModal.rend(); ++iter )
	{
		// Same as handled in child modal of CEtUIDialog::MsgProc.
		(*iter)->SetHWnd( hWnd );
	}

	iter = s_plistDialogModal.rbegin();
	if( (*iter)->IsShow() )
	{
		(*iter)->MsgProc( hWnd, uMsg, wParam, lParam );

		if( CEtUIIME::s_bRESULTSTR_NotSendComp == false )
			return true;
		else
		{
			if( (uMsg == WM_IME_COMPOSITION) && ((lParam & GCS_COMPSTR) || (lParam & GCS_RESULTSTR)) ) {
			}
			else return true;
		}
	}

	return false;
}

bool CEtUIDialogBase::StaticMsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( MsgProcList( s_plistDialogTopMsg, hWnd, uMsg, wParam, lParam ) )	return true;
	if( MsgProcList( s_plistDialogFocus, hWnd, uMsg, wParam, lParam ) )		return true;
	if( MsgProcList( s_plistDialogBottomMsg, hWnd, uMsg, wParam, lParam ) ) return true;

	return false;
}

void CEtUIDialogBase::RenderList( std::list< CEtUIDialog* > &listDialog, float fElapsedTime )
{
	for( std::list< CEtUIDialog* >::iterator iter = listDialog.begin(); iter != listDialog.end(); ++iter )
	{
		if( *iter == NULL )
		{
			_ASSERT(0&&"NULL found in iterator inside CEtUIDialogBase::RenderList!");
			continue;
		}

		if( (*iter)->IsRootDialog() )
		{
			(*iter)->Render( fElapsedTime );
		}
	}
}

void CEtUIDialogBase::SetRenderPriority(CEtUIDialog* pDlg, bool bTail)
{
	if (pDlg == NULL)
		return;

	if (pDlg->GetDialogType() == UI_TYPE_CHILD || pDlg->GetDialogType() == UI_TYPE_CHILD_MODAL)
	{
		CEtUIDialog* pParent = pDlg->GetParentDialog();
		if (pParent)
		{
			pParent->SetChildRenderPriority(pDlg, bTail);
			pParent->SetRenderPriority(pParent, bTail);
		}
	}
	else
	{
#if defined(PRE_FIX_57852)
		int nType = pDlg->GetDialogType();
		AddDialogList(nType, pDlg, bTail);
#else
		std::list< CEtUIDialog* >* pDlgMsgList = NULL;
		switch(pDlg->GetDialogType())
		{
		case UI_TYPE_BOTTOM_MSG :	pDlgMsgList = &s_plistDialogBottomMsg; break;
		case UI_TYPE_BOTTOM :		pDlgMsgList = &s_plistDialogBottom; break;
		case UI_TYPE_TOP_MSG:		pDlgMsgList = &s_plistDialogTopMsg; break;
		case UI_TYPE_TOP:			pDlgMsgList = &s_plistDialogTop; break;
		case UI_TYPE_FOCUS:			pDlgMsgList = &s_plistDialogFocus; break;
		case UI_TYPE_MODAL:			pDlgMsgList = &s_plistDialogModal; break;
		case UI_TYPE_MOST_TOP:		pDlgMsgList = &s_plistDialogMostTop; break;
		default: 
			{
				_ASSERT(0);
				return;
			}
		}

		if (pDlgMsgList == NULL)
			return;

		std::list< CEtUIDialog* >& dlgMsgList = *pDlgMsgList;
		std::list<CEtUIDialog*>::iterator iter = std::find(dlgMsgList.begin(), dlgMsgList.end(), pDlg);
		if (iter != dlgMsgList.end())
		{
			CEtUIDialog* pDlg = *iter;

			if (bTail)
				dlgMsgList.push_back(pDlg);
			else
				dlgMsgList.push_front(pDlg);
			dlgMsgList.erase(iter);
		}
#endif // PRE_FIX_57852
	}
}

void CEtUIDialogBase::ProcessList( std::list< CEtUIDialog* > &listDialog, float fElapsedTime )
{
	// Note : If it crashes at the iter below, the dialog was deleted somewhere or...
	//		The list has been invalidated.
	//
	for( std::list< CEtUIDialog* >::iterator iter = listDialog.begin(); iter != listDialog.end(); ++iter )
	{
		if( *iter == NULL )
		{
			_ASSERT(0&&"NULL found in iterator inside CEtUIDialogBase::ProcessList!");
			continue;
		}

		if( (*iter)->IsRootDialog() )
		{
			(*iter)->Process( fElapsedTime );

			// Note : When a modal dialog calls Show() inside Process and invalidates the list
			//		Skip processing.
			//
			if( s_bShowModal )
			{
				s_bShowModal = false;
				return;
			}
		}
	}
}

bool CEtUIDialogBase::MsgProcList( std::list< CEtUIDialog* > &listDialog, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	for( std::list< CEtUIDialog* >::reverse_iterator iter = listDialog.rbegin(); iter != listDialog.rend(); ++iter )
	{
		if( *iter == NULL )
		{
			_ASSERT(0&&"NULL found in iterator inside CEtUIDialogBase::MsgProcList!");
			continue;
		}

		if( (*iter)->MsgProc( hWnd, uMsg, wParam, lParam ) )
			return true;
	}

	return false;
}

void CEtUIDialogBase::RemoveDialog( std::list< CEtUIDialog* > &listDialog )
{
	// Note : This function is called when the dialog is deleted.
	//		Since the dialog will be deleted from memory, just remove it from the list.
	//
	for( std::list< CEtUIDialog* >::iterator iter = listDialog.begin(); iter != listDialog.end(); ++iter )
	{
		if( (*iter) == this )
		{
			listDialog.erase( iter );
			break;
		}
	}
}

void CEtUIDialogBase::ShowList( std::list< CEtUIDialog* > &listDialog, bool bShow )
{
	for( std::list< CEtUIDialog* >::iterator iter = listDialog.begin(); iter != listDialog.end(); ++iter )
	{
		if( (*iter)->IsRootDialog() )
		{
			// Note : When CloseAllDialogh() is called and this dialog is the top-level dialog and modal,
			//		Must not call ShowModal() inside Show() function. So call SetShowModal( false ).
			//		This part will likely become unnecessary once structural improvements are made.
			//
			(*iter)->SetShowModal( false );
			(*iter)->Show( bShow );
			(*iter)->SetShowModal( true );
		}
	}
}

void CEtUIDialogBase::ShowModal( CEtUIDialog *pDialog, bool bShow )
{
	ASSERT( pDialog );
	ASSERT( m_emDialogType==UI_TYPE_MODAL );

	for( std::list< CEtUIDialog* >::iterator iter = s_plistDialogModal.begin(); iter != s_plistDialogModal.end(); ++iter )
	{
		if( (*iter) == pDialog )
		{
			s_plistDialogModal.erase( iter );
			break;
		}
	}

	if( bShow )
	{
		s_plistDialogModal.push_back( pDialog );
	}
	else
	{
		s_plistDialogModal.push_front( pDialog );
	}

	s_bShowModal = true;
}

void CEtUIDialogBase::CloseModalDialog()
{
	ShowList( s_plistDialogModal, false );
}

bool CEtUIDialogBase::FindControlList( std::vector< CEtUIControl* > &vecControl, std::list< CEtUIDialog* > &listDialog, int nTypeCount, UI_CONTROL_TYPE *pType, bool bCheckCoveredControl, std::vector<SUICoord> &vecDlgCoord )
{
	CEtUIDialog *pDialog = NULL;
	for( std::list< CEtUIDialog* >::reverse_iterator iter = listDialog.rbegin(); iter != listDialog.rend(); ++iter )
	{
		if( *iter == NULL )
		{
			_ASSERT(0&&"NULL found in iterator inside CEtUIDialogBase::FindControlList!");
			continue;
		}

		// Skip LastRender check for now.
		pDialog = (*iter);
		if( !pDialog ) continue;
		if( (pDialog->GetDialogType() != UI_TYPE_SELF && pDialog->IsElementDialog() == false && pDialog->IsShow()) ||
			(pDialog->IsElementDialog() && pDialog->IsShowElementDialog()) )
		{
			if( pDialog->FindControl( vecControl, nTypeCount, pType, bCheckCoveredControl, vecDlgCoord ) )
				return true;
		}
	}

	return false;
}

bool CEtUIDialogBase::StaticFindControl( std::vector< CEtUIControl* > &vecControl, int nTypeCount, UI_CONTROL_TYPE *pType, bool bCheckCoveredControl )
{
	vecControl.clear();

	if( !g_bEtUIRender )
		return false;

	std::vector<SUICoord> vecDlgCoord;
	CEtUIDialog *pDialog = NULL;
	if( !s_plistDialogModal.empty() )
	{
		std::list< CEtUIDialog* >::reverse_iterator iter = s_plistDialogModal.rbegin();
		if( iter != s_plistDialogModal.rend() )
		{
			pDialog = (*iter);
			if( pDialog && pDialog->IsShow() )
			{
				pDialog->FindControl( vecControl, nTypeCount, pType, bCheckCoveredControl, vecDlgCoord );
				return true;
			}
		}
	}

	// Since we are looking for an input-receiving control, only check Msg dialogs.
	if( FindControlList( vecControl, s_plistDialogSelf, nTypeCount, pType, bCheckCoveredControl, vecDlgCoord ) ) return true;
	if( FindControlList( vecControl, s_plistDialogTopMsg, nTypeCount, pType, bCheckCoveredControl, vecDlgCoord ) ) return true;
	if( FindControlList( vecControl, s_plistDialogFocus, nTypeCount, pType, bCheckCoveredControl, vecDlgCoord ) ) return true;
	if( FindControlList( vecControl, s_plistDialogBottomMsg, nTypeCount, pType, bCheckCoveredControl, vecDlgCoord ) ) return true;

	return true;
}

#if defined(PRE_FIX_57852)
void CEtUIDialogBase::AddDialogList(int nType, CEtUIDialog* pDialog, bool bTail)
{
	if (pDialog == NULL)
		return;

	MAP_DIALOG_LIST::iterator findIter = ms_AddDialogList.find(nType);
	if (findIter != ms_AddDialogList.end())
	{
		_AddDialogInfo info(pDialog, bTail);
		findIter->second.push_back(info);
	}
	else
	{
		DIALOG_LIST dialogList;
		_AddDialogInfo info(pDialog, bTail);
		dialogList.push_back(info);

		ms_AddDialogList.insert(std::make_pair(nType, dialogList));
	}
}
void CEtUIDialogBase::ProcessAddDialogList()
{
	MAP_DIALOG_LIST::iterator iter = ms_AddDialogList.begin();
	for (; iter != ms_AddDialogList.end(); ++iter)
	{
		int nType = iter->first;

		std::list< CEtUIDialog* >* pDlgMsgList = NULL;
		switch(nType)
		{
		case UI_TYPE_BOTTOM_MSG :	pDlgMsgList = &s_plistDialogBottomMsg; break;
		case UI_TYPE_BOTTOM :		pDlgMsgList = &s_plistDialogBottom; break;
		case UI_TYPE_TOP_MSG:		pDlgMsgList = &s_plistDialogTopMsg; break;
		case UI_TYPE_TOP:			pDlgMsgList = &s_plistDialogTop; break;
		case UI_TYPE_FOCUS:			pDlgMsgList = &s_plistDialogFocus; break;
		case UI_TYPE_MODAL:			pDlgMsgList = &s_plistDialogModal; break;
		case UI_TYPE_MOST_TOP:		pDlgMsgList = &s_plistDialogMostTop; break;
		}

		if (pDlgMsgList == NULL)
			continue;

		DIALOG_LIST::iterator iterList = iter->second.begin();
		for (; iterList != iter->second.end(); ++iterList)
		{
			_AddDialogInfo &info = (*iterList);
			if (info.m_pDialog == NULL)
				continue;

			std::list< CEtUIDialog* >& dlgMsgList = *pDlgMsgList;
			std::list<CEtUIDialog*>::iterator iter = std::find(dlgMsgList.begin(), dlgMsgList.end(), info.m_pDialog);
			if (iter != dlgMsgList.end())
			{
				CEtUIDialog* pDlg = *iter;

				if (info.m_bTail)
					dlgMsgList.push_back(pDlg);
				else
					dlgMsgList.push_front(pDlg);

				dlgMsgList.erase(iter);
			}
		}
	}

	ms_AddDialogList.clear();
	
}
#endif // PRE_FIX_57852
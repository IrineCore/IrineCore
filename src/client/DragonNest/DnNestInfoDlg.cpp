#include "StdAfx.h"
#include "DnNestInfoDlg.h"
#include "DnNestInfoListItem.h"
#include "DnTableDB.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnNestInfoDlg::CDnNestInfoDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback, true )
, m_pListBoxEx( NULL )
{
}

CDnNestInfoDlg::~CDnNestInfoDlg(void)
{
	m_pListBoxEx->RemoveAllItems();
}

void CDnNestInfoDlg::InitialUpdate()
{
	m_pListBoxEx = GetControl<CEtUIListBoxEx>("ID_LISTBOXEX_NEST");
	m_pListBoxEx->SetRenderSelectBar( false );
	m_pListBoxEx->SetRenderScrollBar( false );
}

void CDnNestInfoDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "PartyNestInfoDlg.ui" ).c_str(), bShow );
}

void CDnNestInfoDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		//RefreshInfo();
	}
	else
	{
		m_pListBoxEx->RemoveAllItems();
	}

	CEtUIDialog::Show( bShow );
}

void CDnNestInfoDlg::RefreshInfo(SCGetPlayerCustomEventUI* pNestInfo)
{
	// Update mission list
	m_pListBoxEx->RemoveAllItems();

	if (pNestInfo == NULL)
		return;

	int nActivate = -1;
	int uiStringID = -1;

	for (int j = 0; j < pNestInfo->cCount; ++j)
	{
		nActivate = -1;
		uiStringID = -1;

		//Retrieve info from the table using missionID.
		if (GetNestInfo(pNestInfo->UIs[j].nMissionID, nActivate, uiStringID) == false)
			continue;

		CDnNestInfoListItem* pItem = m_pListBoxEx->AddItem<CDnNestInfoListItem>();
		if (pItem)
		{
			pItem->SetInfo(pNestInfo->UIs[j], nActivate, uiStringID);
		}
	}

	m_pListBoxEx->SetRenderScrollBar(pNestInfo->cCount > 5);
	
}

bool CDnNestInfoDlg::GetNestInfo(int nMissionID, int &nActivate, int &uiStringID)
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TPLAYERCUSTOMEVENTUI );
	if (pSox == NULL)
		return false;


	int nTableCount = pSox->GetItemCount();
	int nTableMissionID = -1;
	int uiTempStringID = -1;
	int nTempActivate = -1;
	int nItemID = -1;

	for (int i = 0; i < nTableCount; ++i)
	{
		nItemID = pSox->GetItemID( i );
		nTempActivate = pSox->GetFieldFromLablePtr(nItemID, "_Activate")->GetInteger();
		nTableMissionID = pSox->GetFieldFromLablePtr(nItemID, "_Param1")->GetInteger();
		uiTempStringID = pSox->GetFieldFromLablePtr(nItemID, "_Param2")->GetInteger();

		if (nMissionID == nTableMissionID)
		{
			nActivate = nTempActivate;
			uiStringID = uiTempStringID;
			return true;
		}
	}

	return false;
}

void CDnNestInfoDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_BUTTON_CLOSE") )
		{
			Show(false);
		}
		else if (IsCmdControl("ID_BUTTON_CREATE"))
		{
			Show(false);
		}
	}
	else
		CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

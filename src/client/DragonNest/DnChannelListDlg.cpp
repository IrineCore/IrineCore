#include "StdAfx.h"
#include "DnChannelListDlg.h"
#include "DnLoginTask.h"
#include "TaskManager.h"
#include "DnInterface.h"
#include "DnInterfaceDlgID.h"
#include "DnTableDB.h"
#include "LoginSendPacket.h"
#include "PartySendPacket.h"

//blondymarry start
#include "GlobalValue.h"
//blondymarry end

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnChannelListDlg::CDnChannelListDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback, bool bAutoCursor )
	: CEtUIDialog( dialogType, pParentDialog, nID, pCallback, bAutoCursor )
	, m_pChannelList(NULL)
	, m_pButtonOK(NULL)
	, m_pButtonCancel(NULL)
	, m_fElapsedTime(0.0f)
	, m_eChannelAttribute( CHANNEL_ATT_NORMAL )
	, m_pRadioVillage( NULL )
	, m_pRadioArena( NULL )
	, m_pRadioDarklair( NULL )
	, m_pRadioFarm( NULL )
{
}

CDnChannelListDlg::~CDnChannelListDlg(void)
{
}

void CDnChannelListDlg::Initialize(bool bShow)
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "ChannelListDlg.ui" ).c_str(), bShow );
}

void CDnChannelListDlg::InitialUpdate()
{
	m_pChannelList = GetControl<CEtUIListBox>("ID_LISTBOX_CHANNEL");
	m_pButtonOK = GetControl<CEtUIButton>("ID_OK");
	m_pButtonCancel = GetControl<CEtUIButton>("ID_BACK");

	m_pRadioVillage = GetControl<CEtUIRadioButton>("ID_RBT_TAB0");
	m_pRadioArena = GetControl<CEtUIRadioButton>("ID_RBT_TAB1");
	

	m_pRadioDarklair = GetControl<CEtUIRadioButton>("ID_RBT_TAB2");
	m_pRadioFarm = GetControl<CEtUIRadioButton>("ID_RBT_TAB3");

	m_pRadioFarm->Show(true);


	m_pRadioVillage->SetChecked( true );
}

void CDnChannelListDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_BACK" ) ) 
		{
			CDnLoginTask *pTask = (CDnLoginTask *)CTaskManager::GetInstance().GetTask( "LoginTask" );
			if( pTask )
			{
				SendBackButton();
				pTask->ChangeState( CDnLoginTask::CharSelect, false );
			}
			return;
		}
		else if( IsCmdControl("ID_OK" ) )
		{
			SelectChannel();
			return;
		}
	}
	else if( nCommand == EVENT_LISTBOX_ITEM_DBLCLK )
	{
		if( IsCmdControl("ID_LISTBOX_CHANNEL" ) )
		{
			SelectChannel();
			return;
		}
	}
	else if( nCommand == EVENT_LISTBOX_SELECTION )
	{
		if( IsCmdControl("ID_LISTBOX_CHANNEL" ) )
		{
			SListBoxItem *pItem = m_pChannelList->GetSelectedItem();
			if( pItem )
			{
				SChannelInfo *pInfo = (SChannelInfo*)(pItem->pData);
				if( CGlobalInfo::GetInstance().m_cLocalAccountLevel == 0 && pInfo->m_nCongestion == HIGH_CONGESTION )
				{
					m_pButtonOK->Enable( false );
				}
				else
				{
					m_pButtonOK->Enable( true );
				}
			}
		}
	}
	else if( nCommand == EVENT_RADIOBUTTON_CHANGED ) 
	{
		if( IsCmdControl( "ID_RBT_TAB0" ) ) 
		{
			m_eChannelAttribute = CHANNEL_ATT_NORMAL;
			ResetList();
		}
		else if( IsCmdControl( "ID_RBT_TAB1" ) ) 
		{
			m_eChannelAttribute = CHANNEL_ATT_PVP;
			ResetList();
		}
		else if( IsCmdControl( "ID_RBT_TAB2" ) ) 
		{
			m_eChannelAttribute = CHANNEL_ATT_DARKLAIR;
			ResetList();
		}
		else if( IsCmdControl( "ID_RBT_TAB3" ) ) 
		{
			m_eChannelAttribute = CHANNEL_ATT_FARMTOWN;
			ResetList();
		}
	}
}

void CDnChannelListDlg::AddChannelList( sChannelInfo *pChannelInfo )
{
	ASSERT( pChannelInfo&&"CDnChannelListDlg::AddChannelList, pChannel is NULL!" );

	int nCongestion(NO_CONGESTION);
	std::wstring strCongestion;

	wchar_t wszColor[255]={0,};

	float fCongestion = (float)pChannelInfo->nCurrentUserCount / pChannelInfo->nMaxUserCount;
	if( fCongestion >= 0.90f )
	{
		strCongestion = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 100039 );
		nCongestion = HIGH_CONGESTION;
		_wcscpy(wszColor, _countof(wszColor), L"@RGB(204,51,0)", (int)wcslen(L"@RGB(204,51,0)"));
	}
	else if( fCongestion >= 0.20f )
	{
		strCongestion = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 100040 );
		nCongestion = MID_CONGESTION;
		_wcscpy(wszColor, _countof(wszColor), L"@RGB(255,204,0)", (int)wcslen(L"@RGB(255,204,0)"));
	}
	else if( fCongestion >= 0.05f )
	{
		strCongestion = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 100041 );
		nCongestion = LOW_CONGESTION;		
		_wcscpy(wszColor, _countof(wszColor), L"@RGB(102,204,255)", (int)wcslen(L"@RGB(102,204,255)"));
	}
	else
	{
		strCongestion = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 100042 );
		nCongestion = NO_CONGESTION;
		_wcscpy(wszColor, _countof(wszColor), L"@RGB(153,204,0)", (int)wcslen(L"@RGB(153,204,0)"));
	}

	wchar_t strTemp[128] = {0};
	wchar_t szServerFullName[255] = {0,};
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMAP );
	if( pSox )
	{
		int nMapNameID = pSox->GetFieldFromLablePtr( pChannelInfo->nMapIdx, "_MapNameID" )->GetInteger();
				
#ifdef PRE_MOD_CONSTANT_TEXT_CONVERT_TO_TABLE_TEXT
		swprintf_s( strTemp, _countof(strTemp), L"      %s %02d. %s", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7889 ), pChannelInfo->nChannelIdx, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nMapNameID ) ); // UIString : 7889 - "CH."
#else 
		swprintf_s( strTemp, _countof(strTemp), L"      CH %02d. %s", pChannelInfo->nChannelIdx, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nMapNameID ) );
#endif
#if defined(PRE_ADD_CHANNELNAME)
		if( wcslen(pChannelInfo->wszLanguageName) > 0 )
		{
			wcscat(strTemp, L"[");
			wcscat(strTemp, pChannelInfo->wszLanguageName);
			wcscat(strTemp, L"]");
		}
#endif //#if defined(PRE_ADD_CHANNELNAME)
		if( pChannelInfo->nMeritBonusID > 0 ) {
			wchar_t szLevel[64]={0,};
#ifdef PRE_MOD_CONSTANT_TEXT_CONVERT_TO_TABLE_TEXT
			swprintf_s( szLevel, L" (%s%d ~ %s%d) ", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7888 ), pChannelInfo->cMinLevel, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7888 ), pChannelInfo->cMaxLevel ); 
#else
			swprintf_s( szLevel, L" (Lv%d ~ Lv%d) ", pChannelInfo->cMinLevel, pChannelInfo->cMaxLevel );
#endif 
			wcscat_s( strTemp, szLevel);
		}

		_wcscpy(szServerFullName, _countof(szServerFullName), strTemp, (int)wcslen(strTemp));
		wcscat_s(szServerFullName, L"@MOVE(0.337f)[ ");
		wcscat_s(szServerFullName, wszColor );
		wcscat_s(szServerFullName, strCongestion.c_str() );
		wcscat_s(szServerFullName, L"@/RGB ]");
	}

	SChannelInfo * pInfo = new SChannelInfo( pChannelInfo->nChannelID, nCongestion, 0, pChannelInfo->nChannelAttribute, szServerFullName );

	if( pChannelInfo->nChannelAttribute & CHANNEL_ATT_PVP ) 
	{
		if( m_eChannelAttribute & CHANNEL_ATT_PVP )
			m_pChannelList->AddItem( szServerFullName, pInfo, pChannelInfo->nChannelID, D3DCOLOR_XRGB(255, 128, 128) );		
	}
	else if( pChannelInfo->nChannelAttribute & CHANNEL_ATT_DARKLAIR ) 
	{
		if( m_eChannelAttribute & CHANNEL_ATT_DARKLAIR )
			m_pChannelList->AddItem( szServerFullName, pInfo, pChannelInfo->nChannelID, D3DCOLOR_XRGB(255, 0, 128) );		// Temporarily set color, someone please change it later~~
	}
	else if( pChannelInfo->nChannelAttribute & CHANNEL_ATT_FARMTOWN )
	{	
		if( m_eChannelAttribute & CHANNEL_ATT_FARMTOWN )
			m_pChannelList->AddItem( szServerFullName, pInfo, pChannelInfo->nChannelID, D3DCOLOR_XRGB(0, 128, 255) );
	}
	else
	{
		if( !(m_eChannelAttribute & CHANNEL_ATT_DARKLAIR) && !(m_eChannelAttribute & CHANNEL_ATT_PVP) && !(m_eChannelAttribute & CHANNEL_ATT_FARMTOWN) )
			m_pChannelList->AddItem( szServerFullName, pInfo, pChannelInfo->nChannelID );		
	}
	m_vChannelInfo.push_back( pInfo );
}

void CDnChannelListDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	RemoveList();

	if( bShow )
	{
		m_fElapsedTime = CHANNEL_REFRESH_TIME;
		RequestFocus( m_pChannelList );

		m_eChannelAttribute = CHANNEL_ATT_NORMAL;
		m_pRadioVillage->SetChecked( true );

		ResetList();
	}
	else
	{
		m_pButtonOK->Enable( false );
	}

	CEtUIDialog::Show( bShow );
}

void CDnChannelListDlg::Process( float fElapsedTime )
{
	CEtUIDialog::Process( fElapsedTime );

	if( IsShow() )
	{
		if( m_fElapsedTime <= 0.0f )
		{
			RemoveList();
			if( CTaskManager::GetInstance().GetTask( "LoginTask" ) )
				SendReqChannelList();
			else SendReqMoveChannelList();
			m_fElapsedTime = CHANNEL_REFRESH_TIME;
		}
		else
		{
			m_fElapsedTime -= fElapsedTime;
		}

	}
}

void CDnChannelListDlg::SetDefaultList()
{
	if( !m_pChannelList ) return;
	m_pChannelList->SelectItem(0);
}

void CDnChannelListDlg::SelectChannel()
{

	CDnLoginTask *pTask = (CDnLoginTask *)CTaskManager::GetInstance().GetTask( "LoginTask" );
	if( !pTask ) return;

	SListBoxItem *pItem	 = m_pChannelList->GetSelectedItem();
	if( !pItem ) return;

	SChannelInfo *pInfo = (SChannelInfo*)(pItem->pData);
	if( CGlobalInfo::GetInstance().m_cLocalAccountLevel == 0 && pInfo->m_nCongestion == HIGH_CONGESTION )
	{
		GetInterface().MessageBox( 97, MB_OK );
		return;
	}
//blondy pass attributes
	//Server checks attributes with bitmask and sets if pvp; for now just 
	/*
	if( pInfo->m_nChannelType == 1)
		CGlobalInfo::GetInstance().SetPVPArea(true);
	*/
	
	pTask->SelectChannel( pInfo->m_nChannelID );

//blondy end 
}

void CDnChannelListDlg::RemoveList()
{
	for( int i=0; i<(int)m_vChannelInfo.size(); i++ )
		SAFE_DELETE( m_vChannelInfo[i] );

	m_vChannelInfo.clear();
	m_pChannelList->RemoveAllItems();
}

void CDnChannelListDlg::ResetList()
{
	m_pChannelList->RemoveAllItems();

	for( int itr = 0; itr < (int)m_vChannelInfo.size(); ++itr )
	{
		if( m_vChannelInfo[itr]->m_nChannelAttribute & CHANNEL_ATT_PVP ) 
		{
			if( m_eChannelAttribute & CHANNEL_ATT_PVP )
				m_pChannelList->AddItem( m_vChannelInfo[itr]->m_szServerFullName.c_str(), m_vChannelInfo[itr], m_vChannelInfo[itr]->m_nChannelID, D3DCOLOR_XRGB(255, 128, 128) );		
		}
		else if( m_vChannelInfo[itr]->m_nChannelAttribute & CHANNEL_ATT_DARKLAIR ) 
		{
			if( m_eChannelAttribute & CHANNEL_ATT_DARKLAIR )
				m_pChannelList->AddItem( m_vChannelInfo[itr]->m_szServerFullName.c_str(), m_vChannelInfo[itr], m_vChannelInfo[itr]->m_nChannelID, D3DCOLOR_XRGB(255, 0, 128) );		// Temporarily set color, someone please change it later~~
		}
		else if( m_vChannelInfo[itr]->m_nChannelAttribute & CHANNEL_ATT_FARMTOWN ) 
		{
			if( m_eChannelAttribute & CHANNEL_ATT_FARMTOWN )
				m_pChannelList->AddItem( m_vChannelInfo[itr]->m_szServerFullName.c_str(), m_vChannelInfo[itr], m_vChannelInfo[itr]->m_nChannelID, D3DCOLOR_XRGB(0, 128, 255) );		// Temporarily set color, someone please change it later~~
		}
		else 
		{
			if( !(m_eChannelAttribute & CHANNEL_ATT_DARKLAIR) && 
				!(m_eChannelAttribute & CHANNEL_ATT_PVP) &&
				!(m_eChannelAttribute & CHANNEL_ATT_FARMTOWN) )
				m_pChannelList->AddItem( m_vChannelInfo[itr]->m_szServerFullName.c_str(), m_vChannelInfo[itr], m_vChannelInfo[itr]->m_nChannelID );		
		}
	}

	SetDefaultList();
}
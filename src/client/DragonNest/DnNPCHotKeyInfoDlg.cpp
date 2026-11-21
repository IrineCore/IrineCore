#include "StdAfx.h"
#include "DnNPCHotKeyInfoDlg.h"
#include "DnActor.h"
#include "DnLocalPlayerActor.h"
#include "NPCSendPacket.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnNPCHotKeyInfoDlg::CDnNPCHotKeyInfoDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
{
	for( int i = 0; i < MAX_NPC_HOTKEY; ++i )
	{
		m_pBack[i] = NULL;
		m_pButton[i] = NULL;
		m_pNpcName[i] = NULL;
		m_pStatic[i] = NULL;
		m_pText[i] = NULL;
		m_nNpcUIDs[i] = 0;
	}
}

CDnNPCHotKeyInfoDlg::~CDnNPCHotKeyInfoDlg()
{
}

void CDnNPCHotKeyInfoDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "NpcTalkKey.ui" ).c_str(), bShow );
}

void CDnNPCHotKeyInfoDlg::InitialUpdate()
{
	char szName[64] = { 0 };
	for( int i = 0; i < MAX_NPC_HOTKEY; ++i )
	{
		sprintf_s( szName, sizeof( szName ), "ID_BACK%d", i );
		m_pBack[i] = GetControl<CEtUIStatic>( szName );
		sprintf_s( szName, sizeof( szName ), "ID_BUTTON%d", i );
		m_pButton[i] = GetControl<CEtUIButton>( szName );
		sprintf_s( szName, sizeof( szName ), "ID_STATIC_NPCNAME%d", i );
		m_pNpcName[i] = GetControl<CEtUIStatic>( szName );
		sprintf_s( szName, sizeof( szName ), "ID_STATIC%d", i );
		m_pStatic[i] = GetControl<CEtUIStatic>( szName );
		sprintf_s( szName, sizeof( szName ), "ID_TEXT%d", i );
		m_pText[i] = GetControl<CEtUIStatic>( szName );
	}
	ClearNpcHotKeys();
}

void CDnNPCHotKeyInfoDlg::ClearNpcHotKeys()
{
	for( int i = 0; i < MAX_NPC_HOTKEY; ++i )
	{
		m_nNpcUIDs[i] = 0;
		if( m_pBack[i] ) m_pBack[i]->Show( false );
		if( m_pButton[i] ) m_pButton[i]->Show( false );
		if( m_pNpcName[i] ) { m_pNpcName[i]->SetText( L"" ); m_pNpcName[i]->Show( false ); }
		if( m_pStatic[i] ) m_pStatic[i]->Show( false );
		if( m_pText[i] ) { m_pText[i]->SetText( L"" ); m_pText[i]->Show( false ); }
	}
}

void CDnNPCHotKeyInfoDlg::SetNpcHotKey( int nIndex, UINT nNpcUID, const WCHAR *wszNpcName, const WCHAR *wszKeyText )
{
	if( nIndex < 0 || nIndex >= MAX_NPC_HOTKEY )
		return;

	m_nNpcUIDs[nIndex] = nNpcUID;
	if( m_pBack[nIndex] ) m_pBack[nIndex]->Show( true );
	if( m_pButton[nIndex] ) m_pButton[nIndex]->Show( true );
	if( m_pNpcName[nIndex] ) { m_pNpcName[nIndex]->SetText( wszNpcName ? wszNpcName : L"" ); m_pNpcName[nIndex]->Show( true ); }
	if( m_pStatic[nIndex] ) m_pStatic[nIndex]->Show( true );
	if( m_pText[nIndex] ) { m_pText[nIndex]->SetText( wszKeyText ? wszKeyText : L"" ); m_pText[nIndex]->Show( true ); }
}

void CDnNPCHotKeyInfoDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );
	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		int nIdx = -1;
		if( sscanf_s( pControl->GetControlName(), "ID_BUTTON%d", &nIdx ) == 1 && nIdx >= 0 && nIdx < MAX_NPC_HOTKEY )
		{
			if( m_nNpcUIDs[nIdx] != 0 )
				SendNpcTalk( m_nNpcUIDs[nIdx], L"", L"" );
			return;
		}
	}
	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

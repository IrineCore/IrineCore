#include "stdafx.h"
#include "DnAlteaRankDlg.h"
#include "DnAlteaRank_ListDlg.h"
#include "DnLocalPlayerActor.h"
#include "DnGuildTask.h"
#include "DnAlteaTask.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

#if defined( PRE_ALTEIAWORLD_EXPLORE )

CDnAlteaRankDlg::CDnAlteaRankDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCustomDlg(dialogType, pParentDialog, nID, pCallback, true)
, m_pGlodKey_Button( NULL )
, m_pTime_Button( NULL )
, m_pClass_Static( NULL )
, m_pGuildMark( NULL )
, m_pTitleStatic( NULL )
, m_pName_Static( NULL )
, m_pGuildName_Static( NULL )
, m_pKeyCount_Static( NULL )
, m_pNotice_Static( NULL )
, m_pListBox( NULL )
, m_eDialogType( E_PERSONAL_TYPE_DIALOG )
{

}

CDnAlteaRankDlg::~CDnAlteaRankDlg(void)
{

}

void CDnAlteaRankDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "AlteaRankDlg.ui" ).c_str(), bShow );
}

void CDnAlteaRankDlg::InitialUpdate()
{
	m_pGlodKey_Button = GetControl<CEtUIRadioButton>( "ID_RBT0" );
	m_pTime_Button = GetControl<CEtUIRadioButton>( "ID_RBT1" );

	m_pClass_Static = GetControl<CDnJobIconStatic>( "ID_STATIC_CLASS" );
	m_pGuildMark = GetControl<CEtUITextureControl>( "ID_TEXTUREL_GUILDMARK" );
	m_pTitleStatic = GetControl<CEtUIStatic>( "ID_TEXT0" );
	m_pName_Static = GetControl<CEtUIStatic>( "ID_TEXT_NAME" );
	m_pGuildName_Static = GetControl<CEtUIStatic>( "ID_TEXT_GUILDNAME" );
	m_pKeyCount_Static = GetControl<CEtUIStatic>( "ID_TEXT_KEYCOUNT" );
	m_pNotice_Static = GetControl<CEtUIStatic>( "ID_TEXT_NOTICE" );

	m_pListBox = GetControl<CEtUIListBoxEx>( "ID_LISTBOXEX_RANK" );

	GetControl<CEtUIStatic>( "ID_TEXT1" )->Show( false );
}

void CDnAlteaRankDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	if( true == bShow )
	{
		m_pGlodKey_Button->SetChecked( true );
		RefreshMyInfo();
	}

	CDnCustomDlg::Show( bShow );
}

void CDnAlteaRankDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_RADIOBUTTON_CHANGED )
	{
		if( IsCmdControl("ID_RBT0" ) )
		{
			RefreshNotice( E_GOLDKEY_RANK );

			if( E_PERSONAL_TYPE_DIALOG == m_eDialogType )
				GetAlteaTask().RefreshPersonalGoldKeyRankDlg();
			else if( E_GUILD_TYPE_DIALOG == m_eDialogType )
				GetAlteaTask().RefreshGuildGoldKeyRankDlg();
		}
		else if( IsCmdControl("ID_RBT1" ) )
		{
			RefreshNotice( E_PLAYTIME_RANK );

			GetAlteaTask().RefreshPersonalPlayTimeRankDlg();
		}
	}

	CDnCustomDlg::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnAlteaRankDlg::SetGuildRankDlg()
{
	m_eDialogType = E_GUILD_TYPE_DIALOG;
	GetControl<CEtUIRadioButton>( "ID_RBT1" )->Show( false );;
}

void CDnAlteaRankDlg::SetMyGoldKeyCount( const int nCount )
{
	WCHAR wszString[64] = {0,};
	swprintf_s( wszString, _countof(wszString), GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 4625), nCount );	// UISTRING : %d ea
	m_pKeyCount_Static->SetText( wszString );
}

void CDnAlteaRankDlg::SetMyPlayTime( const UINT nPlayTime )
{
	int nSecond = nPlayTime % 60;
	int nMinute = nPlayTime / 60;
	int nTime = nMinute % 60;

	WCHAR wszString[32] = {0,};
	swprintf_s( wszString, _countof(wszString), GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 121137), nTime, nMinute, nSecond );	// UISTRING : %d : %d : %d

	m_pKeyCount_Static->SetText( wszString );
}

void CDnAlteaRankDlg::SetPersonalGoldKeyRank( std::vector<AlteiaWorld::GoldKeyRankMemberInfo> & vPersonalGoldKey )
{
	m_pListBox->RemoveAllItems();

	std::vector<AlteiaWorld::GoldKeyRankMemberInfo>::reverse_iterator Itor = vPersonalGoldKey.rbegin();
	for( ; Itor != vPersonalGoldKey.rend(); ++Itor )
	{
		AlteiaWorld::GoldKeyRankMemberInfo & sData = (*Itor);

		CDnAlteaRank_ListDlg * pItem = m_pListBox->InsertItem<CDnAlteaRank_ListDlg>();
		pItem->SetPersonalData( sData.nRankIndex, sData.cJob, sData.nGoldKeyCount, sData.wszCharacterName, sData.wszGuildName, E_GOLDKEY_RANK );
	}
}

void CDnAlteaRankDlg::SetPersonalPlayTimeRank( std::vector<AlteiaWorld::PlayTimeRankMemberInfo> & vPersonalPlayTime )
{
	m_pListBox->RemoveAllItems();

	std::vector<AlteiaWorld::PlayTimeRankMemberInfo>::reverse_iterator Itor = vPersonalPlayTime.rbegin();
	for( ; Itor != vPersonalPlayTime.rend(); ++Itor )
	{
		AlteiaWorld::PlayTimeRankMemberInfo & sData = (*Itor);

		CDnAlteaRank_ListDlg * pItem = m_pListBox->InsertItem<CDnAlteaRank_ListDlg>();
		pItem->SetPersonalData( sData.nRankIndex, sData.cJob, sData.nPlaySec, sData.wszCharacterName, sData.wszGuildName, E_PLAYTIME_RANK );
	}
}

void CDnAlteaRankDlg::SetGuildGoldKeyRank( std::vector<AlteiaWorld::GuildGoldKeyRankInfo> & vGuildGoldKey )
{
	m_pListBox->RemoveAllItems();

	std::vector<AlteiaWorld::GuildGoldKeyRankInfo>::reverse_iterator Itor = vGuildGoldKey.rbegin();
	for( ; Itor != vGuildGoldKey.rend(); ++Itor )
	{
		AlteiaWorld::GuildGoldKeyRankInfo & sData = (*Itor);

		CDnAlteaRank_ListDlg * pItem = m_pListBox->InsertItem<CDnAlteaRank_ListDlg>();
		pItem->SetGuildData( sData.nRankIndex, sData.nGoldKeyCount, sData.nGuildID, sData.wGuildMark, sData.wGuildMarkBG, sData.wGuildMarkBorder, sData.wszGuildName );
	}
}

void CDnAlteaRankDlg::RefreshMyInfo()
{
	if( !CDnActor::s_hLocalActor )
		return;

	CDnLocalPlayerActor * pLocalPlayer = (CDnLocalPlayerActor *)(CDnActor::s_hLocalActor.GetPointer());

	if( NULL == pLocalPlayer )
		return;

	m_pName_Static->SetText( pLocalPlayer->GetName() );

	if( E_PERSONAL_TYPE_DIALOG == m_eDialogType )
	{
		std::vector< int > vecJobHistory;
		pLocalPlayer->GetJobHistory( vecJobHistory );

		m_pClass_Static->SetIconID( vecJobHistory[vecJobHistory.size() -1], true );
		m_pTitleStatic->SetText( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 8309) );	// UISTRING : Weekly Best Record
	}
	else
		m_pTitleStatic->SetText( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 8311) );	// UISTRING : My Guild

	TGuild *pGuild = GetGuildTask().GetGuildInfo();
	if( false == pGuild->IsSet() )
		return;

	m_pGuildName_Static->SetText( pGuild->GuildView.wszGuildName );

	if( E_GUILD_TYPE_DIALOG == m_eDialogType )
	{
		if( GetGuildTask().IsShowGuildMark( pGuild->GuildView ) )
		{
			EtTextureHandle hGuildMark = GetGuildTask().GetGuildMarkTexture( pGuild->GuildView );
			m_pGuildMark->SetTexture( hGuildMark );
			m_pGuildMark->Show( true );
		}
	}
}

void CDnAlteaRankDlg::RefreshNotice( int nRankType )
{
	WCHAR wszString[512] = {0,};
	if( E_PERSONAL_TYPE_DIALOG == m_eDialogType )
	{
		DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TSCHEDULE );
		if( NULL == pSox )
			return;

		int nHour = pSox->GetFieldFromLablePtr( 4, "_Hour" )->GetInteger();

		if( E_GOLDKEY_RANK == nRankType )
			swprintf_s( wszString, _countof(wszString), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8308 ), nHour );
		else if( E_PLAYTIME_RANK == nRankType )
			swprintf_s( wszString, _countof(wszString), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8310 ), nHour );
	}
	else if( E_GUILD_TYPE_DIALOG == m_eDialogType )
		swprintf_s( wszString, _countof(wszString), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8312 ) );

	m_pNotice_Static->SetText( wszString );
}

#endif	// #if defined( PRE_ALTEIAWORLD_EXPLORE )
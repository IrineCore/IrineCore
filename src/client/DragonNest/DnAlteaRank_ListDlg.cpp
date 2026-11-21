#include "stdafx.h"
#include "DnAlteaRank_ListDlg.h"
#include "DnAlteaRankDlg.h"
#include "DnTableDB.h"
#include "DnItemTask.h"
#include "DnGuildTask.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

#if defined( PRE_ALTEIAWORLD_EXPLORE )

CDnAlteaRank_ListDlg::CDnAlteaRank_ListDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCustomDlg(dialogType, pParentDialog, nID, pCallback, true)
, m_pRank_Static( NULL )
, m_pClass_Static( NULL )
, m_pGuildMark( NULL )
, m_pName_Static( NULL )
, m_pGuildName_Static( NULL )
, m_pTime_Static( NULL )
{
	memset( m_pItemSlot, NULL, sizeof(m_pItemSlot) );
}

CDnAlteaRank_ListDlg::~CDnAlteaRank_ListDlg(void)
{
	for( int itr = 0; itr < E_Slot_Count; ++itr )
	{
		MIInventoryItem * pItem = m_pItemSlot[itr]->GetItem();
		SAFE_DELETE( pItem );
	}
}

void CDnAlteaRank_ListDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "AlteaRank_ListDlg.ui" ).c_str(), bShow );
}

void CDnAlteaRank_ListDlg::InitialUpdate()
{
	m_pRank_Static = GetControl<CEtUIStatic>("ID_TEXT_RANK");
	m_pClass_Static = GetControl<CDnJobIconStatic>("ID_STATIC_CLASS");
	m_pGuildMark = GetControl<CEtUITextureControl>("ID_TEXTUREL_GUILDMARK");
	m_pName_Static = GetControl<CEtUIStatic>("ID_TEXT_NAME");
	m_pGuildName_Static = GetControl<CEtUIStatic>("ID_TEXT_GUILDNAME");
	m_pTime_Static = GetControl<CEtUIStatic>("ID_TEXT_TIME");

	char szString[62] = {0,};
	for( int itr =0; itr < E_Slot_Count; ++itr )
	{
		sprintf_s( szString, _countof(szString), "ID_BT_ITEM%d", itr );
		m_pItemSlot[itr] = GetControl<CDnItemSlotButton>( szString );
	}
}

void CDnAlteaRank_ListDlg::SetPersonalData( int nRank, BYTE cJob, int nCount, WCHAR * wszName, WCHAR * wszGuildName, int nRankType )
{
	m_pRank_Static->SetIntToText( nRank );
	m_pClass_Static->SetIconID( cJob );
	m_pName_Static->SetText( wszName );
	m_pGuildName_Static->SetText( wszGuildName );
	m_pGuildMark->Show( false );

	SetCountData( nCount, nRankType );
	SetRankItem( nRank, CDnAlteaRankDlg::E_PERSONAL_TYPE_DIALOG, nRankType );
}

void CDnAlteaRank_ListDlg::SetGuildData( int nRank, int nCount, int nGuildID, short wGuildMark, short wGuildMarkBG, short wGuildMarkBorder, WCHAR * wszGuildName )
{
	m_pRank_Static->SetIntToText( nRank );
	m_pName_Static->SetText( wszGuildName );
	m_pClass_Static->Show( false );

	SetGuildMark( nGuildID, wGuildMark, wGuildMarkBG, wGuildMarkBorder );
	SetCountData( nCount, CDnAlteaRankDlg::E_GOLDKEY_RANK );
	SetRankItem( nRank, CDnAlteaRankDlg::E_GUILD_TYPE_DIALOG, CDnAlteaRankDlg::E_GOLDKEY_RANK );
}

void CDnAlteaRank_ListDlg::SetCountData( int nCount, int nRankType )
{
	WCHAR wszString[64] = {0,};

	if( CDnAlteaRankDlg::E_GOLDKEY_RANK == nRankType )
		swprintf_s( wszString, _countof(wszString), GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 4625), nCount );	// UISTRING : %d ea
	else
	{
		int nSecond = nCount % 60;
		int nMinute = nCount / 60;
		int nTime = nMinute % 60;
		nMinute = (nMinute / 60) % 60;

		swprintf_s( wszString, _countof(wszString), GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 121137), nTime, nMinute, nSecond );	// UISTRING : %d : %d : %d
	}

	m_pTime_Static->SetText( wszString );
}

void CDnAlteaRank_ListDlg::SetGuildMark( int nGuildID, short wGuildMark, short wGuildMarkBG, short wGuildMarkBorder )
{
	TGuildView GuildView;
	GuildView.GuildUID.nDBID = nGuildID;
	GuildView.wGuildMark = wGuildMark;
	GuildView.wGuildMarkBG = wGuildMarkBG;
	GuildView.wGuildMarkBorder = wGuildMarkBorder;

	if( GetGuildTask().IsShowGuildMark( GuildView ) )
	{
		EtTextureHandle hGuildMark = GetGuildTask().GetGuildMarkTexture( GuildView );
		m_pGuildMark->SetTexture( hGuildMark );
		m_pGuildMark->Show( true );
	}
}

void CDnAlteaRank_ListDlg::SetRankItem( const int nRank, const int nDialogType, const int nRankType )
{
	DNTableFileFormat * pSox = GetDNTable( CDnTableDB::TALTEIARANK );
	if( NULL == pSox )
		return;

	std::vector< std::pair<int, int> > vecRewardItem;

	const int nCount = pSox->GetItemCount();
	for( int itr = 0; itr < nCount; ++itr )
	{
		int nItemID = pSox->GetItemID( itr );

		const int nTable_Rank = pSox->GetFieldFromLablePtr( nItemID, "_Grade" )->GetInteger();
		const int nTble_DilogType = pSox->GetFieldFromLablePtr( nItemID, "_Type" )->GetInteger();
		const int nTble_RankType = pSox->GetFieldFromLablePtr( nItemID, "_ScaleType" )->GetInteger();

		if( nRank != nTable_Rank || nDialogType != nTble_DilogType || nRankType != nTble_RankType )
			continue;

		char szString[32] = {0,};
		for( int jtr = 1; jtr <= E_Slot_Count; ++jtr )
		{
			sprintf_s( szString, _countof(szString), "_Compensation%d", jtr );
			int nRewardItemID = pSox->GetFieldFromLablePtr( nItemID, szString )->GetInteger();

			sprintf_s( szString, _countof(szString), "_CompensationValue%d", jtr );
			int nRewardItemCount = pSox->GetFieldFromLablePtr( nItemID, szString )->GetInteger();

			vecRewardItem.push_back( std::pair<int, int>( nRewardItemID, nRewardItemCount ) );
		}
		break;
	}

	for( DWORD itr = 0; itr < vecRewardItem.size(); ++itr )
	{
		TItemInfo itemInfo;
		if( CDnItem::MakeItemInfo( vecRewardItem[itr].first, 1, itemInfo ) == false )
			continue;
		CDnItem * pItem = GetItemTask().CreateItem( itemInfo );
		m_pItemSlot[itr]->SetItem( pItem, vecRewardItem[itr].second );
		m_pItemSlot[itr]->Show( true );
	}
}

#endif	// #if defined( PRE_ALTEIAWORLD_EXPLORE )
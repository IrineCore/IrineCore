#include "StdAfx.h"
#include "DnMainQuestRewardDlg.h"
#include "DnItem.h"
#include "DnItemTask.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnMainQuestRewardDlg::CDnMainQuestRewardDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
	, m_pStaticGold( NULL )
	, m_pStaticSilver( NULL )
	, m_pStaticBronze( NULL )
	, m_pStaticExp( NULL )
	, m_pStaticComment( NULL )
	, m_pStaticFTG( NULL )
	, m_pStaticLeftBonus( NULL )
	, m_pStaticRightBonus( NULL )
{
	for( int i = 0; i < REWARD_SLOT_MAX; ++i )
	{
		m_pSlotItems[i] = NULL;
		m_pSlotFrames[i] = NULL;
		m_pRewardItems[i] = NULL;
	}
	for( int i = 0; i < BONUS_SLOT_MAX; ++i )
	{
		m_pLeftBonusSlots[i] = NULL;
		m_pLeftBonusSelects[i] = NULL;
		m_pRightBonusSlots[i] = NULL;
		m_pRightBonusSelects[i] = NULL;
	}
}

CDnMainQuestRewardDlg::~CDnMainQuestRewardDlg()
{
	ClearRewardItems();
}

void CDnMainQuestRewardDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "NpcRewardDlg.ui" ).c_str(), bShow );
}

void CDnMainQuestRewardDlg::InitialUpdate()
{
	m_pStaticGold = GetControl<CEtUIStatic>( "ID_MONEY_GOLD" );
	m_pStaticSilver = GetControl<CEtUIStatic>( "ID_MONEY_SILVER" );
	m_pStaticBronze = GetControl<CEtUIStatic>( "ID_MONEY_BRONZE" );
	m_pStaticExp = GetControl<CEtUIStatic>( "ID_MONEY_EXP" );
	m_pStaticComment = GetControl<CEtUIStatic>( "ID_STATIC_COMMENT" );
	m_pStaticFTG = GetControl<CEtUIStatic>( "ID_STATIC_FTG" );
	m_pStaticLeftBonus = GetControl<CEtUIStatic>( "ID_LEFT_BONUS" );
	m_pStaticRightBonus = GetControl<CEtUIStatic>( "ID_RIGHT_BONUS" );

	char szName[64] = { 0 };
	for( int i = 0; i < REWARD_SLOT_MAX; ++i )
	{
		sprintf_s( szName, sizeof( szName ), "ID_BUTTON_ITEM_%02d", i );
		m_pSlotItems[i] = GetControl<CDnItemSlotButton>( szName );

		sprintf_s( szName, sizeof( szName ), "ID_STATIC_ITEM_%02d", i );
		m_pSlotFrames[i] = GetControl<CEtUIStatic>( szName );
	}

	for( int i = 0; i < BONUS_SLOT_MAX; ++i )
	{
		sprintf_s( szName, sizeof( szName ), "ID_LEFT_BONUSITEM%d", i );
		m_pLeftBonusSlots[i] = GetControl<CDnItemSlotButton>( szName );
		sprintf_s( szName, sizeof( szName ), "ID_LEFT_BONUSSLOTSELECT%d", i );
		m_pLeftBonusSelects[i] = GetControl<CEtUIStatic>( szName );

		sprintf_s( szName, sizeof( szName ), "ID_RIGHT_BONUSITEM%d", i );
		m_pRightBonusSlots[i] = GetControl<CDnItemSlotButton>( szName );
		sprintf_s( szName, sizeof( szName ), "ID_RIGHT_BONUSSLOTSELECT%d", i );
		m_pRightBonusSelects[i] = GetControl<CEtUIStatic>( szName );
	}
}

void CDnMainQuestRewardDlg::ClearRewardItems()
{
	for( int i = 0; i < REWARD_SLOT_MAX; ++i )
	{
		if( m_pSlotItems[i] )
			m_pSlotItems[i]->ResetSlot();
		SAFE_DELETE( m_pRewardItems[i] );
	}
}

void CDnMainQuestRewardDlg::Show( bool bShow )
{
	if( !bShow )
		ClearRewardItems();
	CEtUIDialog::Show( bShow );
}

void CDnMainQuestRewardDlg::SetRewardInfo( INT64 nCoin, int nExp, int nFTG, const std::vector<std::pair<int, int> > &vecItems )
{
	ClearRewardItems();

	if( m_pStaticGold ) m_pStaticGold->SetIntToText( (int)( nCoin / 10000 ) );
	if( m_pStaticSilver ) m_pStaticSilver->SetIntToText( (int)( ( nCoin % 10000 ) / 100 ) );
	if( m_pStaticBronze ) m_pStaticBronze->SetIntToText( (int)( nCoin % 100 ) );
	if( m_pStaticExp ) m_pStaticExp->SetIntToText( nExp );
	if( m_pStaticFTG ) m_pStaticFTG->SetIntToText( nFTG );

	for( size_t i = 0; i < vecItems.size() && i < REWARD_SLOT_MAX; ++i )
	{
		TItemInfo info;
		if( CDnItem::MakeItemInfo( vecItems[i].first, vecItems[i].second, info ) )
		{
			m_pRewardItems[i] = GetItemTask().CreateItem( info );
			if( m_pRewardItems[i] && m_pSlotItems[i] )
				m_pSlotItems[i]->SetItem( m_pRewardItems[i], vecItems[i].second );
		}
	}
}

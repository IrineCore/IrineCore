#include "StdAfx.h"
#include "DnDungeonExpectRewardDlg.h"
#include "DnDungeonExpectMissionDlg.h"
#include "DnDungeonExpectQuestDlg.h"
#include "DnPlayerActor.h"
#include "DnLocalPlayerActor.h"
#include "DnTableDB.h"
#include "DnItemTask.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CDnDungeonExpectRewardDlg::CDnDungeonExpectRewardDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback  )
, m_pStaticGold( NULL )
, m_pStaticSilver( NULL )
, m_pStaticBronze( NULL )
, m_pTextBoxExpGaugeValue( NULL )
, m_fExpectTotalExp( 0.0f )
, m_fStartGapTime( 0.0f )
, m_fExpGaugeDelta( 0.0f )
, m_fExpectExpPercent( 0.0f )
, m_fCurrentExpPercent( 0.0f )
, m_fGaugeSpeedFactor( GAUGE_SPEED_FACTOR )
, m_bStartEffectExpectExp( false )
, m_nTempLevel( 0 )
{
	memset( m_pStaticRewardItem, 0, sizeof( m_pStaticRewardItem ) );
	memset( m_pProgressExpGauge, 0, sizeof( m_pProgressExpGauge ) );
}

CDnDungeonExpectRewardDlg::~CDnDungeonExpectRewardDlg()
{
	for( int i=0; i<MAX_REWARD_ITEM; i++ )
	{
		if (m_pStaticRewardItem[i])
		{
			MIInventoryItem* pItem = m_pStaticRewardItem[i]->GetItem();
			if (pItem != NULL)
				SAFE_DELETE(pItem);
		}
	}
}

void CDnDungeonExpectRewardDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "DungeonEnterTabRewardDlg.ui" ).c_str(), bShow );
}

void CDnDungeonExpectRewardDlg::InitialUpdate()
{
	char szStr[64];
	for( int i=0; i<MAX_REWARD_ITEM; i++ )
	{
		sprintf_s( szStr, "ID_ITEM%d", i );
		m_pStaticRewardItem[i] = GetControl<CDnItemSlotButton>( szStr );
	}

	m_pStaticGold = GetControl<CEtUIStatic>( "ID_GOLD" );
	m_pStaticSilver = GetControl<CEtUIStatic>( "ID_SILVER" );
	m_pStaticBronze = GetControl<CEtUIStatic>( "ID_BRONZE" );

	m_pTextBoxExpGaugeValue = GetControl<CEtUITextBox>( "ID_TEXTBOX_EXP" );

	for( int i=0; i<5; i++ )
	{
		sprintf_s( szStr, "ID_EXP_GAUGE%d", i );
		m_pProgressExpGauge[i] = GetControl<CDnExpGauge>( szStr );
		m_pProgressExpGauge[i]->UseGaugeEndElement( 1 );
	}
}

void CDnDungeonExpectRewardDlg::Process( float fElapsedTime )
{
	CEtUIDialog::Process( fElapsedTime );

	if( m_fStartGapTime > 0.0f )
	{
		m_fStartGapTime -= fElapsedTime;
		if( m_fStartGapTime <= 0.0f )
			m_bStartEffectExpectExp = true;
	}

	if( m_bStartEffectExpectExp )
	{
		if( !CDnActor::s_hLocalActor )
			return;

		m_fExpGaugeDelta += ( fElapsedTime * ( m_fGaugeSpeedFactor / 100.0f ) );
		if( m_fExpGaugeDelta > 1.f )
		{
			m_fExpGaugeDelta = 1.f;
			m_bStartEffectExpectExp = false;
		}

		CDnPlayerActor* pActor = (CDnPlayerActor *)CDnActor::s_hLocalActor.GetPointer();
		int nCurExp = pActor->GetExperience() + (int)( m_fExpectTotalExp * m_fExpGaugeDelta );
		int nCurLevExp = 0;
		int nNextLevExp = 0;
		GetLevelExp( pActor, m_nTempLevel, nCurLevExp, nNextLevExp );

		nNextLevExp -= nCurLevExp;
		nCurExp -= nCurLevExp;

		if( nCurExp > nNextLevExp )
		{
			nCurExp = nNextLevExp;
			m_nTempLevel++;
		}

		m_pTextBoxExpGaugeValue->ClearText();
		wchar_t szTemp[32] = {0};
		swprintf_s( szTemp, 32, L"%.2f%%", m_fCurrentExpPercent );
		m_pTextBoxExpGaugeValue->AddText( szTemp );
		swprintf_s( szTemp, 32, L" + %.2f%%", m_fExpectExpPercent * m_fExpGaugeDelta );
		DWORD dwFontColor = D3DCOLOR_ARGB( 0xFF, 0x96, 0xCD, 0x00 );
		m_pTextBoxExpGaugeValue->AppendText( szTemp, dwFontColor );

		for( int i=0; i<5; i++ )
			m_pProgressExpGauge[i]->SetProgress( 0.0f );

		int nMaxExp = nNextLevExp / 5;
		if( nMaxExp == 0 ) return;
		int nIndex = nCurExp / nMaxExp;
		int nExp = nCurExp % nMaxExp;

		if( nIndex > 5 )
			nIndex = 5;

		for( int i=0; i<nIndex; i++ )
			m_pProgressExpGauge[i]->SetProgress( 100.0f );

		if( nIndex >= 0 && nIndex < 5 )
			m_pProgressExpGauge[nIndex]->SetProgress( ( nExp * 100.0f ) / nMaxExp );
	}
}

void CDnDungeonExpectRewardDlg::GetLevelExp( CDnPlayerActor* pActor, int nTempLevel, int& nCurLevExp, int& nNextLevExp )
{
	if( pActor == NULL )
		return;

	nCurLevExp = CPlayerLevelTable::GetInstance().GetValue( pActor->GetJobClassID(), nTempLevel, CPlayerLevelTable::Experience );
	nNextLevExp = CPlayerLevelTable::GetInstance().GetValue( pActor->GetJobClassID(), nTempLevel + 1, CPlayerLevelTable::Experience );
}

void CDnDungeonExpectRewardDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );
	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnDungeonExpectRewardDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	if( !bShow )
	{
		m_fExpectTotalExp = 0.0f;
		m_fStartGapTime = 0.0f;
		m_fExpGaugeDelta = 0.0f;
		m_fExpectExpPercent = 0.0f;
		m_fCurrentExpPercent = 0.0f;
		m_bStartEffectExpectExp = false;
		m_nTempLevel = 0;
	}

	CEtUIDialog::Show( bShow );
}

bool CDnDungeonExpectRewardDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
		return false;

	return CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );
}

int CDnDungeonExpectRewardDlg::GetTableIndex( DNTableFileFormat*  pSox, int nMapIndex, int nDifficult )
{
	int nIndex = -1;
	
	if( pSox != NULL )
	{
		for( int i=0; i<=pSox->GetItemCount(); i++ )
		{
			int nItemID = pSox->GetItemID( i );
			if( nMapIndex == pSox->GetFieldFromLablePtr( nItemID, "_DungeonID" )->GetInteger()
				&& nDifficult == pSox->GetFieldFromLablePtr( nItemID, "_Difficulty" )->GetInteger() )
			{
				nIndex = nItemID;
				break;
			}
		}
	}

	return nIndex;
}

void CDnDungeonExpectRewardDlg::UpdateDungeonExpectReward( int nMapIndex, int nDifficult )
{
	ClearDungeonExpectReward();

	DNTableFileFormat*  pSox = GetDNTable( CDnTableDB::TSTAGEREWARD );
	if( pSox == NULL )
		return;

	int nIndex = GetTableIndex( pSox, nMapIndex, nDifficult );
	if( nIndex == -1 )
		return;

	// Set Expect Reward Item
	for( int i=0; i<MAX_REWARD_ITEM; i++ )
	{
		char szStr[64];
		sprintf_s( szStr, "_AwardItem%d", i+1 );

		TItemInfo itemInfo;
		if( CDnItem::MakeItemInfo( pSox->GetFieldFromLablePtr( nIndex, szStr )->GetInteger(), 1, itemInfo ) )
		{
			CDnItem* pItem = GetItemTask().CreateItem( itemInfo );
			if( pItem )
			{
				m_pStaticRewardItem[i]->SetItem( pItem, CDnSlotButton::ITEM_ORIGINAL_COUNT );
				m_pStaticRewardItem[i]->Show( true );
			}
		}
	}

	// Set Expect Gold
	int nPrice = pSox->GetFieldFromLablePtr( nIndex, "_AwardGold" )->GetInteger();
	int nGold = nPrice / 10000;
	int nSilver = (nPrice % 10000) / 100;
	int nBronze = nPrice % 100;
	m_pStaticGold->SetIntToText( nGold );
	m_pStaticSilver->SetIntToText( nSilver );
	m_pStaticBronze->SetIntToText( nBronze );

	// Set Expect Exp
	int nPartyMemberCount = GetPartyTask().GetPartyCount();

	DNTableFileFormat*  pExpAdjustTable = GetDNTable( CDnTableDB::TEXPADJUST );
	if( pExpAdjustTable == NULL ) return;

	char *szLabelValue[5] = { "_Easy", "_Normal", "_Hard", "_Master", "_Abyss" };
	float fExpPercent = pExpAdjustTable->GetFieldFromLablePtr( nPartyMemberCount, szLabelValue[nDifficult] )->GetFloat();

	m_fExpectTotalExp = fExpPercent * static_cast<float>( pSox->GetFieldFromLablePtr( nIndex, "_AwardEXP" )->GetInteger() );
#if defined( PRE_USA_FATIGUE )
	int nTemp1(0), nTemp2(0), nTemp3(0), nTemp4(0), nTotalTemp(0);
	GetItemTask().GetFatigue( nTemp1, nTemp2, nTemp3, nTemp4 );
	nTotalTemp = nTemp1 + nTemp2 + nTemp3 + nTemp4;
	int nFatigueExpValue = 100;
	if( nTotalTemp == 0 ) nFatigueExpValue = GetItemTask().GetNoFatigueExpValue();
	else if( nTotalTemp > 0 ) nFatigueExpValue = GetItemTask().GetFatigueExpValue();
	m_fExpectTotalExp = m_fExpectTotalExp * (float)nFatigueExpValue / 100.0f;
#endif	// #if defined( PRE_USA_FATIGUE )

	if( !CDnActor::s_hLocalActor )
		return;

	CDnPlayerActor* pActor = (CDnPlayerActor *)CDnActor::s_hLocalActor.GetPointer();
	int nCurLevExp = pActor->GetCurrentLevelExperience();
	int nNextLevExp = pActor->GetNextLevelExperience();
	int nCurExp = pActor->GetExperience();

	if( nCurExp > nNextLevExp )
		nCurExp = nNextLevExp;

	nNextLevExp -= nCurLevExp;
	nCurExp -= nCurLevExp;

	bool bSetExpGauge = true;
	int nLimitLevel = (int)CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::PlayerLevelLimit );
	if( CDnActor::s_hLocalActor && CDnActor::s_hLocalActor->GetLevel() == nLimitLevel )
	{
		CDnLocalPlayerActor *pActor = (CDnLocalPlayerActor *)CDnActor::s_hLocalActor.GetPointer();
		if( pActor->GetAccountLevel() < AccountLevel_Master ) 
		{
			m_pTextBoxExpGaugeValue->ClearText();
			m_pTextBoxExpGaugeValue->AddText( L"Max" );
			bSetExpGauge = false;
		}
	}

	for( int i=0; i<5; i++ )
		m_pProgressExpGauge[i]->SetProgress( 0.0f );

	if( bSetExpGauge ) 
	{
		wchar_t szTemp[32] = {0};
		m_fCurrentExpPercent = nNextLevExp ? ((float)nCurExp / (float)nNextLevExp * 100.0f) : 0.0f;
		swprintf_s( szTemp, 32, L"%.2f%%", m_fCurrentExpPercent );
		m_pTextBoxExpGaugeValue->ClearText();
		m_pTextBoxExpGaugeValue->AddText( szTemp );

		int nMaxExp = nNextLevExp / 5;
		if( nMaxExp == 0 ) return;
		int nIndex = nCurExp / nMaxExp;
		int nExp = nCurExp % nMaxExp;

		for( int i=0; i<nIndex; i++ )
			m_pProgressExpGauge[i]->SetProgress( 100.0f );

		if( nIndex < 5 )
			m_pProgressExpGauge[nIndex]->SetProgress( ( nExp * 100.0f ) / nMaxExp );

		m_fExpGaugeDelta = 0.0f;
		m_bStartEffectExpectExp = false;
		m_nTempLevel = pActor->GetLevel();
		m_fStartGapTime = EFFECT_START_GAP_TIME;

		CaclExpectExpPercent( pActor, nNextLevExp, nCurExp );	// Calculate expected EXP %
	}
}

void CDnDungeonExpectRewardDlg::ClearDungeonExpectReward()
{
	for( int i=0; i<MAX_REWARD_ITEM; i++ )
	{
		if (m_pStaticRewardItem[i])
		{
			MIInventoryItem* pItem = m_pStaticRewardItem[i]->GetItem();
			if (pItem != NULL)
				SAFE_DELETE(pItem);

			m_pStaticRewardItem[i]->ResetSlot();
			m_pStaticRewardItem[i]->Show( false );
		}
	}

	m_pStaticGold->SetIntToText( 0 );
	m_pStaticSilver->SetIntToText( 0 );
	m_pStaticBronze->SetIntToText( 0 );

	m_pTextBoxExpGaugeValue->ClearText();
	for( int i=0; i<5; i++ )
		m_pProgressExpGauge[i]->SetProgress( 0.0f );

	m_fStartGapTime = 0.0f;
	m_bStartEffectExpectExp = false;
}

void CDnDungeonExpectRewardDlg::CaclExpectExpPercent( CDnPlayerActor* pActor, int nNextLevExp, int nCurExp )
{
	if( pActor == NULL )
		return;

	float fFinalExpectExp = pActor->GetExperience() + m_fExpectTotalExp;

	int nExperience = 0;
	int nExpectLevel = 0;
	int nOffset = pActor->GetLevel() - 1;

	if( nOffset < 0 ) nOffset = 0;

	for( int i=nOffset; i<PLAYER_MAX_LEVEL; i++ )
	{
		nExperience = CPlayerLevelTable::GetInstance().GetValue( pActor->GetJobClassID(), i + 1, CPlayerLevelTable::Experience );
		if( fFinalExpectExp >= nExperience )
			nExpectLevel = i + 1;
		else
			break;
	}

	if( nExpectLevel == pActor->GetLevel() )	// When not leveling up with expected EXP
	{
		m_fExpectExpPercent = ( m_fExpectTotalExp / nNextLevExp ) * 100.0f;
	}
	else if( nExpectLevel > pActor->GetLevel() )	// When leveling up with expected EXP
	{
		m_fExpectExpPercent = ( static_cast<float>( nNextLevExp - nCurExp ) / static_cast<float>( nNextLevExp ) ) * 100.0f;
		float fTempExp = m_fExpectTotalExp - ( nNextLevExp - nCurExp );
		for( int i=pActor->GetLevel(); i<PLAYER_MAX_LEVEL; i++ )
		{
			if( i+1 > PLAYER_MAX_LEVEL || i+2 > PLAYER_MAX_LEVEL )
				break;

			nExperience = CPlayerLevelTable::GetInstance().GetValue( pActor->GetJobClassID(), i + 1, CPlayerLevelTable::Experience );
			int nNextExperience = CPlayerLevelTable::GetInstance().GetValue( pActor->GetJobClassID(), i + 2, CPlayerLevelTable::Experience );

			if( nExperience >= nNextExperience )
				break;

			if( fTempExp <= nNextExperience - nExperience )
			{
				m_fExpectExpPercent += ( fTempExp / ( nNextExperience - nExperience ) ) * 100.0f;
				break;
			}
			else
			{
				m_fExpectExpPercent += 100.0f;
				fTempExp -= nNextExperience - nExperience;
			}
		}
	}
}

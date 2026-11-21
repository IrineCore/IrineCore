#include "stdafx.h"
#include "DnGuildWarBossGauge.h"
#include "PvPOccupationScoreSystem.h"
#include "DnMonsterActor.h"
#include "DnTableDB.h"
#include <MMSystem.h>

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnGuildWarBossGaugeDlg::CDnGuildWarBossGaugeDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback  )
, m_eType( BOSS_GAUGE )
, m_nPrevLayerCount(0)
, m_pProgressBoss(NULL)
, m_pProgressProp(NULL)
, m_pProgressSuper(NULL)
, m_pStaticBoss(NULL)
, m_pStaticBossBar(NULL)
, m_pStaticBossLayer(NULL)
, m_pStaticBossSuperBar(NULL)
, m_pStaticProp(NULL)
, m_pStaticPropBar(NULL)
, m_pStaticPropLayer(NULL)
{

}

CDnGuildWarBossGaugeDlg::~CDnGuildWarBossGaugeDlg()
{

}

void CDnGuildWarBossGaugeDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "GuildWarBossGauge.ui" ).c_str(), bShow );	
}

void CDnGuildWarBossGaugeDlg::InitialUpdate()
{
	m_pStaticBoss = GetControl<CEtUIStatic>( "ID_TEXT_BOSSNAME" );
	m_pStaticBossLayer = GetControl<CEtUIStatic>( "ID_TEXT_BOSSHP" );
	m_pStaticBossBar = GetControl<CEtUIStatic>( "ID_STATIC_BOSSGAUGEBAR0" );
	m_pStaticBossSuperBar = GetControl<CEtUIStatic>( "ID_STATIC_BOSSGAUGEBAR1" );
	m_pProgressBoss = GetControl<CEtUIProgressBar>( "ID_HP_BOSSGAUGE" );
	m_pProgressSuper = GetControl<CEtUIProgressBar>( "ID_SUPER_GAUGE" );

	m_pStaticProp = GetControl<CEtUIStatic>( "ID_TEXT_PROPNAME" );
	m_pStaticPropLayer = GetControl<CEtUIStatic>( "ID_TEXT_PROPHP" );
	m_pStaticPropBar = GetControl<CEtUIStatic>( "ID_STATIC_PROPGAUGEBAR" );
	m_pProgressProp = GetControl<CEtUIProgressBar>( "ID_HP_PROPGAUGE" );

	m_pProgressBoss->UseShadeProcessBar( 8, 7 );
	m_pProgressBoss->UseGaugeEndElement( 6 );
	m_pProgressProp->UseShadeProcessBar( 8, 7 );
	m_pProgressProp->UseGaugeEndElement( 6 );

	m_pProgressBoss->SetSmooth( true );
	m_pProgressSuper->SetSmooth( true );
	m_pProgressProp->SetSmooth( true );

	DNTableFileFormat*  pSox = GetDNTable( CDnTableDB::TBATTLEGROUNDMODE );
	char szColumName[32];

	for( int itr = 0; itr < PvPCommon::TeamIndex::Max; ++itr )
	{
		if( PvPCommon::TeamIndex::A == itr )
		{
			for( int jtr = 0; jtr < PvPCommon::Common::DoorCount; ++jtr )
			{
				sprintf_s( szColumName, _countof(szColumName), "_CosmosGate%dID", jtr+1 );
				m_DoorID[itr][jtr] = pSox->GetFieldFromLablePtr( 1, szColumName)->GetInteger();
			}

			sprintf_s( szColumName, _countof(szColumName), "_CosmosBossID" );
			m_BossID[itr] = pSox->GetFieldFromLablePtr( 1, szColumName)->GetInteger();
		}
		else if( PvPCommon::TeamIndex::B == itr )
		{
			for( int jtr = 0; jtr < PvPCommon::Common::DoorCount; ++jtr )
			{
				sprintf_s( szColumName, _countof(szColumName), "_ChaosGate%dID", jtr+1 );
				m_DoorID[itr][jtr] = pSox->GetFieldFromLablePtr( 1, szColumName)->GetInteger();
			}

			sprintf_s( szColumName, _countof(szColumName), "_ChaosBossID" );
			m_BossID[itr] = pSox->GetFieldFromLablePtr( 1, szColumName)->GetInteger();
		}
	}
}

void CDnGuildWarBossGaugeDlg::Show( bool bShow )
{
	CDnCustomDlg::Show( bShow );
}

void CDnGuildWarBossGaugeDlg::Process( float fElapsedTime )
{
	if( m_bShow )
	{
		if( m_LastTime + LAST_TIME < timeGetTime() )
			Show( false );
		else if( m_hActor && !m_hActor->IsDie() )
		{
			float fHP = (float)m_hActor->GetHP();
			float fMaxHP = (float)m_hActor->GetMaxHP();

			float fSuperArmor = (float)m_hActor->GetCurrentSuperArmor();
			float fMaxSuperArmor = (float)m_hActor->GetMaxSuperArmor();

			SetHP( fHP / fMaxHP );
			SetSP( fMaxSuperArmor <= 50.0f ? 0.0f : (fSuperArmor / fMaxSuperArmor) );	// Super armor
		}
		else
			Show( false );
	}

	CDnCustomDlg::Process(fElapsedTime);
}

void CDnGuildWarBossGaugeDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg /* = 0 */ )
{
	CDnCustomDlg::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

bool CDnGuildWarBossGaugeDlg::SetActor( DnMonsterActorHandle hActor )
{
	int nUnipueID = hActor->GetMonsterClassID();

	m_LastTime = timeGetTime();

	if( !hActor )
		return false;

	m_hActor = hActor;

	for( int itr = 0; itr < PvPCommon::TeamIndex::Max; ++itr )
	{
		if( m_BossID[itr] == nUnipueID )
		{
			SetControl( BOSS_GAUGE );
			return true;
		}

		for( int jtr = 0; jtr < PvPCommon::Common::DoorCount; ++jtr )
		{
			if( m_DoorID[itr][jtr] == nUnipueID )
			{
				SetControl( PROP_GAUGE );
				return true;
			}
		}
	}

	return false;
}

void CDnGuildWarBossGaugeDlg::SetControl( int eType )
{
	m_eType = eType;

	if( BOSS_GAUGE == m_eType )
	{
		m_pStaticBoss->Show( true );
		m_pStaticBossLayer->Show( true );
		m_pStaticBossBar->Show( true );
		m_pStaticBossSuperBar->Show( true );
		m_pProgressBoss->Show( true );
		m_pProgressSuper->Show( true );

		m_pStaticProp->Show( false );
		m_pStaticPropLayer->Show( false );
		m_pStaticPropBar->Show( false );
		m_pProgressProp->Show( false );

		m_pStaticBoss->SetText( m_hActor->GetName() );
	}
	else if( PROP_GAUGE == m_eType )
	{
		m_pStaticBoss->Show( false );
		m_pStaticBossLayer->Show( false );
		m_pStaticBossBar->Show( false );
		m_pStaticBossSuperBar->Show( false );
		m_pProgressBoss->Show( false );
		m_pProgressSuper->Show( false );

		m_pStaticProp->Show( true );
		m_pStaticPropLayer->Show( true );
		m_pStaticPropBar->Show( true );
		m_pProgressProp->Show( true );

		m_pStaticProp->SetText( m_hActor->GetName() );
	}
}

void CDnGuildWarBossGaugeDlg::SetHP( float fHP )
{
	int nSeparateCount = GetSeparateCount();
	int nLayerCount = EtMin( nSeparateCount-1, (int)(fHP * nSeparateCount) );

	if( m_nPrevLayerCount != nLayerCount  ) {
		switch( nSeparateCount ) {
				case 1: 
					{
						m_pProgressBoss->UseLayer( true, 1, 0 ); 
						m_pProgressProp->UseLayer( true, 1, 0 ); 
						break;
					}
				case 2: 
					{
						m_pProgressBoss->UseLayer( true, 2, 0, 1 ); 
						m_pProgressProp->UseLayer( true, 2, 0, 1 ); 
						break;
					}
				case 3: 
					{
						m_pProgressBoss->UseLayer( true, 3, 0, 1, 2 ); 
						m_pProgressProp->UseLayer( true, 3, 0, 1, 2 ); 
						break;
					}
				case 4: 
					{
						m_pProgressBoss->UseLayer( true, 4, 0, 1, 2, 3 ); 
						m_pProgressProp->UseLayer( true, 4, 0, 1, 2, 3 ); 
						break;
					}
				case 5: 
					{
						m_pProgressBoss->UseLayer( true, 5, 0, 1, 2, 3, 4 ); 
						m_pProgressProp->UseLayer( true, 5, 0, 1, 2, 3, 4 ); 
						break;
					}
				case 20: 
					{
						m_pProgressBoss->UseLayer( true, 20, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4 ); 
						m_pProgressProp->UseLayer( true, 20, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4 ); 
						break;
					}
		}

		if( nLayerCount == 0 ) {
			m_pStaticBossLayer->SetText( L"" );
			m_pStaticPropLayer->SetText( L"" );
		}
		else {
			m_pStaticBossLayer->SetText( FormatW(L"X%d", nLayerCount+1 ) );
			m_pStaticPropLayer->SetText( FormatW(L"X%d", nLayerCount+1 ) );
		}
	}
	m_nPrevLayerCount = nLayerCount;
	m_pProgressBoss->SetProgress( 100.0f * fHP );
	m_pProgressProp->SetProgress( 100.0f * fHP );
}

void CDnGuildWarBossGaugeDlg::SetSP( float fSP )
{
	m_pProgressSuper->SetProgress( fSP * 100.0f );

	if( m_hActor->GetMaxSuperArmor() <= 50.0f ) {
		m_pProgressSuper->SetImmediateChange();
	}

	int nMaxSuperArmor = m_hActor->GetMaxSuperArmor();
	if( nMaxSuperArmor <= 50 ) 
		m_pProgressSuper->SetProgressIndex( 0 );
	else if( nMaxSuperArmor <= 200 ) 
		m_pProgressSuper->SetProgressIndex( 1 );
	else if( nMaxSuperArmor <= 600 ) 
		m_pProgressSuper->SetProgressIndex( 2 );
	else if( nMaxSuperArmor <= 1800 ) 
		m_pProgressSuper->SetProgressIndex( 3 );
	else
		m_pProgressSuper->SetProgressIndex( 4 );
}

int CDnGuildWarBossGaugeDlg::GetSeparateCount()
{
	int nSeparateCount = 1;
	switch( m_hActor->GetGrade() ) {
		case CDnMonsterState::Named:
			nSeparateCount = 1;
			break;
		case CDnMonsterState::Boss:
			nSeparateCount = 5;
			break;
		case CDnMonsterState::BossHP4:
			nSeparateCount = 4;
			break;
		case CDnMonsterState::NestBoss:
			nSeparateCount = 5;
			break;
		case CDnMonsterState::NestBoss8:
			nSeparateCount = 20;
			break;
	}

	return nSeparateCount;
}

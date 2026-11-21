#include "StdAfx.h"

#if defined(PRE_ADD_TALISMAN_SYSTEM)

#include "DnCharTalismanInfoDlg.h"
#include "DnItemTask.h"
#include "DnPlayerActor.h"
#include "DnInterfaceString.h"
#include "DnUIString.h"
#include "DnTableDB.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

using namespace DN_INTERFACE::STRING;

CDnCharTalismanInfoDlg::CDnCharTalismanInfoDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
, m_pTextBox( NULL )
{
}

CDnCharTalismanInfoDlg::~CDnCharTalismanInfoDlg()
{
}

void CDnCharTalismanInfoDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "CharTalismanInfoDlg.ui" ).c_str(), bShow );
}

void CDnCharTalismanInfoDlg::InitialUpdate()
{
	m_pTextBox = GetControl<CEtUITextBox>( "ID_TEXTBOX_INFO" );
}

void CDnCharTalismanInfoDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	if( bShow && m_pTextBox )
		RequestFocus( m_pTextBox );

	CEtUIDialog::Show( bShow );
}

bool CDnCharTalismanInfoDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
		return false;

	return CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );
}

void CDnCharTalismanInfoDlg::SetText()
{
	if( !m_pTextBox ) return;

	m_pTextBox->ClearText();
	m_vFloatTypeList.clear();
	m_vIntTypeList.clear();
	m_vDoubleFloatTypeList.clear();
	m_vDoubleIntTypeList.clear();

	SetNormalInfo();
	SetDropInfo();
	SetSkillInfo();
}

void CDnCharTalismanInfoDlg::SetText( CDnPlayerActor *pActor )
{
	if( !m_pTextBox ) return;

	m_pTextBox->ClearText();
	m_vFloatTypeList.clear();
	m_vIntTypeList.clear();
	m_vDoubleFloatTypeList.clear();
	m_vDoubleIntTypeList.clear();

	if( !pActor ) return;

	SetNormalInfo( pActor );
	SetDropInfo( pActor );
	SetSkillInfo( pActor );
}

void CDnCharTalismanInfoDlg::AddStateText( int nStringID, int eType, float fFirstData, float fSecondData )
{
	bool bExist = false;
	switch( eType )
	{
	case FLOAT_TYPE:
		for( size_t i = 0; i < m_vFloatTypeList.size(); ++i )
		{
			if( m_vFloatTypeList[i].first == nStringID )
			{
				m_vFloatTypeList[i].second += fFirstData;
				bExist = true;
				break;
			}
		}
		if( !bExist )
			m_vFloatTypeList.push_back( std::make_pair( nStringID, fFirstData ) );
		break;

	case DOUBLE_FLOAT_TYPE:
		for( size_t i = 0; i < m_vDoubleFloatTypeList.size(); ++i )
		{
			if( m_vDoubleFloatTypeList[i].first == nStringID )
			{
				m_vDoubleFloatTypeList[i].second.first += fFirstData;
				m_vDoubleFloatTypeList[i].second.second += fSecondData;
				bExist = true;
				break;
			}
		}
		if( !bExist )
			m_vDoubleFloatTypeList.push_back( std::make_pair( nStringID, std::make_pair( fFirstData, fSecondData ) ) );
		break;

	case INT_TYPE:
		for( size_t i = 0; i < m_vIntTypeList.size(); ++i )
		{
			if( m_vIntTypeList[i].first == nStringID )
			{
				m_vIntTypeList[i].second += (int)fFirstData;
				bExist = true;
				break;
			}
		}
		if( !bExist )
			m_vIntTypeList.push_back( std::make_pair( nStringID, (int)fFirstData ) );
		break;

	case DOUBLE_INT_TYPE:
		for( size_t i = 0; i < m_vDoubleIntTypeList.size(); ++i )
		{
			if( m_vDoubleIntTypeList[i].first == nStringID )
			{
				m_vDoubleIntTypeList[i].second.first += (int)fFirstData;
				m_vDoubleIntTypeList[i].second.second += (int)fSecondData;
				bExist = true;
				break;
			}
		}
		if( !bExist )
			m_vDoubleIntTypeList.push_back( std::make_pair( nStringID, std::make_pair( (int)fFirstData, (int)fSecondData ) ) );
		break;
	}
}

void CDnCharTalismanInfoDlg::SetItemStateText( CDnState *pState )
{
	if( !pState ) return;

	if( pState->GetAttackPMin() > 0 || pState->GetAttackPMax() > 0 )
		AddStateText( 5018, DOUBLE_INT_TYPE, (float)pState->GetAttackPMin(), (float)pState->GetAttackPMax() );
	if( pState->GetAttackMMin() > 0 || pState->GetAttackMMax() > 0 )
		AddStateText( 5019, DOUBLE_INT_TYPE, (float)pState->GetAttackMMin(), (float)pState->GetAttackMMax() );
	if( pState->GetDefenseP() > 0 )
		AddStateText( 5020, INT_TYPE, (float)pState->GetDefenseP() );
	if( pState->GetDefenseM() > 0 )
		AddStateText( 5062, INT_TYPE, (float)pState->GetDefenseM() );

	if( pState->GetStrength() > 0 )
		AddStateText( ITEM::STATIC_2_INDEX(emTOOLTIP_STATIC::ITEM_STRENGTH), INT_TYPE, (float)pState->GetStrength() );
	if( pState->GetAgility() > 0 )
		AddStateText( ITEM::STATIC_2_INDEX(emTOOLTIP_STATIC::ITEM_AGILITY), INT_TYPE, (float)pState->GetAgility() );
	if( pState->GetIntelligence() > 0 )
		AddStateText( ITEM::STATIC_2_INDEX(emTOOLTIP_STATIC::ITEM_INTELLIGENCE), INT_TYPE, (float)pState->GetIntelligence() );
	if( pState->GetStamina() > 0 )
		AddStateText( ITEM::STATIC_2_INDEX(emTOOLTIP_STATIC::ITEM_STAMINA), INT_TYPE, (float)pState->GetStamina() );
	if( pState->GetSpirit() > 0 )
		AddStateText( ITEM::STATIC_2_INDEX(emTOOLTIP_STATIC::ITEM_SPIRIT), INT_TYPE, (float)pState->GetSpirit() );

	if( pState->GetStiff() > 0 )
		AddStateText( ITEM::STATIC_2_INDEX(emTOOLTIP_STATIC::ITEM_STIFFNESS_P), INT_TYPE, (float)pState->GetStiff() );
	if( pState->GetStiffResistance() > 0 )
		AddStateText( ITEM::STATIC_2_INDEX(emTOOLTIP_STATIC::ITEM_STIFFNESS_R), INT_TYPE, (float)pState->GetStiffResistance() );
	if( pState->GetStun() > 0 )
		AddStateText( ITEM::STATIC_2_INDEX(emTOOLTIP_STATIC::ITEM_STUN_P), INT_TYPE, (float)pState->GetStun() );
	if( pState->GetStunResistance() > 0 )
		AddStateText( ITEM::STATIC_2_INDEX(emTOOLTIP_STATIC::ITEM_STUN_R), INT_TYPE, (float)pState->GetStunResistance() );
	if( pState->GetCritical() > 0 )
		AddStateText( ITEM::STATIC_2_INDEX(emTOOLTIP_STATIC::ITEM_CRITICAL_P), INT_TYPE, (float)pState->GetCritical() );
	if( pState->GetCriticalResistance() > 0 )
		AddStateText( ITEM::STATIC_2_INDEX(emTOOLTIP_STATIC::ITEM_CRITICAL_R), INT_TYPE, (float)pState->GetCriticalResistance() );
	if( pState->GetMoveSpeed() > 0 )
		AddStateText( ITEM::STATIC_2_INDEX(emTOOLTIP_STATIC::ITEM_MOVE), INT_TYPE, (float)pState->GetMoveSpeed() );
	if( pState->GetSafeZoneMoveSpeed() > 0 )
		AddStateText( ITEM::STATIC_2_INDEX(emTOOLTIP_STATIC::ITEM_SAFEZONEMOVE), INT_TYPE, (float)pState->GetSafeZoneMoveSpeed() );
	if( pState->GetMaxHP() > 0 )
		AddStateText( 2254, INT_TYPE, (float)pState->GetMaxHP() );
	if( pState->GetMaxSP() > 0 )
		AddStateText( 2255, INT_TYPE, (float)pState->GetMaxSP() );
	if( pState->GetRecoverySP() > 0 )
		AddStateText( 2256, INT_TYPE, (float)pState->GetRecoverySP() );
	if( pState->GetSuperAmmor() > 0 )
		AddStateText( 2279, INT_TYPE, (float)pState->GetSuperAmmor() );
	if( pState->GetFinalDamage() > 0 )
		AddStateText( 2280, INT_TYPE, (float)pState->GetFinalDamage() );

	// Ratios
	if( pState->GetAttackPMinRatio() > 0 || pState->GetAttackPMaxRatio() > 0 )
		AddStateText( 5018, DOUBLE_FLOAT_TYPE, pState->GetAttackPMinRatio() * 100.0f, pState->GetAttackPMaxRatio() * 100.0f );
	if( pState->GetAttackMMinRatio() > 0 || pState->GetAttackMMaxRatio() > 0 )
		AddStateText( 5019, DOUBLE_FLOAT_TYPE, pState->GetAttackMMinRatio() * 100.0f, pState->GetAttackMMaxRatio() * 100.0f );
	if( pState->GetDefensePRatio() > 0 )
		AddStateText( 5020, FLOAT_TYPE, pState->GetDefensePRatio() * 100.0f );
	if( pState->GetDefenseMRatio() > 0 )
		AddStateText( 5062, FLOAT_TYPE, pState->GetDefenseMRatio() * 100.0f );

	if( pState->GetStrengthRatio() > 0 )
		AddStateText( ITEM::STATIC_2_INDEX(emTOOLTIP_STATIC::ITEM_STRENGTH), FLOAT_TYPE, pState->GetStrengthRatio() * 100.0f );
	if( pState->GetAgilityRatio() > 0 )
		AddStateText( ITEM::STATIC_2_INDEX(emTOOLTIP_STATIC::ITEM_AGILITY), FLOAT_TYPE, pState->GetAgilityRatio() * 100.0f );
	if( pState->GetIntelligenceRatio() > 0 )
		AddStateText( ITEM::STATIC_2_INDEX(emTOOLTIP_STATIC::ITEM_INTELLIGENCE), FLOAT_TYPE, pState->GetIntelligenceRatio() * 100.0f );
	if( pState->GetStaminaRatio() > 0 )
		AddStateText( ITEM::STATIC_2_INDEX(emTOOLTIP_STATIC::ITEM_STAMINA), FLOAT_TYPE, pState->GetStaminaRatio() * 100.0f );
	if( pState->GetSpiritRatio() > 0 )
		AddStateText( ITEM::STATIC_2_INDEX(emTOOLTIP_STATIC::ITEM_SPIRIT), FLOAT_TYPE, pState->GetSpiritRatio() * 100.0f );

	if( pState->GetStiffRatio() > 0 )
		AddStateText( ITEM::STATIC_2_INDEX(emTOOLTIP_STATIC::ITEM_STIFFNESS_P), FLOAT_TYPE, pState->GetStiffRatio() * 100.0f );
	if( pState->GetStiffResistanceRatio() > 0 )
		AddStateText( ITEM::STATIC_2_INDEX(emTOOLTIP_STATIC::ITEM_STIFFNESS_R), FLOAT_TYPE, pState->GetStiffResistanceRatio() * 100.0f );
	if( pState->GetStunRatio() > 0 )
		AddStateText( ITEM::STATIC_2_INDEX(emTOOLTIP_STATIC::ITEM_STUN_P), FLOAT_TYPE, pState->GetStunRatio() * 100.0f );
	if( pState->GetStunResistanceRatio() > 0 )
		AddStateText( ITEM::STATIC_2_INDEX(emTOOLTIP_STATIC::ITEM_STUN_R), FLOAT_TYPE, pState->GetStunResistanceRatio() * 100.0f );
	if( pState->GetCriticalRatio() > 0 )
		AddStateText( ITEM::STATIC_2_INDEX(emTOOLTIP_STATIC::ITEM_CRITICAL_P), FLOAT_TYPE, pState->GetCriticalRatio() * 100.0f );
	if( pState->GetCriticalResistanceRatio() > 0 )
		AddStateText( ITEM::STATIC_2_INDEX(emTOOLTIP_STATIC::ITEM_CRITICAL_R), FLOAT_TYPE, pState->GetCriticalResistanceRatio() * 100.0f );
	if( pState->GetMoveSpeedRatio() > 0 )
		AddStateText( ITEM::STATIC_2_INDEX(emTOOLTIP_STATIC::ITEM_MOVE), FLOAT_TYPE, pState->GetMoveSpeedRatio() * 100.0f );
	if( pState->GetSafeZoneMoveSpeedRatio() > 0 )
		AddStateText( ITEM::STATIC_2_INDEX(emTOOLTIP_STATIC::ITEM_SAFEZONEMOVE), FLOAT_TYPE, pState->GetSafeZoneMoveSpeedRatio() * 100.0f );
	if( pState->GetMaxHPRatio() > 0 )
		AddStateText( 2254, FLOAT_TYPE, pState->GetMaxHPRatio() * 100.0f );
	if( pState->GetMaxSPRatio() > 0 )
		AddStateText( 2255, FLOAT_TYPE, pState->GetMaxSPRatio() * 100.0f );
	if( pState->GetRecoverySPRatio() > 0 )
		AddStateText( 2256, FLOAT_TYPE, pState->GetRecoverySPRatio() * 100.0f );
	if( pState->GetSuperAmmorRatio() > 0 )
		AddStateText( 2279, FLOAT_TYPE, pState->GetSuperAmmorRatio() * 100.0f );
	if( pState->GetFinalDamageRatio() > 0 )
		AddStateText( 2280, FLOAT_TYPE, pState->GetFinalDamageRatio() * 100.0f );

	if( pState->GetElementAttack( CDnState::Light ) > 0 )
		AddStateText( ITEM::STATIC_2_INDEX(emTOOLTIP_STATIC::ITEM_LIGHT_A), FLOAT_TYPE, pState->GetElementAttack( CDnState::Light ) * 100.0f );
	if( pState->GetElementAttack( CDnState::Dark ) > 0 )
		AddStateText( ITEM::STATIC_2_INDEX(emTOOLTIP_STATIC::ITEM_DARK_A), FLOAT_TYPE, pState->GetElementAttack( CDnState::Dark ) * 100.0f );
	if( pState->GetElementAttack( CDnState::Fire ) > 0 )
		AddStateText( ITEM::STATIC_2_INDEX(emTOOLTIP_STATIC::ITEM_FIRE_A), FLOAT_TYPE, pState->GetElementAttack( CDnState::Fire ) * 100.0f );
	if( pState->GetElementAttack( CDnState::Ice ) > 0 )
		AddStateText( ITEM::STATIC_2_INDEX(emTOOLTIP_STATIC::ITEM_ICE_A), FLOAT_TYPE, pState->GetElementAttack( CDnState::Ice ) * 100.0f );

	if( pState->GetElementDefense( CDnState::Light ) > 0 )
		AddStateText( ITEM::STATIC_2_INDEX(emTOOLTIP_STATIC::ITEM_LIGHT_R), FLOAT_TYPE, pState->GetElementDefense( CDnState::Light ) * 100.0f );
	if( pState->GetElementDefense( CDnState::Dark ) > 0 )
		AddStateText( ITEM::STATIC_2_INDEX(emTOOLTIP_STATIC::ITEM_DARK_R), FLOAT_TYPE, pState->GetElementDefense( CDnState::Dark ) * 100.0f );
	if( pState->GetElementDefense( CDnState::Fire ) > 0 )
		AddStateText( ITEM::STATIC_2_INDEX(emTOOLTIP_STATIC::ITEM_FIRE_R), FLOAT_TYPE, pState->GetElementDefense( CDnState::Fire ) * 100.0f );
	if( pState->GetElementDefense( CDnState::Ice ) > 0 )
		AddStateText( ITEM::STATIC_2_INDEX(emTOOLTIP_STATIC::ITEM_ICE_R), FLOAT_TYPE, pState->GetElementDefense( CDnState::Ice ) * 100.0f );
}

void CDnCharTalismanInfoDlg::FlushAccumulatedStats()
{
	WCHAR szTemp[256] = {0,};
	for( size_t i = 0; i < m_vIntTypeList.size(); ++i )
	{
		swprintf_s( szTemp, _countof(szTemp), L"%s : %d", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, m_vIntTypeList[i].first ), m_vIntTypeList[i].second );
		m_pTextBox->AddText( szTemp );
	}
	for( size_t i = 0; i < m_vDoubleIntTypeList.size(); ++i )
	{
		swprintf_s( szTemp, _countof(szTemp), L"%s : %d%s%d", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, m_vDoubleIntTypeList[i].first ), m_vDoubleIntTypeList[i].second.first, TILDE, m_vDoubleIntTypeList[i].second.second );
		m_pTextBox->AddText( szTemp );
	}
	for( size_t i = 0; i < m_vFloatTypeList.size(); ++i )
	{
		swprintf_s( szTemp, _countof(szTemp), L"%s : %.1f%%", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, m_vFloatTypeList[i].first ), m_vFloatTypeList[i].second );
		m_pTextBox->AddText( szTemp );
	}
	for( size_t i = 0; i < m_vDoubleFloatTypeList.size(); ++i )
	{
		swprintf_s( szTemp, _countof(szTemp), L"%s : %.1f%%%s%.1f%%", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, m_vDoubleFloatTypeList[i].first ), m_vDoubleFloatTypeList[i].second.first, TILDE, m_vDoubleFloatTypeList[i].second.second );
		m_pTextBox->AddText( szTemp );
	}
}

void CDnCharTalismanInfoDlg::AddTalismanExplain( CDnTalisman *pTalisman, float fEfficiency )
{
	if( !pTalisman || !m_pTextBox ) return;
	std::wstring wszExplain;
	pTalisman->GetTalismanExplainString( wszExplain, fEfficiency );
	if( !wszExplain.empty() )
	{
		m_pTextBox->AddColorText( wszExplain.c_str() );
	}
}

void CDnCharTalismanInfoDlg::SetNormalInfo()
{
	if( !CDnItemTask::IsActive() ) return;
	CDnPlayerActor *pLocalActor = CDnActor::s_hLocalActor ? dynamic_cast<CDnPlayerActor*>( CDnActor::s_hLocalActor.GetPointer() ) : NULL;

	bool bAddedHeader = false;
	for( int i = 0; i < TALISMAN_MAX; ++i )
	{
		CDnTalisman *pTalisman = dynamic_cast<CDnTalisman*>( GetItemTask().GetTalismanItem( i ) );
		if( !pTalisman || pTalisman->GetTalismanType() != CDnTalisman::Normal )
			continue;
		if( pTalisman->IsExpireItem() )
			continue;
		if( i <= TALISMAN_CASH_MAX && !GetItemTask().IsCashTalismanOpen() )
			continue;

		if( !bAddedHeader )
		{
			if( !m_pTextBox->IsEmpty() )
				m_pTextBox->AddText( L"" );
			m_pTextBox->AddText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8493 ), textcolor::FONT_ORANGE );
			bAddedHeader = true;
		}

		CDnState *pScaled = pLocalActor ? pLocalActor->GetTalismanScaledState( i ) : NULL;
		SetItemStateText( pScaled ? pScaled : pTalisman );
	}

	FlushAccumulatedStats();
}

void CDnCharTalismanInfoDlg::SetDropInfo()
{
	if( !CDnItemTask::IsActive() ) return;

	bool bAddedHeader = false;
	for( int i = 0; i < TALISMAN_MAX; ++i )
	{
		CDnTalisman *pTalisman = dynamic_cast<CDnTalisman*>( GetItemTask().GetTalismanItem( i ) );
		if( !pTalisman || pTalisman->GetTalismanType() != CDnTalisman::Drop )
			continue;
		if( pTalisman->IsExpireItem() )
			continue;
		if( i <= TALISMAN_CASH_MAX && !GetItemTask().IsCashTalismanOpen() )
			continue;

		if( !bAddedHeader )
		{
			if( !m_pTextBox->IsEmpty() )
				m_pTextBox->AddText( L"" );
			m_pTextBox->AddText( L"" );
			m_pTextBox->AddText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8492 ), textcolor::FONT_ORANGE );
			bAddedHeader = true;
		}

		AddTalismanExplain( pTalisman, GetItemTask().GetTalismanEfficiency( i ) );
	}
}

void CDnCharTalismanInfoDlg::SetSkillInfo()
{
	if( !CDnItemTask::IsActive() ) return;

	bool bAddedHeader = false;
	for( int i = 0; i < TALISMAN_MAX; ++i )
	{
		CDnTalisman *pTalisman = dynamic_cast<CDnTalisman*>( GetItemTask().GetTalismanItem( i ) );
		if( !pTalisman || pTalisman->GetTalismanType() != CDnTalisman::Skill )
			continue;
		if( pTalisman->IsExpireItem() )
			continue;
		if( i <= TALISMAN_CASH_MAX && !GetItemTask().IsCashTalismanOpen() )
			continue;

		if( !bAddedHeader )
		{
			if( !m_pTextBox->IsEmpty() )
				m_pTextBox->AddText( L"" );
			m_pTextBox->AddText( L"" );
			m_pTextBox->AddText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8491 ), textcolor::FONT_ORANGE );
			bAddedHeader = true;
		}

		AddTalismanExplain( pTalisman, GetItemTask().GetTalismanEfficiency( i ) );
		m_pTextBox->AddText( L"" );
	}
}

void CDnCharTalismanInfoDlg::SetNormalInfo( CDnPlayerActor *pActor )
{
	if( !pActor ) return;

	bool bAddedHeader = false;
	for( int i = 0; i < TALISMAN_MAX; ++i )
	{
		DnTalismanHandle hTalisman = pActor->GetTalisman( i );
		if( !hTalisman || hTalisman->GetTalismanType() != CDnTalisman::Normal )
			continue;
		if( hTalisman->IsExpireItem() )
			continue;

		if( !bAddedHeader )
		{
			if( !m_pTextBox->IsEmpty() )
				m_pTextBox->AddText( L"" );
			m_pTextBox->AddText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8493 ), textcolor::FONT_ORANGE );
			bAddedHeader = true;
		}

		CDnState *pScaled = pActor->GetTalismanScaledState( i );
		SetItemStateText( pScaled ? pScaled : hTalisman.GetPointer() );
	}

	FlushAccumulatedStats();
}

void CDnCharTalismanInfoDlg::SetDropInfo( CDnPlayerActor *pActor )
{
	if( !pActor ) return;

	bool bAddedHeader = false;
	for( int i = 0; i < TALISMAN_MAX; ++i )
	{
		DnTalismanHandle hTalisman = pActor->GetTalisman( i );
		if( !hTalisman || hTalisman->GetTalismanType() != CDnTalisman::Drop )
			continue;
		if( hTalisman->IsExpireItem() )
			continue;

		if( !bAddedHeader )
		{
			if( !m_pTextBox->IsEmpty() )
				m_pTextBox->AddText( L"" );
			m_pTextBox->AddText( L"" );
			m_pTextBox->AddText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8492 ), textcolor::FONT_ORANGE );
			bAddedHeader = true;
		}

		AddTalismanExplain( hTalisman.GetPointer(), pActor->GetTalismanEfficiency( i ) );
	}
}

void CDnCharTalismanInfoDlg::SetSkillInfo( CDnPlayerActor *pActor )
{
	if( !pActor ) return;

	bool bAddedHeader = false;
	for( int i = 0; i < TALISMAN_MAX; ++i )
	{
		DnTalismanHandle hTalisman = pActor->GetTalisman( i );
		if( !hTalisman || hTalisman->GetTalismanType() != CDnTalisman::Skill )
			continue;
		if( hTalisman->IsExpireItem() )
			continue;

		if( !bAddedHeader )
		{
			if( !m_pTextBox->IsEmpty() )
				m_pTextBox->AddText( L"" );
			m_pTextBox->AddText( L"" );
			m_pTextBox->AddText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8491 ), textcolor::FONT_ORANGE );
			bAddedHeader = true;
		}

		AddTalismanExplain( hTalisman.GetPointer(), pActor->GetTalismanEfficiency( i ) );
		m_pTextBox->AddText( L"" );
	}
}

#endif // PRE_ADD_TALISMAN_SYSTEM

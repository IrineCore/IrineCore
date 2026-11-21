#include "StdAfx.h"
#include "DnCharStatusBaseInfoDlg.h"
#include "DnGuildTask.h"
#include "DnPlayerActor.h"
#include "DnInterfaceString.h"
#include "DnTableDB.h"
#include "GameOption.h"
#include "DnGameTask.h"
#include "TaskManager.h"
#include "DnBlow.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnCharStatusBaseInfoDlg::CDnCharStatusBaseInfoDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback ) 
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
{
	m_bActorStateRefresh = true;

	m_pStaticClass = NULL;
	m_pStaticLevel = NULL;
	m_pStaticHP = NULL;
	m_pStaticMP = NULL;
#if defined(PRE_ADD_CHAR_STATUS_SECOND_RENEW)
	m_pStaticMPRegen = NULL;
#else
	m_pStaticGuild = NULL;
#endif

#ifdef PRE_ADD_CHAR_SATUS_RENEW
	m_pPopUpDlg = NULL;

	for(int nPage=0; nPage<eSection::SECTIONMAX; nPage++ )
	{
		m_pButtonChangePage[nPage] = NULL;
		m_pStaticSectionName[nPage] = NULL;

		for(int nSize = 0; nSize< eSection::INFOSIZE; nSize++ )
		{
			m_pStaticInfo[nPage][nSize] = NULL;
			m_pStaticInfoName[nPage][nSize] = NULL;
		}
	}
	m_nEnablePage[eSection::PRIMARY] = ePageType::STATUS;
	m_nEnablePage[eSection::SECONDARY] = ePageType::GENERAL;
#endif

}

CDnCharStatusBaseInfoDlg::~CDnCharStatusBaseInfoDlg(void)
{
#ifdef PRE_ADD_CHAR_SATUS_RENEW
	SAFE_DELETE(m_pPopUpDlg);
#endif
}

void CDnCharStatusBaseInfoDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "CharStatusBaseInfoDlg.ui" ).c_str(), bShow );
}

void CDnCharStatusBaseInfoDlg::InitialUpdate()
{
	m_pStaticClass = GetControl<CEtUIStatic>("ID_STATIC_CLASS");
	m_pStaticLevel = GetControl<CEtUIStatic>("ID_STATIC_LEVEL");
	m_pStaticHP = GetControl<CDnCharStatusStatic>("ID_STATIC_HP");
	m_pStaticMP = GetControl<CDnCharStatusStatic>("ID_STATIC_MP");

#if defined(PRE_ADD_CHAR_STATUS_SECOND_RENEW)
	m_pStaticMPRegen = GetControl<CDnCharStatusStatic>("ID_STATIC_GUILD");
	CEtUIStatic *pStaticMPRegenTitle = GetControl<CEtUIStatic>("ID_STATIC5");
	if( pStaticMPRegenTitle ) 
	{
		pStaticMPRegenTitle->SetText( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 2182 ) );
		pStaticMPRegenTitle->SetTooltipText( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 5195 ) );
	}
#else
	m_pStaticGuild = GetControl<CEtUIStatic>("ID_STATIC_GUILD");
#endif

#ifdef PRE_ADD_CHAR_SATUS_RENEW
	
	m_pPopUpDlg = new CDnCharStatusBasePopDlg(UI_TYPE_CHILD,this);
	m_pPopUpDlg->Initialize(false);
	
	std::string strUIName;
	for(int nPage=0; nPage<eSection::SECTIONMAX; nPage++ )
	{
		strUIName = FormatA("ID_BT_OPTION%d", nPage );
		m_pButtonChangePage[nPage] =  GetControl<CEtUIButton>(strUIName.c_str());
		
		strUIName = FormatA("ID_TEXT_TITLE%d", nPage );
		m_pStaticSectionName[nPage] = GetControl<CEtUIStatic>(strUIName.c_str());

		for(int nSize = 0; nSize < eSection::INFOSIZE; nSize++ )
		{
			strUIName = FormatA("ID_TEXT_COUNT%d", (eSection::INFOSIZE * nPage) + nSize );
			m_pStaticInfo[nPage][nSize] =  GetControl<CDnCharStatusStatic>(strUIName.c_str());

			strUIName = FormatA("ID_TEXT%d", (eSection::INFOSIZE * nPage) + nSize );
			m_pStaticInfoName[nPage][nSize] = GetControl<CEtUIStatic>(strUIName.c_str());
		}
	}

	if( CGameOption::GetInstance().m_nCharStatusSecondaryPage == 0 
		&& CGameOption::GetInstance().m_nCharStatusSecondaryPage == 0)
	{
		CGameOption::GetInstance().m_nCharStatusPrimaryPage = ePageType::STATUS;
		CGameOption::GetInstance().m_nCharStatusSecondaryPage = ePageType::GENERAL;
		CGameOption::GetInstance().Save();
	}

	EnableSatusPage( eSection::PRIMARY , CGameOption::GetInstance().m_nCharStatusPrimaryPage );
	EnableSatusPage( eSection::SECONDARY , CGameOption::GetInstance().m_nCharStatusSecondaryPage );
#endif

}

void CDnCharStatusBaseInfoDlg::Show( bool bShow )
{
	if( bShow != m_bShow && bShow == true )
	{
		m_bActorStateRefresh = true;
	}

	CDnCustomDlg::Show( bShow );
}

void CDnCharStatusBaseInfoDlg::Render( float fElapsedTime )
{
	if( !IsShow() )		
		return;

	if( !CDnActor::s_hLocalActor || !CDnActor::s_hLocalActor->IsPlayerActor())
		return;

#if !defined(PRE_ADD_CHAR_STATUS_SECOND_RENEW)
	DnActorHandle hActor = CDnActor::s_hLocalActor;
	CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor *>(hActor.GetPointer());

	m_pStaticClass->SetText( pPlayer->GetJobName() );
	m_pStaticLevel->SetIntToText( hActor->GetLevel() );
	m_pStaticHP->SetValue( CDnCharStatusStatic::TYPE_02, false, hActor->GetHP(), hActor->GetMaxHP() );
	m_pStaticMP->SetValue( CDnCharStatusStatic::TYPE_02, false, hActor->GetSP(), hActor->GetMaxSP() );

	if( pPlayer->IsJoinGuild() )
		m_pStaticGuild->SetText( pPlayer->GetGuildSelfView().wszGuildName );
	else
		m_pStaticGuild->SetText( L"" );
#endif


#ifdef PRE_ADD_CHAR_SATUS_RENEW
	for(int nSection = 0; nSection < eSection::SECTIONMAX ; nSection++ )
		RenderStatusInfo( m_nEnablePage[nSection] , nSection , m_bActorStateRefresh );

	if( m_bActorStateRefresh )
		m_bActorStateRefresh = false;
#endif

	CEtUIDialog::Render( fElapsedTime );
}

void CDnCharStatusBaseInfoDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
	
	SetCmdControlName( pControl->GetControlName() );
	if( nCommand == EVENT_BUTTON_CLICKED )
	{

#ifdef PRE_ADD_CHAR_SATUS_RENEW
		for(int nSection = 0; nSection < eSection::SECTIONMAX ; nSection++ )
		{
			if( IsCmdControl( FormatA("ID_BT_OPTION%d", nSection ).c_str() ) )
			{
				SUICoord uiCoord;
				SUICoord uiMainCoord;
				m_pButtonChangePage[nSection]->GetUICoord(uiCoord);

				static float fyBGap(24.f /DEFAULT_UI_SCREEN_WIDTH);  

				m_pPopUpDlg->SetPosition( uiCoord.fX + CEtUIDialog::GetXCoord() + fyBGap , uiCoord.fY + CEtUIDialog::GetYCoord() );
				m_pPopUpDlg->SetEnabledPage( nSection );
				m_pPopUpDlg->Show(true);
			}
		}
#endif
	}
}

bool CDnCharStatusBaseInfoDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{

	if( !IsShow() )
	{
		return false;
	}	

	switch( uMsg )
	{

	case WM_LBUTTONDOWN:
		{
			POINT MousePoint;
			float fMouseX, fMouseY;

			MousePoint.x = short( LOWORD( lParam ) );
			MousePoint.y = short( HIWORD( lParam ) );
			PointToFloat( MousePoint, fMouseX, fMouseY );

#ifdef PRE_ADD_CHAR_SATUS_RENEW
			if( m_pPopUpDlg->IsShow() ) {
				SUICoord uiCoord;
				m_pPopUpDlg->GetDlgCoord( uiCoord );
				fMouseX = MousePoint.x / GetScreenWidth();
				fMouseY = MousePoint.y / GetScreenHeight();
				if( !uiCoord.IsInside( fMouseX, fMouseY ) ) {
					ShowChildDialog( m_pPopUpDlg, false );
					return true;
				}
			}
#endif

		}
		break;

	}

	return CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );
}

void CDnCharStatusBaseInfoDlg::OnRefreshPlayerStatus()
{
	m_bActorStateRefresh = true;
}

#ifdef PRE_ADD_CHAR_SATUS_RENEW

void CDnCharStatusBaseInfoDlg::EnableSatusPage( int nPage , int nType )
{
	if( nPage >= eSection::SECTIONMAX || nPage < eSection::PRIMARY )
		return;

	int arrTitleString[ePageType::MAX] = 	{ 
		1000037816 , 1360,	1000037817 , 1000037818 , 1000037819 , 1000037820 , 1000037821 ,  // Stats / Normal / Special Attack / Special Defense / Elemental Attack / Elemental Defense / Misc
	};

	int arrNameString[eSection::MAX_ELEMENT_SIZE] = 	{ 
		2140, 2142, 2141, 2143, // Status
		5018, 5019, 5020, 5062, // General
		5023, 5022, 5021, 2280, // SPECIAL_ATTACK
		  28,   30,   26,    0, // SPECIAL_Defense (Using 3 Types )
		5029, 5030, 5027, 5028, // Element Attack
		5033, 5034, 5031, 5032, // Element Defense
		  24, 0,       0,    0, // ETC Info ( Using 3 Types ) 
	};

	int arrTipString[eSection::MAX_ELEMENT_SIZE] = 	{ 
		1000038111, 1000038112, 1000038113, 1000038114, // Status 
		1000038118, 1000038119, 1000038128, 1000038129, // General
		      5043,       5042,       5041, 1000038127, // SPECIAL_ATTACK
		      5046,       5045,       5044,          0, // SPECIAL_Defense (Using 3 Types )
		      5047,       5048,       5049,       5050, // Element Attack
		      5051,       5052,       5053,       5054, // Element Defense
		1000038117,          0,          0,          0, // ETC Info ( Using 3 Types ) 
	};

	int nStringStartCount = nType * INFOSIZE;

	for(int i=0; i < eSection::INFOSIZE; i++)
	{
		if(arrNameString[nStringStartCount+i] == 0)
			m_pStaticInfoName[nPage][i]->Show(false);
		else
		{
			m_pStaticInfoName[nPage][i]->SetText( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, arrNameString[nStringStartCount+i]) );
			m_pStaticInfoName[nPage][i]->Show(true);
			m_pStaticInfoName[nPage][i]->SetTooltipText( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, arrTipString[nStringStartCount+i]) );
		}
	}

	m_pStaticSectionName[nPage]->SetText(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, arrTitleString[nType]));

	for(int n=0; n< eSection::INFOSIZE; n++)
	{
		m_pStaticInfo[nPage][n]->Show(true);
		m_pStaticInfo[nPage][n]->ClearTooltipText();
		m_pStaticInfo[nPage][n]->ClearValue();
		m_pStaticInfo[nPage][n]->ClearDetailString();
	}

	m_nEnablePage[nPage] = nType;
	m_bActorStateRefresh = true;
}

bool CDnCharStatusBaseInfoDlg::CanRefreshStatus( int nType , bool bActorStatusRefresh )
{
	switch( nType )
	{
	case SIDE_INFO:
		{
			// PlayerMoveSpeed changes independently of ActorState; e.g., if other parameters are added to SideInfo later 
			// If another type of group is added, change and apply the current or another method; implementing simply for now as it is minor.
			return true; 
		}
		break;
	}

	return bActorStatusRefresh;
}

void CDnCharStatusBaseInfoDlg::RenderStatusInfo( int nType, int nPage , bool bActorStatusRefresh )
{
	if( nPage >= eSection::SECTIONMAX || nPage < eSection::PRIMARY )
		return;

#ifdef PRE_FIX_CHARSTATUS_REFRESH
	if( !CanRefreshStatus( nType , bActorStatusRefresh ) )
		return;
#endif

	if( !CDnActor::s_hLocalActor || !CDnActor::s_hLocalActor->IsPlayerActor())
		return;

	DnPlayerActorHandle hActor = CDnActor::s_hLocalActor;
	CDnState pState = *hActor->GetBaseState();
	CDnState pTotalState = *hActor->GetStateStep(0);
	CDnPlayerState *pPlayerState = static_cast<CDnPlayerState*>(hActor.GetPointer());
#if defined(PRE_ADD_SKILLBUF_RENEW) && defined(_CLIENT) && defined(PRE_ADD_STATUS_BUFF_INFORMATION)
	CDnState *pBuffState = (CDnState *)hActor->GetBuffState();
#endif


#if defined(PRE_ADD_CHAR_STATUS_SECOND_RENEW)
	m_pStaticClass->SetText( hActor->GetJobName() );
	m_pStaticLevel->SetIntToText( hActor->GetLevel() );

	m_pStaticHP->SetValue( CDnCharStatusStatic::TYPE_02, false, (int)hActor->GetHP(), (int)hActor->GetMaxHP() );
	m_pStaticMP->SetValue( CDnCharStatusStatic::TYPE_02, false, hActor->GetSP(), hActor->GetMaxSP() );
	m_pStaticMPRegen->SetValue( CDnCharStatusStatic::TYPE_02, false, hActor->GetSP(), hActor->GetMaxSP() );

#if defined(PRE_ADD_SKILLBUF_RENEW) && defined(_CLIENT) && defined(PRE_ADD_STATUS_BUFF_INFORMATION)
	m_pStaticHP->AddBuffValue( (int)pBuffState->GetMaxHP() );
	m_pStaticMP->AddBuffValue( pBuffState->GetMaxSP() );
	m_pStaticMPRegen->AddBuffValue( pBuffState->GetRecoverySP() );
#endif

#ifdef PRE_FIX_STATUS_USE_BASE_REGULATION
	pPlayerState->OnCalcEquipStep( CDnPlayerState::ST_HP, &pState );
	pPlayerState->OnCalcEquipStep( CDnPlayerState::ST_SP, &pState );
	pPlayerState->OnCalcEquipStep( CDnPlayerState::ST_RecoverySP, &pState );
#endif

	m_pStaticHP->SetValue( CDnCharStatusStatic::TYPE_02, true, (int)hActor->GetHP(), (int)hActor->GetMaxHP(), (int)pState.GetMaxHP(), (int)(pTotalState.GetMaxHP() - pState.GetMaxHP()) );
	m_pStaticMP->SetValue( CDnCharStatusStatic::TYPE_02, true, hActor->GetSP(),	hActor->GetMaxSP(), pState.GetMaxSP(), pTotalState.GetMaxSP() - pState.GetMaxSP() );
	m_pStaticMPRegen->AddDetailString( eDetailString::MP_RECOVERY , CDnActor::s_hLocalActor->GetRecoverySP(), false );
	m_pStaticMPRegen->SetValue( CDnCharStatusStatic::TYPE_01, true, CDnActor::s_hLocalActor->GetRecoverySP(), pState.GetRecoverySP() , CDnActor::s_hLocalActor->GetRecoverySP() - pState.GetRecoverySP() );
#endif


	switch(nType)
	{
	case STATUS:
		{
#if defined(PRE_ADD_SKILLBUF_RENEW) && defined(_CLIENT) && defined(PRE_ADD_STATUS_BUFF_INFORMATION)
			m_pStaticInfo[nPage][0]->AddBuffValue( pBuffState->GetStrength() );
			m_pStaticInfo[nPage][1]->AddBuffValue( pBuffState->GetAgility() );
			m_pStaticInfo[nPage][2]->AddBuffValue( pBuffState->GetIntelligence() );
			m_pStaticInfo[nPage][3]->AddBuffValue( pBuffState->GetStamina() );
#endif
			m_pStaticInfo[nPage][0]->SetValue( CDnCharStatusStatic::TYPE_01, true, CDnActor::s_hLocalActor->GetStrength(), pState.GetStrength() , CDnActor::s_hLocalActor->GetStrength() - pState.GetStrength() );
			m_pStaticInfo[nPage][1]->SetValue( CDnCharStatusStatic::TYPE_01, true, CDnActor::s_hLocalActor->GetAgility(), pState.GetAgility() , CDnActor::s_hLocalActor->GetAgility() - pState.GetAgility() );
			m_pStaticInfo[nPage][2]->SetValue( CDnCharStatusStatic::TYPE_01, true, CDnActor::s_hLocalActor->GetIntelligence(), pState.GetIntelligence() , CDnActor::s_hLocalActor->GetIntelligence() - pState.GetIntelligence() );
			m_pStaticInfo[nPage][3]->SetValue( CDnCharStatusStatic::TYPE_01, true, CDnActor::s_hLocalActor->GetStamina(), pState.GetStamina() , CDnActor::s_hLocalActor->GetStamina() - pState.GetStamina() );
		}
		break;
	case GENERAL:
		{

#if defined(PRE_ADD_SKILLBUF_RENEW) && defined(_CLIENT) && defined(PRE_ADD_STATUS_BUFF_INFORMATION)
			m_pStaticInfo[nPage][0]->AddBuffValue( pBuffState->GetAttackPMin() , pBuffState->GetAttackPMax() );
			m_pStaticInfo[nPage][1]->AddBuffValue( pBuffState->GetAttackMMin() , pBuffState->GetAttackMMax() );
			m_pStaticInfo[nPage][2]->AddBuffValue( pBuffState->GetDefenseP() , CalcDefenseRate( pBuffState->GetDefenseP() ,hActor ) );
			m_pStaticInfo[nPage][3]->AddBuffValue( pBuffState->GetDefenseM() , CalcDefenseRate( pBuffState->GetDefenseM() ,hActor ) );
#endif
#ifdef PRE_FIX_STATUS_USE_BASE_REGULATION
			pPlayerState->OnCalcEquipStep( CDnPlayerState::ST_AttackP, &pState );
			pPlayerState->OnCalcEquipStep( CDnPlayerState::ST_AttackM, &pState );
			pPlayerState->OnCalcEquipStep( CDnPlayerState::ST_DefenseP, &pState );
			pPlayerState->OnCalcEquipStep( CDnPlayerState::ST_DefenseM, &pState );
#endif

#ifdef PRE_ADD_STAGE_LIMIT_INTERFACE
			m_pStaticInfo[nPage][0]->SetTextColor( pPlayerState->GetDamageLimitInfo()->bPhysicalAttack == true ? D3DCOLOR_ARGB( 255, 255, 100, 155 ) : D3DCOLOR_ARGB( 255, 255, 153, 0 ) );
			m_pStaticInfo[nPage][1]->SetTextColor( pPlayerState->GetDamageLimitInfo()->bMagicalAttack == true ? D3DCOLOR_ARGB( 255, 255, 100, 155 ) : D3DCOLOR_ARGB( 255, 255, 153, 0 ) );
#endif
			m_pStaticInfo[nPage][0]->SetValue( CDnCharStatusStatic::TYPE_04, true, pTotalState.GetAttackPMin(), pTotalState.GetAttackPMax(),pState.GetAttackPMin() , pState.GetAttackPMax() , pTotalState.GetAttackPMin() - pState.GetAttackPMin(), pTotalState.GetAttackPMax() - pState.GetAttackPMax() );
			m_pStaticInfo[nPage][1]->SetValue( CDnCharStatusStatic::TYPE_04, true, pTotalState.GetAttackMMin(), pTotalState.GetAttackMMax(), pState.GetAttackMMin() , pState.GetAttackMMax() , pTotalState.GetAttackMMin() - pState.GetAttackMMin(), pTotalState.GetAttackMMax() - pState.GetAttackMMax() );
		
			m_pStaticInfo[nPage][2]->AddDetailString( eDetailString::PHYSICAL_DEFENSE , CalcDefenseRate( pTotalState.GetDefenseP() ,hActor ) );
			m_pStaticInfo[nPage][2]->SetValue( 
				CDnCharStatusStatic::TPYE_05, 
				true, 
				pTotalState.GetDefenseP(), 
				pState.GetDefenseP() ,  
				pTotalState.GetDefenseP() - pState.GetDefenseP() , 
				CalcDefenseRate( pTotalState.GetDefenseP() ,hActor )
				);
				
			m_pStaticInfo[nPage][3]->AddDetailString( eDetailString::MEGICAL_DEFENSE , CalcDefenseRate( pTotalState.GetDefenseM() ,hActor ) );
			m_pStaticInfo[nPage][3]->SetValue( 
				CDnCharStatusStatic::TPYE_05, 
				true, 
				pTotalState.GetDefenseM(), 
				pState.GetDefenseM() , 
				pTotalState.GetDefenseM() - pState.GetDefenseM() , 
				CalcDefenseRate( pTotalState.GetDefenseM() ,hActor )
				);
	
		}
		break;
	case SPECIAL_ATTACK:
		{

#if defined(PRE_ADD_SKILLBUF_RENEW) && defined(_CLIENT) && defined(PRE_ADD_STATUS_BUFF_INFORMATION)
			m_pStaticInfo[nPage][0]->AddBuffValue( pBuffState->GetCritical() , CalcCriticalRate( pBuffState->GetCritical(), hActor ) );
			m_pStaticInfo[nPage][1]->AddBuffValue( pBuffState->GetStun() );
			m_pStaticInfo[nPage][2]->AddBuffValue( pBuffState->GetStiff() );
			m_pStaticInfo[nPage][3]->AddBuffValue( (int)pBuffState->GetFinalDamage() , CalcFianlDamageRate( pBuffState->GetFinalDamage() , hActor ) );
#endif
#ifdef PRE_FIX_STATUS_USE_BASE_REGULATION
			pPlayerState->OnCalcEquipStep( CDnPlayerState::ST_Critical, &pState );
			pPlayerState->OnCalcEquipStep( CDnPlayerState::ST_Stun, &pState );
			pPlayerState->OnCalcEquipStep( CDnPlayerState::ST_Stiff, &pState );
			pPlayerState->OnCalcEquipStep( CDnPlayerState::ST_FinalDamage, &pState );
#endif
			
			int nTotalCriticalRate = CalcCriticalRate( pTotalState.GetCritical(), hActor );
#ifdef PRE_ADD_CHAR_STATUS_SECOND_RENEW
			int nAdditionalCriticlaInfo = CalcStateBlowCriticalRate( hActor );
			nTotalCriticalRate += nAdditionalCriticlaInfo;
			if( nTotalCriticalRate > (int)( CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::CriticalMax ) * 100 ) )
				nTotalCriticalRate = (int)( CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::CriticalMax ) * 100 );
#endif
			m_pStaticInfo[nPage][0]->AddDetailString( eDetailString::CRIT_RATIO , nTotalCriticalRate );
			m_pStaticInfo[nPage][0]->SetValue( 
				CDnCharStatusStatic::TPYE_05, 
				true, 
				pTotalState.GetCritical() ,
				pState.GetCritical() , 
				pTotalState.GetCritical() - pState.GetCritical() , 
				nTotalCriticalRate 
#if defined (PRE_ADD_CHAR_STATUS_SECOND_RENEW)
				,nAdditionalCriticlaInfo
#endif
				);

		
			m_pStaticInfo[nPage][1]->AddDetailString( eDetailString::STUN_RATIO , 0 );
			m_pStaticInfo[nPage][1]->SetValue( 
				CDnCharStatusStatic::TYPE_01, 
				true, 
				pTotalState.GetStun(), 
				pState.GetStun() , 
				pTotalState.GetStun() - pState.GetStun()
				);


			m_pStaticInfo[nPage][2]->AddDetailString( eDetailString::STIFF_RATIO , 0);
			m_pStaticInfo[nPage][2]->SetValue( 
				CDnCharStatusStatic::TYPE_01, 
				true, 
				pTotalState.GetStiff() , 
				pState.GetStiff() , 
				pTotalState.GetStiff() - pState.GetStiff()
				);

			int nTotalFinalDamageValue = pTotalState.GetFinalDamage();
			int nBaseFinalDamageValue = pState.GetFinalDamage();

			m_pStaticInfo[nPage][3]->AddDetailString( eDetailString::FINAL_DAMAGE ,CalcFianlDamageRate( pTotalState.GetFinalDamage() , hActor ) );
			m_pStaticInfo[nPage][3]->SetValue( 
				CDnCharStatusStatic::TPYE_05,
				true,
				nTotalFinalDamageValue ,
				nBaseFinalDamageValue ,
				nTotalFinalDamageValue - nBaseFinalDamageValue ,
				CalcFianlDamageRate( nTotalFinalDamageValue , hActor )
				);
		}
		break;
	case SPECIAL_DEFENSE:
		{

#if defined(PRE_ADD_SKILLBUF_RENEW) && defined(_CLIENT) && defined(PRE_ADD_STATUS_BUFF_INFORMATION)
			m_pStaticInfo[nPage][0]->AddBuffValue( pBuffState->GetCriticalResistance() , CalcCriticalRegistanceRate( pBuffState->GetCriticalResistance() , hActor ) );
			m_pStaticInfo[nPage][1]->AddBuffValue( pBuffState->GetStunResistance() );
			m_pStaticInfo[nPage][2]->AddBuffValue( pBuffState->GetStiffResistance() );
#endif
#ifdef PRE_FIX_STATUS_USE_BASE_REGULATION
			pPlayerState->OnCalcEquipStep( CDnPlayerState::ST_CriticalResistance, &pState );
			pPlayerState->OnCalcEquipStep( CDnPlayerState::ST_StunResistance, &pState );
			pPlayerState->OnCalcEquipStep( CDnPlayerState::ST_StiffResistance, &pState );
#endif

			m_pStaticInfo[nPage][0]->AddDetailString( eDetailString::CRIT_RESIST , CalcCriticalRegistanceRate( pTotalState.GetCriticalResistance() , hActor ) );
			m_pStaticInfo[nPage][0]->SetValue( 
				CDnCharStatusStatic::TPYE_05,
				true, 
				pTotalState.GetCriticalResistance() , 
				pState.GetCriticalResistance() , 
				pTotalState.GetCriticalResistance() - pState.GetCriticalResistance() , 
				CalcCriticalRate( pTotalState.GetCriticalResistance() , hActor )				
				);

			m_pStaticInfo[nPage][1]->AddDetailString( eDetailString::STUN_RESIST  , 0);
			m_pStaticInfo[nPage][1]->SetValue( 
				CDnCharStatusStatic::TYPE_01, 
				true, 
				pTotalState.GetStunResistance(), 
				pState.GetStunResistance() , 
				pTotalState.GetStunResistance() - pState.GetStunResistance()
				);
	
			m_pStaticInfo[nPage][2]->AddDetailString( eDetailString::STIFF_RESIT , 0);
			m_pStaticInfo[nPage][2]->SetValue( 
				CDnCharStatusStatic::TYPE_01, 
				true, 
				pTotalState.GetStiffResistance(), 
				pState.GetStiffResistance() , 
				pTotalState.GetStiffResistance() - pState.GetStiffResistance()
				);
			
			m_pStaticInfo[nPage][3]->Show(false);
		}
		break;
	case MAGICAL_ATTACK:
		{

#if defined(PRE_ADD_SKILLBUF_RENEW) && defined(_CLIENT) && defined(PRE_ADD_STATUS_BUFF_INFORMATION)
			m_pStaticInfo[nPage][0]->AddBuffValue( pBuffState->GetElementAttack( CDnState::Fire ) * 100.0f );
			m_pStaticInfo[nPage][1]->AddBuffValue( pBuffState->GetElementAttack( CDnState::Ice ) * 100.0f );
			m_pStaticInfo[nPage][2]->AddBuffValue( pBuffState->GetElementAttack( CDnState::Light ) * 100.0f );
			m_pStaticInfo[nPage][3]->AddBuffValue( pBuffState->GetElementAttack( CDnState::Dark ) * 100.0f );
#endif

			m_pStaticInfo[nPage][0]->AddDetailString( eDetailString::FIRE_DAMAGE , pTotalState.GetElementAttack( CDnState::Fire ) * 100.0f );
			m_pStaticInfo[nPage][0]->SetValue_Float( 
				CDnCharStatusStatic::TYPE_03, 
				true, 
				pTotalState.GetElementAttack( CDnState::Fire ) * 100.0f , 
				pState.GetElementAttack( CDnState::Fire ) * 100.0f , 
				pTotalState.GetElementAttack( CDnState::Fire ) * 100.0f - pState.GetElementAttack( CDnState::Fire ) * 100.0f
				);

			m_pStaticInfo[nPage][1]->AddDetailString( eDetailString::WATER_DAMAGE , pTotalState.GetElementAttack( CDnState::Ice ) * 100.0f );
			m_pStaticInfo[nPage][1]->SetValue_Float( 
				CDnCharStatusStatic::TYPE_03, 
				true, 
				pTotalState.GetElementAttack( CDnState::Ice ) * 100.0f , 
				pState.GetElementAttack( CDnState::Ice ) * 100.0f , 
				pTotalState.GetElementAttack( CDnState::Ice ) * 100.0f - pState.GetElementAttack( CDnState::Ice ) * 100.0f
				);
	
			m_pStaticInfo[nPage][2]->AddDetailString( eDetailString::LIGHT_DAMAGE , pTotalState.GetElementAttack( CDnState::Light ) * 100.0f );
			m_pStaticInfo[nPage][2]->SetValue_Float( 
				CDnCharStatusStatic::TYPE_03,
				true, 
				pTotalState.GetElementAttack( CDnState::Light ) * 100.0f , 
				pState.GetElementAttack( CDnState::Light ) * 100.0f , 
				pTotalState.GetElementAttack( CDnState::Light ) * 100.0f - pState.GetElementAttack( CDnState::Light ) * 100.0f
				);

			m_pStaticInfo[nPage][3]->AddDetailString( eDetailString::DARK_DAMAGE , pTotalState.GetElementAttack( CDnState::Dark ) * 100.0f );
			m_pStaticInfo[nPage][3]->SetValue_Float( 
				CDnCharStatusStatic::TYPE_03, 
				true, 
				pTotalState.GetElementAttack( CDnState::Dark ) * 100.0f , 
				pState.GetElementAttack( CDnState::Dark ) * 100.0f , 
				pTotalState.GetElementAttack( CDnState::Dark ) * 100.0f - pState.GetElementAttack( CDnState::Dark ) * 100.0f
				);

		}
		break;
	case MAGICAL_DEFENSE:
		{

#if defined(PRE_ADD_SKILLBUF_RENEW) && defined(_CLIENT) && defined(PRE_ADD_STATUS_BUFF_INFORMATION)
			m_pStaticInfo[nPage][0]->AddBuffValue( pBuffState->GetElementDefense( CDnState::Fire ) * 100.0f );
			m_pStaticInfo[nPage][1]->AddBuffValue( pBuffState->GetElementDefense( CDnState::Ice ) * 100.0f );
			m_pStaticInfo[nPage][2]->AddBuffValue( pBuffState->GetElementDefense( CDnState::Light ) * 100.0f );
			m_pStaticInfo[nPage][3]->AddBuffValue( pBuffState->GetElementDefense( CDnState::Dark ) * 100.0f );
#endif

			m_pStaticInfo[nPage][0]->AddDetailString( eDetailString::FIRE_DEFENSE , pTotalState.GetElementDefense( CDnState::Fire ) * 100.0f );
			m_pStaticInfo[nPage][0]->SetValue_Float( 
				CDnCharStatusStatic::TYPE_03, 
				true, 
				pTotalState.GetElementDefense( CDnState::Fire ) * 100.0f , 
				pState.GetElementDefense( CDnState::Fire ) * 100.0f , 
				pTotalState.GetElementDefense( CDnState::Fire ) * 100.0f - pState.GetElementDefense( CDnState::Fire ) * 100.0f
				);


			m_pStaticInfo[nPage][1]->AddDetailString( eDetailString::WATER_DEFENSE , pTotalState.GetElementDefense( CDnState::Ice ) * 100.0f );
			m_pStaticInfo[nPage][1]->SetValue_Float( 
				CDnCharStatusStatic::TYPE_03,
				true, 
				pTotalState.GetElementDefense( CDnState::Ice ) * 100.0f , 
				pState.GetElementDefense( CDnState::Ice ) * 100.0f , 
				pTotalState.GetElementDefense( CDnState::Ice ) * 100.0f - pState.GetElementDefense( CDnState::Ice ) * 100.0f
				);

			m_pStaticInfo[nPage][2]->AddDetailString( eDetailString::LIGHT_DEFENSE , pTotalState.GetElementDefense( CDnState::Light ) * 100.0f );
			m_pStaticInfo[nPage][2]->SetValue_Float( 
				CDnCharStatusStatic::TYPE_03, 
				true, 
				pTotalState.GetElementDefense( CDnState::Light ) * 100.0f , 
				pState.GetElementDefense( CDnState::Light ) * 100.0f , 
				pTotalState.GetElementDefense( CDnState::Light ) * 100.0f - pState.GetElementDefense( CDnState::Light ) * 100.0f
				);
			
			m_pStaticInfo[nPage][3]->AddDetailString( eDetailString::DARK_DEFENSE , pTotalState.GetElementDefense( CDnState::Dark ) * 100.0f );
			m_pStaticInfo[nPage][3]->SetValue_Float( 
				CDnCharStatusStatic::TYPE_03, 
				true, 
				pTotalState.GetElementDefense( CDnState::Dark ) * 100.0f , 
				pState.GetElementDefense( CDnState::Dark ) * 100.0f , 
				pTotalState.GetElementDefense( CDnState::Dark ) * 100.0f - pState.GetElementDefense( CDnState::Dark ) * 100.0f
				);
		
		}
		break;
	case SIDE_INFO:
		{
			float fMovementRate = 0;

			if( hActor && hActor->IsPlayerActor() )
			{
				CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor*>(hActor.GetPointer());
				if( pPlayer->IsVehicleMode() && pPlayer->GetMyVehicleActor() )
				{
					fMovementRate = ( (float)pPlayer->GetMyVehicleActor()->GetMoveSpeed() - (float)pState.GetMoveSpeed() ) / (float)pState.GetMoveSpeed() * 100.f;

					m_pStaticInfo[nPage][0]->AddDetailString( eDetailString::MOVE_SPEED , (int)fMovementRate );
					m_pStaticInfo[nPage][0]->SetValue( 
						CDnCharStatusStatic::TPYE_05, 
						true , 
						pPlayer->GetMyVehicleActor()->GetMoveSpeed() , 
						pState.GetMoveSpeed() , 
						pPlayer->GetMyVehicleActor()->GetMoveSpeed() - pState.GetMoveSpeed() ,  
						(int)fMovementRate
						);
				}
				else
				{
					fMovementRate = (hActor->GetMoveSpeed() - pState.GetMoveSpeed()) / (float)pState.GetMoveSpeed() * 100.f;

#if defined(PRE_ADD_SKILLBUF_RENEW) && defined(_CLIENT) && defined(PRE_ADD_STATUS_BUFF_INFORMATION)
					float fBuffMovementRate = pBuffState->GetMoveSpeed() / pState.GetMoveSpeed() * 100.f;
					m_pStaticInfo[nPage][0]->AddBuffValue( pBuffState->GetMoveSpeed() , (int)fBuffMovementRate );
#endif

					m_pStaticInfo[nPage][0]->AddDetailString( eDetailString::MOVE_SPEED , (int)fMovementRate );
					m_pStaticInfo[nPage][0]->SetValue( 
						CDnCharStatusStatic::TPYE_05, 
						true , 
						hActor->GetMoveSpeed() , 
						pState.GetMoveSpeed() , 
						hActor->GetMoveSpeed() - pState.GetMoveSpeed() ,  
						(int)fMovementRate
						);
				}
			}
		
			m_pStaticInfo[nPage][1]->Show(false);
			m_pStaticInfo[nPage][2]->Show(false);
			m_pStaticInfo[nPage][3]->Show(false);

		}
		break;
	}
}

int CDnCharStatusBaseInfoDlg::CalcDefenseRate( int nDefense , DnActorHandle hActor )
{
	float fDefensePower = 1.f;
	float fDefenseWeight = 0.f;

	fDefenseWeight = hActor->GetDefenseConstant();
	fDefensePower = nDefense / fDefenseWeight;

	if( fDefensePower < 0.f ) 
		fDefensePower = 0.f;
	else if( fDefensePower > CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::DefenseMax ) ) 
		fDefensePower = CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::DefenseMax );

	return (int)(fDefensePower * 100.f);
}


int CDnCharStatusBaseInfoDlg::CalcCriticalRate( int nCrit ,DnActorHandle hActor )
{
	float fCriticalValue = 0.f;
	fCriticalValue = hActor->GetLevelWeightValue() * CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::CriticalValue );

	float fCriticalProb = 0.f;
	if( fCriticalValue > 0.f )
		fCriticalProb = nCrit / fCriticalValue;

	if( fCriticalProb > CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::CriticalMax ) )
		fCriticalProb = CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::CriticalMax );

	return (int)( fCriticalProb * 100.f );
}


int CDnCharStatusBaseInfoDlg::CalcStateBlowCriticalRate( DnActorHandle hActor )
{
	float fAddCiriticalValue = 0.f;

	if( hActor->IsAppliedThisStateBlow(STATE_BLOW::BLOW_251) )
	{
		DNVector( DnBlowHandle ) vlhCriticalIncBlows;
		hActor->GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_251, vlhCriticalIncBlows );
		int nListCount = (int)vlhCriticalIncBlows.size();
		for (int i = 0; i < nListCount; ++i)
		{
			DnBlowHandle hBlow = vlhCriticalIncBlows[i];
			if (hBlow)
				fAddCiriticalValue += hBlow->GetFloatValue();
		}
	}

	return (int)( fAddCiriticalValue * 100.f );
}


int CDnCharStatusBaseInfoDlg::CalcCriticalRegistanceRate( int nCritReg ,DnActorHandle hActor )
{

	float fCriticalValue = hActor->GetCriticalConstant();
	
	float fCriticalResistProb = nCritReg / fCriticalValue;
	if( fCriticalResistProb > CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::CriticalResistanceMax ) )
		fCriticalResistProb = CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::CriticalResistanceMax );


	return (int)( fCriticalResistProb * 100.f );
}


int CDnCharStatusBaseInfoDlg::CalcFianlDamageRate( int nFinalDanage , DnActorHandle hActor )
{

	float fFinalDamageValue[4] = 
	{ 
		CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::FinalDamageValue1 ), 
		CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::FinalDamageValue2 ), 
		CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::FinalDamageValue3 ),
		CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::FinalDamageValue4 ),
	};

	float fFValue = 0.f;

	fFValue = hActor->GetFinalDamageConstant();

	float fFinalDamage1 = (float)( nFinalDanage / fFValue ) * fFinalDamageValue[3];
	float fFinalDamage2 = powf( (float)( nFinalDanage / fFValue ), fFinalDamageValue[1] ) * fFinalDamageValue[2];
	return (int)(min( max(fFinalDamage1, fFinalDamage2), fFinalDamageValue[2] ) * 100);
}

#endif

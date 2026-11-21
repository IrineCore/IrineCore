#include "stdafx.h"
#include "DnActorClassDefine.h"
#include "DnTableDB.h"

// Player
// Template
#include "TDnPlayerWarrior.h"
#include "TDnPlayerArcher.h"
#include "TDnPlayerCleric.h"
#include "TDnPlayerSoceress.h"


// Non Local
#include "DnNLPAArcher.h"
#include "DnNLPASoceress.h"
#if defined(PRE_ADD_ACADEMIC)
#include "DnNLPAAcademic.h"
#endif	// #if defined(PRE_ADD_ACADEMIC)
#ifdef PRE_ADD_KALI
#include "DnNLPAKali.h"
#endif // #ifdef PRE_ADD_KALI

// Monster
#include "DnMonsterActor.h"
#include "DnNormalMonsterActor.h"
#include "DnGoldMonsterActor.h"
#include "DnCannonMonsterActor.h"
#include "DnDefenseMonsterActor.h"
#include "DnGiantMonsterActor.h"
#include "DnSimpleRushMonsterActor.h"
#include "DnBasiliskMonsterActor.h"
#include "DnBeholderMonsterActor.h"
#include "DnManticoreMonsterActor.h"
#include "DnCelberosMonsterActor.h"
#include "DnClericRelicMonster.hpp"
#include "DnLotusGolemMonsterActor.h"

// Custom Monster
#include "DnGhoulMonsterActor.h"
#include "DnLamiaMonsterActor.h"
#include "DnSpittlerMonsterActor.h"
#include "DnSeadragonMonsterActor.h"
#include "DnGreenDragonMonsterActor.h"
#include "DnNormalNestBossMonsterActor.h"

// npc
#include "DnNPCActor.h"

// Prop Actor
#include "DnPropActor.h"

// Reference Actor Mathod Class
#include "MAPartsBody.h"
#include "DnActorState.h"

#include "PerfCheck.h"

#include "DnHideMonsterActor.h"
#ifdef PRE_FIX_MEMOPT_EXT
#include "DNGameDataManager.h"
#endif


DnActorHandle CreateActor( CMultiRoom *pRoom, int nActorTableID, int nTeamSetting )
{
	DnActorHandle hActor;

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TACTOR );
	if( !pSox ) 
	{
		g_Log.Log( LogType::_FILELOG, L"ActorTable.ext failed\r\n");
		return hActor;
	}

	int nDefaultTeam = 0;
	if( nActorTableID == -1 ) 
	{
		hActor = (new IBoostPoolDnMonsterActor(pRoom, -1))->GetMySmartPtr();
		nDefaultTeam = 1;
	}
	else {
		if( !pSox->IsExistItem( nActorTableID ) ) return CDnActor::Identity();
		CDnActorState::ActorTypeEnum ActorType = (CDnActorState::ActorTypeEnum)pSox->GetFieldFromLablePtr( nActorTableID, "_Class" )->GetInteger();

		char *pName = pSox->GetFieldFromLablePtr( nActorTableID, "_StaticName" )->GetString();
		switch( ActorType ) {
			case CDnActorState::Warrior:	hActor = (new TDnPlayerWarrior<CDnPlayerActor>(pRoom, nActorTableID))->GetMySmartPtr();	break;
			case CDnActorState::Archer:		hActor = (new CDnNLPAArcher(pRoom, nActorTableID))->GetMySmartPtr();	break;
			case CDnActorState::Soceress:	hActor = (new CDnNLPASoceress(pRoom, nActorTableID))->GetMySmartPtr();	break;
			case CDnActorState::Cleric:		hActor = (new TDnPlayerCleric<CDnPlayerActor>(pRoom, nActorTableID))->GetMySmartPtr();	break;
#if defined(PRE_ADD_ACADEMIC)
			case CDnActorState::Academic:	hActor = (new CDnNLPAAcademic(pRoom, nActorTableID))->GetMySmartPtr();	break;
#endif	// #if defined(PRE_ADD_ACADEMIC)
#ifdef PRE_ADD_KALI
			case CDnActorState::Kali:		hActor = (new CDnNLPAKali(pRoom, nActorTableID))->GetMySmartPtr();	break;
#endif // #ifdef PRE_ADD_KALI

			case CDnActorState::Defense:	
				if( strstr( pName, "Lamia" ) ) {
					hActor = (new CDnLamiaMonsterActor(pRoom, nActorTableID))->GetMySmartPtr();
					break;
				}
				hActor = (new IBoostPoolDnDefenseMonsterActor(pRoom, nActorTableID))->GetMySmartPtr(); 
				break;
			case CDnActorState::SimpleRush:
				if( strstr( pName, "Ghoul" ) ) {
					hActor = (new CDnGhoulMonsterActor(pRoom, nActorTableID))->GetMySmartPtr();
					break;
				}
				else if( strstr( pName, "Spittler" ) ) 
				{
					hActor = (new CDnSpittlerMonsterActor(pRoom, nActorTableID))->GetMySmartPtr();
					break;
				}
				else if( strstr( pName, "Cleric_Relic" ) ) 
				{
					hActor = (new CDnClericRelicMonsterActor(pRoom, nActorTableID))->GetMySmartPtr();
					break;
				}
				else if (strstr(pName, "HideMonster"))
				{
					hActor = (new CDnHideMonsterActor(pRoom, nActorTableID))->GetMySmartPtr();
					break;
				}
#if defined( PRE_ADD_ACADEMIC )
				else if (strstr(pName, "Academic_Automat"))
				{
					hActor = (new CDnAcademicAutomatMonsterActor(pRoom, nActorTableID))->GetMySmartPtr();
					break;
				}
#endif // #if defined( PRE_ADD_ACADEMIC )

				hActor = (new IBoostPoolDnSimpleRushMonsterActor(pRoom, nActorTableID))->GetMySmartPtr();
				break;
			case CDnActorState::Normal:
			case CDnActorState::Disturbance:
			case CDnActorState::Range:
			case CDnActorState::Rush:
			case CDnActorState::Magic:
			case CDnActorState::Trap:
			case CDnActorState::NoAggro:
			{
				hActor = (new IBoostPoolDnNormalMonsterActor(pRoom, nActorTableID))->GetMySmartPtr();
				break;
			}
			case CDnActorState::NoAggroTrap:
			{
				hActor = (new CDnNoAggroTrapMonsterActor(pRoom,nActorTableID))->GetMySmartPtr();
				break;
			}
			case CDnActorState::GoldMonster:
			{
				hActor = (new IBoostPoolDnGoldMonsterActor(pRoom, nActorTableID))->GetMySmartPtr();
				break;
			}
			case CDnActorState::Cannon:		hActor = (new CDnCannonMonsterActor(pRoom, nActorTableID))->GetMySmartPtr(); break;
			case CDnActorState::Giant:		hActor = (new CDnGiantMonsterActor(pRoom, nActorTableID))->GetMySmartPtr(); break;
			case CDnActorState::Basilisk:	hActor = (new CDnBasiliskMonsterActor(pRoom, nActorTableID))->GetMySmartPtr();	break;
			case CDnActorState::Beholder:	hActor = (new CDnBeholderMonsterActor(pRoom, nActorTableID))->GetMySmartPtr();	break;
			case CDnActorState::Manticore:	hActor = (new CDnManticoreMonsterActor(pRoom, nActorTableID))->GetMySmartPtr();	break;
			case CDnActorState::Cerberos:	hActor = (new CDnCelberosMonsterActor(pRoom, nActorTableID))->GetMySmartPtr();	break;
			case CDnActorState::SeaDragon: hActor = (new CDnSeadragonMonsterActor(pRoom, nActorTableID))->GetMySmartPtr(); break;
			case CDnActorState::GreenDragon: hActor = (new CDnGreenDragonMonsterActor(pRoom, nActorTableID))->GetMySmartPtr(); break;
			case CDnActorState::LotusGolem: hActor = (new CDnLotusGolemMonsterActor(pRoom, nActorTableID))->GetMySmartPtr(); break;
			case CDnActorState::Vehicle : hActor = (new CDnVehicleActor(pRoom,nActorTableID))->GetMySmartPtr(); break;
			case CDnActorState::Npc:		hActor = (new CDnNPCActor(pRoom, nActorTableID))->GetMySmartPtr();	break;
			case CDnActorState::PropActor:	hActor = (new CDnPropActor(pRoom, nActorTableID))->GetMySmartPtr();	break;
			case CDnActorState::NESTBOSS:	hActor = (new CDnNormalNestBossMonsterActor(pRoom, nActorTableID))->GetMySmartPtr();	break;
			default:
				return CDnActor::Identity();
		}

#ifdef PRE_FIX_MEMOPT_EXT
		char* szSkinName = NULL, *szAniName = NULL, *szActName = NULL;
		DNTableFileFormat* pFileNameSox = GetDNTable( CDnTableDB::TFILE );
		if (g_pDataManager && pFileNameSox)
		{
			szSkinName = g_pDataManager->GetFileNameFromFileEXT(pSox, nActorTableID, "_SkinName", pFileNameSox);
			szAniName = g_pDataManager->GetFileNameFromFileEXT(pSox, nActorTableID, "_AniName", pFileNameSox);
			szActName = g_pDataManager->GetFileNameFromFileEXT(pSox, nActorTableID, "_ActName", pFileNameSox);
		}
#else
		// Create object!
		char* szSkinName = pSox->GetFieldFromLablePtr( nActorTableID, "_SkinName" )->GetString();
		char* szAniName = pSox->GetFieldFromLablePtr( nActorTableID, "_AniName" )->GetString();
		char* szActName = pSox->GetFieldFromLablePtr( nActorTableID, "_ActName" )->GetString();
#endif
		if( !szAniName || !szActName ) 
		{
			SAFE_RELEASE_SPTR( hActor );
			return CDnActor::Identity();
		}

		if( hActor->bIsSingleBody() )
		{
			if( !szSkinName ) {
				SAFE_RELEASE_SPTR( hActor );
				return CDnActor::Identity();
			}
			hActor->LoadSkin( CEtResourceMng::GetInstance().GetFullName( szSkinName ).c_str(), CEtResourceMng::GetInstance().GetFullName( szAniName ).c_str() );
		}
		else if( hActor->bIsPartsBody() )
		{
			hActor->LoadSkin( CEtResourceMng::GetInstance().GetFullName( szSkinName ).c_str(), CEtResourceMng::GetInstance().GetFullName( szAniName ).c_str() );
		}
		
		hActor->LoadAction( CEtResourceMng::GetInstance().GetFullName( szActName ).c_str() );

		// Team setup section
		if ( nTeamSetting < 0 )
		{
			if( ActorType <= CDnActorState::Reserved6 ) nDefaultTeam = 0;
			else nDefaultTeam = 1;
		}
		else nDefaultTeam = nTeamSetting;
	}

#if defined( STRESS_TEST )
	static int iStaticTeam = 0;
	nDefaultTeam = ++iStaticTeam;
#endif

	hActor->SetTeam( nDefaultTeam );
	hActor->SetMovement( hActor->CreateMovement() );
	hActor->GetMovement()->CMultiElement::Initialize( pRoom );
	hActor->GetMovement()->Initialize( hActor.GetPointer(), hActor->GetMatEx() );

	return hActor;
}

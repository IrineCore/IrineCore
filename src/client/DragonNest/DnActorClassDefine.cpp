#include "StdAfx.h"
#include "DnActorClassDefine.h"
#include "DnTableDB.h"

// Player
// Template
#include "TDnPlayerWarrior.h"
#include "TDnPlayerArcher.h"


// Local
#include "DnLocalPlayerActor.h"
#include "DnLPAWarrior.h"
#include "DnLPAArcher.h"
#include "DnLPASoceress.h"
#include "DnLPACleric.h"
#if defined(PRE_ADD_ACADEMIC)
#include "DnLPAAcademic.h"
#endif	// #if defined(PRE_ADD_ACADEMIC)
#ifdef PRE_ADD_KALI
#include "DnLPAKali.h"
#endif // #ifdef PRE_ADD_KALI

// Non Local
#include "DnNLPAArcher.h"
#include "DnNLPASoceress.h"
#if defined(PRE_ADD_ACADEMIC)
#include "DnNLPAAcademic.h"
#endif	// #if defined(PRE_ADD_ACADEMIC)
#ifdef PRE_ADD_KALI
#include "DnNLPAKali.h"
#endif // #ifidef PRE_ADD_KALI
// Monster
#include "DnMonsterActor.h"

#include "DnNormalMonsterActor.h"
#include "DnCannonMonsterActor.h"
#include "DnDefenseMonsterActor.h"
#include "DnGiantMonsterActor.h"
#include "DnSimpleRushMonsterActor.h"

// Custom Monster
#include "DnGhoulMonsterActor.h"
#include "DnLamiaMonsterActor.h"
#include "DnBasiliskMonsterActor.h"
#include "DnBeholderMonsterActor.h"
#include "DnManticoreMonsterActor.h"
#include "DnCelberosMonsterActor.h"
#include "DnEvilRootsMonsterActor.h"
#include "DnSeadragonMonsterActor.h"
#include "DnGreenDragonMonsterActor.h"
#include "DnHideMonsterActor.h"
#ifdef PRE_ADD_LOTUSGOLEM
#include "DnLotusGolemMonsterActor.h"
#endif
#include "DnNoAggroTrapActor.h"
#include "DnNormalNestBossMonsterActor.h"
#include "DnGoldMonsterActor.h"

// npc
#include "DnNPCActor.h"

// Prop Actor
#include "DnPropActor.h"

// Reference Actor Mathod Class
#include "MAPartsBody.h"
#include "DnActorState.h"

#include "PerfCheck.h"

#include "DnPetActor.h"

#ifdef PRE_FIX_MEMOPT_EXT
#include "DnCommonUtil.h"
#endif

#ifndef USE_BOOST_MEMPOOL
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif

DnActorHandle CreateActor( int nActorTableID, bool bLocal, bool bAllocOnly, bool bProcess )
{
	ThreadDelay();
	DnActorHandle hActor;

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TACTOR );
	if( !pSox ) return hActor;

	int nDefaultTeam = 0;
	if( nActorTableID == -1 ) {
		hActor = (new CDnMonsterActor(-1, false))->GetMySmartPtr();
		nDefaultTeam = 1;
	}
	else {
		if( !pSox->IsExistItem( nActorTableID ) ) return CDnActor::Identity();
		CDnActorState::ActorTypeEnum ActorType = (CDnActorState::ActorTypeEnum)pSox->GetFieldFromLablePtr( nActorTableID, "_Class" )->GetInteger();

		if( bLocal == true ) {
			nDefaultTeam = 0;
			switch( ActorType ) {
				case CDnActorState::Warrior:	hActor = (new CDnLPAWarrior(nActorTableID, false))->GetMySmartPtr();	break;
				case CDnActorState::Archer:		hActor = (new CDnLPAArcher(nActorTableID, false))->GetMySmartPtr();	break;
				case CDnActorState::Soceress:	hActor = (new CDnLPASoceress(nActorTableID, false))->GetMySmartPtr();	break;
				case CDnActorState::Cleric:		hActor = (new CDnLPACleric(nActorTableID, false))->GetMySmartPtr();	break;
#if defined(PRE_ADD_ACADEMIC)
				case CDnActorState::Academic:	hActor = (new CDnLPAAcademic(nActorTableID, false))->GetMySmartPtr();	break;
#endif	// #if defined(PRE_ADD_ACADEMIC)
#ifdef PRE_ADD_KALI
				case CDnActorState::Kali:		hActor = (new CDnLPAKali(nActorTableID, false))->GetMySmartPtr();	break;
#endif // #ifdef PRE_ADD_KALI

				case CDnActor::Vehicle:         hActor = (new CDnLocalVehicleActor(nActorTableID, false))->GetMySmartPtr(); break;

				default:
					hActor = (new CDnLocalPlayerActor(nActorTableID, false))->GetMySmartPtr();	break;
			}
		}
		else {
			char *pName = pSox->GetFieldFromLablePtr( nActorTableID, "_StaticName" )->GetString();
			switch( ActorType ) {
				case CDnActorState::Warrior:	hActor = (new TDnPlayerWarrior<CDnPlayerActor>(nActorTableID, false))->GetMySmartPtr();	break;
				case CDnActorState::Archer:		hActor = (new CDnNLPAArcher(nActorTableID, false))->GetMySmartPtr();	break;
				case CDnActorState::Soceress:	hActor = (new CDnNLPASoceress(nActorTableID, false))->GetMySmartPtr();	break;
				case CDnActorState::Cleric:		hActor = (new TDnPlayerCleric<CDnPlayerActor>(nActorTableID, false))->GetMySmartPtr();	break;
#if defined(PRE_ADD_ACADEMIC)
				case CDnActorState::Academic:	hActor = (new CDnNLPAAcademic(nActorTableID, false))->GetMySmartPtr();	break;
#endif	// #if defined(PRE_ADD_ACADEMIC)
#ifdef PRE_ADD_KALI
				case CDnActorState::Kali:		hActor = (new CDnNLPAKali(nActorTableID, false))->GetMySmartPtr();	break;
#endif // #ifdef PRE_ADD_KALI
				case CDnActorState::Defense:	
					if( strstr( pName, "Lamia" ) ) {
						hActor = (new CDnLamiaMonsterActor(nActorTableID, false))->GetMySmartPtr(); 
						break;
					}
					hActor = (new CDnDefenseMonsterActor(nActorTableID, false))->GetMySmartPtr(); 
					break;
				case CDnActorState::SimpleRush:
					if( strstr( pName, "Ghoul" ) ) {
						hActor = (new CDnGhoulMonsterActor(nActorTableID, false))->GetMySmartPtr();
						break;
					}
					if( strcmp( pName, "ShadowDark") == 0) {
						hActor = (new CDnEvilRootsMonsterActor(nActorTableID, false))->GetMySmartPtr();
						break;
					}
					if( strcmp( pName, "HideMonster" ) == 0 ) {
						hActor = (new CDnHideMonsterActor(nActorTableID, false))->GetMySmartPtr(); // #32115 To distinguish monsters used for specific purposes on the client.
						break;
					}
					hActor = (new CDnSimpleRushMonsterActor(nActorTableID, false))->GetMySmartPtr();
					break;					
				case CDnActorState::Normal:		hActor = (new CDnNormalMonsterActor(nActorTableID, false))->GetMySmartPtr();	break;
				case CDnActorState::Disturbance:hActor = (new CDnNormalMonsterActor(nActorTableID, false))->GetMySmartPtr();	break;
				case CDnActorState::Range:		hActor = (new CDnNormalMonsterActor(nActorTableID, false))->GetMySmartPtr(); break;
				case CDnActorState::Rush:		hActor = (new CDnNormalMonsterActor(nActorTableID, false))->GetMySmartPtr(); break;
				case CDnActorState::Magic:		hActor = (new CDnNormalMonsterActor(nActorTableID, false))->GetMySmartPtr(); break;
				case CDnActorState::Trap:		hActor = (new CDnNormalMonsterActor(nActorTableID, false))->GetMySmartPtr(); break;
				case CDnActorState::NoAggro:	hActor = (new CDnNormalMonsterActor(nActorTableID, false))->GetMySmartPtr(); break;
				case CDnActorState::NoAggroTrap:hActor = (new CDnNoAggroTrapActor(nActorTableID, false))->GetMySmartPtr(); break;
				case CDnActorState::GoldMonster:hActor = (new CDnGoldMonsterActor(nActorTableID, false))->GetMySmartPtr(); break;
				case CDnActorState::Cannon:		hActor = (new CDnCannonMonsterActor(nActorTableID, false))->GetMySmartPtr(); break;
				case CDnActorState::BlackDragon:hActor = (new CDnNormalMonsterActor(nActorTableID, false))->GetMySmartPtr(); break;
				case CDnActorState::GoldDragon: hActor = (new CDnNormalMonsterActor(nActorTableID, false))->GetMySmartPtr(); break;
				case CDnActorState::Giant:		hActor = (new CDnGiantMonsterActor(nActorTableID, false))->GetMySmartPtr(); break;
				case CDnActorState::Basilisk:	hActor = (new CDnBasiliskMonsterActor(nActorTableID, false))->GetMySmartPtr();	break;
				case CDnActorState::Beholder:	hActor = (new CDnBeholderMonsterActor(nActorTableID, false))->GetMySmartPtr();	break;
				case CDnActorState::Manticore:	hActor = (new CDnManticoreMonsterActor(nActorTableID, false))->GetMySmartPtr(); break;
				case CDnActorState::Cerberos:	hActor = (new CDnCelberosMonsterActor(nActorTableID, false))->GetMySmartPtr();	break;
				case CDnActorState::SeaDragon:	hActor = (new CDnSeadragonMonsterActor(nActorTableID, false))->GetMySmartPtr(); break;
				case CDnActorState::GreenDragon:	hActor = (new CDnGreenDragonMonsterActor(nActorTableID, false))->GetMySmartPtr(); break;
#ifdef PRE_ADD_LOTUSGOLEM
				case CDnActorState::LotusGolem:	hActor = (new CDnLotusGolemMonsterActor(nActorTableID, false))->GetMySmartPtr(); break;
#endif
				case CDnActorState::Npc:		hActor = (new CDnNPCActor(nActorTableID, false))->GetMySmartPtr();	break;

				case CDnActorState::Vehicle:	hActor = (new CDnVehicleActor(nActorTableID, false))->GetMySmartPtr(); break;
				case CDnActorState::Pet:		hActor = (new CDnPetActor( nActorTableID, false ))->GetMySmartPtr(); break;
				case CDnActorState::PropActor:	hActor = (new CDnPropActor(nActorTableID, false))->GetMySmartPtr(); break;
				case CDnActorState::NESTBOSS:	hActor = (new CDnNormalNestBossMonsterActor(nActorTableID, false))->GetMySmartPtr(); break;

				default:
				{
				    OutputDebug("[%s] Invalid Class: %d", __FUNCTION__, ActorType);
					return CDnActor::Identity();
				}
			}
			if( ActorType <= CDnActorState::Reserved6 ) nDefaultTeam = 0;
			else nDefaultTeam = 1;
		}

		hActor->SetMovement( hActor->CreateMovement() );
		hActor->GetMovement()->Initialize( hActor.GetPointer(), hActor->GetMatEx() );

		if( bAllocOnly == true ) {
			if( bProcess ) hActor->SetProcess( true );
			return hActor;
		}

		// Create object!
#ifdef PRE_FIX_MEMOPT_EXT
		std::string szSkinName, szAniName, szActName;
		CommonUtil::GetFileNameFromFileEXT(szSkinName, pSox, nActorTableID, "_SkinName");
		CommonUtil::GetFileNameFromFileEXT(szAniName, pSox, nActorTableID, "_AniName");
		CommonUtil::GetFileNameFromFileEXT(szActName, pSox, nActorTableID, "_ActName");
#else
		std::string szSkinName = pSox->GetFieldFromLablePtr( nActorTableID, "_SkinName" )->GetString();
		std::string szAniName = pSox->GetFieldFromLablePtr( nActorTableID, "_AniName" )->GetString();
		std::string szActName = pSox->GetFieldFromLablePtr( nActorTableID, "_ActName" )->GetString();
#endif

		bool bTwoPassAlpha = pSox->GetFieldFromLablePtr( nActorTableID, "_TwoPassAlpha" )->GetInteger() != 0;
		if( szAniName.empty() || szActName.empty() ) {
			SAFE_RELEASE_SPTR( hActor );
			return CDnActor::Identity();
		}

		MASingleBody *pSingleBody = dynamic_cast<MASingleBody *>(hActor.GetPointer());
		MAPartsBody *pPartsBody = dynamic_cast<MAPartsBody *>(hActor.GetPointer());

		if( pSingleBody && !pPartsBody ) {
			if( szSkinName.empty() ) {
				SAFE_RELEASE_SPTR( hActor );
				return CDnActor::Identity();
			}

			pSingleBody->LoadSkin( CEtResourceMng::GetInstance().GetFullName( szSkinName.c_str() ).c_str(), CEtResourceMng::GetInstance().GetFullName( szAniName.c_str() ).c_str(), bTwoPassAlpha );
		}
		else if( pPartsBody && !pSingleBody ) {
			pPartsBody->LoadSkin( CEtResourceMng::GetInstance().GetFullName( szSkinName.c_str() ).c_str(), CEtResourceMng::GetInstance().GetFullName( szAniName.c_str() ).c_str() );
		}
		hActor->LoadAction( CEtResourceMng::GetInstance().GetFullName( szActName.c_str() ).c_str() );

		if( ActorType >= CDnActorState::Basilisk ) hActor->SetDamageCheckType( CDnActor::Anyone_DifferentTeam );

	}
	hActor->SetTeam( nDefaultTeam );

	if( bProcess ) hActor->SetProcess( true );
	return hActor;
}
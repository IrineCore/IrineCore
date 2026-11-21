#include "stdafx.h"
#include "DnWeapon.h"
#include "DnTableDB.h"
#include "MAActorRenderBase.h"
#include "DnProjectile.h"
#include "DnMonsterActor.h"
#ifdef PRE_FIX_MEMOPT_EXT
#include "DNGameDataManager.h"
#endif

#if defined(PRE_FIX_65287)
#include "DnBlow.h"
#endif // PRE_FIX_65287

DECL_DN_SMART_PTR_STATIC( CDnWeapon, MAX_SESSION_COUNT, 100 )

STATIC_DECL_INIT( CDnWeapon, DWORD, s_dwUniqueCount ) = { 0, };

CDnWeapon::CDnWeapon( CMultiRoom *pRoom, bool bProcess, bool bIncreaseUniqueID )
: CDnUnknownRenderObject( pRoom, bProcess )
, CDnItem( pRoom )
{
	SetWeaponType( WeaponTypeEnum::Weapon );

	m_pParentWeapon = NULL;
	EtMatrixIdentity( &m_matWorld );
	CDnActionBase::Initialize( this );
	m_nLength = 0;
	m_nOriginalLength = 0;
	m_bLinkBone = false;
	m_dwUniqueID = -1;

	m_nDurability = m_nMaxDurability = 0;
	m_bDestroyThis = false;
	m_bCreateObject = false;

	// Issue
	// Later, when changing weapons or other items, UniqueID may differ from other clients.
	// (Especially for Projectile) In that case, do not decide UniqueID here; objects that need to be searched by UniqueID (currently only Projectile, or item throwing)
	// Increment and set UniqueID when creating, and send UniqueID together at creation time to fix mismatch issues.
	if( bIncreaseUniqueID ) {
		SetUniqueID( STATIC_INSTANCE(s_dwUniqueCount) );
		STATIC_INSTANCE(s_dwUniqueCount)++;
	}
	else SetUniqueID( -1 );

	m_nAniIndex = -1;
	m_nBlendAniIndex = -1;
	m_fBlendAniFrame = 0.f;
	m_fBlendFrame = 0.f;
	m_fBlendStartFrame = 0.f;

	m_nSetItemID = 0;
#if defined( PRE_ADD_OVERLAP_SETEFFECT )	
	m_bOverLapSet = false;
#endif

	m_eElement = ElementEnum_Amount;

	m_RecreateCashEquipType = (EquipTypeEnum)-1;
}

CDnWeapon::~CDnWeapon()
{
	FreeObject();
}

bool CDnWeapon::Initialize( int nWeaponTableID, int nSeed, char cOptionIndex, char cEnchantLevel, char cPotentialIndex, char cSealCount, bool bSoulBound )
{
	m_nClassID = nWeaponTableID;
	DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TWEAPON );
	if( !pSox ) 
	{
		g_Log.Log( LogType::_FILELOG, L"WeaponTable.ext failed\r\n");
		return false;
	}
	if( !pSox->IsExistItem( m_nClassID ) ) return false;

	m_nLength = pSox->GetFieldFromLablePtr( nWeaponTableID, "_Length" )->GetInteger();
	m_nDurability = m_nMaxDurability = pSox->GetFieldFromLablePtr( nWeaponTableID, "_Durability" )->GetInteger();
	m_EquipType = (EquipTypeEnum)pSox->GetFieldFromLablePtr( nWeaponTableID, "_EquipType" )->GetInteger();
	m_nOriginalLength = m_nLength;
	m_nSetItemID = pSox->GetFieldFromLablePtr( nWeaponTableID, "_SetItemID" )->GetInteger();
#if defined( PRE_ADD_OVERLAP_SETEFFECT )	
	m_bOverLapSet = pSox->GetFieldFromLablePtr( nWeaponTableID, "_SetOverlap" )->GetInteger() ? true : false;
#endif

	// Initialize Item info
	if( CDnItem::Initialize( nWeaponTableID, nSeed, cOptionIndex, cEnchantLevel, cPotentialIndex, cSealCount, bSoulBound ) == false ) return false;

	// For weapons, if base stats have an element, it is set as the base element.
	for( int i=0; i<ElementEnum_Amount; i++ ) {
		if( GetElementAttack( (ElementEnum)i ) > 0 ) {
			m_eElement = (ElementEnum)i;
			break;
		}
	}

	return true;
}

void CDnWeapon::Process( LOCAL_TIME LocalTime, float fDelta )
{
	if( m_bDestroyThis ) {
		if( m_hActor ) {
			for( int i=0; i<2; i++ ) {
				if( m_hActor->GetWeapon(i) == this ) {
					m_hActor->DetachWeapon(i);
					return;
				}
			}
		}
		else {
			delete this;
		}
		return;
	}
	if( m_hObject ) {
		if( m_bLinkBone ) {
			m_matWorld = *m_hObject->GetWorldMat();
		}
		else {
			m_hObject->Update( &m_matWorld );
		}
	}

	CDnActionBase::ProcessAction( LocalTime, fDelta );
	Process( &m_matWorld, LocalTime, fDelta );
}

void CDnWeapon::Update( EtMatrix *matWorld )
{
	m_matWorld = *matWorld;
}

void CDnWeapon::ChangeAnimation( int nAniIndex, float fFrame, float fBlendFrame )
{
	m_nBlendAniIndex = m_nAniIndex;
	m_fBlendStartFrame = fFrame;
	m_fBlendFrame = fBlendFrame;
	if( m_fBlendFrame == 0.f ) m_nBlendAniIndex = -1;

	if( m_nBlendAniIndex != -1 ) {
		m_fBlendAniFrame = m_fFrame;

		float fAniLength = (float)m_hObject->GetLastFrame( m_nBlendAniIndex );
		if( m_fBlendAniFrame >= fAniLength ) {
			m_fBlendAniFrame = fAniLength;
		}
	}

	m_nAniIndex = nAniIndex;
}

void CDnWeapon::Process( EtMatrix *pmatWorld, LOCAL_TIME LocalTime, float fDelta )
{
	if( !m_hObject ) return;
	if( m_nActionIndex == -1 ) return;
	if( m_nActionIndex >= (int)m_nVecAniIndexList.size() ) return;
	if( m_nAniIndex == -1 ) return;
	m_hObject->SetAniFrame( m_nAniIndex, m_fFrame );
}

int CDnWeapon::GetAniIndex( const char *szAniName )
{
	if( !m_hObject ) return -1;
	if( !m_hObject->GetAniHandle() ) return -1;
	for( int i=0; i<m_hObject->GetAniCount(); i++ ) {
		if( strcmp( m_hObject->GetAniName(i), szAniName ) == NULL ) return i;
	}
	return -1;
}

int CDnWeapon::GetBoneIndex( const char *szBoneName )
{
	if( !m_hObject ) return -1;
	EtAniHandle handle = m_hObject->GetAniHandle();
	return handle->GetBoneIndex( szBoneName );
}

void CDnWeapon::ShowWeapon( bool bShow )
{
}

void CDnWeapon::OnSignal( SignalTypeEnum Type, void *pPtr, LOCAL_TIME LocalTime, LOCAL_TIME SignalStartTime, LOCAL_TIME SignalEndTime, int nSignalIndex )
{
	switch( Type ) {
		case STE_Projectile:
			{
				//// If this skill projectile fires another projectile (Distortion Arrow, Revolutionary Ballista), inherit the shooter actor's stats.
				ProjectileStruct *pStruct = (ProjectileStruct *)pPtr;
				DnActorHandle hActor;

				// Note: Hanki - On server side, weapon's world matrix is identity matrix.... seems it's not calculated separately. 
				// So on the game server side weapon projectile signal, pass the actor's world matrix value. 
				// This part must be considered when setting Projectile/TargetPosition/Accell type projectiles affected by gravity..

				// Case occurred where a monster fires a projectile from a projectile. (#15557)
				// In that case, fire from the world matrix of the flying projectile. Also inherit state effects.
				CDnProjectile *pProjectile = NULL;
				MatrixEx LocalCross;
				bool bMonsterProjectileShootProjectile = false;
				if( (WeaponTypeEnum::Projectile == GetWeaponType()) )
				{
					CDnProjectile* pMyselfProjectile = static_cast<CDnProjectile*>(this);
					hActor = pMyselfProjectile->GetShooterActor();

					if( hActor )
					{
						if( hActor->IsMonsterActor() )
						{
							LocalCross = *pMyselfProjectile->GetMatEx();
							pProjectile = CDnProjectile::CreateProjectile( GetRoom(), hActor, LocalCross, pStruct );
							if( pProjectile == NULL ) 
								break;
							pProjectile->SetShooterType( GetMySmartPtr(), m_nActionIndex, nSignalIndex );

							if( pMyselfProjectile->GetShooterStateSnapshot() )
								pProjectile->SetShooterStateSnapshot( pMyselfProjectile->GetShooterStateSnapshot() );

#if defined(PRE_FIX_65287)

							float fFinalDamageRate = 0.0f;
							if (hActor && hActor->IsAppliedThisStateBlow(STATE_BLOW::BLOW_050))
							{
								DNVector(DnBlowHandle) vlhBlows;
								hActor->GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_050, vlhBlows );
								int iNumBlow = (int)vlhBlows.size();
								for( int i = 0; i < iNumBlow; ++i )
								{
									fFinalDamageRate += vlhBlows[i]->GetFloatValue();
								}
							}

							pProjectile->SetShooterFinalDamageRate(fFinalDamageRate);
#endif // PRE_FIX_65287

#if defined(PRE_ADD_50903)
							//When firing a projectile from a projectile, register the state effects that were on the parent projectile.
							pProjectile->ApplyParentProjectile(pMyselfProjectile);
							//Projectile -> Projectile: when firing a projectile from a projectile, get info from parent projectile to set whether it is from a skill.
							pProjectile->FromSkill(pMyselfProjectile->IsFromSkill());

							if (pMyselfProjectile->GetParentSkill())
								pProjectile->SetParentSkill(pMyselfProjectile->GetParentSkill());
#endif // PRE_ADD_50903
						
							bMonsterProjectileShootProjectile = true;

							CDnMonsterActor* pMonsterActor = static_cast<CDnMonsterActor*>(hActor.GetPointer());
							pMonsterActor->SendProjectileFromProjectile( pProjectile, nSignalIndex );
						}
					}
					else
						// Firing a projectile from a projectile fired by a monster actor, when the monster object is invalid.. 
						// Set flag so the routine below for player handling does not run.
						bMonsterProjectileShootProjectile = true;
				}
				
				if( false == bMonsterProjectileShootProjectile )
				{
					hActor = m_hActor;
					if( !m_hActor ) 
						break;

					LocalCross = *hActor->GetMatEx();
					pProjectile = CDnProjectile::CreateProjectile( GetRoom(), hActor, LocalCross, pStruct );
					if( pProjectile == NULL ) 
						break;

					pProjectile->SetShooterType( GetMySmartPtr(), m_nActionIndex, nSignalIndex );
					hActor->OnProjectile( pProjectile, pStruct, LocalCross, nSignalIndex );
					hActor->OnSkillProjectile( pProjectile );
				}
			}
			break;
		case STE_Destroy:
			if( IsProcess() ) SetDestroy();
			else m_bDestroyThis = true;
			break;
	}
}

void CDnWeapon::OnHitSuccess( DnActorHandle hTarget, HitTypeEnum HitType, EtVector3 &vHitPosition, bool bPlaySound )
{
}

void CDnWeapon::LinkWeapon( DnActorHandle hActor, int nEquipIndex )
{
	m_hActor = hActor;

	if( !m_hObject ) return;
	m_bLinkBone = true;

	if( GetElementIndex( "Idle" ) != -1 )
		SetAction( "Idle", 0.f, 0.f );
}

void CDnWeapon::LinkWeapon( DnActorHandle hActor, CDnWeapon *pWeapon )
{
	m_hActor = hActor;
	m_pParentWeapon = pWeapon;
	m_bLinkBone = true;

	if( GetElementIndex( "Idle" ) != -1 )
		SetAction( "Idle", 0.f, 0.f );
}

void CDnWeapon::UnlinkWeapon()
{
	m_bLinkBone = false;
	m_pParentWeapon = NULL;
}

CDnWeapon &CDnWeapon::operator = ( CDnWeapon &e )
{
	m_WeaponType = e.m_WeaponType;
	m_nClassID = e.m_nClassID;
	m_nLength = e.m_nLength;
	m_EquipType = e.m_EquipType;
	m_szFileName = e.m_szFileName;
	if( e.m_hObject ) {
		m_hObject = EternityEngine::CreateAniObject( GetRoom(), e.m_hObject->GetSkin()->GetFullName(), ( e.m_hObject->GetAniHandle() ) ? e.m_hObject->GetAniHandle()->GetFullName() : NULL );
	}
	LoadAction( CDnActionBase::m_szFileName.c_str() );

	return *this;
}

bool CDnWeapon::InitializeClass( CMultiRoom *pRoom )
{
	STATIC_INSTANCE_(s_dwUniqueCount) = 0;
	return true;
}

void CDnWeapon::ProcessClass( CMultiRoom *pRoom, LOCAL_TIME LocalTime, float fDelta )
{
	for( DWORD i=0; i<STATIC_INSTANCE_(s_pVecProcessList).size(); i++ ) {
		STATIC_INSTANCE_(s_pVecProcessList)[i]->Process( LocalTime, fDelta );
		if( STATIC_INSTANCE_(s_pVecProcessList)[i]->IsDestroy() ) 
		{
			CDnWeapon* pWeapon = STATIC_INSTANCE_(s_pVecProcessList)[i];
			SAFE_DELETE(pWeapon);
			i--;
		}
	}
}

void CDnWeapon::ReleaseClass( CMultiRoom *pRoom )
{
	DeleteAllObject( pRoom );
	STATIC_INSTANCE_(s_pVecProcessList).clear();
}

DnWeaponHandle CDnWeapon::FindWeaponFromUniqueID( CMultiRoom *pRoom, DWORD dwUniqueID )
{
	if( dwUniqueID == -1 ) return CDnWeapon::Identity();	
	DnWeaponHandle hWeapon;
	for( int i=0; i<GetItemCount(pRoom); i++ ) {
		hWeapon = GetItem(pRoom, i);
		if( !hWeapon ) continue;
		if( hWeapon->GetUniqueID() == dwUniqueID ) return hWeapon;
	}
	return CDnWeapon::Identity();
}

DnWeaponHandle CDnWeapon::FindWeaponFromUniqueIDAndShooterUniqueID( CMultiRoom *pRoom, DWORD dwUniqueID, DWORD dwShooterUniqueID )
{
	if( dwUniqueID == -1 ) return CDnWeapon::Identity();	
	DnWeaponHandle hWeapon;
	for( int i=0; i<GetItemCount(pRoom); i++ ) {
		if( !IsValidItem( pRoom, i ) )
		{
			g_Log.Log( LogType::_WEAPONTYPE, L"IsValidItem() validation failed!\r\n" );
			continue;
		}
		hWeapon = GetItem(pRoom, i);
		if( !hWeapon ) continue;
		if( hWeapon->GetUniqueID() == dwUniqueID ) {
			if( !( hWeapon->GetWeaponType() & CDnWeapon::Projectile ) ) continue;
			if( ((CDnProjectile*)hWeapon.GetPointer())->GetShooterActor() &&
				((CDnProjectile*)hWeapon.GetPointer())->GetShooterActor()->GetUniqueID() == dwShooterUniqueID ) return hWeapon;
		}
	}
	return CDnWeapon::Identity();
}

DnWeaponHandle CDnWeapon::CreateWeapon( CMultiRoom *pRoom, int nWeaponTableID, int nSeed, char cOptionIndex, char cEnchantLevel, char cPotentialIndex, bool bProcess, bool bIncreaseUniqueID, char cSealCount, bool bSoulBound )
{
	DnWeaponHandle hWeapon = (new IBoostPoolDnWeapon( pRoom, bProcess, bIncreaseUniqueID ))->GetMySmartPtr();
	hWeapon->Initialize( nWeaponTableID, nSeed, cOptionIndex, cEnchantLevel, cPotentialIndex, cSealCount, bSoulBound );
	return hWeapon;
}

void CDnWeapon::CreateObject()
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TWEAPON );
	if( !pSox->IsExistItem( m_nClassID ) ) return;

#ifdef PRE_FIX_MEMOPT_EXT
	DNTableFileFormat* pFileNameSox = GetDNTable( CDnTableDB::TFILE );
	if (pFileNameSox == NULL || g_pDataManager == NULL) return;

	std::string szSkinName, szAniName, szActName;
	g_pDataManager->GetFileNameFromFileEXT(szSkinName, pSox, m_nClassID, "_SkinName", pFileNameSox);
	g_pDataManager->GetFileNameFromFileEXT(szAniName, pSox, m_nClassID, "_AniName", pFileNameSox);
	g_pDataManager->GetFileNameFromFileEXT(szActName, pSox, m_nClassID, "_ActName", pFileNameSox);
#else
	std::string szSkinName = pSox->GetFieldFromLablePtr( m_nClassID, "_SkinName" )->GetString();
	std::string szAniName = pSox->GetFieldFromLablePtr( m_nClassID, "_AniName" )->GetString();
	std::string szActName = pSox->GetFieldFromLablePtr( m_nClassID, "_ActName" )->GetString();
#endif

	if( !szSkinName.empty() ) {
		m_hObject = EternityEngine::CreateAniObject( GetRoom(), CEtResourceMng::GetInstance().GetFullName( szSkinName ).c_str(), ( szAniName.empty() ) ? NULL : CEtResourceMng::GetInstance().GetFullName( szAniName ).c_str() );
		if( !m_hObject ) return;
	}

	if( !szActName.empty() ) LoadAction( CEtResourceMng::GetInstance().GetFullName( szActName ).c_str() );

	if( !szActName.empty() ) {
		if( GetElementIndex( "Idle" ) != -1 )
			SetAction( "Idle", 0.f, 0.f );
	}
	m_bCreateObject = true;
}

void CDnWeapon::RecreateCashWeapon( DnActorHandle hActor, int nEquipIndex )
{
	if( !IsCreateObject() ) return;
	if( !hActor ) return;
	if( !hActor->IsPlayerActor() ) return;
	if( nEquipIndex < 0 || nEquipIndex > 1 ) return;
	if( !IsCashItem() ) return;

	DnWeaponHandle hMainWeapon = hActor->GetWeapon( nEquipIndex );
	if( !hMainWeapon ) return;
	if( hMainWeapon->GetEquipType() == m_RecreateCashEquipType ) return;
	switch( hMainWeapon->GetEquipType() ) {
		case Gauntlet:
		case Staff:
		case Shield:
		case Arrow:
			return;
	}

	m_RecreateCashEquipType = hMainWeapon->GetEquipType();

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TWEAPON );
	if( !pSox->IsExistItem( m_nClassID ) ) return;

#ifdef PRE_FIX_MEMOPT_EXT
	DNTableFileFormat* pFileNameSox = GetDNTable( CDnTableDB::TFILE );
	if (!pFileNameSox)
	{
		_ASSERT(0);
		return;
	}
#endif

	static char *szSubStr[] = { "", "Sub1", "Sub2" };
	std::string szSkinName;
	std::string szAniName;
	std::string szActName;
	bool bValid = false;

	char szStr[64];
	for( int i=0; i<3; i++ ) {
		sprintf_s( szStr, "_EquipType%s", szSubStr[i] );
		int nEquipType = pSox->GetFieldFromLablePtr( m_nClassID, szStr )->GetInteger();
		if( nEquipType == m_RecreateCashEquipType ) {
#ifdef PRE_FIX_MEMOPT_EXT
			sprintf_s( szStr, "_SkinName%s", szSubStr[i] );
			g_pDataManager->GetFileNameFromFileEXT(szSkinName, pSox, m_nClassID, szStr, pFileNameSox);
			sprintf_s( szStr, "_AniName%s", szSubStr[i] );
			g_pDataManager->GetFileNameFromFileEXT(szAniName, pSox, m_nClassID, szStr, pFileNameSox);
			sprintf_s( szStr, "_ActName%s", szSubStr[i] );
			g_pDataManager->GetFileNameFromFileEXT(szActName, pSox, m_nClassID, szStr, pFileNameSox);
#else
			sprintf_s( szStr, "_SkinName%s", szSubStr[i] );
			szSkinName = pSox->GetFieldFromLablePtr( m_nClassID, szStr )->GetString();
			sprintf_s( szStr, "_AniName%s", szSubStr[i] );
			szAniName = pSox->GetFieldFromLablePtr( m_nClassID, szStr )->GetString();
			sprintf_s( szStr, "_ActName%s", szSubStr[i] );
			szActName = pSox->GetFieldFromLablePtr( m_nClassID, szStr )->GetString();
#endif

			bValid = true;
			break;
		}
	}
	if( !bValid ) return;

	SAFE_RELEASE_SPTR( m_hObject );
	FreeAction();

	int nMainWeaponClassID = hMainWeapon->GetClassID();
#ifdef PRE_FIX_MEMOPT_EXT
	if( szSkinName.empty() ) g_pDataManager->GetFileNameFromFileEXT(szSkinName, pSox, nMainWeaponClassID, "_SkinName", pFileNameSox);
	if( szAniName.empty() ) g_pDataManager->GetFileNameFromFileEXT(szAniName, pSox, nMainWeaponClassID, "_AniName", pFileNameSox);
	if( szActName.empty() ) g_pDataManager->GetFileNameFromFileEXT(szActName, pSox, nMainWeaponClassID, "_ActName", pFileNameSox);
#else
	if( szSkinName.empty() ) szSkinName = pSox->GetFieldFromLablePtr( nMainWeaponClassID, "_SkinName" )->GetString();
	if( szAniName.empty() ) szAniName = pSox->GetFieldFromLablePtr( nMainWeaponClassID, "_AniName" )->GetString();
	if( szActName.empty() ) szActName = pSox->GetFieldFromLablePtr( nMainWeaponClassID, "_ActName" )->GetString();
#endif

	if( !szSkinName.empty() ) {
		m_hObject = EternityEngine::CreateAniObject( GetRoom(), CEtResourceMng::GetInstance().GetFullName( szSkinName ).c_str(), ( szAniName.empty() ) ? NULL : CEtResourceMng::GetInstance().GetFullName( szAniName ).c_str() );
	}

	if( !szActName.empty() ) {
		LoadAction( CEtResourceMng::GetInstance().GetFullName( szActName ).c_str() );
	}

	if( !szActName.empty() ) {
		if( GetElementIndex( "Idle" ) != -1 )
			SetAction( "Idle", 0.f, 0.f );
	}
}

void CDnWeapon::FreeObject()
{
	UnlinkWeapon();
	m_bCreateObject = false;
	m_RecreateCashEquipType = (EquipTypeEnum)-1;
	SAFE_RELEASE_SPTR( m_hObject );
	FreeAction();
}

void CDnWeapon::ReleaseClass( CMultiRoom *pRoom, WeaponTypeEnum WeaponType )
{
	CDnWeapon *pWeapon;
	for( DWORD i=0; i<STATIC_INSTANCE_(s_pVecProcessList).size(); i++ ) {
		pWeapon = STATIC_INSTANCE_(s_pVecProcessList)[i];
		if( pWeapon->GetWeaponType() & WeaponType ) {
			SAFE_DELETE(pWeapon);
			i--;
		}
	}
}

CDnItem &CDnWeapon::operator = ( TItem &e )
{
	CDnItem::operator = (e);

	m_nDurability = e.wDur;

	return *this;
}

bool CDnWeapon::IsSubWeapon( CDnWeapon::EquipTypeEnum emType )
{
	switch( emType )
	{
	case CDnWeapon::Sword:
	case CDnWeapon::Axe:
	case CDnWeapon::Hammer:
	case CDnWeapon::SmallBow:
	case CDnWeapon::BigBow:
	case CDnWeapon::CrossBow:
	case CDnWeapon::Staff:
	case CDnWeapon::Mace:
	case CDnWeapon::Flail:
	case CDnWeapon::Wand:		
	case CDnWeapon::Cannon:
	case CDnWeapon::BubbleGun:	
	case CDnWeapon::Chakram:
	case CDnWeapon::Fan:
		return false;
	case CDnWeapon::Shield:
	case CDnWeapon::Arrow:
	case CDnWeapon::Book:
	case CDnWeapon::Orb:
	case CDnWeapon::Puppet:
	case CDnWeapon::Gauntlet:	
	case CDnWeapon::Glove:		
	case CDnWeapon::Charm:
		return true;
	default:
		ASSERT(0&&"CDnWeapon::IsSubWeapon");
		break;
	}
	return false;
}
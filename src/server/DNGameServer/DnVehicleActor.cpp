#include "stdafx.h"
#include <mmsystem.h>
#include "TaskManager.h"
#include "DNGameDataManager.h"
#include "DnWorld.h"
#include "navigationmesh.h"
#include "DnVehicleActor.h"
#include "DnActorClassDefine.h"
#include "DnMovableChecker.h"
#include "DnJumpableChecker.h"
#include "DnGroundMovableChecker.h"
#include "DnStateBlow.h"
#include "MAWalkMovement.h"
#include "DnGameTask.h"
#include "DNUserSession.h"

CDnVehicleActor::CDnVehicleActor( CMultiRoom *pRoom, int nClassID )
: CDnActor( pRoom, nClassID )
{
	CDnActionBase::Initialize( this );
	CDnVehicleState::Initialize(nClassID);

	m_bAttachToPlayer = false; 
	m_hMyActor.Identity();
	for(int i=0;i<2;i++)
		m_hSimpleParts[i].Identity();

	m_cMovePushKeyFlag = 0;
	m_dwHairColor = 0;
	// Related to speed hack
	m_dwSyncDatumTick = 0;
	m_vLastValidPos = EtVector3( 0.f, 0.f, 0.f );

#ifdef PRE_ADD_VEHICLE_ACTION_STRING
	m_strVehicleActionString = "Vehicle_"; // As Default
#endif

	SetHP(10);  // Mount HP currently does not use a separate table or data. Taking a hit immediately dismounts.
	SetMaxHP(10);
}

CDnVehicleActor::~CDnVehicleActor()
{
	for(int i=0;i<2;i++)
		SAFE_RELEASE_SPTR( m_hSimpleParts[i] );
}

void CDnVehicleActor::ProcessDie( LOCAL_TIME LocalTime, float fDelta )
{
	if( !IsDie() ) return;

	if(GetMyPlayerActor())
	{
		CDnPlayerActor *pPlayerActor = static_cast<CDnPlayerActor*>(GetMyPlayerActor().GetPointer());
		if(pPlayerActor->IsVehicleMode())
			pPlayerActor->ForceUnRideVehicle(); 
		// Mounts do not self-destruct; instead, handle it as the character dismounting.
		// After sending the packet and calling UnrideVehicle, SetDestroy is called.
	}

// 	m_fDieDelta -= fDelta; // Skip for now since die delta is not used anywhere.
// 	if( m_fDieDelta <= 0.f ) {
// 		SetDestroy();
// 		return;
// 	}
}


MAMovementBase* CDnVehicleActor::CreateMovement()
{
	MAMovementBase* pMovement = new IBoostPoolMAWalkMovement();
	return pMovement;
}

bool CDnVehicleActor::Initialize()
{
	CDnActor::Initialize();

	if( m_hObject ) {
		m_hObject->SetCollisionGroup( COLLISION_GROUP_DYNAMIC( 1 ) );
		m_hObject->SetTargetCollisionGroup( COLLISION_GROUP_STATIC( 1 ) | COLLISION_GROUP_DYNAMIC( 2 ) | COLLISION_GROUP_DYNAMIC( 3 ) ); // Temporarily set collision group to the same level as player actor.*/
	} 

	// SetPartsColor(m_hSimpleParts[Hair]->GetObjectHandle(),"g_CustomColor",EtInterface::textcolor::SILVER); // Set hair color. - No info yet -
	// SetPartsColor(m_hObject,"g_MaterialAmbient",EtInterface::textcolor::SILVER);
	// g_MaterialAmbient
	// g_MaterialDiffuse  Value applied to the mount body!  

	return true;
}

void CDnVehicleActor::EquipItem(TItem tInfo)
{
	TVehiclePartsData *pVehicleParts = g_pDataManager->GetVehiclePartsData(tInfo.nItemID);
	if (!pVehicleParts) return;

	if((pVehicleParts->nVehiclePartsType != Vehicle::Slot::Saddle) && (pVehicleParts->nVehiclePartsType != Vehicle::Slot::Hair))
		return;

	if(m_hSimpleParts[pVehicleParts->nVehiclePartsType-1])
	{
		SAFE_RELEASE_SPTR( m_hSimpleParts[pVehicleParts->nVehiclePartsType-1] );
		m_hSimpleParts[pVehicleParts->nVehiclePartsType-1] = CDnSimpleParts::CreateParts( GetRoom(), tInfo.nItemID, pVehicleParts->strSkinName.c_str(), m_szAniFileName.c_str() );
		
		// Set item info //
		m_hSimpleParts[pVehicleParts->nVehiclePartsType-1]->InitializeItem(tInfo.nItemID,0,tInfo.cOption,0,tInfo.cPotential,tInfo.cSealCount,tInfo.bSoulbound);
		m_hSimpleParts[pVehicleParts->nVehiclePartsType-1]->SetSerialID(tInfo.nSerial);
		//////////////////////

		if(pVehicleParts->nVehiclePartsType-1 == Vehicle::Parts::Hair)
			ChangeHairColor(m_dwHairColor);
	}
	else if(m_hSimpleParts[pVehicleParts->nVehiclePartsType-1] == NULL)
	{
		m_hSimpleParts[pVehicleParts->nVehiclePartsType-1] = CDnSimpleParts::CreateParts( GetRoom(), tInfo.nItemID, pVehicleParts->strSkinName.c_str(), m_szAniFileName.c_str() );
		m_hSimpleParts[pVehicleParts->nVehiclePartsType-1]->InitializeItem(tInfo.nItemID,0);

		// Set item info //
		m_hSimpleParts[pVehicleParts->nVehiclePartsType-1]->InitializeItem(tInfo.nItemID,0,tInfo.cOption,0,tInfo.cPotential,tInfo.cSealCount,tInfo.bSoulbound);
		m_hSimpleParts[pVehicleParts->nVehiclePartsType-1]->SetSerialID(tInfo.nSerial);
		//////////////////////

		if(pVehicleParts->nVehiclePartsType-1 == Vehicle::Parts::Hair)
			ChangeHairColor(m_dwHairColor);
	}
}

void CDnVehicleActor::UnEquipItem(int nSlot)
{
	TVehiclePartsData *pVehicleParts = g_pDataManager->GetVehiclePartsData(nSlot);
	if (!pVehicleParts) return;

	if((pVehicleParts->nVehiclePartsType != Vehicle::Slot::Saddle) && (pVehicleParts->nVehiclePartsType != Vehicle::Slot::Hair))
		return;

	if(m_hSimpleParts[pVehicleParts->nVehiclePartsType-1])
	{
		m_hSimpleParts[pVehicleParts->nVehiclePartsType-1]->FreeObject();
	}
	else
	{
		OutputDebug("Attempted to unequip mount item when none is equipped");
	}
}

void CDnVehicleActor::SetDefaultParts()
{
	TVehicleData *pVehicle = g_pDataManager->GetVehicleData(m_nItemID);
	if (!Vehicle) return;

	// Default parts only have information about item ID. //
	TItem tItem;
	memset(&tItem, 0, sizeof(tItem));
	tItem.nItemID = pVehicle->nDefaultParts1;
	EquipItem(tItem);
	tItem.nItemID = pVehicle->nDefaultParts2;
	EquipItem(tItem);
}

void CDnVehicleActor::ChangeHairColor(DWORD dwColor)
{
	if(!m_hSimpleParts[Vehicle::Parts::Hair])
		return;

	if(dwColor !=0 )
		m_dwHairColor = dwColor;
}

void CDnVehicleActor::SetDefaultHairColor()
{
	int nVehicleDefaultColor = g_pDataManager->GetItemTypeParam1(m_nItemID);

	ChangeHairColor(nVehicleDefaultColor); 
	if(m_dwHairColor == 0)
		m_dwHairColor = nVehicleDefaultColor; 
}

void CDnVehicleActor::Process(LOCAL_TIME LocalTime, float fDelta )
{
	CDnActor::Process( LocalTime, fDelta );//
	PreProcess( LocalTime, fDelta);//

	if( 0 < GetCantXZMoveSEReferenceCount() )
		m_vAniDistance.x = m_vAniDistance.z = 0.0f;

	m_pMovement->Process( LocalTime, fDelta );
	MASingleBody::Process( m_Cross, LocalTime, fDelta );//

	// SetParents effect on game server
	if(m_hMyActor && !m_hMyActor->IsDestroy()) 
		m_hMyActor->SetPosition(*GetPosition()); 
}

void CDnVehicleActor::OnSignal( SignalTypeEnum Type, void *pPtr, LOCAL_TIME LocalTime, LOCAL_TIME SignalStartTime, LOCAL_TIME SignalEndTime, int nSignalIndex )
{
	switch( Type ) {
		case STE_Projectile:
			return;

		case STE_Jump:
			{
				JumpStruct *pStruct = (JumpStruct *)pPtr;
				EtVector2 vVec( 0.f, 0.f );

				// Ignore directional keys when movement is disabled until the animation key.
				if( 0 == GetCantXZMoveSEReferenceCount() )
				{
					if( !pStruct->bIgnoreJumpDir) {
						if( m_cMovePushKeyFlag & 0x01 ) vVec.x -= 1.f;
						if( m_cMovePushKeyFlag & 0x02 ) vVec.x += 1.f;
						if( m_cMovePushKeyFlag & 0x04 ) vVec.y += 1.f;
						if( m_cMovePushKeyFlag & 0x08 ) vVec.y -= 1.f;

						EtVec2Normalize( &vVec, &vVec );
					}
				}

				if( pStruct->bResetPrevVelocity ) {
					SetVelocityY( 0.f );
					SetResistanceY( 0.f );
				}

				Jump( pStruct->fJumpVelocity , vVec );
				SetResistanceY( pStruct->fJumpResistance );
			}
			return;
	}

	CDnActor::OnSignal( Type, pPtr, LocalTime, SignalStartTime, SignalEndTime, nSignalIndex );
}

void CDnVehicleActor::CmdMove( EtVector3 &vPos, const char *szActionName, int nLoopCount, float fBlendFrame )
{
	MovePos( vPos, false );
	SetActionQueue( szActionName, nLoopCount, fBlendFrame );
}

void CDnVehicleActor::CmdStop( const char *szActionName, int nLoopCount, float fBlendFrame, float fStartFrame, bool bResetStop )
{
	MAMovementBase *pMovement = GetMovement();
	if( !pMovement ) return;

	pMovement->ResetMove();
	SetActionQueue( szActionName, nLoopCount, fBlendFrame, fStartFrame );
}

void CDnVehicleActor::CmdAction( const char *szActionName, int nLoopCount, float fBlendFrame, bool bCheckOverlapAction, 
								bool bFromStateBlow/* = false*/, bool bSkillChain/* = false*/ )
{
	CDnActor::CmdAction( szActionName, nLoopCount, fBlendFrame, bCheckOverlapAction, bFromStateBlow, bSkillChain );
}

void CDnVehicleActor::OnDrop( float fCurVelocity )
{
	if( IsAir() ) {
		if( !IsHit() ) {

			char szStr[64];
			sprintf_s( szStr, "%s_Landing", GetCurrentAction() );
			if( IsExistAction( szStr ) )
			{
				SetActionQueue( szStr, 0, 0.f, 0.f, true, false );
			}
			else {// Put this in because otherwise it bugs out in the air. Just matching and filling it in for now
				if( GetVelocity()->y != 0.f )
					SetActionQueue( "Stand", 0, 0.f, 0.f, true, false );
			}

			SetMovable( false );
		}
	}
}

void CDnVehicleActor::OnFall( float fCurVelocity )
{
	if( ( IsStay() || IsMove() ) && !IsFloorCollision() ) {
		// Also check movement angle to compensate for looking like falling when going down stairs, etc.
		EtVector3 vDir = *GetPosition() - *GetPrevPosition();
		EtVec3Normalize( &vDir, &vDir );
		float fDot = EtVec3Dot( &EtVector3( 0.f, 1.f, 0.f ), &vDir );
		float fAngle = EtToDegree( EtAcos( fDot ) );

		if( fCurVelocity < -4.f && fAngle > 155.f ) {
			ActionElementStruct *pStruct = GetElement( "Jump" );
			if( pStruct ) {
				SetActionQueue( "Jump", 0, 10.f, (float)pStruct->dwLength / 3.5f );
				SetMovable( false );
			}
		}
	}
}

int CDnVehicleActor::GetMoveSpeed()
{
	int nMoveSpeed = CDnActor::GetMoveSpeed();

	if( CDnWorld::IsActive(GetRoom()) && ( CDnWorld::GetInstance(GetRoom()).GetMapType() == EWorldEnum::MapTypeVillage || CDnWorld::GetInstance(GetRoom()).GetMapType() == EWorldEnum::MapTypeWorldMap ) )
		nMoveSpeed += GetSafeZoneMoveSpeed();

	if( m_cMovePushKeyFlag & 0x08 ) nMoveSpeed = 200;
	return nMoveSpeed;
}

bool CDnVehicleActor::SetActionQueue( const char *szActionName, int nLoopCount, float fBlendFrame, float fStartFrame, bool bCheck, bool bCheckStateEffect )
{
	std::string VehicleAction;
	std::string szCurAction = szActionName; 
#ifdef PRE_ADD_VEHICLE_ACTION_STRING
	VehicleAction = GetVehicleActionString() + szCurAction;
#else
	VehicleAction = "Vehicle_" + szCurAction;
#endif

	if(m_hMyActor && !m_hMyActor->IsDestroy() )
		m_hMyActor->SetActionQueue(VehicleAction.c_str(),nLoopCount,fBlendFrame,fStartFrame,bCheck,bCheckStateEffect); // Sync the player's action to the horse's action.


	return CDnActor::SetActionQueue( szActionName , nLoopCount, fBlendFrame, fStartFrame, bCheck, bCheckStateEffect );
}

void CDnVehicleActor::OnDispatchMessage( CDNUserSession *pSession, DWORD dwActorProtocol, BYTE *pPacket )
{
	switch( dwActorProtocol ) {
		case eActor::CS_CMDMOVE:
			{
				CPacketCompressStream Stream( pPacket, 128 );

				int	nActionIndex;
				int nMoveSpeed;
				EtVector3	vPos, vXVec;
				EtVector2	vZVec, vLook;
				char		cFlag;
				DWORD		dwGap;

				Stream.Read( &dwGap, sizeof(DWORD) );
				Stream.Read( &nActionIndex, sizeof(int), CPacketCompressStream::INTEGER_SHORT );
				Stream.Read( &vPos, sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
				Stream.Read( &vZVec, sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );
				Stream.Read( &vLook, sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );
				Stream.Read( &cFlag, sizeof(char) );
				Stream.Read( &nMoveSpeed, sizeof(int) );

				Look( vZVec, false );

				EtVec3Cross( &vXVec, &EtVector3( 0.f, 1.f, 0.f ), &EtVec2toVec3( vZVec ) );
				SetMoveVectorX( vXVec );
				SetMoveVectorZ( EtVec2toVec3( vZVec ) );
				SetMagnetPosition( vPos );


				m_cMovePushKeyFlag = cFlag;
				float fXSpeed = 0.f, fZSpeed = 0.f;
				if( cFlag & 0x01 ) fXSpeed = -100000.f;
				if( cFlag & 0x02 ) fXSpeed = 100000.f;
				if( cFlag & 0x04 ) fZSpeed = 100000.f;
				if( cFlag & 0x08 ) fZSpeed = -100000.f;
				vPos += ( vXVec * fXSpeed );
				vPos += ( EtVec2toVec3( vZVec ) * fZSpeed );

				ActionElementStruct *pStruct = m_hMyActor->GetElement( nActionIndex ); // !IMPORTANT! Must detect using the player actor's index 
				if( pStruct == NULL ) break;
	
				CmdMove( vPos, pStruct->szName.c_str(), -1, 8.f );
			}
			break;
		case eActor::CS_CMDSTOP:
			{
				CPacketCompressStream Stream( pPacket, 128 );

				EtVector3	vPos;
				bool		bResetStop;
				DWORD		dwGap;

				Stream.Read( &dwGap, sizeof(DWORD) );
				Stream.Read( &vPos, sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
				Stream.Read( &bResetStop, sizeof(bool) );

				std::string szActionName = GetCurrentAction();
				if( EtVec2Length( &( EtVector2( vPos.x, vPos.z ) - EtVector2( GetPosition()->x, GetPosition()->z ) ) ) > 100.f ) {
					if( GetState() != ActorStateEnum::Move ) szActionName = "Move_Front";
				}

				if(strstr(GetCurrentAction(),"Stand") == NULL)
				{
					CmdStop("Stand",-1,6.f);
				}
			}
			break;
		case eActor::CS_CMDACTION:
			{
				CPacketCompressStream Stream( pPacket, 128 );

				int nActionIndex, nLoopCount;
				float fBlendFrame;
				EtVector3 vPos, vXVec;
				EtVector2 vLook, vZVec;
				DWORD dwGap;
				bool bFromStateBlow = false;
				Stream.Read( &nActionIndex, sizeof(int), CPacketCompressStream::INTEGER_SHORT );
				Stream.Read( &dwGap, sizeof(DWORD) );
				Stream.Read( &nLoopCount, sizeof(int), CPacketCompressStream::INTEGER_CHAR );
				Stream.Read( &fBlendFrame, sizeof(float), CPacketCompressStream::FLOAT_SHORT, 10.f );
				Stream.Read( &vPos, sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
				Stream.Read( &vZVec, sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );
				Stream.Read( &vLook, sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );
				Stream.Read( &m_cMovePushKeyFlag, sizeof(char) );
				Stream.Read( &bFromStateBlow, sizeof(bool) );

				EtVec3Cross( &vXVec, &EtVector3( 0.f, 1.f, 0.f ), &EtVec2toVec3( vZVec ) );

				m_pMovement->ResetMove();

				SetMoveVectorX( vXVec );
				SetMoveVectorZ( EtVec2toVec3( vZVec ) );
				Look( vLook, true );
				SetMagnetPosition( vPos );

				ActionElementStruct *pStruct = GetElement( nActionIndex );
				if( pStruct == NULL ) break;

#ifdef PRE_ADD_VEHICLE_SPECIAL_ACTION
				if( CheckSpecialAction( pStruct->szName.c_str() ) == false )
				{
					if( m_hMyActor )
					{
						CDnPlayerActor *pPlayerActor = static_cast<CDnPlayerActor*>(m_hMyActor.GetPointer());
						pPlayerActor->ReportInvalidAction();
					}
					break;
				}
#endif
				CmdAction( pStruct->szName.c_str(), nLoopCount, fBlendFrame, false );
			}
			break;
		case eActor::CS_CMDLOOK:
			{
				CPacketCompressStream Stream( pPacket, 128 );
				EtVector2 vLook, vZVec;
				EtVector3 vXVec;

				Stream.Read( &vZVec, sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );                         
				Stream.Read( &vLook, sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );

				CmdLook( vLook );

				EtVec3Cross( &vXVec, &EtVector3( 0.f, 1.f, 0.f ), &EtVec2toVec3( vZVec ) );

				SetMoveVectorX( vXVec );
				SetMoveVectorZ( EtVec2toVec3( vZVec ) );
			}
			break;

		case eActor::CS_POSREV:
			{
				CPacketCompressStream Stream( pPacket, 128 );

				EtVector3 vPos;
				bool bMove;
				DWORD dwGap;

				Stream.Read( &vPos, sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
				Stream.Read( &dwGap, sizeof(DWORD) );
				Stream.Read( &bMove, sizeof(bool) );

				SetMagnetPosition( vPos );
				if( bMove ) {
					EtVector3 vXVec;
					EtVector2 vZVec;

					Stream.Read( &vZVec, sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );

					EtVec3Cross( &vXVec, &EtVector3( 0.f, 1.f, 0.f ), &EtVec2toVec3( vZVec ) );

					SetMoveVectorX( vXVec );
					SetMoveVectorZ( EtVec2toVec3( vZVec ) );

					if( EtVec3LengthSq( m_pMovement->GetMovePos() ) > 0.f ) {
						float fXSpeed = 0.f, fZSpeed = 0.f;
						if( m_cMovePushKeyFlag & 0x01 ) fXSpeed = -100000.f;
						if( m_cMovePushKeyFlag & 0x02 ) fXSpeed = 100000.f;
						if( m_cMovePushKeyFlag & 0x04 ) fZSpeed = 100000.f;
						if( m_cMovePushKeyFlag & 0x08 ) fZSpeed = -100000.f;
						vPos += ( vXVec * fXSpeed );
						vPos += ( EtVec2toVec3( vZVec ) * fZSpeed );
						MovePos( vPos, false );
					}
				}

				OutputDebug( "PosRev\n" );
			}
			break;
		case eActor::CS_VIEWSYNC:
			{
				CPacketCompressStream Stream( pPacket, 128 );

				EtVector3 vPos, vXVec;
				EtVector2 vZVec, vLook;
				DWORD dwGap;

				Stream.Read( &dwGap, sizeof(DWORD) );
				Stream.Read( &vPos, sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
				Stream.Read( &vZVec, sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );
				Stream.Read( &vLook, sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );

				Look( vLook, true );

				EtVec3Cross( &vXVec, &EtVector3( 0.f, 1.f, 0.f ), &EtVec2toVec3( vZVec ) );

				SetMoveVectorX( vXVec );
				SetMoveVectorZ( EtVec2toVec3( vZVec ) );

				SetMagnetPosition( vPos );

				OutputDebug( "ViewSync\n" );

			}
			break;
		case eActor::CS_ONDROP:
			{
				CPacketCompressStream Stream( pPacket, 128 );
				EtVector3 vPos;
				DWORD dwGap;

				Stream.Read( &vPos, sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
				Stream.Read( &dwGap, sizeof(DWORD) );

				SetMagnetPosition( vPos );
				SetJumpMovement( EtVector2( 0.f, 0.f ) );
			}
			break;
		case eActor::CS_CMDESCAPE:
			{
				CPacketCompressStream Stream( pPacket, 32 );

				EtVector3 vPos;
				Stream.Read( &vPos, sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );

				SetPosition( vPos );
				SetPrevPosition( vPos );
				SetActionQueue( "Stand" );

				CmdWarp( vPos, EtVec3toVec2( *GetMoveVectorZ() ) );
			}
			break;
	}
}

int CDnVehicleActor::CmdAddStateEffect( const CDnSkill::SkillInfo* pParentSkill, STATE_BLOW::emBLOW_INDEX emBlowIndex, int nDurationTime, const char *szParam, bool bOnPlayerInit/* = false */, bool bCheckCanBegin/* = true*/ , bool bEternity )
{
	int iID = CDnActor::CmdAddStateEffect( pParentSkill, emBlowIndex, nDurationTime, szParam, bOnPlayerInit, bCheckCanBegin , bEternity );
	if( -1 == iID ) 
		return -1;

	DnBlowHandle hAddedBlow = m_pStateBlow->GetStateBlowFromID( iID );

	const CPacketCompressStream* pPacket = hAddedBlow->GetPacketStream( szParam, bOnPlayerInit );
	Send( eActor::SC_CMDADDSTATEEFFECT, const_cast<CPacketCompressStream*>(pPacket) );

	return iID;
}

void CDnVehicleActor::SendRemoveStateEffect( STATE_BLOW::emBLOW_INDEX emBlowIndex )
{
	BYTE pBuffer[32];
	CPacketCompressStream Stream( pBuffer, 32 );
	Stream.Write( &emBlowIndex, sizeof(STATE_BLOW::emBLOW_INDEX) );

	Send( eActor::SC_CMDREMOVESTATEEFFECT, &Stream );
}

void CDnVehicleActor::SetMagnetPosition( EtVector3 &vPos )
{
	EtVector2 vMagnet;

	float fHeightDiff = vPos.y - GetMatEx()->m_vPosition.y;
	vMagnet.x = vPos.x - GetMatEx()->m_vPosition.x;
	vMagnet.y = vPos.z - GetMatEx()->m_vPosition.z;
	float fAberraionLength = EtVec2Length( &vMagnet );

	if( ( fAberraionLength > 1.f && fAberraionLength < 300.f ) && fHeightDiff < 300.f ) {
		SetMagnetLength( fAberraionLength );
		EtVec2Normalize( &vMagnet, &vMagnet );
		SetMagnetDir( vMagnet );
	}
	else {
		SetPosition( vPos );
	}
}

#ifdef PRE_ADD_VEHICLE_SPECIAL_ACTION
bool CDnVehicleActor::CheckSpecialAction( const char *szAction )
{
	if( !m_hMyActor )
		return false;
	CDnPlayerActor *pPlayerActor = static_cast<CDnPlayerActor*>(m_hMyActor.GetPointer());

	int nCurrentMapID = 0;
	CDnGameTask *pGameTask = (CDnGameTask *)CTaskManager::GetInstance(GetRoom()).GetTask( "GameTask" );
	if( pGameTask )
		nCurrentMapID = pGameTask->GetMapTableID();
	else
		nCurrentMapID = pPlayerActor->GetUserSession()->GetMapIndex();

	bool bEnableSpecialAction = false; // g_pDataManager->IsVehicleSpecialActionMode(nCurrentMapID);

	if( bEnableSpecialAction == false )
	{
		if( strstr( szAction, "Special_" ) )
			return false;
	}

	return true;
}
#endif
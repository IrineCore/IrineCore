#include "stdafx.h"
#include "DnNPCActor.h"

#include "DnTableDB.h"
#include "DnActorClassDefine.h"
#include "DNQuest.h"
#include "DNQuestManager.h"
#include "DnActor.h"
#include "DNRUDPGameServer.h"
#include "DNUserSession.h"
#include "DNGameServerManager.h"
#include "DnWorld.h"
#include "DnWorldActProp.h"

DnActorHandle CreateNpcActor(CMultiRoom *pRoom, UINT nNpcID, TNpcData* pNpcData, EtVector3 Position, float fRotate)
{
	std::string szName;
	int  nActorTableID = -1;

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TNPC );

	DNTableCell* pSoxField = pSox->GetFieldFromLablePtr( nNpcID, "_ActorIndex" );
	if ( !pSoxField )
		return CDnActor::Identity();

	nActorTableID  = pSoxField->GetInteger();

	// Spawn NPC
	DnActorHandle hNPC;
	hNPC = CreateActor( pRoom, nActorTableID );
	if( !hNPC )
		return CDnActor::Identity();

	CDnNPCActor* pNpc = dynamic_cast<CDnNPCActor*>(hNPC.GetPointer());
	if ( pNpc )
	{
		pNpc->SetUniqueID( STATIC_INSTANCE_(CDnActor::s_dwUniqueCount)++ );
		pNpc->SetPosition( Position );
		pNpc->Initialize(pNpcData);

		EtVector2 vDir( sin( EtToRadian( fRotate ) ), cos( EtToRadian( fRotate ) ) );
		pNpc->Look( vDir );

		hNPC->SetActionQueue( "Stand" );
	}
	return hNPC;
}


CDnNPCActor::CDnNPCActor( CMultiRoom *pRoom, int nClassID )
: CDnActor( pRoom, nClassID )
{
	CDnActionBase::Initialize( this );
	CDnActorState::Initialize(nClassID);
	SetActionQueue("Stand");

	SetHP(INT_MAX);	

	m_bIsTriggerNpc = false;
	m_bNpcMoveActive = false;
	m_vNpcMoveTarget = EtVector3( 0.f, 0.f, 0.f );
	m_fNpcMoveSpeed = 360.f;
}

CDnNPCActor::~CDnNPCActor()
{

}

MAMovementBase* CDnNPCActor::CreateMovement()
{
	MAMovementBase* pMovement = new IBoostPoolMAWalkMovement();
	return pMovement;
}

void CDnNPCActor::Initialize(TNpcData* pNpcData)
{
#ifdef PRE_FIX_GAMESERVER_OPTIMIZE
	CDnNPCActor::Initialize();
#else
	CDnActor::Initialize();
#endif

	memset(&m_NpcData, 0, sizeof(TNpcData));
	if( pNpcData )
		memcpy(&m_NpcData, pNpcData, sizeof(TNpcData));
	
	CDnNpc::Create(pNpcData);
}
#ifdef PRE_FIX_GAMESERVER_OPTIMIZE
bool CDnNPCActor::Initialize()
{
	bool bResult = CDnActor::Initialize();

	if( m_hObject ) {
		m_hObject->SetCollisionGroup( COLLISION_GROUP_DYNAMIC( 3 ) );
		m_hObject->SetTargetCollisionGroup( 0 );
		m_nPressLevel = 0;
	}

	return bResult;
}
#endif

void CDnNPCActor::Process(LOCAL_TIME LocalTime, float fDelta )
{
	CDnActor::Process( LocalTime, fDelta );

	MASingleBody::PreProcess( LocalTime, fDelta );

	if( m_bNpcMoveActive )
	{
		if( EtVector3* pAniDist = GetAniDistance() )
			*pAniDist = EtVector3( 0.f, 0.f, 0.f );

		EtVector3 vDiff = m_vNpcMoveTarget - m_Cross.m_vPosition;
		vDiff.y = 0.f;
		float fDist = EtVec3Length( &vDiff );
		if( fDist <= 18.f )
		{
			m_Cross.m_vPosition.x = m_vNpcMoveTarget.x;
			m_Cross.m_vPosition.z = m_vNpcMoveTarget.z;
			m_Cross.m_vPosition.y = INSTANCE(CDnWorld).GetHeight( m_Cross.m_vPosition );
			CmdStop( "Stand" );
		}
		else if( fDelta > 0.f )
		{
			EtVector3 vDir = vDiff / fDist;
			float fStep = min( fDist, m_fNpcMoveSpeed * fDelta );
			m_Cross.m_vPosition.x += vDir.x * fStep;
			m_Cross.m_vPosition.z += vDir.z * fStep;
			m_Cross.m_vPosition.y = INSTANCE(CDnWorld).GetHeight( m_Cross.m_vPosition );

			EtVector2 vLook2D( vDir.x, vDir.z );
			Look( vLook2D, false );
		}
	}

	m_pMovement->Process( LocalTime, fDelta );
	MASingleBody::Process( m_Cross, LocalTime, fDelta );
}

bool CDnNPCActor::Talk(CDNUserBase* pUserBase, UINT nNpcUniqueID, IN std::wstring& wszIndex, IN std::wstring& wszTarget)
{
	CDNUserSession* pUserSession = static_cast<CDNUserSession*>(pUserBase);
	UINT userObjectID = pUserSession->GetSessionID();

	CDNRUDPGameServer * pServer = g_pGameServerManager->GetGameServerByUID(userObjectID);
	if (pServer == NULL)	
	{
		_DANGER_POINT();
		return false;
	}

	DN_ASSERT(NULL != pUserBase,	"Invalid!");

	std::transform(wszTarget.begin(), wszTarget.end(), wszTarget.begin(), towlower); 
	TALK_MAP_IT iter = m_TalkMap.find(wszTarget);
	
	// If not NPC talk 
	if ( iter == m_TalkMap.end() )
	{
		// Search through the quests.
		CDNQuest* pQuest = g_pQuestManager->GetQuest(wszTarget);

		if ( !pQuest )
		{
			g_Log.Log(LogType::_ERROR, L"QuestFind : Cannot found TalkTarget : %s\n",  wszTarget.c_str());
			_DANGER_POINT();
			return false;
		}

		pUserSession->SetCalledNpcResponse(false, false);

		// If the dialogue file target is a quest, execute quest '..._OnTalk'
		return pQuest->OnTalk(pUserSession->GetGameRoom(), userObjectID, GetUniqueID(),  wszIndex, wszTarget);
	}

	pUserSession->SetCalledNpcResponse(false, false);

	return m_TalkMap[wszTarget]->OnTalk(userObjectID, GetUniqueID(),  wszIndex, wszTarget);
	
}

void CDnNPCActor::CmdAction( const char *szActionName, int nLoopCount, float fBlendFrame, bool bCheckOverlapAction, 
							 bool bFromStateBlow/* = false*/, bool bSkillChain/* = false*/ )
{
	m_bNpcMoveActive = false;

	if( SetActionQueue( szActionName, nLoopCount, fBlendFrame, 0.f, bCheckOverlapAction ) == false ) 
		return;

	int nActionIndex = GetElementIndex( szActionName );

	BYTE pBuffer[128];
	CPacketCompressStream Stream( pBuffer, 128 );

	Stream.Write( &nActionIndex, sizeof(int), CPacketCompressStream::INTEGER_SHORT );
	Stream.Write( &nLoopCount, sizeof(int), CPacketCompressStream::INTEGER_CHAR );
	Stream.Write( &fBlendFrame, sizeof(float), CPacketCompressStream::FLOAT_SHORT, 1.f );

	Send( eActor::SC_CMDACTION, &Stream );
}

void CDnNPCActor::CmdMove( EtVector3 &vPos, const char *szActionName, int nLoopCount, float fBlendFrame )
{
	if( !szActionName || GetElementIndex( szActionName ) == -1 )
		return;

	m_vNpcMoveTarget = vPos;
	m_bNpcMoveActive = true;
	m_fNpcMoveSpeed = 360.f;

	EtVector3 vDiff = vPos - m_Cross.m_vPosition;
	vDiff.y = 0.f;
	EtVector2 vDir2D( vDiff.x, vDiff.z );
	if( EtVec2LengthSq( &vDir2D ) > 0.0001f )
	{
		EtVec2Normalize( &vDir2D, &vDir2D );
		Look( vDir2D, false );
	}
	else
	{
		vDir2D = EtVec3toVec2( *GetLookDir() );
	}

	int nActionIndex = GetElementIndex( szActionName );
	if( GetCurrentActionIndex() != nActionIndex )
		SetActionQueue( szActionName, nLoopCount, fBlendFrame, 0.f, true );

	BYTE pBuffer[128];
	CPacketCompressStream Stream( pBuffer, 128 );

	Stream.Write( &nActionIndex, sizeof(int), CPacketCompressStream::INTEGER_SHORT );
	Stream.Write( GetPosition(), sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
	Stream.Write( &vPos, sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
	Stream.Write( &vDir2D, sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );
	Stream.Write( &vDir2D, sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );
	Stream.Write( &nLoopCount, sizeof(int), CPacketCompressStream::INTEGER_CHAR );

	Send( eActor::SC_CMDMOVE, &Stream );
}

void CDnNPCActor::CmdStop( const char *szActionName, int nLoopCount, float fBlendFrame, float fStartFrame )
{
	m_bNpcMoveActive = false;

	if( !szActionName || GetElementIndex( szActionName ) == -1 )
		return;

	int nActionIndex = GetElementIndex( szActionName );
	if( GetCurrentActionIndex() != nActionIndex )
		SetActionQueue( szActionName, nLoopCount, fBlendFrame, fStartFrame, true );

	ResetMove();
	ResetLook();

	BYTE pBuffer[128];
	CPacketCompressStream Stream( pBuffer, 128 );

	Stream.Write( &nActionIndex, sizeof(int), CPacketCompressStream::INTEGER_SHORT );
	Stream.Write( &nLoopCount, sizeof(int), CPacketCompressStream::INTEGER_CHAR );
	Stream.Write( GetPosition(), sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );

	Send( eActor::SC_CMDSTOP, &Stream );
}

void CDnNPCActor::CmdEnableOperator( DnActorHandle hActor, bool bEnable )
{
	if( bEnable ) {
		DNVector(DnActorHandle)::iterator it = std::find( m_hVecDisableActor.begin(), m_hVecDisableActor.end(), hActor );
		if( it == m_hVecDisableActor.end() ) return;
		m_hVecDisableActor.erase( it );
	}
	else {
		if( std::find( m_hVecDisableActor.begin(), m_hVecDisableActor.end(), hActor ) != m_hVecDisableActor.end() ) return;
		m_hVecDisableActor.push_back( hActor );
	}

	BYTE pBuffer[128];
	CPacketCompressStream Stream( pBuffer, 128 );

	Stream.Write( &bEnable, sizeof(bool) );

	Send( eActor::SC_CMDENABLEOPERATOR, hActor, &Stream );
}

void CDnNPCActor::OnSignal( SignalTypeEnum Type, void *pPtr, LOCAL_TIME LocalTime, LOCAL_TIME SignalStartTime, LOCAL_TIME SignalEndTime, int nSignalIndex )
{
	switch( Type ) {
		case STE_Projectile:
			return;
	}
	CDnActor::OnSignal( Type, pPtr, LocalTime, SignalStartTime, SignalEndTime, nSignalIndex );
}

void CDnNPCActor::OnChangeAction( const char *szPrevAction )
{
	if( m_hPropHandle ) ((CDnWorldActProp*)m_hPropHandle.GetPointer())->SetAction( m_szAction.c_str(), CDnActionBase::m_fFrame, 0.f, false );
}

void CDnNPCActor::OnChangeActionQueue( const char *szPrevAction )
{
	if( m_hPropHandle ) ((CDnWorldActProp*)m_hPropHandle.GetPointer())->SetActionQueue( m_szActionQueue.c_str(), m_nLoopCount, m_fQueueBlendFrame, m_fQueueStartFrame );
}

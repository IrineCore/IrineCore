#include "stdafx.h"
#include "DnBasiliskMonsterActor.h"
#include "MAAiBase.h"
//#include "MAAiBasilisk.h"
#include "DnWeapon.h"
#include "CollisionAndPush.h"

static std::map<std::string, float>	__gsActionAgleMap;

class __FindActionAngle
{
public:
	__FindActionAngle()
	{
		__gsActionAgleMap.insert(std::pair<std::string, float>("Pillar_ComeDown", -90.0f));
		__gsActionAgleMap.insert(std::pair<std::string, float>("Turn_Left", 90.0f));
		__gsActionAgleMap.insert(std::pair<std::string, float>("Turn_Right", -90.0f));
		__gsActionAgleMap.insert(std::pair<std::string, float>("RunAttack", 180.0f));
		__gsActionAgleMap.insert(std::pair<std::string, float>("StandUp", 180.0f));
	}
	virtual ~__FindActionAngle()	{ }

	static float Find(const char* szAction)
	{
		if ( __NULL_STR == szAction )
			return 0.0f;

		std::map<std::string, float>::iterator it = __gsActionAgleMap.find(std::string(szAction));
		if ( it == __gsActionAgleMap.end() )
			return 0.0f;

		float fAngle = it->second;
		return fAngle;
	}
}__FindActionAngleInit;

CDnBasiliskMonsterActor::CDnBasiliskMonsterActor( CMultiRoom *pRoom, int nClassID )
: TDnMonsterActor( pRoom, nClassID )
{
	m_HitCheckType = HitCheckTypeEnum::Collision;

	m_fShakeDelta = 0.f;
	m_nCustomParamIndex = -1;
	SetHittable(false);
	m_pLua = NULL;
	m_nLastLegRepairTime = 0;
	m_szPrevAction =  "";

	m_fArmorTotalDurability = 0.0f;
	m_fLegTotalDurablility = 0.0f;
	m_nArmorCnt = 0;
	memset (&m_Property, 0x00, sizeof(m_Property));
}

CDnBasiliskMonsterActor::~CDnBasiliskMonsterActor()
{
	

	if ( m_pLua) 		lua_close(m_pLua);
}

MAMovementBase* CDnBasiliskMonsterActor::CreateMovement()
{
	MAMovementBase* pMovement = new IBoostPoolMAStaticMovement();
	return pMovement;
}

bool
CDnBasiliskMonsterActor::Initialize()
{
	TDnMonsterActor< MASingleBody, MAMultiDamage >::Initialize();
	//m_hObject->SetCalcPositionFlag(CALC_POSITION_X|CALC_POSITION_Y|CALC_POSITION_Z);
	m_hObject->SetCalcPositionFlag(0);
	m_hObject->SetCollisionGroup( COLLISION_GROUP_DYNAMIC( 3 ) );
	m_hObject->SetTargetCollisionGroup( 0 );

	_LoadProperty("BasiliskAI.lua");
	_InitRegionDamage();
	m_nArmorCnt = (int)m_Property.fArmorCnt;

	m_nLastLegRepairTime = 0;
	return true;
}


void 
CDnBasiliskMonsterActor::_InitRegionDamage()
{
	m_LegDamageMap.clear();

	/*
Right
	Bone . Object       Bone . Object
	1   16  =>  24          17  =>  25
	2   24  =>  26          25  =>  27
	3   28  =>  28          29  =>  29
	4   36  =>  30          37  =>  31

Left
	Bone . Object       Bone . Object
	1   12  =>  22          13  =>  23
	2   20  =>  20          21  =>  21
	3   32  =>  18          33  =>  19
	4   40  =>  01          41  =>  17
	*/
	LEG_INFO info[] =
	{
		// Left
		{ "Bone12", "Object02"},
		{ "Bone20", "Object03"},
		{ "Bone32", "Object04"},
		{ "Bone40", "Object05"},

		{ "Bone13", "Object23"},
		{ "Bone21", "Object21"},
		{ "Bone33", "Object19"},
		{ "Bone41", "Object17"},

		// Right
		{ "Bone16", "Object09"},
		{ "Bone24", "Object08"},
		{ "Bone28", "Object07"},
		{ "Bone36", "Object06"},

		{ "Bone17", "Object25"},
		{ "Bone25", "Object27"},
		{ "Bone29", "Object29"},
		{ "Bone37", "Object31"},
	};

	for ( int i = 0 ; i < _countof(info) ; i++ )
	{
		m_LegDamageMap.insert(std::pair<std::string, LEG_INFO>( info[i].szBoneName, info[i] ));
	}

	ClearMeshDiffuseColor();
}

void CDnBasiliskMonsterActor::Process( LOCAL_TIME LocalTime, float fDelta )
{
	TDnMonsterActor::Process(LocalTime, fDelta);
	
	// Recover a fixed amount of leg/bridge durability per hour.
	if ( LocalTime - m_nLastLegRepairTime >= LOCAL_TIME(m_Property.fLegDurabilityRepairTime) )
	{
		m_fLegTotalDurablility += m_Property.fLegDurabilityRepairValue;
		m_nLastLegRepairTime = LocalTime;

		// If durability exceeds max value, reset to max value.
		if ( m_fLegTotalDurablility >= m_Property.fLegDurability * m_Property.fLegCnt )
		{
			m_fLegTotalDurablility = m_Property.fLegDurability * m_Property.fLegCnt;
		}

		//LogWnd::Log(3, _T("Leg recovered! Durability:%.2f") , m_fLegTotalDurablility);
	}


	// When colliding with a monster.. code for taking damage. 
	if ( false )
	{
		EtVector3 *pvPos = GetPosition();
		DNVector(DnActorHandle) hVecList;
		int nCount = CDnActor::ScanActor( GetRoom(), *pvPos, 3000.f, hVecList );

		for ( size_t i = 0 ; i < hVecList.size() ; ++i )
		{
			DNVector(SCollisionResponse) vecResult;

			DnActorHandle hActor = hVecList[i];

			if( hVecList[i] == GetMySmartPtr() ) continue;
			if( hVecList[i]->GetTeam() == GetTeam() ) continue;
			if( hVecList[i]->IsDie() ) continue;

			LOCAL_TIME nLastDamageTime = hActor->GetDamageRemainTime( GetUniqueID() );
			if( nLastDamageTime  > LocalTime ) 
				continue;

			bool bCollision = m_hObject->FindCollision( hActor->GetObjectHandle(), vecResult, false );
			if ( bCollision /* && !bOnce */ )
			{
				// Push it out roughly for now..
				EtVector3 vVel = *(hActor->GetVelocity());
				EtVector3 vPos =  *(hActor->GetPosition());

				//EtVec3Normalize(&vVel, &vVel);
				vPos -= vVel;
				//EtVec3Normalize(&vPos, &vPos);

				hActor->GetMovement()->MoveToWorld(EtVector2(-vVel.x, -vVel.z));

				/*
LogWnd::Log(3,_T("Collision"));
				EtVector3 vDir;
				EtVector2 vView;
				vDir = vDestPoint - vMyPoint;
				vView = EtVec3toVec2( -vDir );
				EtVec3Normalize(&vDir, &vDir);
				EtVec2Normalize( &vView, &vView );

				CDnDamageBase::SHitParam HitParam;
				HitParam.RemainTime = LocalTime+2000;
				HitParam.hWeapon = GetMySmartPtr();
				HitParam.bFirstHit = true;
				HitParam.vPosition = vDestPoint;
				HitParam.vVelocity = EtVector3( 0.f, 0.f, -20.f );
				HitParam.vResistance = EtVector3( 0.f, 0.f, 30.f );
				HitParam.fDamage = 1.0f;
				HitParam.fDurability = 1.0f;
				HitParam.fStiffProb = 1.0f;
				HitParam.szActionName = "Down_KnockBack";
				HitParam.vViewVec = vView;
				//hActor->OnDamage( GetMySmartPtr(), HitParam);
				*/
			}
		}
	}

	ProcessCollisionAndPush( m_hObject, m_Cross );
}

void
CDnBasiliskMonsterActor::OnDamage( CDnDamageBase *pHitter, SHitParam &HitParam, HitStruct *pHitStruct )
{
	HitParam.szActionName.clear();
	CDnMonsterActor::OnDamage( pHitter, HitParam, pHitStruct );
	m_fShakeDelta = 0.2f;

	switch( pHitter->GetDamageObjectType() ) {
		case DamageObjectTypeEnum::Actor:
			m_pAi->OnDamage( pHitter->GetActorHandle(), HitParam, CDnActionBase::m_LocalTime );
			break;
		case DamageObjectTypeEnum::Prop:
			break;
	}

	if ( IsDie() )
	{
		m_pAi->OnDie(CDnActionBase::m_LocalTime);
		SetDieDelta(9999.0f);
	}

}

void
CDnBasiliskMonsterActor::ProcessDie( LOCAL_TIME LocalTime, float fDelta )
{
	//@
	//m_fLegTotalDurablility = 0.0f;
	//m_fArmorTotalDurability = 0.0f;
	//@

	if( !IsDie() ) return;
	if( m_fDieDelta == 0.f ) return;
	m_fDieDelta -= fDelta;
	if( m_fDieDelta <= 0.f ) {
		SetDestroy();
		return;
	}
}

void CDnBasiliskMonsterActor::CmdAction( const char *szActionName, int nLoopCount, float fBlendFrame, bool bCheckOverlapAction, bool bFromStateBlow/* = false*/, bool bSkillChain/* = false*/ )
{

	float fAngle = __FindActionAngle::Find(m_szPrevAction.c_str());
	m_hActor->GetMatEx()->RotateYaw(fAngle);

	std::wstring wszLog;
	std::string szLog =	FormatA("[[PrevAction:%s CurAction:%s yaw:%0.2f]]\n", m_hActor->GetCurrentAction(), szActionName, fAngle);
	ToWideString(szLog, wszLog);

	//g_Log.Log((WCHAR*)wszLog.c_str());

	OutputDebug("[[PrevAction:%s CurAction:%s yaw:%0.2f]]\n", m_hActor->GetCurrentAction(), szActionName, fAngle);

	CDnMonsterActor::CmdAction(szActionName, nLoopCount, fBlendFrame, false, bFromStateBlow );
	if( strlen( m_hActor->GetCurrentAction() ) > 0 )
		m_szPrevAction = m_hActor->GetCurrentAction();
}

void 
CDnBasiliskMonsterActor::SetMeshDiffuseColor(const char* szMeshName, EtColor& clr)
{
	std::map<std::string, EtColor>::iterator it = m_MeshDiffuseColorMap.find(std::string(szMeshName));

	if ( it != m_MeshDiffuseColorMap.end() )
	{
		EtColor& _clr = (it->second);
		_clr = clr;
	}
	else
	{
		m_MeshDiffuseColorMap.insert(std::pair<std::string, EtColor>(std::string(szMeshName), clr));
	}
}


bool 
CDnBasiliskMonsterActor::_LoadProperty(const char* szFile)
{
	std::string szFullName;
	szFullName = CEtResourceMng::GetInstance().GetFullName( szFile ).c_str();

	m_pLua =  lua_open();
	luaL_openlibs(m_pLua); 
	if ( lua_tinker::dofile(m_pLua, szFullName.c_str()) != 0 )
	{
		lua_close(m_pLua);
		return false;
	}

	lua_tinker::table PropTable = lua_tinker::get<lua_tinker::table>(m_pLua, "g_luaBasiliskPropertyTable");

	m_Property.nGenerationMonsterGroupID	= PropTable.get<int>("nGenerationMonsterGroupID");
	m_Property.nGenerationMonsterCount		= PropTable.get<int>("nGenerationMonsterCount");
	m_Property.nMaxActorSize				= PropTable.get<int>("nMaxActorSize");
	
	m_Property.fArmorDurability				= PropTable.get<float>("fArmorDurability");
	m_Property.fArmorDefensePercent			= PropTable.get<float>("fArmorDefensePercent");
	m_Property.fArmorCnt					= PropTable.get<float>("fArmorCnt");
	m_Property.fRangeDefensePercent			= PropTable.get<float>("fRangeDefensePercent");
	m_Property.fArmorDurabilityDamage		= PropTable.get<float>("fArmorDurabilityDamage");
	
	m_Property.fFall_to_the_ground_damage	= PropTable.get<float>("fFall_to_the_ground_damage");
	m_Property.fLegCnt						= PropTable.get<float>("fLegCnt");
	m_Property.fLegDurabilityDamage			= PropTable.get<float>("fLegDurabilityDamage");
	m_Property.fLegDurability				= PropTable.get<float>("fLegDurability");
	m_Property.fLegDurabilityRepairValue	= PropTable.get<float>("fLegDurabilityRepairValue");
	m_Property.fLegDurabilityRepairTime		= PropTable.get<float>("fLegDurabilityRepairTime");
	m_Property.fLoseBalanceValue			= PropTable.get<float>("fLoseBalanceValue");
	m_Property.fFallDownValue				= PropTable.get<float>("fFallDownValue");

	lua_close(m_pLua);
	m_pLua = NULL;

	// Calculate total leg durability
	m_fLegTotalDurablility = m_Property.fLegDurability * m_Property.fLegCnt;

	// Calculate total shell durability
	m_fArmorTotalDurability = m_Property.fArmorDurability * m_Property.fArmorCnt;


	return true;
}

float
CDnBasiliskMonsterActor::CalcDamage( DnActorHandle hHitter, SHitParam &HitParam )
{
	float fResult = TDnMonsterActor< MASingleBody, MAMultiDamage >::CalcDamage( hHitter, HitParam );

	/*
nState : current Basilisk state, 1 if Overturned
AttackType : if 0
AttackFoot : 1 for leg attack, 0 for other body parts
nOriginalDamage : original damage value,
nArmorCnt : remaining shell count
   */

	int nState = 0;
	int AttackType = 0;
	int AttackFoot = 0;
	float nOriginalDamage = fResult;
	int nArmorCnt = m_nArmorCnt;


	//MAAiBasilisk* pBasilAi = dynamic_cast<MAAiBasilisk*>(m_pAi);
	
	if ( GetCurrentPlayAction() == "OverTurned" )
	{
		nState = 1;
	}
	
	if ( HitParam.hWeapon->GetWeaponType() & CDnWeapon::Projectile )
	{
		AttackType = 1;
	}


	// Slightly reduce armor durability.
	m_fArmorTotalDurability -= m_Property.fArmorDurabilityDamage;

	float fFinalResult = _CalcDamage( nState, AttackType, AttackFoot, nOriginalDamage, nArmorCnt);	
	//float fFinalResult = lua_tinker::call<float>(m_pLua, "lua_CalcDamage", nState, AttackType, AttackFoot, nOriginalDamage, nArmorCnt);	
	

	if( fFinalResult < 0.f ) fFinalResult = 0.f;
#if defined(PRE_ADD_MISSION_COUPON)
	SetHP((INT64)( m_nHP - fFinalResult ));
#else
	m_nHP = (INT64)( m_nHP - fFinalResult );
	if( m_nHP <= 0 ) m_nHP = 0;
#endif

	return fFinalResult;
}


float CDnBasiliskMonsterActor::_CalcDamage( int nOverTurned, int AttackType, int AttackFoot, float nOriginalDamage, int nArmorCnt )
{
	float	nResultDamage = 0.0;
	float   nAttackFoot = 1.0f;

	// 50% bonus when attacking the foot
	if ( AttackFoot == 1 ) 
		nAttackFoot = 0.5f;
	

	// overturned
	if ( nOverTurned == 1 ) 
	{
		// In case of melee attack
		if ( AttackType == 0 )
		{
			nResultDamage = nOriginalDamage;
		}

		// If ranged attack
		else
		{
			nResultDamage = nOriginalDamage * (m_Property.fRangeDefensePercent/100.0f) ;
		}
	}

	// not overturned 			
	else
	{
		// In case of melee attack
		if ( AttackType == 0 ) 
		{
			nResultDamage = nOriginalDamage * ( 1 - ( nArmorCnt * m_Property.fArmorDefensePercent * nAttackFoot )/100.0f );
		}

		// If ranged attack
		else
		{
			nResultDamage = nOriginalDamage * (m_Property.fRangeDefensePercent/100.0f) * ( 1.0f - ( float(nArmorCnt) * m_Property.fArmorDefensePercent * nAttackFoot) /100.0f );
		}
		
	}

	return nResultDamage;
}

void CDnBasiliskMonsterActor::OnFallToTheGround()
{
	// Calculate total armor durability lost on drop
	m_fArmorTotalDurability -= m_Property.fFall_to_the_ground_damage;
	if ( m_fArmorTotalDurability < 0.0f )
	{
		m_fArmorTotalDurability = 0.0f;
	}

	// Determine the number of surviving armors.
	float fCnt = m_fArmorTotalDurability / m_Property.fArmorDurability; 
	m_nArmorCnt = (int)fCnt;

}

// Additional Packet Message
// Damage Packet
void CDnBasiliskMonsterActor::OnDamageWriteAdditionalPacket( CPacketCompressStream *pStream )
{
	pStream->Write( &m_fArmorTotalDurability, sizeof(float) );
	pStream->Write( &m_nArmorCnt, sizeof(int) );
	pStream->Write( &m_fLegTotalDurablility, sizeof(float) );
	pStream->Write( &m_nLastLegRepairTime, sizeof(LOCAL_TIME) );
}

void CDnBasiliskMonsterActor::OnDamageReadAdditionalPacket( CPacketCompressStream *pStream ) 
{
	pStream->Read( &m_fArmorTotalDurability, sizeof(float) );
	pStream->Read( &m_nArmorCnt, sizeof(int) );
	pStream->Read( &m_fLegTotalDurablility, sizeof(float) );
	pStream->Read( &m_nLastLegRepairTime, sizeof(LOCAL_TIME) );
}

/*
void CDnBasiliskMonsterActor::OnSignal( SignalTypeEnum Type, void *pPtr, LOCAL_TIME LocalTime, LOCAL_TIME SignalStartTime, LOCAL_TIME SignalEndTime, int nSignalIndex )
{
	switch( Type ) {
		case STE_EmptySignal:
			{
				MAAiBasilisk* pAi = dynamic_cast<MAAiBasilisk*>(m_pAi);
				if ( pAi )
					pAi->OnGenerationMonster();
			}
			return;
	}
	TDnMonsterActor< MAStaticMovement, MASingleBody, MAMultiDamage >::OnSignal( Type, pPtr, LocalTime, SignalStartTime, SignalEndTime, nSignalIndex );
}
*/

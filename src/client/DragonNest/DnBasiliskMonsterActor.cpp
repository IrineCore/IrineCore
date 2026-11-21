#include "StdAfx.h"
#include "DnBasiliskMonsterActor.h"
#include "MAAiBase.h"
#include "DnWeapon.h"
#include "DnInterface.h"
#include "CollisionAndPush.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


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

CDnBasiliskMonsterActor::CDnBasiliskMonsterActor( int nClassID, bool bProcess )
: TDnMonsterActor( nClassID, bProcess )
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

	memset( &m_Property, 0, sizeof( BasiliskProperty ) );
}

CDnBasiliskMonsterActor::~CDnBasiliskMonsterActor()
{
	

	if ( m_pLua) 		lua_close(m_pLua);
}

bool
CDnBasiliskMonsterActor::Initialize()
{
	TDnMonsterActor< MASingleBody, MAMultiDamage >::Initialize();
	//m_hObject->SetCalcPositionFlag(CALC_POSITION_X|CALC_POSITION_Y|CALC_POSITION_Z);
	m_hObject->SetCalcPositionFlag(0);
	m_hObject->SetCollisionGroup( COLLISION_GROUP_DYNAMIC( 3 ) );
	m_hObject->SetTargetCollisionGroup( 0 );
	m_hObject->EnableCull( false );

	m_nCustomParamIndex = m_hObject->AddCustomParam( "g_MaterialDiffuse" );

	LoadShellMesh();
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
--Left---                            ---Right--
	1 Bone12 :   Object02        Bone16 5 :   Object09
	2 Bone20 :   Object03  	     Bone24 6 :   Object08
	3 Bone32 :   Object04  	     Bone28 7 :   Object07
	4 Bone40 :   Object05 	     Bone36 8 :   Object06
	*/
	LEG_INFO info[] =
	{
		{ "Bone12", "Object02"},
		{ "Bone20", "Object03"},
		{ "Bone32", "Object04"},
		{ "Bone40", "Object05"},

		{ "Bone16", "Object09"},
		{ "Bone24", "Object08"},
		{ "Bone28", "Object07"},
		{ "Bone36", "Object06"},
	};

	for ( int i = 0 ; i < _countof(info) ; i++ )
	{
		m_LegDamageMap.insert(std::pair<std::string, LEG_INFO>( info[i].szBoneName, info[i] ));
	}

	ClearMeshDiffuseColor();
}

bool 
CDnBasiliskMonsterActor::LoadShellMesh()
{
	MAActorRenderBase *pRender = dynamic_cast<MAActorRenderBase *>(this);
	if ( !pRender )
		return false;

	for ( int i = 1 ; i <= 12 ; i++ )
	{
		char szBuff[256] = "";
		sprintf_s(szBuff, "Basilisk\\basilisk_shell\\shell_%d.skn", i);
		EtObjectHandle hObject = EternityEngine::CreateStaticObject( CEtResourceMng::GetInstance().GetFullName( szBuff ).c_str() );
		m_VecShellObject.push_back(hObject);

		if ( i < 10 )
			sprintf_s(szBuff, "~SkinBox0%d", i);
		else
			sprintf_s(szBuff, "~SkinBox%d", i);

		int nBoneIndex = pRender->GetBoneIndex( szBuff );
		hObject->SetParent( pRender->GetObjectHandle(), nBoneIndex );
	}
	
	return true;
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
		int nCount = CDnActor::ScanActor( *pvPos, 3000.f, hVecList );

		for ( size_t i = 0 ; i < hVecList.size() ; ++i )
		{
			std::vector< SCollisionResponse > vecResult;
//			EtVector3 vMyPoint;
//			EtVector3 vDestPoint;

			DnActorHandle hActor = hVecList[i];

			if( hVecList[i] == GetMySmartPtr() ) continue;
			if( hVecList[i]->GetTeam() == GetTeam() ) continue;
			if( hVecList[i]->IsDie() ) continue;

			LOCAL_TIME nLastDamageTime = hActor->GetDamageRemainTime();
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

				hActor->MoveToWorld(EtVector2(-vVel.x, -vVel.z));

				/*
LogWnd::Log(3,_T("Collision"));
				EtVector3 vDir;
				EtVector2 vView;
				vDir = vDestPoint - vMyPoint;
				vView = EtVec3toVec2( -vDir );
				EtVec3Normalize(&vDir, &vDir);
				EtVec2Normalize( &vView, &vView );

				CDnActor::SHitParam HitParam;
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

	// Shake a bit if damaged. lol
	float fValue = 1.f;
	if( m_fShakeDelta > 0.f ) 
	{
		m_fShakeDelta -= fDelta;
		if( m_fShakeDelta <= 0.f ) 
		{
			m_fShakeDelta = 0.f;
		}
		fValue = 1.f - ( ( _rand()%100 ) * 0.0005f );

		// Change this later to check the current action and shake it
		/*
		MAAiBasilisk* pBasilAi = dynamic_cast<MAAiBasilisk*>(m_pAi);
		if ( pBasilAi && GetCurrentPlayAction() == "OverTurned" )
		{
			EtColor Color( fValue, 0.0f, 0.0f, 1.0f );
			m_hObject->SetCustomParam( m_nCustomParamIndex, EPT_VECTOR, &Color );
		}
		*/

		EtColor Color( fValue, 0.0f, 0.0f, 1.0f );
		m_hObject->SetCustomParam( m_nCustomParamIndex, &Color );
	}
	else
	{
		// Set leg damage color
		static EtColor DefaultColor( 0.682f, 0.682f, 0.682f, 1.0f );
		m_hObject->SetCustomParam( m_nCustomParamIndex, &DefaultColor );
	}

	MAActorRenderBase::SetScale( EtVector3( GetScale() * fValue, GetScale(), GetScale() * fValue ) );
	std::map<std::string, EtColor>::iterator it = m_MeshDiffuseColorMap.begin();

	for ( ; it != m_MeshDiffuseColorMap.end() ; ++it )
	{
		EtColor clr = it->second;
		
		int nSubMeshIdx = m_hObject->GetSubMeshIndex(it->first.c_str());
		m_hObject->SetCustomParam( m_nCustomParamIndex, &clr, nSubMeshIdx );
	}

	ProcessCollisionAndPush( m_hObject, m_matexWorld );

	
	//LogWnd::Log(3,_T("%.2f %.2f %.2f"), m_matExWorld.m_vPosition.x, m_matExWorld.m_vPosition.y, m_matExWorld.m_vPosition.z);
}

void
CDnBasiliskMonsterActor::OnDamage( CDnDamageBase *pHitter, SHitParam &HitParam )
{
	//@

	CDnMonsterActor::OnDamage( pHitter, HitParam );
	m_fShakeDelta = 0.2f;

	if( pHitter )
	{
		switch( pHitter->GetDamageObjectType() ) 
		{
			case DamageObjectTypeEnum::Actor:
			{
				m_pAi->OnDamage( dynamic_cast<CDnActor*>(pHitter)->GetMySmartPtr(), HitParam, CDnActionBase::m_LocalTime );
				break;
			}
			case DamageObjectTypeEnum::Prop:
				break;
		}
	}

	if ( IsDie() )
	{
		m_pAi->OnDie(CDnActionBase::m_LocalTime);
		//CDnInterface::GetInstance().HideBossGauge();
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
	MAActorRenderBase *pRender = dynamic_cast<MAActorRenderBase *>(this);
	if( pRender ) {
		if( m_fDieDelta < 5.f && pRender->IsEnableCastShadow() ) {
			pRender->EnableCastShadow( false );
		}
		pRender->SetAlphaBlend( m_fDieDelta * 0.1f  );
	}

	
}

void CDnBasiliskMonsterActor::CmdAction( const char *szActionName, int nLoopCount, float fBlendFrame, bool bCheckOverlapAction, bool bFromStateBlow/* = false*/, bool bSkillChain/* = false*/ )
{
	static DWORD s_LastTime = GetTickCount();

	DWORD CurTime = GetTickCount();

	OutputDebug( "[%d:%d]%s\n", CurTime, CurTime - s_LastTime, szActionName );
	s_LastTime = CurTime;
/*
	TCHAR szString[512];
	MultiByteToWideChar( CP_ACP, 0, m_szPrevAction.c_str(), -1, szString, 512 );

	TCHAR szString2[512];
	MultiByteToWideChar( CP_ACP, 0, szActionName, -1, szString2, 512 );

	LogWnd::Log(4, _T("PrevAction:%s CurAction:%s yaw:%0.2f"), szString, szString2, fAngle);
*/
	
	float fAngle = __FindActionAngle::Find(m_szPrevAction.c_str());
	m_hActor->GetMatEx()->RotateYaw(fAngle);
	OutputDebug("[[PrevAction:%s CurAction:%s yaw:%0.2f]]\n", m_hActor->GetCurrentAction(), szActionName, fAngle);

	CDnMonsterActor::CmdAction(szActionName, nLoopCount, fBlendFrame, false );
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

	m_Property.fArmorDurability				= PropTable.get<float>("fArmorDurability");
	m_Property.fArmorDefencePercent			= PropTable.get<float>("fArmorDefencePercent");
	m_Property.fArmorCnt					= PropTable.get<float>("fArmorCnt");
	m_Property.fRangeDefencePercent			= PropTable.get<float>("fRangeDefencePercent");
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




float CDnBasiliskMonsterActor::CalcDamage( DnActorHandle hHitter, SHitParam &HitParam )
{
	/*
	float fHitPer = HitParam.fDamage;
	float fAttack = ( hHitter->GetAttackMin() + ( _rand() % ( hHitter->GetAttackMax() - hHitter->GetAttackMin() + 1 ) ) ) * fHitPer;
	float fDefenseDecrease = GetDefense() / (float)( powl( hHitter->GetLevel(), 2 ) + 500 + GetDefense() );
	float fResult = fAttack * ( 1.f - fDefenseDecrease );
	*/
//	OutputDebug( "Damage : %.2f\n", fResult );


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
	float nOriginalDamage = (float)HitParam.nDamage;
	int nArmorCnt = m_nArmorCnt;

	/*
	MAAiBasilisk* pBasilAi = dynamic_cast<MAAiBasilisk*>(m_pAi);
	
	if ( GetCurrentPlayAction() == "OverTurned" )
	{
		nState = 1;
	}
	*/
	
	if ( HitParam.hWeapon->GetWeaponType() & CDnWeapon::Projectile )
	{
		AttackType = 1;
	}


	// Slightly reduce armor durability.
	m_fArmorTotalDurability -= m_Property.fArmorDurabilityDamage;

	float fFinalResult = _CalcDamage( nState, AttackType, AttackFoot, nOriginalDamage, nArmorCnt);	
	//float fFinalResult = lua_tinker::call<float>(m_pLua, "lua_CalcDamage", nState, AttackType, AttackFoot, nOriginalDamage, nArmorCnt);	

	if( fFinalResult < 0.f ) fFinalResult = 0.f;
	m_nHP = (INT64)( m_nHP - fFinalResult );
	if( m_nHP <= 0 ) m_nHP = 0;

	LogWnd::Log(3,_T("Damage : %0.2f ShellCount:%d"), fFinalResult, nArmorCnt);

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
			nResultDamage = nOriginalDamage * (m_Property.fRangeDefencePercent/100.0f) ;
		}
	}

	// not overturned 			
	else
	{
		// In case of melee attack
		if ( AttackType == 0 ) 
		{
			nResultDamage = nOriginalDamage * ( 1 - ( nArmorCnt * m_Property.fArmorDefencePercent * nAttackFoot )/100.0f );
		}

		// If ranged attack
		else
		{
			nResultDamage = nOriginalDamage * (m_Property.fRangeDefencePercent/100.0f) * ( 1.0f - ( float(nArmorCnt) * m_Property.fArmorDefencePercent * nAttackFoot) /100.0f );
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
	pStream->Write( &m_fArmorTotalDurability, sizeof(float ) );
	pStream->Write( &m_nArmorCnt, sizeof(int ) );
	pStream->Write( &m_fLegTotalDurablility, sizeof(float ) );
	pStream->Write( &m_nLastLegRepairTime, sizeof(LOCAL_TIME ) );
}

void CDnBasiliskMonsterActor::OnDamageReadAdditionalPacket( CPacketCompressStream *pStream ) 
{
	pStream->Read( &m_fArmorTotalDurability, sizeof(float ) );
	pStream->Read( &m_nArmorCnt, sizeof(int ) );
	pStream->Read( &m_fLegTotalDurablility, sizeof(float ) );
	pStream->Read( &m_nLastLegRepairTime, sizeof(LOCAL_TIME ) );
}

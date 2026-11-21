#include "stdafx.h"
#include "DnSkillBase.h"
#include "DnTableDB.h"
#include "DnSkillDefine.h"
#include "DnItemTask.h"
#include "DnPlayerActor.h"
#include "PerfCheck.h"

DECL_MULTISMART_PTR_STATIC( CDnSkillBase, MAX_SESSION_COUNT, 100 )


#define TIMETOGGLE_DECREASE_TIME 3000
CDnSkillBase::CDnSkillBase( DnActorHandle hActor )
: CMultiSmartPtrBase< CDnSkillBase, MAX_SESSION_COUNT >(hActor->GetRoom())
{
	m_nSkillID = 0;
	m_nNeedItemID = 0;
	m_nNeedItemDecreaseCount = 0;

	m_nLevel = 0;
	m_nMaxLevel = 0;
	m_nIncreaseRange = 0;
	m_nDecreaseHP = m_nDecreaseSP = 0;
	m_SkillType = SkillTypeEnum::Active;
	m_DurationType = DurationTypeEnum::Instantly;
	m_TargetType = TargetTypeEnum::Self;
	m_nLevelLimit = 0;
	m_nDelayTime = 0;
	m_UseTime = 0;
	m_fCoolTime = 0.f;
	m_nAdditionalThreat = 0;

	m_hActor = hActor;
}

CDnSkillBase::~CDnSkillBase()
{
	Finish( 0, 0.f );
}

DnSkillHandle_ CDnSkillBase::CreateSkill( DnActorHandle hActor, int nSkillTableID, int nLevel )
{
	DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TSKILL );
	DNTableFileFormat *pLevelSox = GetDNTable( CDnTableDB::TSKILLLEVEL );

	int nUsingMethod = pSox->GetFieldFromLablePtr( nSkillTableID, "_UsingMethod" )->GetInteger();
	int nActionMethod = pSox->GetFieldFromLablePtr( nSkillTableID, "_ActionMethod" )->GetInteger();

	std::vector<int> nVecLevel;
	if( pLevelSox->GetItemIDListFromField( "_SkillIndex", nSkillTableID, nVecLevel ) == 0 ) return CDnSkillBase::Identity();

	int nSkillLevelTableID = -1;
	for( DWORD i=0; i<nVecLevel.size(); i++ ) {
		int nTemp = pLevelSox->GetFieldFromLablePtr( nVecLevel[i], "_SkillLevel" )->GetInteger();
		if( nTemp == nLevel ) {
			nSkillLevelTableID = nVecLevel[i];
			break;
		}
	}
	if( nSkillLevelTableID == -1 ) return CDnSkillBase::Identity();

	char szLabel[32];
	std::string szUsingParam[10];
	std::string szActionParam[10];
	for( int i=0; i<10; i++ ) {
		sprintf_s( szLabel, "_UsingMethodParam%d", i + 1 );
		szUsingParam[i] = pLevelSox->GetFieldFromLablePtr( nSkillLevelTableID, szLabel )->GetString();

		sprintf_s( szLabel, "_ActionMethodParam%d", i + 1 );
		szActionParam[i] = pLevelSox->GetFieldFromLablePtr( nSkillLevelTableID, szLabel )->GetString();
	}

	CDnSkillBase *pBase = NULL;

	CREATE_SKILL_METHOD( pBase, nUsingMethod, nActionMethod );
	if( pBase == NULL ) return CDnSkillBase::Identity();

	if( hActor ) {
		MASkillUser *pUser = dynamic_cast<MASkillUser *>(hActor.GetPointer());
		pUser->InsertUsingMethod( dynamic_cast<MSUsingTypeBase*>(pBase) );
		pUser->InsertActionMethod( dynamic_cast<MSActionTypeBase*>(pBase) );
	}

	return pBase->GetMySmartPtr();
}

bool CDnSkillBase::Initialize( int nSkillTableID, int nSkillLevelTableID )
{
	m_nSkillID = nSkillTableID;
	DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TSKILL );
	DNTableFileFormat *pLevelSox = GetDNTable( CDnTableDB::TSKILLLEVEL );

	// Refer to Skill Table
	std::string szTemp;

	m_szStaticName = pSox->GetFieldFromLablePtr( nSkillTableID, "_StaticName" )->GetString();

	m_RequireWeaponType = (CDnWeapon::EquipTypeEnum)pSox->GetFieldFromLablePtr( nSkillTableID, "_NeedWeaponType" )->GetInteger();

	m_nMaxLevel = pSox->GetFieldFromLablePtr( nSkillTableID, "_MaxLevel" )->GetInteger();
	m_SkillType = (SkillTypeEnum)pSox->GetFieldFromLablePtr( nSkillTableID, "_SkillType" )->GetInteger();
	m_DurationType = (DurationTypeEnum)pSox->GetFieldFromLablePtr( nSkillTableID, "_DurationType" )->GetInteger();
	m_TargetType = (TargetTypeEnum)pSox->GetFieldFromLablePtr( nSkillTableID, "_TargetType" )->GetInteger();

	// Refer to Skill Level Table
	m_nLevel = pLevelSox->GetFieldFromLablePtr( nSkillLevelTableID, "_SkillLevel" )->GetInteger();

	m_nNeedJobClassID = pLevelSox->GetFieldFromLablePtr( nSkillLevelTableID, "_NeedJobClass" )->GetInteger();

	m_nNeedItemID = pLevelSox->GetFieldFromLablePtr( nSkillLevelTableID, "_NeedItem" )->GetInteger();
	m_nNeedItemDecreaseCount = pLevelSox->GetFieldFromLablePtr( nSkillLevelTableID, "_NeedItemDecreaseCount" )->GetInteger();
	m_nIncreaseRange = pLevelSox->GetFieldFromLablePtr( nSkillLevelTableID, "_AddRange" )->GetInteger();
	m_nDecreaseHP = pLevelSox->GetFieldFromLablePtr( nSkillLevelTableID, "_DecreaseHP" )->GetInteger();
	m_nDecreaseSP = pLevelSox->GetFieldFromLablePtr( nSkillLevelTableID, "_DecreaseSP" )->GetInteger();
	m_nLevelLimit = pLevelSox->GetFieldFromLablePtr( nSkillLevelTableID, "_LevelLimit" )->GetInteger();
	m_nDelayTime = pLevelSox->GetFieldFromLablePtr( nSkillLevelTableID, "_DelayTime" )->GetInteger();
	m_nAdditionalThreat = pLevelSox->GetFieldFromLablePtr( nSkillLevelTableID, "_AddThreat" )->GetInteger();

	StateEffectStruct Struct;
	char szLabel[64];
	for( int i=0; i<5; i++ ) {
		sprintf_s( szLabel, "_EffectClass%d", i + 1 );
		Struct.nID = pSox->GetFieldFromLablePtr( nSkillTableID, szLabel )->GetInteger();
		if( Struct.nID < 1 ) continue;

		sprintf_s( szLabel, "_EffectClass%dApplyType", i + 1 );
		Struct.ApplyType = (StateEffectApplyType)pSox->GetFieldFromLablePtr( nSkillTableID, szLabel )->GetInteger();

		sprintf_s( szLabel, "_EffectClassValue%d", i + 1 );
		Struct.szValue = pLevelSox->GetFieldFromLablePtr( nSkillLevelTableID, szLabel )->GetString();

		sprintf_s( szLabel, "_EffectClassValue%dDuration", i + 1 );
		Struct.nDurationTime = pLevelSox->GetFieldFromLablePtr( nSkillLevelTableID, szLabel )->GetInteger();

		m_VecStateEffectList.push_back( Struct );
	}

	return true;
}

void CDnSkillBase::Ready( LOCAL_TIME LocalTime, float fDelta )
{
	MSUsingTypeBase *pBase = dynamic_cast<MSUsingTypeBase *>(this);

	pBase->Ready( LocalTime, fDelta );
}

void CDnSkillBase::ProcessReady( LOCAL_TIME LocalTime, float fDelta )
{
	MSUsingTypeBase *pBase = dynamic_cast<MSUsingTypeBase *>(this);

	pBase->ProcessReady( LocalTime, fDelta );
}

void CDnSkillBase::Execute( LOCAL_TIME LocalTime, float fDelta )
{
	MSActionTypeBase *pBase = dynamic_cast<MSActionTypeBase *>(this);

	m_UseTime = LocalTime;
	m_fCoolTime = ( m_nDelayTime == 0 ) ? 0.f : 1.f;

	CheckAndAddStateEffect();
	pBase->Execute( LocalTime, fDelta );
}

void CDnSkillBase::ProcessExecute( LOCAL_TIME LocalTime, float fDelta )
{
	MSActionTypeBase *pBase = dynamic_cast<MSActionTypeBase *>(this);

	pBase->Process( LocalTime, fDelta );
}

void CDnSkillBase::Finish(LOCAL_TIME LocalTime, float fDelta )
{
	MSActionTypeBase *pBase = dynamic_cast<MSActionTypeBase*>(this);

	CheckAndRemoveStateEffect();
	if (pBase) pBase->Finish( LocalTime, fDelta );
}

bool CDnSkillBase::CanReady()
{
	if( m_hActor->GetSP() < m_nDecreaseSP ) return false;
	if( m_hActor->GetHP() <= m_nDecreaseHP ) return false;
	if( m_hActor->GetLevel() < m_nLevelLimit ) return false;

	if( m_nNeedItemID > 0 ) {

		int nCount = CDnItemTask::GetInstance(m_hActor->GetRoom()).ScanItemFromID( m_hActor, m_nNeedItemID, NULL );
		if( nCount < m_nNeedItemDecreaseCount ) return false;
	}

	if( m_hActor->GetClassID() <= CDnActor::Reserved6 ) {
		CDnPlayerActor *pActor = dynamic_cast<CDnPlayerActor *>(m_hActor.GetPointer());
		if( !pActor ) return false;
		if( m_nNeedJobClassID ) {
			if( pActor->IsPassJob( m_nNeedJobClassID ) == false ) return false;
		}
		if( !pActor->IsBattleMode() ) return false;
	}

	switch( m_DurationType ) {
		case Instantly:
		case Buff:
		case Debuff:
			if( GetCoolTime() > 0.f ) return false;
			break;
		case TimeToggle: break;
		case ActiveToggle: break;
		case Aura: break;
	}

	MSUsingTypeBase *pBase = dynamic_cast<MSUsingTypeBase *>(this);

	return pBase->CanReady();
}

bool CDnSkillBase::CanExecute()
{
	MSUsingTypeBase *pBase = dynamic_cast<MSUsingTypeBase *>(this);

	return pBase->CanExecute();
}

bool CDnSkillBase::IsFinish()
{
	MSActionTypeBase *pBase = dynamic_cast<MSActionTypeBase *>(this);

	return pBase->IsFinishAction();
}

void CDnSkillBase::CheckAndAddStateEffect()
{
	//int nID(-1);
	//StateEffectStruct *pStruct(NULL);

	//for( DWORD i=0; i<m_VecStateEffectList.size(); i++ ) 
	//{
	//	pStruct = &m_VecStateEffectList[i];
	//	if( !pStruct ) continue;

	//	if( pStruct->ApplyType == ApplySelf )
	//	{
	//		if( pStruct->nDurationTime == 0 )
	//		{
	//			nID = m_hActor->AddStateBlow( (STATE_BLOW::emBLOW_INDEX)pStruct->nID, GetClassID(), -1, pStruct->szValue.c_str() );
	//			if( nID == -1 ) continue;
	//			m_nVecApplyStateEffectList.push_back( nID );
	//		}
	//		else
	//		{
	//			// Think about when to apply it later.. for now apply immediately on use and maybe move to a signal later.
	//			m_hActor->CmdAddStateEffect( m_hActor, GetClassID(), (STATE_BLOW::emBLOW_INDEX)pStruct->nID, pStruct->nDurationTime, pStruct->szValue.c_str() );
	//		}
	//	}
	//}
}

void CDnSkillBase::CheckAndRemoveStateEffect()
{
	for( DWORD i=0; i<m_nVecApplyStateEffectList.size(); i++ ) 
	{
		m_hActor->RemoveStateBlowFromID( m_nVecApplyStateEffectList[i] );
	}
	SAFE_DELETE_VEC( m_nVecApplyStateEffectList );
}

void CDnSkillBase::Process( LOCAL_TIME LocalTime, float fDelta )
{
	switch( m_DurationType ) {
		case Instantly:
		case Buff:
		case Debuff:
			if( m_fCoolTime == 0.f ) return;
			m_fCoolTime = 1.f - ( ( 1.f / (float)m_nDelayTime ) * (float)( LocalTime - m_UseTime ) );
			if( m_fCoolTime < 0.f ) m_fCoolTime = 0.f;
			if( m_fCoolTime > 1.f ) m_fCoolTime = 1.f;
			break;
		case TimeToggle:
			return;
			{
				int nValue = (int)( LocalTime - m_UseTime );
				int nCount = nValue / TIMETOGGLE_DECREASE_TIME;
				if( nCount > 0 ) {
					m_UseTime += TIMETOGGLE_DECREASE_TIME * nCount;
					int nDecreaseSP = m_nDecreaseSP * nCount;

					int nSP = m_hActor->GetSP();
					if( nSP < nDecreaseSP ) {
						MASkillUser *pSkillUser = dynamic_cast<MASkillUser *>(m_hActor.GetPointer());
						pSkillUser->OnSkillToggle( GetMySmartPtr(), false );
					}
					else m_hActor->SetSP( nSP - nDecreaseSP );
				}
			}
			break;
		case ActiveToggle:
			return;
		case Aura:
			return;		
	}
}


DWORD CDnSkillBase::GetStateEffectCount()
{
	return (DWORD)m_VecStateEffectList.size();
}

CDnSkillBase::StateEffectStruct *CDnSkillBase::GetStateEffectFromIndex( DWORD dwIndex )
{
	if( dwIndex < 0 || dwIndex >= m_VecStateEffectList.size() ) return NULL;
	return &m_VecStateEffectList[dwIndex];
}

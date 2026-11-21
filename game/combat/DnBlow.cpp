#include "stdafx.h"
#include "DnBlow.h"
#include "DnPlayerActor.h"

#ifdef _GAMESERVER
#include "DNGameRoom.h"
#include "DNGameDataManager.h"
#include "DNUserSession.h"
#include "DnMonsterActor.h"
#include "DnDLGameTask.h"
#ifdef PRE_ADD_IGNORE_RESTORE_HP
#include "DnIgnoreRestoreHPBlow.h"
#include "DnStateBlow.h"
#endif // PRE_ADD_IGNORE_RESTORE_HP
DECL_MULTISMART_PTR_STATIC( CDnBlow, MAX_SESSION_COUNT, 10 )
#else
DECL_SMART_PTR_STATIC( CDnBlow, 100 )
#endif
#if defined( PRE_ADD_DRAGON_FELLOWSHIP ) && defined( _CLIENT )
#include "DnWorld.h"
#endif	// #if defined( PRE_ADD_DRAGON_FELLOWSHIP )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnBlow::CDnBlow( DnActorHandle hActor )
#ifdef _GAMESERVER
: CMultiSmartPtrBase< CDnBlow, MAX_SESSION_COUNT >(hActor->GetRoom())
  ,m_pPacketStream( NULL )
#else
: CSmartPtrBase()
#endif
	, m_hActor(hActor)
	, m_nBlowID(0)
	, m_bPermanentBlow(false)
	, m_dwCallBackType(SB_ONBEGIN|SB_ONEND)
	, m_nThreatAggro(0)
#ifndef _GAMESERVER
	, m_pEffectOutputInfo( NULL )
	, m_bUseTableDefinedGraphicEffect( true )
	, m_fAccumTime( 0.0f )
	, m_bAllowDiffuseVariation( false )
	, m_nServerBlowID( 0 )
	, m_bIgnoreEffectAction( false )
#else
	, m_bStartedMonsterActionInSETableInfo( false )
#endif
	, m_bFromSourceItem( false )
#ifdef _GAMESERVER
	, m_bHPMPFullWhenBegin( false )
#endif
	, m_LastLocalTime( 0 )
	, m_bDuplicated( false )
	, m_iDuplicateCount( 1 )		// Duplicate count starts at 1 for itself.
	, m_fOriginalDurationTime( 0.0f )
	, m_bHasParentSkill( false )
	, m_bGuildBlow(false)			//Whether the created state effect was used by a guild skill
	, m_eOriginalBlowIndex( STATE_BLOW::BLOW_NONE )
{
	m_StateBlow.emBlowState = STATE_BLOW::STATE_BEGIN;
	m_bIgnoreEffectAction = false;
	m_fValue = 0.0f;

	m_bEternity = false;

#if defined(_GAMESERVER)
	memset(m_PacketBuffer, 0, sizeof(m_PacketBuffer));
#endif // _GAMESERVER
	
}

CDnBlow::~CDnBlow()
{
#ifdef _GAMESERVER
	CheckAndStopActorActionInEffectInfo();

	SAFE_DELETE( m_pPacketStream );
#else
	if( m_bUseTableDefinedGraphicEffect )
		DetachGraphicEffectDefaultType();
#endif
}


void CDnBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
#ifndef _GAMESERVER
	// If it's one that changes color, handle it accordingly.
	// Display only the highest priority color.
	//if( m_pEffectOutputInfo && m_pEffectOutputInfo->iUseDiffuseVariationMethod > 0 )
	if( m_bAllowDiffuseVariation )
	{
		m_fAccumTime += fDelta;
		float fSin = sinf( m_fAccumTime * m_pEffectOutputInfo->fDiffuseChangeSpeed );
		float fRed = (m_pEffectOutputInfo->afDiffuseRed[ 0 ] * fSin) + (m_pEffectOutputInfo->afDiffuseRed[ 1 ] * (1.0f-fSin));
		float fGreen = (m_pEffectOutputInfo->afDiffuseGreen[ 0 ] * fSin) + (m_pEffectOutputInfo->afDiffuseGreen[ 1 ] * (1.0f-fSin));
		float fBlue = (m_pEffectOutputInfo->afDiffuseBlue[ 0 ] * fSin) + (m_pEffectOutputInfo->afDiffuseBlue[ 1 ] * (1.0f-fSin));

		_SetDiffuse( fRed, fGreen, fBlue, 1.0f );
	}
#endif

	if( IsPermanent() )
		return;


	if( LocalTime < m_LastLocalTime )
		LocalTime += m_LastLocalTime;

	m_StateBlow.fDurationTime -= fDelta;

	if( m_StateBlow.fDurationTime < 0.0f )
	{
		SetState( STATE_BLOW::STATE_END );
	}

	m_LastLocalTime = LocalTime;
}

void CDnBlow::SetDurationTime( float fTime )
{ 
	if( fTime < 0.0f )
		fTime = 0.0f;

	m_StateBlow.fDurationTime = fTime;
}

void CDnBlow::SetDelayTime( float fTime )
{ 
	if( fTime < 0.0f ) 
		fTime = 0.0f;

	m_StateBlow.fDelayTime = fTime;
}

int CDnBlow::GetThreatAggro()
{
	return m_nThreatAggro;
}


void CDnBlow::SetParentSkillInfo( const CDnSkill::SkillInfo* pParentSkillInfo )
{	
	if( pParentSkillInfo )
	{
		m_ParentSkillInfo = *pParentSkillInfo;
		m_bHasParentSkill = true;
	}
	else
	{
		m_bHasParentSkill = false;
	}

	//Case where data must be set when creating a state effect and setting skill info.
	OnSetParentSkillInfo();
}


CDnActorState::AddBlowStateType CDnBlow::GetAddBlowStateType()
{
	CDnActorState::AddBlowStateType eAddBlowStateType = CDnActorState::Equip_Buff_Level;

	if( 0.0f == GetDurationTime() )
		eAddBlowStateType = CDnActorState::Skill_Level;

#ifdef PRE_ADD_SKILLBUF_RENEW
	// If it has a duration and the parent skill is classified as Buff/Debuff/Aura
	// Is a Weapon.
	if( (CDnSkill::Instantly == m_ParentSkillInfo.eDurationType ||
		CDnSkill::ActiveToggle == m_ParentSkillInfo.eDurationType ||
		CDnSkill::ActiveToggleForSummon == m_ParentSkillInfo.eDurationType ||
		CDnSkill::SummonOnOff == m_ParentSkillInfo.eDurationType ||
		CDnSkill::StanceChange == m_ParentSkillInfo.eDurationType) 
		&&
		(CDnSkill::Active == m_ParentSkillInfo.eSkillType ||
		CDnSkill::AutoActive == m_ParentSkillInfo.eSkillType ||
		CDnSkill::AutoPassive == m_ParentSkillInfo.eSkillType) )
		eAddBlowStateType = CDnActorState::Skill_Level;

	if( CDnSkill::Aura == m_ParentSkillInfo.eDurationType )
		eAddBlowStateType = CDnActorState::Equip_Buff_Level;

#ifdef PRE_FIX_PASSIVE_BUFF
	if( CDnSkill::Buff == m_ParentSkillInfo.eDurationType && ( m_ParentSkillInfo.eSkillType == CDnSkill::Passive || m_ParentSkillInfo.eSkillType == CDnSkill::AutoPassive ) )
		eAddBlowStateType = CDnActorState::Equip_Buff_Level;
#endif

#else
	// Active/autopassive buffs are also classified as skill state effects!
	if( (CDnSkill::Instantly == m_ParentSkillInfo.eDurationType ||
		 CDnSkill::Buff == m_ParentSkillInfo.eDurationType ||
		 CDnSkill::Debuff == m_ParentSkillInfo.eDurationType ||
		 CDnSkill::ActiveToggle == m_ParentSkillInfo.eDurationType ||
		 CDnSkill::ActiveToggleForSummon == m_ParentSkillInfo.eDurationType ||
		 CDnSkill::SummonOnOff == m_ParentSkillInfo.eDurationType ||
		 CDnSkill::StanceChange == m_ParentSkillInfo.eDurationType) &&
		 CDnSkill::Active == m_ParentSkillInfo.eSkillType ||
		 CDnSkill::AutoActive == m_ParentSkillInfo.eSkillType ||
		 CDnSkill::AutoPassive == m_ParentSkillInfo.eSkillType )
		eAddBlowStateType = CDnActorState::Skill_Level;
#endif // #ifdef PRE_ADD_SKILLBUF_RENEW

	// If it has a duration and is a state effect originating from an InputHasPassive skill, classify as a post state effect.
	if( CDnSkill::Passive == m_ParentSkillInfo.eSkillType &&
		true == m_ParentSkillInfo.bFromInputHasPassiveSignal )
	{
		eAddBlowStateType = CDnActorState::Skill_Level;
	}

	// #32220 Specific state effects used in cooking are classified here so their calculation order is handled separately.
	if( STATE_BLOW::BLOW_200 == m_StateBlow.emBlowIndex || 
		STATE_BLOW::BLOW_201 == m_StateBlow.emBlowIndex ||
		STATE_BLOW::BLOW_202 == m_StateBlow.emBlowIndex ||
		STATE_BLOW::BLOW_203 == m_StateBlow.emBlowIndex )
	{
		eAddBlowStateType = CDnActorState::Equip_Skill_Level;
	}

	if (STATE_BLOW::BLOW_234 == m_StateBlow.emBlowIndex)
		eAddBlowStateType = CDnActorState::Equip_Skill_Level;
	

	return eAddBlowStateType;
}


#ifndef _GAMESERVER
DnEtcHandle CDnBlow::_AttachGraphicEffect( bool bAllowSameSkillDuplicate /* = false */ )
{
	if( m_pEffectOutputInfo )
	{
		bool bResult = m_hActor->AttachSEEffect( m_ParentSkillInfo, m_StateBlow.emBlowIndex, m_nBlowID, 
												 m_pEffectOutputInfo, m_hEtcObjectEffect, m_vlhDummyBoneEtcObjects, bAllowSameSkillDuplicate );

		// Also sends packet to server via CmdAction, though CmdAction is not sent to server for monsters
		// Action must be executed only when in OWNER_HANDLING mode..
		//_ASSERT( EffectOutputInfo::OWNER_HANDLING == m_pEffectOutputInfo->iShowTimingType );
		//if( false == m_pEffectOutputInfo->strActorActionName.empty() &&
		//	EffectOutputInfo::OWNER_HANDLING == m_pEffectOutputInfo->iShowTimingType )
			if( false == m_bIgnoreEffectAction && false == m_pEffectOutputInfo->strActorActionName.empty() )
				m_hActor->CmdAction( m_pEffectOutputInfo->strActorActionName.c_str() );
	}

	return m_hEtcObjectEffect;
}

bool CDnBlow::_DetachGraphicEffect( void )
{
	bool bResult = false;

	if( m_pEffectOutputInfo )
		bResult = m_hActor->DetachSEEffect( m_ParentSkillInfo, m_StateBlow.emBlowIndex, m_nBlowID, m_pEffectOutputInfo );

	if( m_hEtcObjectEffect ) {
		m_hEtcObjectEffect.Identity();
	}
	else
	if( false == m_vlhDummyBoneEtcObjects.empty() )
		m_vlhDummyBoneEtcObjects.clear();

	return bResult;
}

void CDnBlow::AllowDiffuseVariation( bool bAllowDiffuseVariation ) 
{
	if( bAllowDiffuseVariation )
	{
		// Apply graphic effect to targets matching the skill's target type
		bool bAppropriateTarget = false;
		if( CDnSkill::Enemy == m_ParentSkillInfo.eTargetType )
		{
			if( m_hActor->GetTeam() != m_ParentSkillInfo.iSkillUserTeam )
				bAppropriateTarget = true;
		}
		else
		if( m_hActor->GetTeam() == m_ParentSkillInfo.iSkillUserTeam )
			bAppropriateTarget = true;

		// When changing from allowed to disallowed, restore original color.
		if( bAllowDiffuseVariation == false )
		{
			_SetDiffuse( 1.0f, 1.0f, 1.0f, 1.0f );
		}

		if( bAppropriateTarget )
			m_bAllowDiffuseVariation = true;
	}
	else
		m_bAllowDiffuseVariation = false;
}


void CDnBlow::_SetDiffuse( float fRed, float fGreen, float fBlue, float fAlpha )
{
	if( !m_hActor ) return;
	EtAniObjectHandle hAniObject = m_hActor->GetObjectHandle();
	if( !hAniObject ) return;
	int iCustomParamIndex = hAniObject->AddCustomParam( "g_MaterialDiffuse" );
	EtColor Color( fRed, fGreen, fBlue, fAlpha );

	if( -1 != iCustomParamIndex )
	{
		hAniObject->SetCustomParam( iCustomParamIndex, &Color );
	}
	else
	{
		// TODO: For multi-part actors, we probably need to specify colors per part.
		// If player
		CDnPlayerActor* pPlayerActor = dynamic_cast<CDnPlayerActor*>(m_hActor.GetPointer());
		if( pPlayerActor )
		{
			EtAniObjectHandle hCombineObject = pPlayerActor->GetCombineObject();
			if( hCombineObject ) {
				int iCustomParam = hCombineObject->AddCustomParam( "g_MaterialDiffuse" );
				hCombineObject->SetCustomParam( iCustomParam, &Color );				
			}
			
			for( int iPart = (int)CDnParts::Face; iPart < CDnParts::PartsTypeEnum_Amount; ++iPart )
			{
				DnPartsHandle hBodyParts = pPlayerActor->GetParts( (CDnParts::PartsTypeEnum)iPart );
				if( hBodyParts )
				{
					EtAniObjectHandle hBodyObject = hBodyParts->GetObjectHandle();
					if( hBodyObject ) {
						int iCustomParam = hBodyObject->AddCustomParam( "g_MaterialDiffuse" );
						hBodyObject->SetCustomParam( iCustomParam, &Color );
					}
				}
			}

			for( int iPart = (int)CDnParts::CashHelmet; iPart < CDnParts::CashPartsTypeEnum_Amount; ++iPart )
			{
				DnPartsHandle hBodyParts = pPlayerActor->GetCashParts( (CDnParts::PartsTypeEnum)iPart );
				if( hBodyParts )
				{
					EtAniObjectHandle hBodyObject = hBodyParts->GetObjectHandle();
					if( hBodyObject ) {
						int iCustomParam = hBodyObject->AddCustomParam( "g_MaterialDiffuse" );
						hBodyObject->SetCustomParam( iCustomParam, &Color );
					}
				}
			}
		}
	}
}

bool CDnBlow::AttachGraphicEffectDefaultType( void )
{
	bool bResult = false;

	if( m_pEffectOutputInfo )
	{
		// Most state effects attach like this by default; others are directly controlled in derived specific state effects.
		bool bIsDummyBoneAttach = (EffectOutputInfo::DUMMY_BONE_ATTACH == m_pEffectOutputInfo->iOutputType);
		bool bMatchSEIndex = ((m_pEffectOutputInfo->iStateEffectIndex == 0) || (m_StateBlow.emBlowIndex == m_pEffectOutputInfo->iStateEffectIndex));

		if( ((EffectOutputInfo::ALL_TIME == m_pEffectOutputInfo->iShowTimingType) && bMatchSEIndex) &&
			((EffectOutputInfo::ATTACH == m_pEffectOutputInfo->iOutputType) || bIsDummyBoneAttach ) )
		{
			// Apply graphic effect to targets matching the skill's target type
			bool bAllow = CanAddGraphicsEffect();
			if( bAllow )
			{
				_AttachGraphicEffect();
				if( m_hEtcObjectEffect )
					bResult = true;
			}
		}

		//if( m_pEffectOutputInfo->bUseDiffuseVariation )
		//{
			//_SetDiffuse( m_pEffectOutputInfo->afDiffuseRed, m_pEffectOutputInfo->afDiffuseGreen, m_pEffectOutputInfo->afDiffuseBlue, 1.0f );
		//}
	}

	return bResult;
}


bool CDnBlow::DetachGraphicEffectDefaultType( void )
{
	bool bResult = false;

	if( m_hActor && m_pEffectOutputInfo )
	{
		bool bMatchSEIndex = ((m_pEffectOutputInfo->iStateEffectIndex == 0) || (m_StateBlow.emBlowIndex == m_pEffectOutputInfo->iStateEffectIndex));
		bool bIsDummyBoneAttach = (EffectOutputInfo::DUMMY_BONE_ATTACH == m_pEffectOutputInfo->iOutputType);

		if( ((EffectOutputInfo::ALL_TIME == m_pEffectOutputInfo->iShowTimingType) && bMatchSEIndex) &&
			((EffectOutputInfo::ATTACH == m_pEffectOutputInfo->iOutputType) || bIsDummyBoneAttach) )
		{
			if( m_hEtcObjectEffect || bIsDummyBoneAttach )
			{
				bResult = _DetachGraphicEffect();
				m_hEtcObjectEffect.Identity();
			}
			else
			{
				m_hActor->DetachSEEffectSkillInfo( m_ParentSkillInfo );
			}
		}

		if( 0 < m_pEffectOutputInfo->iUseDiffuseVariationMethod )
		{
			_SetDiffuse( 1.0f, 1.0f, 1.0f, 1.0f );
		}
	}

	return bResult;
}
#endif

bool CDnBlow::CanAddGraphicsEffect()
{
	bool bAllow = false;
	if( CDnSkill::Enemy == m_ParentSkillInfo.eTargetType )
	{
		if( m_hActor->GetTeam() != m_ParentSkillInfo.iSkillUserTeam )
			bAllow = true;
	}
	else if( CDnSkill::All == m_ParentSkillInfo.eTargetType ) // If the target is All, set it without checking conditions.
	{
		bAllow = true;
	}
	else
	{
		if( m_hActor->GetTeam() == m_ParentSkillInfo.iSkillUserTeam )
			bAllow = true;
	}

	// If it is a buff from a prop, just add the effect.
	if( m_ParentSkillInfo.bFromBuffProp )
		bAllow = true;

	//If m_bIgnoreEffectAction of Freezing/Frostbite/Escape/ElectricsShock state effects is set to true
	//Prevent effect processing.
	if (m_bIgnoreEffectAction == true)
		bAllow = false;

	return bAllow;
}

void CDnBlow::OnDuplicate( const STATE_BLOW& StateBlowInfo )
{
	if (StateBlowInfo.szValue.empty())
		return;

	// Time is replaced with the passed value
	m_StateBlow.fDurationTime = StateBlowInfo.fDurationTime;

	const char* szTempValue = StateBlowInfo.szValue.c_str();
	SetValue( szTempValue );
	if( false == CalcDuplicateValue( szTempValue ) )
		m_fValue += (float)atof( szTempValue );
}

void CDnBlow::Duplicate( const STATE_BLOW& StateBlowInfo )
{
	m_bDuplicated = true;
	m_iDuplicateCount++;

	OnDuplicate( StateBlowInfo );
}


#ifdef _GAMESERVER
void CDnBlow::CheckAndStartActorActionInEffectInfo( void )
{
	bool bAllow = CanAddGraphicsEffect();
	if( false == bAllow )
		return;

	// #31477 Do not execute action if it is a nest boss monster.
	bool bNestMap = false;
	CDNGameRoom* pGameRoom = static_cast<CDNGameRoom*>(m_hActor->GetRoom());
	if( false == pGameRoom->bIsPvPRoom() )
	{
		UINT uiSessionID = 0;
		pGameRoom->GetLeaderSessionID( uiSessionID );
		CDNUserSession *pUserSession = pGameRoom ? pGameRoom->GetUserSession(uiSessionID) : NULL;

		TMapInfo* pMapData = NULL;
		if( pUserSession )
		{
			const TMapInfo* pMapData = g_pDataManager->GetMapInfo( pUserSession->GetMapIndex() );
			if( pMapData )
#if defined(PRE_ADD_DRAGON_FELLOWSHIP)
				bNestMap = CDnBlow::CheckEffectIgnoreMapType(pMapData->MapType, pMapData->MapSubType);
#else	// #if defined(PRE_ADD_DRAGON_FELLOWSHIP)
				bNestMap = (GlobalEnum::MAP_DUNGEON == pMapData->MapType) && (GlobalEnum::MAPSUB_NEST == pMapData->MapSubType);
#endif	// #if defined(PRE_ADD_DRAGON_FELLOWSHIP)
		}
	}

	if( bNestMap )
	{
		if( m_hActor->IsMonsterActor() )
		{
			CDnMonsterActor* pMonsterActor = static_cast<CDnMonsterActor*>(m_hActor.GetPointer());
			if( (CDnMonsterState::Boss == pMonsterActor->GetGrade() ||
				CDnMonsterState::BossHP4 == pMonsterActor->GetGrade() ||
				 CDnMonsterState::NestBoss == pMonsterActor->GetGrade() ||
				 CDnMonsterState::NestBoss8 == pMonsterActor->GetGrade()) )
			{
				// This ensures boss-grade monsters do not execute the action defined in the state effect table.
				return;
			}
		}
	}

	// #31067 For monsters, execute directly if there is an action to execute.
	if( m_hActor->IsMonsterActor() )
	{
		if (m_ParentSkillInfo.szEffectOutputIDs.empty() == false)
		{
			DNTableFileFormat* pStateEffectTable = CDnTableDB::GetInstancePtr()->GetTable( CDnTableDB::TSTATEEFFECT );

			std::vector<std::string> infoTokens;
			std::string delimiters = ";";

			TokenizeA(m_ParentSkillInfo.szEffectOutputIDs, infoTokens, delimiters);
			int nTokenSize = (int)infoTokens.size();
			for (int i = 0; i < nTokenSize; ++i)
			{
				int nEffectOutputID = atoi(infoTokens[i].c_str());
				if (IsMatchStateEffectIndex(nEffectOutputID))
				{
					const char* pActionName = pStateEffectTable->GetFieldFromLablePtr( nEffectOutputID, "_ActorActionName" )->GetString();

					if( pActionName && 0 < strlen( pActionName ) )
					{
						if( m_hActor->IsExistAction( pActionName ) )
						{
							m_hActor->CmdAction( pActionName );
							m_bStartedMonsterActionInSETableInfo = true;
						}
					}
				}
			}
		}
	}
}

void CDnBlow::CheckAndStopActorActionInEffectInfo( void )
{
	// #31067 For monsters, execute directly if there is an action to execute.
	if( m_hActor && m_hActor->IsMonsterActor() && m_bStartedMonsterActionInSETableInfo )
	{
		m_hActor->CmdAction( "Stand" );
		m_bStartedMonsterActionInSETableInfo = false;

	}
}
#endif // #ifdef _GAMESERVER

#ifdef _GAMESERVER
const CPacketCompressStream* CDnBlow::GetPacketStream( const char* pszOrgParam/*=NULL*/, const bool bOnInit/*=false*/ )
{
	if( NULL == m_pPacketStream )
	{
		m_pPacketStream = new CPacketCompressStream( m_PacketBuffer, sizeof(m_PacketBuffer) );

		DWORD dwHitterUniqueID = -1;
		BYTE cItemSkill	= 0;
		if( 0 != m_ParentSkillInfo.iSkillID )
		{
			if( m_ParentSkillInfo.hSkillUser )
				dwHitterUniqueID = m_ParentSkillInfo.hSkillUser->GetUniqueID();

			cItemSkill = m_ParentSkillInfo.bIsItemSkill ? 1 : 0;
		}

		m_pPacketStream->Write( &dwHitterUniqueID, sizeof(DWORD) );
		m_pPacketStream->Write( &cItemSkill, sizeof(BYTE) );

		if( 0 != m_ParentSkillInfo.iSkillID )
		{
			m_pPacketStream->Write( &m_ParentSkillInfo.iSkillID, sizeof(int) );
			m_pPacketStream->Write( &m_ParentSkillInfo.iSkillLevelID, sizeof(int) );
		}
		else
		{
			int iTemp = 0;
			m_pPacketStream->Write( &iTemp, sizeof(int) );
			m_pPacketStream->Write( &iTemp, sizeof(int) );
		}

		m_pPacketStream->Write( &m_StateBlow.emBlowIndex, sizeof(int) );

		int iDurationTime = int(m_StateBlow.fDurationTime * 1000.0f);
		if( m_bPermanentBlow )
			iDurationTime = -1;

		m_pPacketStream->Write( &iDurationTime, sizeof(int) );

		// Blows that process szParam like RebirthBlow build the packet using the original szParam.
		if( pszOrgParam )
		{
			int nLength = static_cast<int>(strlen(pszOrgParam));
			m_pPacketStream->Write( &nLength, sizeof(int) );
			m_pPacketStream->Write( pszOrgParam, nLength );
		}
		else
		{
			int nLength = m_StateBlow.szValue.empty() ? 0 : static_cast<int>(m_StateBlow.szValue.size());
			m_pPacketStream->Write( &nLength, sizeof(int) );
			if( !m_StateBlow.szValue.empty() )
				m_pPacketStream->Write( m_StateBlow.szValue.c_str(), nLength );
		}

		BYTE cOnInit = bOnInit ? 1: 0;
		m_pPacketStream->Write( &cOnInit, sizeof(BYTE) );

		// ID generated on the server.
		m_pPacketStream->Write( &m_nBlowID, sizeof(int) );

		int nLength = m_ParentSkillInfo.szEffectOutputIDToClient.empty() ? 0 : static_cast<int>(m_ParentSkillInfo.szEffectOutputIDToClient.size());
		m_pPacketStream->Write( &nLength, sizeof(int) );
		if( !m_ParentSkillInfo.szEffectOutputIDToClient.empty() )
			m_pPacketStream->Write( m_ParentSkillInfo.szEffectOutputIDToClient.c_str(), nLength );

		m_pPacketStream->Write( &m_ParentSkillInfo.bFromInputHasPassiveSignal, sizeof(bool) );
		m_pPacketStream->Write( &m_ParentSkillInfo.bFromBuffProp, sizeof(bool) );
		// Normally follows the skill table value, except for cases like shock transfer state effect (168)
		// Sometimes modifies and sends this value.
		m_pPacketStream->Write( &m_ParentSkillInfo.eTargetType, sizeof(CDnSkill::TargetTypeEnum) );

		// Notify client that this is an item prefix skill state effect. Whether to call OnBegin immediately on add [2011/03/18 semozz]
		m_pPacketStream->Write( &m_ParentSkillInfo.bItemPrefixSkill, sizeof(m_ParentSkillInfo.bItemPrefixSkill) );

		// Added to ignore immunity and use guild war skill (gate-related) [2011/04/14 semozz]
		m_pPacketStream->Write( &m_ParentSkillInfo.bIgnoreImmune, sizeof(m_ParentSkillInfo.bIgnoreImmune) );

#if defined(PRE_FIX_NEXTSKILLINFO)
		m_pPacketStream->Write( &m_ParentSkillInfo.iLevel, sizeof(m_ParentSkillInfo.iLevel) );
#endif // PRE_FIX_NEXTSKILLINFO

		m_pPacketStream->Write( &m_bEternity, sizeof(bool) );

#ifdef PRE_FIX_SYNC_ENCHANT_SKILL
		m_pPacketStream->Write(&m_ParentSkillInfo.iAppliedEnchantSkillID, sizeof(int));
#endif

		// If it is a chain attack state effect (ID 60), add projectile-related info.
#if !defined(SW_ADD_CHAINATTACK_STATEEFFECT_20091029_jhk8211 )
		this->WriteAdditionalPacket();
#endif
	}

	return m_pPacketStream;
}
#endif


#if defined(_GAMESERVER)
#if defined(PRE_FIX_BLOCK_CONDITION)
bool CDnBlow::IsCanBlock(DnActorHandle hHitter, DnActorHandle hHitted, const CDnDamageBase::SHitParam &HitParam)
{
	//Because Ice Prison has a team inversion effect,
	int nHitterTeam = hHitter->GetTeam();
	if (hHitter->IsAppliedThisStateBlow(STATE_BLOW::BLOW_149))
		nHitterTeam = hHitter->GetOriginalTeam();

	int nHittedTeam = hHitted->GetTeam();
	if (hHitted->IsAppliedThisStateBlow(STATE_BLOW::BLOW_149))
		nHittedTeam = hHitted->GetOriginalTeam();

	//When on the same team
	if (nHitterTeam == nHittedTeam)
	{
		//Cannot block if damage ratio is 0..
		if (HitParam.fDamage == 0.0f)
			return false;
	}
	
	return true;
}
#endif // PRE_FIX_BLOCK_CONDITION

#if defined(PRE_ADD_49166)
bool CDnBlow::IsInVaildBlockCondition(DnActorHandle hActor)
{
	//Block does not work when Freezing/FrameStop state is applied..
	if (hActor &&
		(hActor->IsAppliedThisStateBlow(STATE_BLOW::BLOW_041) ||
		hActor->IsAppliedThisStateBlow(STATE_BLOW::BLOW_146)
		) )
		return true;

	return false;
}
#endif // PRE_ADD_49166
#endif // _GAMESERVER

bool CDnBlow::IsMatchStateEffectIndex(int nStateEffectIndex)
{
	bool bMatchSEIndex = ((nStateEffectIndex == 0) || (m_StateBlow.emBlowIndex == nStateEffectIndex));

	return bMatchSEIndex;
}

#if defined(PRE_ADD_DARKLAIR_HEAL_REGULATION) && defined(_GAMESERVER) 
float CDnBlow::CalcModifiedHealValue( float fModifyValue )
{
#ifdef PRE_ADD_IGNORE_RESTORE_HP
	if( fModifyValue > 0 && m_hActor && m_hActor->IsAppliedThisStateBlow( STATE_BLOW::BLOW_275 ) )
	{
		if( m_bHasParentSkill )
		{
			DNVector(DnBlowHandle) vlBlows;
			m_hActor->GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_275, vlBlows );
			for( DWORD i=0; i<vlBlows.size(); i++ )
			{
				if( vlBlows[i] )
				{
					CDnIgnoreRestoreHPBlow* pIgnoreRestoreHPBlow = static_cast<CDnIgnoreRestoreHPBlow*>( vlBlows[i].GetPointer() );
					if( pIgnoreRestoreHPBlow && GetParentSkillInfo()->iSkillID != pIgnoreRestoreHPBlow->GetAllowSkillID() )
					{
						m_hActor->SendAddSEFail( CDnStateBlow::ADD_FAIL_BY_IMMUNE, (STATE_BLOW::emBLOW_INDEX)GetBlowID() );
						fModifyValue = 0.0f;
						break;
					}
				}
			}
		}
		else
		{
			m_hActor->SendAddSEFail( CDnStateBlow::ADD_FAIL_BY_IMMUNE, (STATE_BLOW::emBLOW_INDEX)GetBlowID() );
			fModifyValue = 0.0f;
		}
	}
#endif // PRE_ADD_IGNORE_RESTORE_HP

	if( m_bHasParentSkill == true && GetParentSkillInfo() && GetParentSkillInfo()->bIsItemSkill == false )
	{
		if( fModifyValue > 0 && m_hActor && m_hActor->IsPlayerActor() && m_hActor->GetRoom() )
		{
			CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor*>(m_hActor.GetPointer());
			if( pPlayer )
			{
				CDNGameRoom *pRoom = pPlayer->GetGameRoom();
				if( pRoom && pRoom->bIsDLRoom() )
				{
#ifdef PRE_ADD_CHALLENGE_DARKLAIR
					CDnGameTask *pTask = pRoom->GetGameTask();
					if( pTask )
					{
						CDnDLGameTask *pDLTask = (CDnDLGameTask *)pTask;
						if( pDLTask && pDLTask->IsChallengeDarkLair() )
						{
							fModifyValue *=  CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::Chalange_DarkLair_Heal_Limit );
						}
					}
#endif
				}
			}
		}
	}

	return fModifyValue;
}
#endif

#if defined( _GAMESERVER )
void CDnBlow::CalcHealValueLimit( const STATE_BLOW::emBLOW_INDEX emBlowIndex, float & fModifyValue )
{
	DnActorHandle hTargetActor = m_hActor;

	if( emBlowIndex == STATE_BLOW::BLOW_227 )
		hTargetActor = GetParentSkillInfo()->hSkillUser;

#ifdef PRE_ADD_IGNORE_RESTORE_HP
	if( fModifyValue > 0 && hTargetActor && hTargetActor->IsAppliedThisStateBlow( STATE_BLOW::BLOW_275 ) )
	{
		if( m_bHasParentSkill )
		{
			DNVector(DnBlowHandle) vlBlows;
			hTargetActor->GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_275, vlBlows );
			for( DWORD i=0; i<vlBlows.size(); i++ )
			{
				if( vlBlows[i] )
				{
					CDnIgnoreRestoreHPBlow* pIgnoreRestoreHPBlow = static_cast<CDnIgnoreRestoreHPBlow*>( vlBlows[i].GetPointer() );
					if( pIgnoreRestoreHPBlow && GetParentSkillInfo()->iSkillID != pIgnoreRestoreHPBlow->GetAllowSkillID() )
					{
						hTargetActor->SendAddSEFail( CDnStateBlow::ADD_FAIL_BY_IMMUNE, (STATE_BLOW::emBLOW_INDEX)GetBlowID() );
						fModifyValue = 0.0f;
						break;
					}
				}
			}
		}
		else
		{
			hTargetActor->SendAddSEFail( CDnStateBlow::ADD_FAIL_BY_IMMUNE, (STATE_BLOW::emBLOW_INDEX)GetBlowID() );
			fModifyValue = 0.0f;
		}
	}
#endif // PRE_ADD_IGNORE_RESTORE_HP

#if defined( PRE_ADD_HEAL_TABLE_REGULATION )
	if( m_bHasParentSkill == false
		|| GetParentSkillInfo() == NULL
		|| GetParentSkillInfo()->bIsItemSkill == true
		|| fModifyValue <= 0
		|| hTargetActor == CDnActor::Identity()
		|| ( hTargetActor->IsPlayerActor() == false && STATE_BLOW::BLOW_227 != emBlowIndex )
		|| hTargetActor->GetRoom() == NULL )
		return;

	CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor*>(hTargetActor.GetPointer());
	if( NULL == pPlayer )
		return;

	CDNGameRoom *pRoom = pPlayer->GetGameRoom();
	if( NULL == pRoom )
		return;

	CDnGameTask *pTask = pRoom->GetGameTask();
	if( NULL == pTask )
		return;

	float fHealLimit_Type1 = pTask->GetStageHealLimit().fHealLimit_Type1;
	float fHealLimit_Type2 = pTask->GetStageHealLimit().fHealLimit_Type2;
	if( 0 == fHealLimit_Type1 || 0 == fHealLimit_Type2 )
		return;

	switch( emBlowIndex )
	{
	case STATE_BLOW::BLOW_011:
	case STATE_BLOW::BLOW_012:
	case STATE_BLOW::BLOW_015:
	case STATE_BLOW::BLOW_016:
	case STATE_BLOW::BLOW_091:
	case STATE_BLOW::BLOW_092:
	case STATE_BLOW::BLOW_140:
		fModifyValue *= fHealLimit_Type1;
		break;

	case STATE_BLOW::BLOW_141:
	case STATE_BLOW::BLOW_227:
	case STATE_BLOW::BLOW_248:
		fModifyValue *= fHealLimit_Type2;
		break;
	}
#endif // PRE_ADD_HEAL_TABLE_REGULATION
}
#endif // _GAMESERVER
#if defined(PRE_ADD_DRAGON_FELLOWSHIP)
//Check if state effects (freeze/shock, etc.) are ignored on this map (Nest/Nest (Practice Mode)/Dragon Expedition)
#if defined( _GAMESERVER )
bool CDnBlow::CheckEffectIgnoreMapType(GlobalEnum::eMapTypeEnum eMapType, GlobalEnum::eMapSubTypeEnum eSubMapType)
{
	if(eMapType != GlobalEnum::MAP_DUNGEON) return false;

	if( (eSubMapType == GlobalEnum::MAPSUB_NEST) || 
		(eSubMapType == GlobalEnum::MAPSUB_NESTNORMAL) || 
		(eSubMapType == GlobalEnum::MAPSUB_FELLOWSHIP) )
		return true;

	return false;
}
#else
bool CDnBlow::CheckEffectIgnoreMapType()
{
	CDnWorld *pWorld = CDnWorld::GetInstancePtr();
	if(pWorld->GetMapType() != CDnWorld::MapTypeEnum::MapTypeDungeon) return false;

	if( (pWorld->GetMapSubType() == CDnWorld::MapSubTypeEnum::MapSubTypeNest) || 
		(pWorld->GetMapSubType() == CDnWorld::MapSubTypeEnum::MapSubTypeNestNormal) || 
		(pWorld->GetMapSubType() == CDnWorld::MapSubTypeEnum::MapSubTypeFellowship) )
		return true;

	return false;
}
#endif
#endif	// #if defined(PRE_ADD_DRAGON_FELLOWSHIP)
#include "stdafx.h"
#include "DnStateBlow.h"
#include "DnCreateBlow.h"
#include "DnBlow.h"
#include "DnImmuneBlow.h"
#include "DnAllImmuneBlow.h"
#include "DnUsingSkillWhenDieBlow.h"
#include "DnTableDB.h"
#include "DnPlayerActor.h"
#include "DnProbInvincibleAtBlow.h"
#include "DnIgnoreEffectBlow.h"
#include "DnComboDamageLimitBlow.h"
#include "DnPileAddEffectBlow.h"
#ifdef PRE_ADD_DECREASE_EFFECT
#include "DnPuppetBlow.h"
#endif // PRE_ADD_DECREASE_EFFECT

#ifdef PRE_FIX_MEMOPT_EXT
#ifndef _GAMESERVER
#include "DnCommonUtil.h"
#endif
#endif

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

#ifndef _GAMESERVER
std::map<int, EffectOutputInfo*> CDnStateBlow::s_nMapEffectOutputInfo;
bool CDnStateBlow::s_bStopProcess = false;
#else
STATIC_DECL_INIT( CDnStateBlow, bool, s_bStopProcess ) = { false, };
#endif

#ifndef _GAMESERVER
int CDnStateBlow::s_iStateBlowIDCount = 0;
#else
STATIC_DECL_INIT( CDnStateBlow, int, s_iStateBlowIDCount ) = { 0, };
#endif

CDnStateBlow::CDnStateBlow( DnActorHandle hActor )
{
	m_hActor = hActor;

	ZeroMemory( m_aiApplied, sizeof(m_aiApplied) );

#ifndef _GAMESERVER
	m_bAllowDiffuseVariation = true;
	_LoadStateEffectOutputInfo();
#endif

	m_bLockStateBlowList = false;
	m_bLockRemoveReservedList = false;
}

CDnStateBlow::~CDnStateBlow(void)
{
	DelAllStateBlow();
}

bool CDnStateBlow::InitializeClass()
{
#ifndef _GAMESERVER
	_LoadStateEffectOutputInfo();
#endif
	return true;
}

void CDnStateBlow::ReleaseClass()
{
#ifndef _GAMESERVER
	_ReleaseStateEffectOutputInfo();
#endif
}


#ifndef _GAMESERVER
void CDnStateBlow::_LoadStateEffectOutputInfo( void )
{
	if( !s_nMapEffectOutputInfo.empty() )
		return;

	DNTableFileFormat* pStateEffectTable = CDnTableDB::GetInstancePtr()->GetTable( CDnTableDB::TSTATEEFFECT );

#ifdef PRE_FIX_MEMOPT_EXT
	DNTableFileFormat* pFileNameSox = CDnTableDB::GetInstancePtr()->GetTable(CDnTableDB::TFILE);
	if (pFileNameSox == NULL)
		return;
#endif
	
	int iNumItem = pStateEffectTable->GetItemCount();
	for( int i=0; i<iNumItem; i++ ) 
	{
		int iItem = pStateEffectTable->GetItemID(i);

		EffectOutputInfo* pNewInfo = new EffectOutputInfo;

		int iStateEffectIndex = pStateEffectTable->GetFieldFromLablePtr( iItem, "_StateEffectIndex" )->GetInteger(); 
		_ASSERT( pNewInfo->iStateEffectIndex < STATE_BLOW::BLOW_MAX );

		pNewInfo->iStateEffectIndex = iStateEffectIndex;
		pNewInfo->iShowTimingType = pStateEffectTable->GetFieldFromLablePtr( iItem, "_ShowTimingType" )->GetInteger();
		pNewInfo->iOutputType = pStateEffectTable->GetFieldFromLablePtr( iItem, "_EffectOutputType" )->GetInteger();
		pNewInfo->iPlayType = pStateEffectTable->GetFieldFromLablePtr( iItem, "_EffectPlayType" )->GetInteger();
#ifdef PRE_FIX_MEMOPT_EXT
		CommonUtil::GetFileNameFromFileEXT(pNewInfo->strSkinFileName, pStateEffectTable, iItem, "_EffectSkinFileName", pFileNameSox);
		CommonUtil::GetFileNameFromFileEXT(pNewInfo->strAniFileName, pStateEffectTable, iItem, "_EffectAniFileName", pFileNameSox);
		CommonUtil::GetFileNameFromFileEXT(pNewInfo->strActFileName, pStateEffectTable, iItem, "_EffectActFileName", pFileNameSox);
#else
		pNewInfo->strSkinFileName = pStateEffectTable->GetFieldFromLablePtr( iItem, "_EffectSkinFileName" )->GetString();
		pNewInfo->strAniFileName = pStateEffectTable->GetFieldFromLablePtr( iItem, "_EffectAniFileName" )->GetString();
		pNewInfo->strActFileName = pStateEffectTable->GetFieldFromLablePtr( iItem, "_EffectActFileName" )->GetString();
#endif
		pNewInfo->strActorActionName = pStateEffectTable->GetFieldFromLablePtr( iItem, "_ActorActionName" )->GetString();
		pNewInfo->iUseDiffuseVariationMethod = pStateEffectTable->GetFieldFromLablePtr( iItem, "_UseDiffuseVariation" )->GetInteger();
		pNewInfo->afDiffuseRed[ 0 ] = (float)pStateEffectTable->GetFieldFromLablePtr( iItem, "_DiffuseR1" )->GetFloat();
		pNewInfo->afDiffuseBlue[ 0 ] = (float)pStateEffectTable->GetFieldFromLablePtr( iItem, "_DiffuseG1" )->GetFloat();
		pNewInfo->afDiffuseGreen[ 0 ] = (float)pStateEffectTable->GetFieldFromLablePtr( iItem, "_DiffuseB1" )->GetFloat();
		pNewInfo->afDiffuseRed[ 1 ] = (float)pStateEffectTable->GetFieldFromLablePtr( iItem, "_DiffuseR2" )->GetFloat();
		pNewInfo->afDiffuseBlue[ 1 ] = (float)pStateEffectTable->GetFieldFromLablePtr( iItem, "_DiffuseG2" )->GetFloat();
		pNewInfo->afDiffuseGreen[ 1 ] = (float)pStateEffectTable->GetFieldFromLablePtr( iItem, "_DiffuseB2" )->GetFloat();
		pNewInfo->fDiffuseChangeSpeed = (float)pStateEffectTable->GetFieldFromLablePtr( iItem, "_DiffuseChangeSpeed" )->GetFloat();

		// Retrieve index values separated by ';'
		const char* pIndices = pStateEffectTable->GetFieldFromLablePtr( iItem, "_LinkBoneIndex" )->GetString();
		string strIndices( pIndices );
		if( (int)strIndices.length() > 0 )
		{
			int nFoundPos = -1;
			if( strIndices.at(strIndices.length()-1) != ';' )
				strIndices.push_back( ';' );
			while( true )
			{
				int nStartPos = nFoundPos + 1; 
				nFoundPos = (int)strIndices.find_first_of( ';', nStartPos );
				if( nFoundPos != (int)string::npos )
				{
					string strIndex( strIndices.substr(nStartPos, nFoundPos-nStartPos) );
					pNewInfo->vlDummyBoneIndices.push_back( atoi(strIndex.c_str()) );
				}
				else
					break;
			} 
		}
		s_nMapEffectOutputInfo.insert( make_pair( iItem, pNewInfo ) );
	}
}

void CDnStateBlow::_ReleaseStateEffectOutputInfo( void )
{
	SAFE_DELETE_PMAP( TMapEffectOutputInfo, s_nMapEffectOutputInfo );
}

void CDnStateBlow::_ProcessDuplicateEffectBoneRotateShow( LOCAL_TIME LocalTime, float fDelta )
{
	// Remove invalid effects before processing.
	for( int i = 0; i < CDnActor::Max_FX_Dummy_Bone; ++i )
	{
		list<S_BONE_EFFECT>& listBoneEffect = m_aListBoneEffectStatus[ i ];

		list<S_BONE_EFFECT>::iterator iter = listBoneEffect.begin();
		for( iter; iter != listBoneEffect.end();  )
		{
			if( !iter->hEffect )
			{
				bool bNextShow = iter->bShow;
				iter = listBoneEffect.erase( iter );

				if( listBoneEffect.end() != iter )
					if( bNextShow )
						iter->Show( bNextShow );

				continue;
			}

			++iter;
		}
	}


	for( int i = 0; i < CDnActor::Max_FX_Dummy_Bone; ++i )
	{
		list<S_BONE_EFFECT>& listBoneEffect = m_aListBoneEffectStatus[ i ];

		// Alternate rendering for overlapping multiple bones.
		if( 1 < (int)listBoneEffect.size() )
		{
			list<S_BONE_EFFECT>::iterator iter = listBoneEffect.begin();
			for( iter; iter != listBoneEffect.end(); ++iter )
			{
				if( iter->bShow )
				{
					iter->fOutputElapsedTime += fDelta;

					if( 1.0f < iter->fOutputElapsedTime )
					{
						list<S_BONE_EFFECT>::iterator iterNext = iter;
						iterNext++;
						if( listBoneEffect.end() == iterNext )
							iterNext = listBoneEffect.begin();
						
						// Hide previously displayed effect.
						iter->Show( false );
						iter->fOutputElapsedTime = 0.0f;

						// Show the new effect to display.
						iterNext->Show( true );
						iterNext->fOutputElapsedTime = 0.f;
						break;
					}
				}
			}
		}
	}
}
#endif


void CDnStateBlow::Process( LOCAL_TIME LocalTime, float fDelta, bool bForceInitialize )
{
#ifdef _GAMESERVER
	if( m_hActor && true == s_bStopProcess[ m_hActor->GetRoom()->GetRoomID() ] )
		return;
#else
	if( s_bStopProcess )
		return;
#endif

	if( !m_listBlowHandle.empty() ) 
	{
		BLOW_HANDLE_LIST_ITER iter =  m_listBlowHandle.begin();

#ifndef _GAMESERVER
		if( m_bAllowDiffuseVariation )
		{
			// Output color change only for the single one with the highest diffuse variation number
			BLOW_HANDLE_LIST_ITER iterAllowDV = m_listBlowHandle.end();
			int iBiggest = 0;
			for( ; iter != m_listBlowHandle.end(); ++iter )
			{
				if( (*iter) && (*iter)->IsUseTableDefinedGraphicEffect() )
				{
					(*iter)->AllowDiffuseVariation( false );
					const EffectOutputInfo* pEffectInfo = (*iter)->GetEffectOutputInfo();
					if( pEffectInfo && pEffectInfo->iUseDiffuseVariationMethod > 0 )
					{
						if( iBiggest < pEffectInfo->iUseDiffuseVariationMethod )
						{
							iBiggest = pEffectInfo->iUseDiffuseVariationMethod;
							iterAllowDV = iter;
						}
					}
				}
			}

			if( m_listBlowHandle.end() != iterAllowDV )
			{
				(*iterAllowDV)->AllowDiffuseVariation( true );
			}
		}
#endif

#ifndef _FINAL_BUILD
		_CrashIfProcessListLocked();
#endif // #ifdef _FINAL_BUILD
		
		// Check in case the list is cleared during process while looping over state effect objects.
		// Terminate cleanly if not a final release build.
		m_bLockStateBlowList = true;

		Process_StateBlow_New(LocalTime, fDelta, bForceInitialize);
    }
	m_bLockStateBlowList = false;

#ifndef _GAMESERVER

#if !defined(SW_MODIFY_SE_EFFECTOUTPUT_20091119_jhk8211)
	_ProcessDuplicateEffectBoneRotateShow( LocalTime, fDelta );
#endif

#endif

#if defined( _GAMESERVER )
	// If StateBlow::Process() OnRebith() is called here, StateBlow::Process() cannot run freely internally so
	// Structurally change it to run as post-processing after everything finishes.
	if( m_hActor )
		m_hActor->OnStateBlowProcessAfter();
#endif // #if defined( _GAMESERVER )

	RemoveStateBlowReservedList();
}

int CDnStateBlow::CanAddThisBlow( const CDnSkill::SkillInfo* pParentSkill, STATE_BLOW::emBLOW_INDEX BlowIndex )
{
	int iResult = ADD_SUCCESS;

	// #35335 Burn and poison state effects are not applied while invincible state effect is active.
	if( IsApplied( STATE_BLOW::BLOW_099 ) )
	{
		if( (STATE_BLOW::BLOW_042 == BlowIndex) ||
			(STATE_BLOW::BLOW_044 == BlowIndex) )
		return ADD_FAIL_BY_INVINCIBLE;
	}

#ifdef _GAMESERVER
	// Whether state blow #226 (chance invincibility against specific skill) is active for this skill.
	if( pParentSkill && IsApplied( STATE_BLOW::BLOW_226 ) )
	{
		DNVector( DnBlowHandle ) vlhBlows;
		GetStateBlowFromBlowIndex( STATE_BLOW::BLOW_226, vlhBlows );
		
		for( int i = 0; i < (int)vlhBlows.size(); ++i )
		{
			bool bCanAdd = static_cast<CDnProbInvincibleAtBlow*>(vlhBlows.at(i).GetPointer())->CanAddThisSkillsStateBlow( pParentSkill->iSkillID );
			if( false == bCanAdd )
				return ADD_FAIL_BY_PROB_SKILL_INVINCIBLE;
		}
	}
#endif // #ifdef _GAMESERVER

	if (pParentSkill && pParentSkill->bIgnoreImmune)
		return iResult;

#if defined (_GAMESERVER) 
	//Related to issue #34452 
	bool bCheckGuildPriority = false;	
	if (pParentSkill && pParentSkill->bIsGuildSkill == false)
		bCheckGuildPriority = true;
#endif

	// Check if currently immune to this state effect
	// During resurrection (invincible) state effect, immune to damage-dealing state effects.
	list<DnBlowHandle>::iterator iter = m_listBlowHandle.begin();
	for( iter; iter != m_listBlowHandle.end(); )
	{
		DnBlowHandle hNowBlow = (*iter);
		if( hNowBlow )
		{
			if( (hNowBlow->GetBlowIndex() == STATE_BLOW::BLOW_077) )
			{
				bool bImmuned = static_cast<CDnImmuneBlow*>(hNowBlow.GetPointer())->IsImmuned( pParentSkill, BlowIndex );
				if( bImmuned )
					iResult = ADD_FAIL_BY_IMMUNE;
			}
			else if( (hNowBlow->GetBlowIndex() == STATE_BLOW::BLOW_150) )
			{
				bool bImmuned = static_cast<CDnAllImmuneBlow*>(hNowBlow.GetPointer())->IsImmuned( pParentSkill, BlowIndex );
				if( bImmuned )
					iResult = ADD_FAIL_BY_IMMUNE;
			}
			else if( IsApplied(STATE_BLOW::BLOW_057)  )
			{
				// While resurrecting, only apply self-granted state effects from own skills or state effects from buff/aura skills.
				if( pParentSkill )
				{
					CDnSkill::SkillInfo* pSkillInfo = const_cast<CDnSkill::SkillInfo*>(pParentSkill);
					if( pSkillInfo->hSkillUser )
					{
						if(	!(pSkillInfo->hSkillUser == m_hActor) &&
							(CDnSkill::Buff != pSkillInfo->eDurationType) && 
							(CDnSkill::Aura != pSkillInfo->eDurationType) &&
							(CDnSkill::StanceChange != pSkillInfo->eDurationType) )
						{
							iResult = ADD_FAIL_BY_REVIVAL;
						}
					}
				}
			}
			else if (hNowBlow->GetBlowIndex() == STATE_BLOW::BLOW_252)
			{
				//#53722 Develop skill effect for immunity to specific skills (allow hit, but prevent adding state effects..)
				CDnIngnoreEffectBlow* pIgnoreEffectBlow = static_cast<CDnIngnoreEffectBlow*>(hNowBlow.GetPointer());
				if (pIgnoreEffectBlow && pParentSkill && pIgnoreEffectBlow->IsInvincibleAt(pParentSkill->iSkillID))
					iResult = ADD_FAIL_BY_IMMUNE;
			}
#if defined(_GAMESERVER)
			else if ( hNowBlow->GetBlowIndex() == STATE_BLOW::BLOW_242 && BlowIndex == STATE_BLOW::BLOW_242)
			{
				//Prevent state effect 242 from being added again if it has the same skill ID and same user.
				if (pParentSkill)
				{
					CDnSkill::SkillInfo* pParentSkillInfo = const_cast<CDnSkill::SkillInfo*>(pParentSkill);
					CDnSkill::SkillInfo* pExistSkillInfo = const_cast<CDnSkill::SkillInfo*>(hNowBlow->GetParentSkillInfo());

					// #56880 Also need to check whether the skill start time is the same..
					LOCAL_TIME parentSkillStartTime = 0;
					LOCAL_TIME nowBlowSkillStartTime = 0;

					CDnComboDamageLimitBlow* pNowBlow = static_cast<CDnComboDamageLimitBlow*>(hNowBlow.GetPointer());
					if (pNowBlow)
						nowBlowSkillStartTime = pNowBlow->GetSkillStartTime();

					DnSkillHandle hParentSkill;
					if (pParentSkillInfo->hSkillUser)
						hParentSkill = pParentSkillInfo->hSkillUser->FindSkill(pParentSkillInfo->iSkillID);
					
					if (hParentSkill)
						parentSkillStartTime = pParentSkillInfo->projectileSkillStartTime != 0 ? pParentSkillInfo->projectileSkillStartTime : hParentSkill->GetSkillStartTime();

					if (pExistSkillInfo)
					{
						if (pParentSkillInfo->hSkillUser && 
							pParentSkillInfo->hSkillUser == pExistSkillInfo->hSkillUser &&	//If the skill user is the same and
							pParentSkillInfo->iSkillID == pExistSkillInfo->iSkillID &&		//When the skill is the same
							nowBlowSkillStartTime == parentSkillStartTime					//When skill start time is the same
							)
						{
							iResult = ADD_FAIL_BY_COMBOLIMITBLOW;
						}
					}
				}
			}
			else if (hNowBlow->GetBlowIndex() == STATE_BLOW::BLOW_249)
			{
				//Prevent state effect 249 from being added once stacked and activated? (when stack count reaches configured value)..
				CDnPileAddEffectBlow* pPileAddEffectBlow = static_cast<CDnPileAddEffectBlow*>(hNowBlow.GetPointer());
				if (pPileAddEffectBlow && pPileAddEffectBlow->IsActivatedBlow())
					iResult = ADD_FAIL_BY_COMBOLIMITBLOW;	//Using this directly without adding a separate enum..
			}
#endif // _GAMESERVER


#if defined (_GAMESERVER)
			if (bCheckGuildPriority && hNowBlow->GetBlowIndex() == BlowIndex)
			{
				const CDnSkill::SkillInfo * pNowSkillInfo = static_cast<CDnBlow*>(hNowBlow.GetPointer())->GetParentSkillInfo();
				if (pNowSkillInfo->eDurationType == pParentSkill->eDurationType && pNowSkillInfo->bIsGuildSkill)
				{
					//If the same blow effect exists and was used as a guild skill, cannot use!
					iResult = ADD_FAIL_BY_GUILDBLOW_PRIORITY;
				}
			}
#endif
		}

		++iter;
	}

	return iResult;
}

void CDnStateBlow::_CheckImmediatelyBegin( DnBlowHandle hBlow )
{
	if( STATE_BLOW::BLOW_025 == hBlow->GetBlowIndex() )
	{
		// Since there are state effects like team inversion, when the team is inverted, the charm state effect 
		// Since it becomes the same team as the mob and would not hit, process the effect first. #14128
#ifndef _GAMESERVER
		if( hBlow->IsUseTableDefinedGraphicEffect() )
			hBlow->AttachGraphicEffectDefaultType();
#else
		hBlow->CheckAndStartActorActionInEffectInfo();
#endif
		hBlow->OnBegin( 0, 0.0f );
		m_hActor->OnBeginStateBlow( hBlow );

		// Check because some blows set state to end immediately.
		if( STATE_BLOW::STATE_END != hBlow->GetBlowState() )
			hBlow->SetState( STATE_BLOW::STATE_DURATION );
	}
}


int CDnStateBlow::AddStateBlow( DnBlowHandle hBlow )
{
	ASSERT(hBlow&&"CDnStateBlow::AddStateBlow");
	if( !hBlow ) return -1;
	
	if( false == hBlow->IsDuplicated() )
		hBlow->SetBlowID( GenerateStateBlowID() );
	
#ifndef _GAMESERVER
	if( m_bAllowDiffuseVariation )
	{
		const CDnSkill::SkillInfo* pParentSkillInfo = hBlow->GetParentSkillInfo();
		std::string effectOutputIDs = pParentSkillInfo ? pParentSkillInfo->effectOutputIDs : "";

#ifdef PRE_ADD_SKILL_ADDTIONAL_BUFF // If the skill affects enemies and Debuff is configured, set it with the configured debuff info.
		
		if(pParentSkillInfo && pParentSkillInfo->iSkillUserTeam != hBlow->GetActorHandle()->GetTeam())
		{
			//Check if there is a Debuff effect setting...
			std::vector<std::string> infoTokens;
			std::string delimiters = ";";

			bool isDebuggEffect = false;
			TokenizeA(pParentSkillInfo->debuffEffectOutputIDs, infoTokens, delimiters);
			int nTokenSize = (int)infoTokens.size();
			for (int i = 0; i < nTokenSize; ++i)
			{
				int nEffectOutputID = atoi(infoTokens[i].c_str());
				if (nEffectOutputID != 0)
				{
					isDebuggEffect = true;
					break;
				}
			}

			//If there is a valid value, change to debuff effect setting.
			if (isDebuggEffect)
				effectOutputIDs = pParentSkillInfo->debuffEffectOutputIDs;
		}
#endif

		EffectOutputInfo *pEffectInfo = NULL;

		std::vector<std::string> infoTokens;
		std::string delimiters = ";";

		TokenizeA(effectOutputIDs, infoTokens, delimiters);
		int nTokenSize = (int)infoTokens.size();
		for (int i = 0; i < nTokenSize; ++i)
		{
			int nEffectOutputID = atoi(infoTokens[i].c_str());
			pEffectInfo = GetEffectOutputInfo(nEffectOutputID);
			if (pEffectInfo && hBlow->IsMatchStateEffectIndex(pEffectInfo->iStateEffectIndex))
			{
				hBlow->SetEffectOutputInfo(pEffectInfo);
			}
		}
	}
#endif

	++m_aiApplied[ hBlow->GetBlowIndex() ];

	m_listBlowHandle.push_back( hBlow );

	_CheckImmediatelyBegin( hBlow );

	return hBlow->GetBlowID();
}

int CDnStateBlow::RemoveStateBlowByBlowDefineIndex( STATE_BLOW::emBLOW_INDEX emBlowIndex )
{
	// No handler for receiving this BlowID. Returning -1 for now.. not sure how it'll work...[2010/12/14 semozz]
	AddRemoveStateBlowInfo(RemoveStateBlowInfo(RemoveStateBlowInfo::RSBI_BLOW_INDEX, emBlowIndex));
	return -1;
}

void CDnStateBlow::GetStateBlowFromBlowIndex( STATE_BLOW::emBLOW_INDEX emBlowIndex, /*IN OUT*/ DNVector(DnBlowHandle)& vlhResult )
{
	int nIndex = -1;
	DnBlowHandle hFindedBlow;

	BLOW_HANDLE_LIST_ITER iter =  m_listBlowHandle.begin();

	for( ; iter != m_listBlowHandle.end(); )
	{
		DnBlowHandle hBlow = (*iter);

		if( hBlow && (hBlow->GetBlowIndex() == emBlowIndex) )
		{
			vlhResult.push_back( hBlow );
		}

		++iter;
	}
}

bool CDnStateBlow::IsExistStateBlowFromBlowIndex( STATE_BLOW::emBLOW_INDEX emBlowIndex )
{
	BLOW_HANDLE_LIST_ITER iter =  m_listBlowHandle.begin();

	for( ; iter != m_listBlowHandle.end(); ++iter)
	{
		if( (*iter) && ((*iter)->GetBlowIndex() == emBlowIndex) )
			return true;
	}

	return false;
}

bool CDnStateBlow::IsExistStateBlowFromBlowID( int nBlowID )
{
	BLOW_HANDLE_LIST_ITER iter =  m_listBlowHandle.begin();
	for( ; iter != m_listBlowHandle.end(); ++iter )
	{
		if( (*iter) && ((*iter)->GetBlowID() == nBlowID) )
			return true;
	}
	return false;
}

DnBlowHandle CDnStateBlow::GetStateBlowFromID( int nStateBlowID )
{
	DnBlowHandle hFindedBlow;

	BLOW_HANDLE_LIST_ITER iter =  m_listBlowHandle.begin();

	for( ; iter != m_listBlowHandle.end(); ++iter )
	{
		DnBlowHandle hBlow = (*iter);

		if( hBlow && (hBlow->GetBlowID() == nStateBlowID) )
		{
			hFindedBlow = hBlow;
			break;
		}
	}

	return hFindedBlow;
}

#ifndef _GAMESERVER
DnBlowHandle CDnStateBlow::GetStateBlowFromServerID( int nServerID )
{
	DnBlowHandle hFindedBlow;

	BLOW_HANDLE_LIST_ITER iter =  m_listBlowHandle.begin();

	for( ; iter != m_listBlowHandle.end(); ++iter )
	{
		DnBlowHandle hBlow = (*iter);
		if( hBlow && (hBlow->GetServerBlowID() == nServerID) )
		{
			hFindedBlow = hBlow;
			break;
		}
	}

	return hFindedBlow;
}
#endif


void CDnStateBlow::RemoveStateBlowFromID( int nStateBlowID )
{
	AddRemoveStateBlowInfo(RemoveStateBlowInfo(RemoveStateBlowInfo::RSBI_BLOW_ID, nStateBlowID));
}

DnBlowHandle CDnStateBlow::CreateStateBlow( DnActorHandle hActor, const CDnSkill::SkillInfo* pParentSkill, 
											STATE_BLOW::emBLOW_INDEX emBlowIndex, int nDurationTime, const char *szParam )
{
	DnBlowHandle hBlow;

	//Auto-set duration of state effect 242 to infinite?..
	//Auto-terminate the state effect in its Process function when conditions are met (server-side only)
	if (emBlowIndex == STATE_BLOW::BLOW_242)
		nDurationTime = -1;

	// Related to issue 6190.
	//Stack
	//	For skills with an effect stack count of 1 or more, the same skill at the same level stacks up to the max stack count.
	//	If the effect stack count is 1 or more but the skill level differs, apply separately.
	//	When stacked up to the max count, applying another stackable condition effect only resets the duration.
	// Do not stack if it is a prefix skill state effect.
	bool bItemPrefixSkill = false;
	if( pParentSkill && pParentSkill->bItemPrefixSkill )
		bItemPrefixSkill = true;

	bool bDuplicated = false;


	float fReduceTimeValue = 1.0f;
	if (pParentSkill && pParentSkill->eDurationType != CDnSkill::DurationTypeEnum::Instantly)
	{
		fReduceTimeValue = GetImmuneReduceTimeValue(emBlowIndex);
		nDurationTime = (int)((float)nDurationTime * fReduceTimeValue);

#ifdef PRE_ADD_DECREASE_EFFECT
#ifdef _GAMESERVER
		if( hActor && emBlowIndex != STATE_BLOW::BLOW_247 && fReduceTimeValue != 1.0f )
		{
			hActor->SendAddSEFail( CDnStateBlow::ADD_DECREASE_EFFECT_BY_IMMUNE, emBlowIndex );
		}
#endif // _GAMESERVER
#endif // PRE_ADD_DECREASE_EFFECT
	}

	//#56880 State effect 242 must not stack...
	//#52905 State effect 253 must not stack...
	if( emBlowIndex != STATE_BLOW::BLOW_242 &&
		emBlowIndex != STATE_BLOW::BLOW_253 &&
		false == bItemPrefixSkill && 
		0 < m_aiApplied[ emBlowIndex ] )
	{
		DNVector(DnBlowHandle) vlhResult;
		GetStateBlowFromBlowIndex( emBlowIndex, vlhResult );
		_ASSERT( !vlhResult.empty() );

		int iNumBlow = (int)vlhResult.size();
		for( int iBlow = 0; iBlow < iNumBlow; ++iBlow )
		{
			DnBlowHandle hExistBlow = vlhResult.at( iBlow );

			if( STATE_BLOW::STATE_END != hExistBlow->GetBlowState() )
			{
				const CDnSkill::SkillInfo* pExistParentSkillInfo = hExistBlow->GetParentSkillInfo();
				if( pExistParentSkillInfo && pParentSkill )
				{
					// Stack effects when the skill and level are the same.
					bool bLevelCheck = pExistParentSkillInfo->iSkillLevelID == pParentSkill->iSkillLevelID;
					bool bIndexCheck = pExistParentSkillInfo->iSkillID == pParentSkill->iSkillID;

					// #65533 Configure effects like PileAddEffect to stack even when created by different skill levels.
					if( emBlowIndex == STATE_BLOW::BLOW_249 ) 
						bLevelCheck = true;

					if( bLevelCheck && bIndexCheck )
					{
						STATE_BLOW BlowInfo;
						BlowInfo.emBlowIndex = emBlowIndex;

						if( nDurationTime != -1 )
							BlowInfo.fDurationTime = (float)nDurationTime * 0.001f;
						else
							BlowInfo.fDurationTime = 0.0f;

						BlowInfo.szValue = szParam;

						//#53448 Must update SkillUser to the final attacker when stacking.
						if (emBlowIndex == STATE_BLOW::BLOW_249)
							hExistBlow->SetParentSkillInfo(pParentSkill);

						hExistBlow->Duplicate( BlowInfo );

						hBlow = hExistBlow;
						bDuplicated = true;

						// Remove items that are in the Remove List.
						REMOVE_STATEBLOW_LIST::iterator removeInfoiter = m_RemoveStateBlowList.begin();
						REMOVE_STATEBLOW_LIST::iterator removeInfoEndIter = m_RemoveStateBlowList.end();

						for( REMOVE_STATEBLOW_LIST::iterator it = m_RemoveStateBlowList.begin(); it != m_RemoveStateBlowList.end(); it++ ) {
							if( it->m_Type == RemoveStateBlowInfo::RSBI_BLOW_ID && hExistBlow->GetBlowID() == it->m_Value ) {
								m_RemoveStateBlowList.erase( it );
								break;
							}
						}
					}
				}
			}
		}
	}
	
	if( false == bDuplicated )
	{	
		static CDnCreateBlow createBlow;
#ifdef PRE_MOD_MAGICALBREEZE_CHANGE_BLOW
		STATE_BLOW::emBLOW_INDEX emOriginalBlowIndex = emBlowIndex;
		emBlowIndex = CheckMagicalBreezeChangeBlow( pParentSkill, emBlowIndex, hActor );
		hBlow = createBlow.CreateBlow( emBlowIndex, hActor, szParam );
		if( !hBlow ) return hBlow;
		hBlow->SetOriginalBlowIndex( emOriginalBlowIndex );
#else // PRE_MOD_MAGICALBREEZE_CHANGE_BLOW
		hBlow = createBlow.CreateBlow( emBlowIndex, hActor, szParam );
		if( !hBlow ) return hBlow;
#endif // PRE_MOD_MAGICALBREEZE_CHANGE_BLOW

		if( nDurationTime == -1 ) 
			hBlow->SetPermanent( true );
		else 
			hBlow->SetDurationTime( nDurationTime * 0.001f );

#ifdef PRE_ADD_DECREASE_EFFECT
#ifdef _GAMESERVER
		if( emBlowIndex == STATE_BLOW::BLOW_247 && fReduceTimeValue != 1.0f )
		{
			CDnPuppetBlow* pDnPuppetBlow = dynamic_cast<CDnPuppetBlow*>( hBlow.GetPointer() );
			if( pDnPuppetBlow )
				pDnPuppetBlow->SetShowReduce( true );
		}
#endif // _GAMESERVER
#endif // PRE_ADD_DECREASE_EFFECT
	}

	return hBlow;
}

#ifdef PRE_MOD_MAGICALBREEZE_CHANGE_BLOW
STATE_BLOW::emBLOW_INDEX CDnStateBlow::CheckMagicalBreezeChangeBlow( const CDnSkill::SkillInfo* pParentSkill, STATE_BLOW::emBLOW_INDEX emBlowIndex, DnActorHandle hActor )
{
	STATE_BLOW::emBLOW_INDEX emRtnBlowIndex = emBlowIndex;
#ifdef _GAMESERVER
	if( hActor == NULL ) return emRtnBlowIndex;
	if( pParentSkill == NULL ) return emRtnBlowIndex;
	if( pParentSkill->eApplyType != CDnSkill::StateEffectApplyType::ApplySelf || pParentSkill->eSkillType != CDnSkill::SkillTypeEnum::Active ) return emRtnBlowIndex;
	if( pParentSkill->bIsItemSkill ) return emRtnBlowIndex;

	if( hActor->IsAppliedThisStateBlow( STATE_BLOW::BLOW_180 ) )
	{
		switch( emBlowIndex )
		{
		case STATE_BLOW::BLOW_001:	// Physical attack absolute value change -> Magic attack absolute value change
			emRtnBlowIndex = STATE_BLOW::BLOW_028;
			break;
		case STATE_BLOW::BLOW_002:	// Physical attack absolute value change -> Magic attack absolute value change
			emRtnBlowIndex = STATE_BLOW::BLOW_029;
			break;
		case STATE_BLOW::BLOW_200:	// Physical attack absolute value modification state effect. Calculated before skill state effects.
			emRtnBlowIndex = STATE_BLOW::BLOW_202;	// -> Magic attack flat value modifier state effect. Calculated before skill state effects.
			break;
		case STATE_BLOW::BLOW_201:	// Physical attack ratio modification state effect. Calculated before skill state effects.
			emRtnBlowIndex = STATE_BLOW::BLOW_203;	// -> Magic attack ratio modifier state effect. Calculated before skill state effects.
			break;
		}
	}
#endif // _GAMESERVER
	return emRtnBlowIndex;
}
#endif // PRE_MOD_MAGICALBREEZE_CHANGE_BLOW


int CDnStateBlow::GenerateStateBlowID()
{
#ifdef _GAMESERVER
	s_iStateBlowIDCount[ m_hActor->GetRoom()->GetRoomID() ]++;

	if( s_iStateBlowIDCount[ m_hActor->GetRoom()->GetRoomID() ] >= INT_MAX )
		s_iStateBlowIDCount[ m_hActor->GetRoom()->GetRoomID() ] = 0;

	return s_iStateBlowIDCount[ m_hActor->GetRoom()->GetRoomID() ];
#else
	s_iStateBlowIDCount++;

	if( s_iStateBlowIDCount >= INT_MAX )
		s_iStateBlowIDCount = 0;

	return s_iStateBlowIDCount;
#endif
}


void CDnStateBlow::OnChangedWeapon()
{
	DnBlowHandle hBlow;

	m_bLockStateBlowList = true;

	BLOW_HANDLE_LIST_ITER iter =  m_listBlowHandle.begin();
	for( ; iter != m_listBlowHandle.end(); ++iter )
	{
		hBlow = (*iter);

		if( hBlow && (hBlow->GetCallBackType()&SB_ONCHANGEDWEAPON) )
		{
			hBlow->OnChangedWeapon();
		}
	}

	m_bLockStateBlowList = false;
}

bool CDnStateBlow::OnDefenseAttack( DnActorHandle hHitter, CDnState* pAttackerState, CDnDamageBase::SHitParam &HitParam, bool bHitSuccess )
{
	DnBlowHandle hBlow;
	bool bRetDefense(bHitSuccess);

	m_bLockStateBlowList = true;
	BLOW_HANDLE_LIST_ITER iter = m_listBlowHandle.begin();
	for( ; iter != m_listBlowHandle.end(); ++iter )
	{
		hBlow = (*iter);

		if( hBlow && (hBlow->GetCallBackType()&SB_ONDEFENSEATTACK) )
		{
			if( hBlow->OnDefenseAttack( hHitter, pAttackerState, HitParam, bHitSuccess ) )
			{
				bRetDefense = false;
			}
		}
	}

	if( bRetDefense )
	{
		iter = m_listBlowHandle.begin();
		for( ; iter != m_listBlowHandle.end(); ++iter )
		{
			hBlow = (*iter);

			if( hBlow && (hBlow->GetCallBackType()&SB_AFTERONDEFENSEATTACK) )
			{
				if( hBlow->OnDefenseAttack( hHitter, pAttackerState, HitParam, bHitSuccess ) )
				{
					bRetDefense = false;
				}
			}
		}
	}

	m_bLockStateBlowList = false;

	return bRetDefense;
}

int CDnStateBlow::OnUseMP( int iMPDelta )
{
	int iResult = iMPDelta;
	DnBlowHandle hBlow;

	m_bLockStateBlowList = true;

	BLOW_HANDLE_LIST_ITER iter =  m_listBlowHandle.begin();

	int iDelta = 0;
	for( ; iter != m_listBlowHandle.end(); ++iter )
	{
		hBlow = (*iter);

		if( hBlow && (hBlow->GetCallBackType()&SB_ONUSEMP) )
			iDelta += hBlow->OnUseMP( iMPDelta );
	}

	m_bLockStateBlowList = false;

	iResult += iDelta;

	return iResult;
}


void CDnStateBlow::DelAllStateBlow()
{
	DnBlowHandle hBlow;

	BLOW_HANDLE_LIST_ITER iter =  m_listBlowHandle.begin();

	for( ; iter != m_listBlowHandle.end(); ++iter )
	{
		hBlow = (*iter);
		if( hBlow )
		{
			if( hBlow->IsDuration() )
			{
				if( !m_hActor )
					g_Log.Log(LogType::_ROOMCRASH, L"[DelAllStateBlow] Invalid ActorHandle\n" );
			}

			SAFE_RELEASE_SPTR( hBlow );
		}
	}

	m_listBlowHandle.clear();
}


DnBlowHandle CDnStateBlow::GetStateBlow( int iIndex )
{
	BLOW_HANDLE_LIST_ITER iter = m_listBlowHandle.begin();

	for( int i = 0; i < iIndex; ++i )
		++iter;

	return *iter;
}

float CDnStateBlow::OnCalcDamage( float fOriginalDamage, CDnDamageBase::SHitParam& HitParam )
{
	float fResult = 0.0f;
	DnBlowHandle hBlow;

	DnBlowHandle hHighLanderBlow;
	DnBlowHandle hProbInvincibleAtBlow;

	float fStateBlowResult = 0.0f;
	float fHighLanderResult = 0.0f;
	float fProbInvincibleAtBlow = 0.0f;

	m_bLockStateBlowList = true;

	BLOW_HANDLE_LIST_ITER iter =  m_listBlowHandle.begin();
	for( ; iter != m_listBlowHandle.end(); ++iter )
	{
		hBlow = (*iter);
		if( hBlow && (hBlow->GetCallBackType()&SB_ONCALCDAMAGE) )
		{
			fStateBlowResult = hBlow->OnCalcDamage( fOriginalDamage, HitParam );

			if( STATE_BLOW::BLOW_143 == hBlow->GetBlowIndex() )
			{
				hHighLanderBlow = hBlow;
				fHighLanderResult = fStateBlowResult;
			}
			else
			if( STATE_BLOW::BLOW_226 == hBlow->GetBlowIndex() )
			{
				hProbInvincibleAtBlow = hBlow;
				fProbInvincibleAtBlow = fStateBlowResult;
			}

			fResult += fStateBlowResult;
		}
	}

	m_bLockStateBlowList = false;

	// If Highlander or specific skill probability ignore state effect is active, that result takes priority. (No damage)
	if( hHighLanderBlow )
		fResult = fHighLanderResult;
	else 	
	if( hProbInvincibleAtBlow && fProbInvincibleAtBlow != 0.0f)	//Only when a damage correction value exists.
		fResult = fProbInvincibleAtBlow;
	
	return fResult;
}


bool CDnStateBlow::OnDie( DnActorHandle hHitter )
{
	bool bProcessed = false;

	// Currently only the skill-on-death state effect exists, so handle it directly without looping.
	if( IsApplied( STATE_BLOW::BLOW_137 ) )
	{
		BLOW_HANDLE_LIST_ITER iter =  m_listBlowHandle.begin();
		for( ; iter != m_listBlowHandle.end(); ++iter )
		{
			DnBlowHandle hBlow = (*iter);
			if( hBlow && hBlow->GetBlowIndex() == STATE_BLOW::BLOW_137 )
			{
				static_cast<CDnUsingSkillWhenDieBlow*>(hBlow.GetPointer())->OnDie();

#if defined(PRE_FIX_44884)
				//For actors with UsingSkillWhenDieBlow set, HP is set to 1 on OnDie call, deferring the OnDie call timing to state effect processing.
				//Store it here because the actual Hitter cannot be determined.
				static_cast<CDnUsingSkillWhenDieBlow*>(hBlow.GetPointer())->SetFinalHitterActor(hHitter);
#endif // PRE_FIX_44884

				bProcessed = true;
			}
		}
	}

	return bProcessed;
}

void CDnStateBlow::OnTargetHit( DnActorHandle hTargetActor )
{
	DnBlowHandle hBlow;

	m_bLockStateBlowList = true;

	BLOW_HANDLE_LIST_ITER iter =  m_listBlowHandle.begin();
	for( ; iter != m_listBlowHandle.end(); ++iter )
	{
		hBlow = (*iter);
		if( hBlow && (hBlow->GetCallBackType()&SB_ONTARGETHIT) )
		{
			hBlow->OnTargetHit( hTargetActor );
		}
	}

	m_bLockStateBlowList = false;
}


void CDnStateBlow::ResetStateBlowBySkillType( int nSkillDurationType )
{
	AddRemoveStateBlowInfo(RemoveStateBlowInfo(RemoveStateBlowInfo::RSBI_SKILL_DURATION_TYPE, nSkillDurationType));
}

#ifndef _GAMESERVER
void CDnStateBlow::RestoreAllBlowGraphicEffect()
{
	BLOW_HANDLE_LIST_ITER iter = m_listBlowHandle.begin();
	
	for( ; iter != m_listBlowHandle.end(); ++iter) {
		DnBlowHandle hBlow = *iter;
		if( !hBlow ) continue;
		if( hBlow->IsUseTableDefinedGraphicEffect() ) {
			hBlow->AttachGraphicEffectDefaultType();
		}
	}
}

EffectOutputInfo *CDnStateBlow::GetEffectOutputInfo( int nItemID )
{
	std::map<int, EffectOutputInfo*>::iterator it = s_nMapEffectOutputInfo.find( nItemID );
	if( it == s_nMapEffectOutputInfo.end() ) return NULL;
	return it->second;
}

#ifndef _GAMESERVER
void CDnStateBlow::RemoveStateBlowFromServerID( int nServerBlowID )
{
	AddRemoveStateBlowInfo(RemoveStateBlowInfo(RemoveStateBlowInfo::RSBI_BLOW_SERVER_ID, nServerBlowID));
}
#endif

#if !defined(SW_MODIFY_SE_EFFECTOUTPUT_20091119_jhk8211)
void CDnStateBlow::AddEffectAttachedBone( int iBone, DnEtcHandle hEffectHandle )
{
	// Currently does not alternate overlapping output except bone 0. Just shows all. #16291
	if( CDnActor::FXDummyBoneEnum::FX_01 != iBone )
		return;

	// Loop and cycle overlapping bones periodically when displaying.
	_ASSERT( 0 <= iBone && iBone < CDnActor::Max_FX_Dummy_Bone );
	if( 0 <= iBone && iBone < CDnActor::Max_FX_Dummy_Bone )
	{
		S_BONE_EFFECT BoneEffect;
		BoneEffect.hEffect = hEffectHandle;

		// If added for the first time, handle show.
		// show( false ) stops working once rendering has started.
		if( m_aListBoneEffectStatus[ iBone ].empty() )
			BoneEffect.Show( true );
		else
			BoneEffect.Show( false );

		m_aListBoneEffectStatus[ iBone ].push_back( BoneEffect );
	}
}

void CDnStateBlow::DelEffectAttachedBone( int iBone, DnEtcHandle hEffectHandle )
{
	// Currently does not alternate overlapping output except bone 0. Just shows all. #16291
	if( CDnActor::FXDummyBoneEnum::FX_01 != iBone )
		return;

	_ASSERT( 0 <= iBone && iBone < CDnActor::Max_FX_Dummy_Bone );
	if( 0 <= iBone && iBone < CDnActor::Max_FX_Dummy_Bone )
	{
		list<S_BONE_EFFECT>& listBoneEffect = m_aListBoneEffectStatus[ iBone ];
		list<S_BONE_EFFECT>::iterator iter = find( listBoneEffect.begin(), 
												   listBoneEffect.end(), hEffectHandle );

		if( listBoneEffect.end() != iter )
		{
			// If this is an overlapped effect bone slot and the effect being deleted is currently active.. 
			// Immediately pass the turn to the next bone.
			// If the effect object was already deleted somewhere, just remove it from the bone effect status list.
			if( 1 < (int)listBoneEffect.size() )
			{
				if( iter->bShow )
				{
					list<S_BONE_EFFECT>::iterator iterNext = iter;
					iterNext++;
					if( listBoneEffect.end() == iterNext )
						iterNext = listBoneEffect.begin();

					iterNext->Show( true );
				}
			}

			m_aListBoneEffectStatus[ iBone ].erase( iter );
		}
	}
}
#endif

#endif

void CDnStateBlow::OnCmdActionFromPacket( const char* pActionName )
{
	BLOW_HANDLE_LIST_ITER iter = m_listBlowHandle.begin();
	BLOW_HANDLE_LIST_ITER iterEnd = m_listBlowHandle.end();

	for( iter; iter != iterEnd; ++iter )
	{
		if( *iter )
			(*iter)->OnCmdActionFromPacket( pActionName );
	}
}


#ifdef _GAMESERVER
CDnSkill::CanApply CDnStateBlow::CanApplySkillStateEffect(const CDnSkill::SkillInfo *pUsingSkillInfo, const CDnSkill::StateEffectStruct &newStateEffect)
{
	// Initialize reset list when checking state effects [2010/12/09 semozz]
	if (m_hActor)
		m_hActor->InitStateBlowIDToRemove();

	CDnSkill::CanApply eResult = CDnSkill::CanApply::Apply;

	// 1. Add the new state if it has a duration.
	if ((CDnSkill::IsNeedCheckApplyStateBlow( (STATE_BLOW::emBLOW_INDEX)newStateEffect.nID)) ||
		(newStateEffect.nDurationTime > 0) )
	{
		// [2010/11/12 semozz]
		// 2. In case of StateBlow with duration 
		// For now, use the same code as existing skill usability check..
		// This part needs review
		STATE_BLOW::emBLOW_INDEX newBlowIndex = (STATE_BLOW::emBLOW_INDEX)newStateEffect.nID;
		if( IsApplied( newBlowIndex ) )
		{
			DNVector(DnBlowHandle) vlhResult;
			GetStateBlowFromBlowIndex( newBlowIndex, vlhResult );
			_ASSERT( !vlhResult.empty() );

			vector<int> vlStateBlowIDToRemove;
			DNVector( DnBlowHandle ) vlhSameSkillBlows;

			bool bExistingSameSkill = false;
			int iExistingSameSkillCount = 0;

			int iNumBlow = (int)vlhResult.size();
			for( int iBlow = 0; iBlow < iNumBlow; ++iBlow )
			{
				DnBlowHandle hExistingBlow = vlhResult.at( iBlow );
				const CDnSkill::SkillInfo* pExistingParentSkillInfo = hExistingBlow ? hExistingBlow->GetParentSkillInfo() : NULL;

				// State effects without parent skill info were not applied by a skill, so they are not handled here
				if ( NULL == pExistingParentSkillInfo )
					continue;

				// Check skill duplicate count
				// If the same state effect appears again for the same skill ID, it is a duplicate skill.
				bool bIsSameSkillID = false;
#if defined(PRE_FIX_58505)
				//If suffix skill type is not set, compare by skill ID,
				//If suffix skill type is set, compare whether suffix skill types match.
				if ( pUsingSkillInfo->bItemPrefixSkill == true && pUsingSkillInfo->nPrefixSkillType != CDnPrefixSkill::Prefix_Non )
					bIsSameSkillID = pUsingSkillInfo->nPrefixSkillType == pExistingParentSkillInfo->nPrefixSkillType;
				else
					bIsSameSkillID = pUsingSkillInfo->iSkillID == pExistingParentSkillInfo->iSkillID;
#else
				if( pUsingSkillInfo->iSkillID == pExistingParentSkillInfo->iSkillID )
					bIsSameSkillID = true;
#endif // PRE_FIX_58505

				if (bIsSameSkillID == true)
				{
					// If the same skill ID appears for the first time, increment the active skill count by 1.
					iExistingSameSkillCount += hExistingBlow->GetDuplicateCount();
					bExistingSameSkill = true;

					vlhSameSkillBlows.push_back( hExistingBlow );
				}

				if (pUsingSkillInfo->eTargetType == CDnSkill::TargetTypeEnum::All  //TargetType of the currently used skill is All, and 
					&& (0 != pUsingSkillInfo->iSkillDuplicateMethod && (pUsingSkillInfo->iSkillDuplicateMethod == pExistingParentSkillInfo->iSkillDuplicateMethod)) //When overlap process ID is set and it matches the existing skill ID,
					&& (m_hActor == pUsingSkillInfo->hSkillUser) //Self
					)
					continue;

				// If the persistent effect category index is not 0, duplicate skill effect handling is required.
				// If the persistent effect category index is 0, it simply stacks.
				// Or if it's the same skill and max stack count is 1 or less, replace the skill.
#if defined(PRE_FIX_58505)
				if( 0 != pUsingSkillInfo->iSkillDuplicateMethod ||
					(bIsSameSkillID == true && pUsingSkillInfo->iDuplicateCount <= 1) )
#else
				if( 0 != pUsingSkillInfo->iSkillDuplicateMethod ||
					(pUsingSkillInfo->iSkillID == pExistingParentSkillInfo->iSkillID && pUsingSkillInfo->iDuplicateCount <= 1) )
#endif // PRE_FIX_58505
				{
					// Check if the state effect index matches the currently running skill; if so, check whether stacking is allowed.
					// Also stack only when the skill caster's team matches.
					// When an enemy team uses the same skill, my debuff state effect gets removed. (#19812)
					if( (pUsingSkillInfo->iSkillDuplicateMethod == pExistingParentSkillInfo->iSkillDuplicateMethod) &&
						(pUsingSkillInfo->iSkillUserTeam == pExistingParentSkillInfo->iSkillUserTeam) )
					{
						if( pUsingSkillInfo->iLevel >= pExistingParentSkillInfo->iLevel )
						{
							// State effect 242 must not be reset once added.

							// If it passes the probability check, add to the state effects to reset.
							// The object that actually checks probability uses the already-applied state effect object, since it only checks probability
							// Use as is.
							if( hExistingBlow->GetBlowIndex() != STATE_BLOW::BLOW_242 && 
								hExistingBlow->CanBegin() )
							{
								vlStateBlowIDToRemove.push_back( hExistingBlow->GetBlowID() );
								//mapDuplicateResult[ hExistingBlow->GetBlowIndex() ] = true;

								// Store states to be reset. [2010/12/08 semozz]
								// When applying states in this list at OnSignal's STE_ApplyStateEffect time
								// Remove existing state.
								m_hActor->AddStateBlowIDToRemove(hExistingBlow->GetBlowID());
							}
							else
							{
								eResult = CDnSkill::CanApply::Fail;		// Do not add because the probability check failed during effect refresh.
							}
						}
						else if( pUsingSkillInfo->iLevel < pExistingParentSkillInfo->iLevel )
						{
							// Effect not applied.
							eResult = CDnSkill::CanApply::Fail;

							// Note Hanki: If something needs to be displayed graphically later, handle it here.
						}
					}
				}

			}

			if( false == vlStateBlowIDToRemove.empty() )
			{
				// Replace skill effect. No need to handle stack count tracking.
				bExistingSameSkill = false;
				eResult = CDnSkill::CanApply::ApplyDuplicateSameSkill;
			}

			// When stacking state effects, must not exceed the maximum stack count.
			// Do not stack if a higher-level state effect is already applied.
			if( CDnSkill::CanApply::Fail != eResult )
			{
				if( bExistingSameSkill )
				{
					if( pUsingSkillInfo->iDuplicateCount <= iExistingSameSkillCount )
					{
						// If exceeding max stack count, only reset the duration of the existing state effect.
						int iNumSameSkillBlowsToResetDurationTime = (int)vlhSameSkillBlows.size();
						for( int iBlow = 0; iBlow < iNumSameSkillBlowsToResetDurationTime; ++iBlow )
						{
							DnBlowHandle hBlow = vlhSameSkillBlows.at( iBlow );

							//#53448 State effect 249 cannot be added beyond its max stack count.
							//If max stack count is exceeded, no need to loop below to remove existing state effect and re-add.
							if (hBlow && hBlow->GetBlowIndex() == STATE_BLOW::BLOW_249)
								continue;

							// Deleting completely and re-adding is clearer.
							int nDurationTime = int(hBlow->GetDurationTime() * 1000.0f);
							hBlow->ResetDurationTime();

							CDnSkill::SkillInfo SkillInfo = *(hBlow->GetParentSkillInfo());
							STATE_BLOW::emBLOW_INDEX BlowIndex = hBlow->GetBlowIndex();
							string strValue( hBlow->GetValue() );
							
							// Must be deleted immediately at this point. [2011/01/18 semozz]
							int nBlowID = hBlow->GetBlowID();
							// Send packets to be sent
							m_hActor->SendRemoveStateEffectFromID(nBlowID);
							// Delete immediately.
							if (m_hActor->GetStateBlow())
								m_hActor->GetStateBlow()->RemoveImediatlyStateEffectFromID(nBlowID);

							// Since probabilistic state effects like freeze must succeed 100% here, 
							// Call with bCheckCanBegin flag off so CanBegin is not called.
							m_hActor->CmdAddStateEffect( &SkillInfo, BlowIndex, nDurationTime, strValue.c_str(), false, false );
						}

						eResult = CDnSkill::CanApply::Fail;
					}
				}
			}
		}	
	}

	return eResult;
}
#endif

bool RemoveStateBlowInfo::Compare(DnBlowHandle hBlow)
{
	bool result = false;
	
	if (!hBlow)
		return result;

	switch(m_Type)
	{
	case RSBI_BLOW_INDEX:
		{
			if( hBlow->IsEternity() == false && hBlow->IsFromSourceItem() == false )
				result = (hBlow->GetBlowIndex() == m_Value);
		}
		break;
	case RSBI_BLOW_ID:
		{
			result = (hBlow->GetBlowID() == m_Value);
		}
		break;
	case RSBI_SKILL_DURATION_TYPE:
		{
			const CDnSkill::SkillInfo* pSkillInfo = hBlow->GetParentSkillInfo();
			result = ( pSkillInfo && (CDnSkill::DurationTypeEnum)m_Value == pSkillInfo->eDurationType );
		}
		break;
#if !defined(_GAMESERVER)
	case RSBI_BLOW_SERVER_ID:
		{
			result = (hBlow->GetServerBlowID() == m_Value);
		}
		break;
#endif
	}

	return result;
}


void CDnStateBlow::RemoveStateBlowReservedList()
{
	REMOVE_STATEBLOW_LIST::iterator removeInfoiter = m_RemoveStateBlowList.begin();
	
#ifndef _FINAL_BUILD
	_CrashIfRemoveListLocked();
#endif // #ifdef _FINAL_BUILD
	m_bLockRemoveReservedList = true;
	for (; removeInfoiter != m_RemoveStateBlowList.end(); ++removeInfoiter)
	{
		RemoveStateBlowInfo &removeInfo = (*removeInfoiter);

		DnBlowHandle hBlow;

#ifndef _FINAL_BUILD
		_CrashIfProcessListLocked();
#endif // #ifdef _FINAL_BUILD
		m_bLockStateBlowList = true;
		BLOW_HANDLE_LIST_ITER blowListiter =  m_listBlowHandle.begin();
		for( ; blowListiter != m_listBlowHandle.end();  )
		{
			hBlow = (*blowListiter);

			if (removeInfo.Compare(hBlow))
			{

				//ResetStateBlowBySkillType has code that sends a packet to the client.
				if (RemoveStateBlowInfo::RSBI_SKILL_DURATION_TYPE == removeInfo.m_Type)
				{
#if defined(_GAMESERVER)
					STATE_BLOW::emBLOW_INDEX emBlowIndex = hBlow->GetStateBlow().emBlowIndex;
					// Also send packet to client to delete
					m_hActor->SendRemoveStateEffect(emBlowIndex);
#endif // _GAMESERVER
				}

				_ASSERT( m_aiApplied[ hBlow->GetBlowIndex() ] > 0 );
				--m_aiApplied[ hBlow->GetBlowIndex() ];

				hBlow->OnEnd( 0, 0 );
				m_hActor->OnEndStateBlow( hBlow );

				SAFE_RELEASE_SPTR( hBlow );
				blowListiter = m_listBlowHandle.erase(blowListiter);

				//Find and delete all BlowIndex and DuratioType.
				if (RemoveStateBlowInfo::RSBI_BLOW_INDEX == removeInfo.m_Type ||
					RemoveStateBlowInfo::RSBI_SKILL_DURATION_TYPE == removeInfo.m_Type)
					continue;
				else
					break;
			}

			++blowListiter;
		}
		m_bLockStateBlowList = false;
	}

	m_RemoveStateBlowList.clear();
	m_bLockRemoveReservedList = false;
}

void CDnStateBlow::AddRemoveStateBlowInfo(RemoveStateBlowInfo info)
{
	//Registration for state effect removal is possible even while looping.
	m_RemoveStateBlowList.push_back(info);
}

void CDnStateBlow::RemoveImediatlyStateEffectFromID(int nStateBlowID)
{
#ifndef _FINAL_BUILD
	_CrashIfProcessListLocked();
#endif // #ifdef _FINAL_BUILD

	DnBlowHandle hBlow;

	BLOW_HANDLE_LIST_ITER iter =  m_listBlowHandle.begin();

	for( ; iter != m_listBlowHandle.end(); ++iter )
	{
		hBlow = (*iter);

		if( hBlow && (hBlow->GetBlowID() == nStateBlowID) )
		{
			_ASSERT( m_aiApplied[ hBlow->GetBlowIndex() ] > 0 );
			--m_aiApplied[ hBlow->GetBlowIndex() ];

			hBlow->OnEnd( 0, 0 );
			m_hActor->OnEndStateBlow( hBlow );

			OutputDebug("%s EndBlowID: %d Index: %d", __FUNCTION__, hBlow->GetBlowID(), hBlow->GetBlowIndex());

			SAFE_RELEASE_SPTR( hBlow );
			m_listBlowHandle.erase(iter);
			break;
		}
	}
}

void CDnStateBlow::RemoveImediatlyStateEffectByBlowIndex(STATE_BLOW::emBLOW_INDEX emBlowIndex)
{
#ifndef _FINAL_BUILD
	_CrashIfProcessListLocked();
#endif // #ifdef _FINAL_BUILD

	int nIndex = -1;
	DnBlowHandle hBlow;

	BLOW_HANDLE_LIST_ITER iter =  m_listBlowHandle.begin();

	for( ; iter != m_listBlowHandle.end(); )
	{
		hBlow = (*iter);

		if( hBlow && ( ( hBlow->IsEternity() == false && hBlow->IsFromSourceItem() == false ) && hBlow->GetBlowIndex() == emBlowIndex ) )
		{
			_ASSERT( m_aiApplied[ hBlow->GetBlowIndex() ] > 0 );
			--m_aiApplied[ hBlow->GetBlowIndex() ];

			hBlow->OnEnd( 0, 0 );
			m_hActor->OnEndStateBlow( hBlow );

			nIndex = hBlow->GetBlowID();
			SAFE_RELEASE_SPTR( hBlow );
			iter = m_listBlowHandle.erase(iter);

			continue;
		}

		++iter;
	}
}

#if !defined(_GAMESERVER)
void CDnStateBlow::RemoveImediatlyStateEffectByServerID(int nServerBlowID)
{
#ifndef _FINAL_BUILD
	_CrashIfProcessListLocked();
#endif // #ifdef _FINAL_BUILD

	DnBlowHandle hBlow;

	BLOW_HANDLE_LIST_ITER iter =  m_listBlowHandle.begin();

	for( ; iter != m_listBlowHandle.end(); ++iter )
	{
		hBlow = (*iter);

		if( hBlow && (hBlow->GetServerBlowID() == nServerBlowID) )
		{
			_ASSERT( m_aiApplied[ hBlow->GetBlowIndex() ] > 0 );
			--m_aiApplied[ hBlow->GetBlowIndex() ];

			hBlow->OnEnd( 0, 0 );
			m_hActor->OnEndStateBlow( hBlow );

			SAFE_RELEASE_SPTR( hBlow );
			m_listBlowHandle.erase(iter);
			break;
		}
	}
}
#endif // _GAMESERVER


bool CDnStateBlow::IsImmuned(STATE_BLOW::emBLOW_INDEX blowIndex)
{
	// Exclude all state effects except buff state effects when invincible state effect is active.
	if( IsApplied( STATE_BLOW::BLOW_099 ) )
	{
		return false;
	}

	// Check if currently immune to this state effect
	// During resurrection (invincible) state effect, immune to damage-dealing state effects.
	list<DnBlowHandle>::iterator iter = m_listBlowHandle.begin();
	for( iter; iter != m_listBlowHandle.end(); )
	{
		DnBlowHandle hNowBlow = (*iter);
		if( hNowBlow )
		{
			if( (hNowBlow->GetBlowIndex() == STATE_BLOW::BLOW_077) )
			{
				bool bImmuned = static_cast<CDnImmuneBlow*>(hNowBlow.GetPointer())->IsImmuned( blowIndex );
				if (bImmuned)
					return true;
			}
			else
			if( (hNowBlow->GetBlowIndex() == STATE_BLOW::BLOW_150) )
			{
				bool bImmuned = static_cast<CDnAllImmuneBlow*>(hNowBlow.GetPointer())->IsImmuned( blowIndex );
				if (bImmuned)
					return true;
			}
		}

		++iter;
	}

	return false;
}

#ifndef _FINAL_BUILD
void CDnStateBlow::_CrashIfProcessListLocked( void )
{
	if( m_bLockStateBlowList )
	{
		_ASSERT( 0 );
		int* pNull = NULL;
		*pNull = 0xffffeeee;
	}
}

void CDnStateBlow::_CrashIfRemoveListLocked( void )
{
	if( m_bLockRemoveReservedList )
	{
		_ASSERT( 0 );
		int* pNull = NULL;
		*pNull = 0xeeeeffff;
	}
}
#endif // #ifdef _FINAL_BUILD


void CDnStateBlow::Process_StateBlow_Old(LOCAL_TIME LocalTime, float fDelta, bool bForceInitialize)
{
	BLOW_HANDLE_LIST::iterator iter;
	DnBlowHandle hBlow;

	for( iter = m_listBlowHandle.begin(); iter != m_listBlowHandle.end(); )
	{
		hBlow = (*iter);

		if( hBlow )
		{
			// For verifying game server dump fix.
			if( !(hBlow->GetActorHandle()) )
				hBlow->SetActorHandle( m_hActor );
			//////////////////////////////////////////////////////////////////////////

			if( hBlow->IsBegin() )
			{
				// Since there are state effects like team inversion, when the team is inverted, the charm state effect 
				// Since it becomes the same team as the mob and would not hit, process the effect first. #14128
#ifndef _GAMESERVER
				if( hBlow->IsUseTableDefinedGraphicEffect() )
					hBlow->AttachGraphicEffectDefaultType();
#else
				hBlow->CheckAndStartActorActionInEffectInfo();
#endif
				hBlow->OnBegin( LocalTime, fDelta );
				m_hActor->OnBeginStateBlow( hBlow );

				// Check because some blows set state to end immediately.
				if( STATE_BLOW::STATE_END != hBlow->GetBlowState() )
					hBlow->SetState( STATE_BLOW::STATE_DURATION );
			}
			else if( hBlow->IsDuration() )
			{
				hBlow->Process( LocalTime, fDelta );
			}
			else if( hBlow->IsEnd() )
			{
				RemoveStateBlowFromID(hBlow->GetBlowID());
			}
		}

		// After each blow and process, if the target dies from poison/burn interval damage, the passive skill immediately reattaches 
		// When dead, process only the rebirth state effect.
#ifdef _GAMESERVER
		// On game server, even when actor is gm trace it enters the resurrection routine below, so add one more condition.
		if( !m_hActor->IsGMTrace() && !bForceInitialize && m_hActor->IsPlayerActor() && m_hActor->IsDie() )
#else
		if( !bForceInitialize && m_hActor->IsPlayerActor() && m_hActor->IsDie() )
#endif
		{
			// Exit loop if there is no resurrection state effect
			bool bExistRebirth = false;
			for( iter = m_listBlowHandle.begin(); m_listBlowHandle.end() != iter; ++iter )
			{
				if( !(*iter) )
					continue;

				if( (*iter)->GetBlowIndex() == STATE_BLOW::BLOW_057 || (*iter)->GetBlowIndex() == STATE_BLOW::BLOW_230 )
				{
					bExistRebirth = true;
					break;
				}
			}
			if( false == bExistRebirth )
				break;
		}
		else
			++iter;
	}
}

void CDnStateBlow::Process_StateBlow_New(LOCAL_TIME LocalTime, float fDelta, bool bForceInitialize)
{
	//Dead state, whether Rebirth state effect exists
	bool bIsDie = false;
	bool bExistRebirth = false;
	BLOW_HANDLE_LIST::iterator rebirthStateIter = m_listBlowHandle.end();
	BLOW_HANDLE_LIST::iterator spectatorStateIter = m_listBlowHandle.end();

#ifdef _GAMESERVER
	// On game server, even when actor is gm trace it enters the resurrection routine below, so add one more condition.
	if( !m_hActor->IsGMTrace() && !bForceInitialize && m_hActor->IsPlayerActor() && m_hActor->IsDie() )
#else
	if( !bForceInitialize && m_hActor->IsPlayerActor() && m_hActor->IsDie() )
#endif
	{
		bIsDie = true;
		for( BLOW_HANDLE_LIST::iterator iter = m_listBlowHandle.begin(); m_listBlowHandle.end() != iter; ++iter )
		{
			if( !(*iter) )
				continue;

			if( (*iter)->GetBlowIndex() == STATE_BLOW::BLOW_057 )
			{
				bExistRebirth = true;
				rebirthStateIter = iter;
			}

			if( (*iter)->GetBlowIndex() == STATE_BLOW::BLOW_230 )
			{
				bExistRebirth = true;
				spectatorStateIter = iter;
			}
		}
	}

	//If in dead state,
	if (bIsDie)
	{
		// If Rebirth state effect exists while dead, process only Rebirth state effect
		if (rebirthStateIter != m_listBlowHandle.end())
		{
			DnBlowHandle hRebirthBlow = (*rebirthStateIter);
			Process_BlowHandle(hRebirthBlow, LocalTime, fDelta);
		}
		if (spectatorStateIter != m_listBlowHandle.end())
		{
			DnBlowHandle hSpectatorBlow = (*spectatorStateIter);
			Process_BlowHandle(hSpectatorBlow, LocalTime, fDelta);
		}
		//Return here so other state effects are not processed when dead...
		return;
	}


	DnBlowHandle hBlow;
	for( BLOW_HANDLE_LIST::iterator iter = m_listBlowHandle.begin(); iter != m_listBlowHandle.end(); )
	{
		hBlow = (*iter);

		//Immediately remove invalid state effects from the list.
		if (!hBlow)
		{
			iter = m_listBlowHandle.erase(iter);
			continue;
		}

		Process_BlowHandle(hBlow, LocalTime, fDelta);
		
		++iter;
	}
}

void CDnStateBlow::Process_BlowHandle(DnBlowHandle hBlow, LOCAL_TIME LocalTime, float fDelta)
{
	if (!hBlow)
		return;

	if( hBlow->IsBegin() )
	{
		// Since there are state effects like team inversion, when the team is inverted, the charm state effect 
		// Since it becomes the same team as the mob and would not hit, process the effect first. #14128
#ifndef _GAMESERVER
		if( hBlow->IsUseTableDefinedGraphicEffect() )
			hBlow->AttachGraphicEffectDefaultType();
#else
		hBlow->CheckAndStartActorActionInEffectInfo();
#endif
		hBlow->OnBegin( LocalTime, fDelta );
		m_hActor->OnBeginStateBlow( hBlow );

		// Check because some blows set state to end immediately.
		if( STATE_BLOW::STATE_END != hBlow->GetBlowState() )
			hBlow->SetState( STATE_BLOW::STATE_DURATION );
	}
	else if( hBlow->IsDuration() )
	{
		hBlow->Process( LocalTime, fDelta );
	}
	else if( hBlow->IsEnd() )
	{
		RemoveStateBlowFromID(hBlow->GetBlowID());
	}
}


float CDnStateBlow::GetImmuneReduceTimeValue(STATE_BLOW::emBLOW_INDEX blowIndex)
{
	float fReduceTimeValue = 1.0f;

	//Iterate through immunity state effects to get the duration ratio modifier from the state effect immune to blowIndex.
	DNVector(DnBlowHandle) vlhResult;
	GetStateBlowFromBlowIndex( STATE_BLOW::BLOW_077, vlhResult );

	int nListCount = (int)vlhResult.size();
	for (int i = 0; i < nListCount; ++i)
	{
		DnBlowHandle hExistBlow = vlhResult.at( i );
		int nBlowIndex = (int)hExistBlow->GetFloatValue();
		if ((STATE_BLOW::emBLOW_INDEX)(nBlowIndex) == blowIndex)
		{
			//Get the smallest value among blowIndex duration ratio modification values..
			float fValue = static_cast<CDnImmuneBlow*>(hExistBlow.GetPointer())->GetReduceTimeValue();
			if (fReduceTimeValue > fValue)
				fReduceTimeValue = fValue;
		}
	}

	return fReduceTimeValue;
}


#ifdef _GAMESERVER
#ifdef PRE_ADD_PROJECTILE_SE_INFO
void CDnStateBlow::MakeCloneStateBlowList( BLOW_HANDLE_LIST BlowList )
{
	if( BlowList.empty() )
		return;
	static CDnCreateBlow createBlow;
	BLOW_HANDLE_LIST_ITER iter = BlowList.begin();
	for( iter; iter != BlowList.end() ; ++iter )
	{
		DnBlowHandle hBlow = (*iter);
		if( hBlow )
		{
			if( hBlow->GetBlowIndex() > STATE_BLOW::BLOW_NONE && hBlow->GetBlowIndex() < STATE_BLOW::BLOW_MAX )
			{
				DnBlowHandle hBlow = createBlow.CreateBlow( (*iter)->GetBlowIndex() , (*iter)->GetActorHandle(), (*iter)->GetValue() );
				m_listBlowHandle.push_back( hBlow );
				++m_aiApplied[ hBlow->GetBlowIndex() ];
			}
		}
	}
}
#endif
#endif

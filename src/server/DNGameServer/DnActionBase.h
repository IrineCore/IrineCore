#pragma once

#include "EtActionBase.h"
#include "Timer.h"
#include "SignalHeader.h"
#include "EtActionSignal.h"
#include "DnActionSpecificInfo.h"

class CDnRenderBase;
class CDnActionBase : public CEtActionBase 
{
public:
	CDnActionBase();
	virtual ~CDnActionBase();

protected:
	CDnRenderBase *m_pRender;
	std::vector<int> m_nVecAniIndexList;

	std::string m_szAction;
	LOCAL_TIME m_LocalTime;
	LOCAL_TIME m_ActionTime;
	float m_fFrame;
	float m_fPrevFrame;
	int m_nActionIndex;
	int m_nPrevActionIndex;
	std::string m_szActionQueue;
	float m_fQueueBlendFrame;
	float m_fQueueStartFrame;

	std::string m_szCustomAction;
	LOCAL_TIME m_CustomActionTime;
	int m_nCustomActionIndex;
	float m_fCustomPrevFrame;
	bool m_bCustomProcessSignal;

	int m_nLoopCount;
	float m_fFPS;

	// Set the number of projectiles that can be fired per action index.
	CDnActionSpecificInfo::S_PROJECTILE_COUNT_INFO m_ProjectileCountInfoForInit;
	const CDnActionSpecificInfo::S_PROJECTILE_COUNT_INFO* m_pProjectileCountInfo;

	// To prevent skill spam hacks in China that tamper with the skill chain flag, when the skill chain flag
	// Filter existing STE_Input signals and collect their action indices.
	CDnActionSpecificInfo::S_SKILLCHAIN_INPUT_INFO m_SkillChainInfoForInit;
	const CDnActionSpecificInfo::S_SKILLCHAIN_INPUT_INFO* m_pSetSkillChainAction;

	// Info to prevent hacks using passive skill packets
	CDnActionSpecificInfo::S_INPUTHASPASSIVESKILL_INFO m_PassiveSkillInfoForInit;
	const CDnActionSpecificInfo::S_INPUTHASPASSIVESKILL_INFO* m_pPassiveSkillInfo;

	CDnActionSpecificInfo::S_BASIC_ATTACK_INFO m_BasicAttackInfoForInit;
	const CDnActionSpecificInfo::S_BASIC_ATTACK_INFO* m_pBasicAttackInfo;

protected:
	void ProcessSignal( ActionElementStruct *pStruct, float fFrame, float fPrevFrame );
	void ResetActionBase();

	void InsertBasicAttackInfo( int nElementIndex, CEtActionSignal *pSignal );
	void InsertSkillChainInfo( ActionElementStruct *pElement, int nElementIndex, CEtActionSignal *pSignal );
	void InsertBasicShootActionCoolTime( ActionElementStruct *pElement, int nElementIndex, CEtActionSignal *pSignal );
	void InsertStandChangeSEShootSkillCoolTime( ActionElementStruct *pElement, int nElementIndex, CEtActionSignal *pSignal );

	void InsertPassiveSkillInfo( int nElementIndex, CEtActionSignal *pSignal );
	void InsertProjectileCountInfo( int nElementIndex, CEtActionSignal *pSignal );
	void InsertSendActionProjectileCountInfo( int nElementIndex, CEtActionSignal *pSignal );

public:
	// Hanki: Moved from protected to public to immediately cancel the ongoing action on an immobilization state effect.
	virtual void SetAction( const char *szActionName, float fFrame, float fBlendFrame, bool bLoop = false );

	bool Initialize( CDnRenderBase *pRender );
	virtual bool LoadAction( const char *szFullPathName );
	virtual void FreeAction();
	virtual void OnSignal( SignalTypeEnum Type, void *pPtr, LOCAL_TIME LocalTime, LOCAL_TIME SignalStartTime, LOCAL_TIME SignalEndTime, int nSignalIndex ) {}
	static void CheckPreSignal( ActionElementStruct *pElement, int nElementIndex, CEtActionSignal *pSignal, int nSignalIndex, CEtActionBase *pActionBase );
	static void CheckPostSignal( ActionElementStruct *pElement, int nElementIndex, CEtActionSignal *pSignal, int nSignalIndex, CEtActionBase *pActionBase );
	virtual bool IsIgnoreSignal( int nSignalIndex );


	const char *GetCurrentAction();
	int GetCurrentActionIndex( void ) { return m_nActionIndex; };
	inline std::string& GetCurrentPlayAction() { return m_szAction; }

	float GetCurFrame() { return m_fFrame; }
	void CacheAniIndex();
	LOCAL_TIME GetLocalTime() { return m_LocalTime; }

	__inline bool IsSignalRange( SignalTypeEnum Type ) {
		if( m_nActionIndex == -1 ) return false;
		ActionElementStruct *pStruct = GetElement( m_nActionIndex );
		if (pStruct == NULL) return false;

		std::map<int, std::vector<CEtActionSignal *>>::iterator it = pStruct->MapSearch.find( Type );
		if( it == pStruct->MapSearch.end() ) return false;

		CEtActionSignal *pSignal;
		for( DWORD i=0; i<it->second.size(); i++ ) {
			pSignal = it->second[i];
			if( m_fFrame >= pSignal->GetStartFrame() && 
				m_fFrame < pSignal->GetEndFrame() ) 
				return true;
		}
		return false;
	}

	void ProcessAction( LOCAL_TIME LocalTime, float fDelta );

	virtual void SetActionQueue( const char *szActionName, int nLoopCount = 0, float fBlendFrame = 3.f, float fStartFrame = 0.f );

	void SetFPS( float fValue );
	float GetFPS();

	// Used when running an action different from the main action
	virtual void SetCustomAction( const char *szActionName, float fFrame );
	bool IsCustomAction();
	void ResetCustomAction();
	bool IsCustomProcessSignal() { return m_bCustomProcessSignal; }
	int GetCustomActionIndex() { return m_nCustomActionIndex; }

	// Get the current upper-body action frame during split upper/lower body action.
	float GetCustomActionCurrentFrame( void ) 
	{  
		if( IsCustomAction() )
		{
			float fFrame = ((m_LocalTime - m_CustomActionTime) / 1000.f) * m_fFPS;
			return fFrame;
		}

		return 0.0f;
	};

	virtual void OnNextAction( const char* szPrevAction, const char *szNextAction ) {}
	virtual void OnFinishAction(const char* szPrevAction, LOCAL_TIME time) {}
	virtual void OnChangeAction( const char *szPrevAction ) {}
	virtual void OnChangeActionQueue( const char *szPrevAction ) {}
	virtual void OnChangeCustomAction() {}
	virtual void OnLoopAction( float fFrame, float fPrevFrame ) {}

	int GetMaxProjectileCountInAction( int nActionIndex ) const
	{ 
		int iResult = 0;

		if( m_pProjectileCountInfo )
		{
			map<int, int>::const_iterator iterProj = m_pProjectileCountInfo->mapMaxProjectileCountInAction.find( nActionIndex );
			if( m_pProjectileCountInfo->mapMaxProjectileCountInAction.end() != iterProj )
		 		iResult = iterProj->second;
		}

		return iResult;
	};

	void AddUsingProjectileWeaponTableIDs( int nActionIndex, /*IN OUT*/ multiset<int>& setWeaponTableIDs ) const
	{
		if( m_pProjectileCountInfo )
		{
			map<int, multiset<int> >::const_iterator iter = m_pProjectileCountInfo->mapUsingProjectileWeaponTableIDs.find( nActionIndex );
			if( m_pProjectileCountInfo->mapUsingProjectileWeaponTableIDs.end() != iter )
			{
				setWeaponTableIDs.insert( iter->second.begin(), iter->second.end() );
			}
		}
	}

	// Called only for weapon class. When weapon has a projectile signal and player executes weapon action signal 
	// When owned, also pass the offset frame where the SendWeaponAction signal is placed.
	// Therefore, the resulting frame becomes the player-based frame.
	void AddProjectileSignalOffset( int nActionIndex, int iSendWeaponActionOffset, /*IN OUT*/ deque<int>& dqProjectileSignalOffset ) const
	{
		if( m_pProjectileCountInfo )
		{
			map<int, deque<int> >::const_iterator iterProjOffset = m_pProjectileCountInfo->mapProjectileSignalFrameOffset.find( nActionIndex );
			if( m_pProjectileCountInfo->mapProjectileSignalFrameOffset.end() != iterProjOffset )
			{
				const deque<int>& vlMyProjectileSignalOffset = iterProjOffset->second;
				for( int i = 0; i < (int)vlMyProjectileSignalOffset.size(); ++i )
				{
					dqProjectileSignalOffset.push_back( iSendWeaponActionOffset + vlMyProjectileSignalOffset.at(i) );
				}
			}
		}
	}

	bool IsValidSkillChain( int iElementIndex, int iElementIndexToChange )
	{
		bool bResult = false;
		if( m_pSetSkillChainAction )
		{
			if( m_pSetSkillChainAction->setSkillChainAction.end() != 
				m_pSetSkillChainAction->setSkillChainAction.find( iElementIndex ) )
			{
				map<int, vector<string> >::const_iterator iter = m_pSetSkillChainAction->mapCanChainToThisAction.find( iElementIndex );
				if( m_pSetSkillChainAction->mapCanChainToThisAction.end() != iter )
				{
					const vector<string>& vlCanChangeActionNames = iter->second;
					for( int i = 0; i < (int)vlCanChangeActionNames.size(); ++i )
					{
						int iCanChangeActionIndex = GetElementIndex( vlCanChangeActionNames.at(i).c_str() );
						if( iCanChangeActionIndex == iElementIndexToChange )
						{
							bResult = true;
							break;
						}
					}
				}
			}
			
		}

		return bResult;
	}
};
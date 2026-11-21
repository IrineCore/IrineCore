
#pragma once

#include "PvPGameModeScoreSystem.h"
#include "DnSkill.h"

#ifndef _GAMESERVER
#include "DNTableFile.h"
#endif

class CDNUserSession;
class CDNGameRoom;

class IScoreSystem
{
public:

	IScoreSystem():m_pGameModeScoreSystem(NULL),m_uiGameMode(PvPCommon::GameMode::Max),m_uiVictoryExp(0),m_uiVictoryBonusRate(0),m_uiDefeatExp(0),m_uiDefeatBonusRate(0)
	{
	}

	virtual ~IScoreSystem()
	{
		SAFE_DELETE( m_pGameModeScoreSystem );
	}

#ifdef _GAMESERVER
	bool InitializeBase( const TPvPGameModeTable* pPvPGameModeTable, const UINT uiWinCondition )
	{
		m_pGameModeScoreSystem	= CreatePvPGameModeScoreSystem( pPvPGameModeTable->uiGameMode );
		m_uiGameMode			= pPvPGameModeTable->uiGameMode;

		int iIndex = pPvPGameModeTable->GetWinconditionIndex( uiWinCondition );
		if( iIndex >= 0 )
		{
			m_uiVictoryExp			= pPvPGameModeTable->vVictoryExp[iIndex];
			m_uiVictoryBonusRate	= pPvPGameModeTable->vVictoryBonusRate[iIndex];
			m_uiDefeatExp			= pPvPGameModeTable->vDefeatExp[iIndex];
			m_uiDefeatBonusRate		= pPvPGameModeTable->vDefeatBonusRate[iIndex];
		}
		return true;
	}
#else // #ifdef _GAMESERVER
	bool InitializeBase( const UINT uiItemID, DNTableFileFormat* pSox )
	{
		if (pSox)
		{
			m_uiGameMode = pSox->GetFieldFromLablePtr( uiItemID, "GamemodeID" )->GetInteger();
			m_pGameModeScoreSystem	= CreatePvPGameModeScoreSystem( m_uiGameMode );
			return true;
		}
		return false;
	}
#endif // #ifdef _GAMESERVER

#if defined (_GAMESERVER)
	virtual bool InitializeOccupationSystem(TBattleGourndModeInfo * pModeInfo)	{ return true; }
#endif		//#if defined (_GAMESERVER)
#if defined(_CLIENT)
	virtual void InitializeOccupationSystem()	{ return; }
#endif	// #if defined(_CLIENT)

	const IPVPGameModeScoreSystem* GetPvPGameModeScoreSystem(){ return m_pGameModeScoreSystem; }

	void SetGameModeScore( const UINT uiATeamScore, const UINT uiBTeamScore )
	{
		if( m_pGameModeScoreSystem )
			m_pGameModeScoreSystem->SetGameModeScore( uiATeamScore, uiBTeamScore );
	}

	void GetGameModeScore( UINT& uiATeamScore, UINT& uiBTeamScore )
	{
		uiATeamScore = uiBTeamScore = 0;
		if( m_pGameModeScoreSystem )
			m_pGameModeScoreSystem->GetGameModeScore( uiATeamScore, uiBTeamScore );
	}

#if defined( _GAMESERVER )
	void FinishRound( CDNGameRoom* pGameRoom, UINT uiWinTeam )
	{
		if( m_pGameModeScoreSystem )
			m_pGameModeScoreSystem->OnFinishRound( pGameRoom, uiWinTeam );
	}
#endif // #if defined( _GAMESERVER )

#ifdef _GAMESERVER
	virtual void SendScore( CDNUserSession* pGameSession ) = 0;
	virtual void SendXPScore( CDNGameRoom* pGameRoom, CDNUserSession* pGameSession ) = 0;
	virtual bool QueryUpdatePvPData( const UINT uiWinTeam, CDNUserSession* pGameSession, PvPCommon::QueryUpdatePvPDataType::eCode Type ) = 0;
#endif

#ifdef _CLIENT
	virtual bool IsRedBossKill() = 0;
	virtual bool IsBlueBossKill() = 0;
#endif	//#ifdef _CLIENT

	virtual bool						IsPvPScoreSystem(){ return false; }
	//
	virtual void						ClearAllScore() = 0;
	virtual void						OnFinishRound( DnActorHandle hActor, const bool bIsWin ) = 0;
	virtual bool						AddUserScore( DnActorHandle hActor, const void* pData ) = 0;
#if !defined( _GAMESERVER )
	virtual bool						SetCount( DnActorHandle hActor, const void* pData ) = 0;
	virtual bool						SetCaptainKillCount( DnActorHandle hActor, const void* pData ){ return false; }
#endif
	virtual IPVPGameModeScoreSystem*	CreatePvPGameModeScoreSystem( const UINT uiGameMode ) = 0;
	// Count-related
	virtual UINT						GetKillCount( DnActorHandle hActor ){ return 0; }
	virtual UINT						GetDeathCount( DnActorHandle hActor ){ return 0; }
	// Score related
	virtual UINT						GetTotalScore( DnActorHandle hActor ) = 0;
	virtual UINT						GetKillScore( DnActorHandle hActor ) = 0;
	virtual UINT						GetAssistScore( DnActorHandle hActor ) = 0;
	virtual UINT						GetXPScore( DnActorHandle hActor, const UINT uiWinTeam ) = 0;
	virtual const void*					GetScoreStruct( const WCHAR* pwszCharName ) = 0;
	virtual UINT GetOccupationAcquireScore(DnActorHandle hActor) = 0;
	virtual bool GetOccupationTeamScore(int nTeam, int &nTeamScore) = 0;
	virtual UINT GetOccupationStealScore(DnActorHandle hActor ) = 0;
	virtual bool GetOccupationTeamScore(int nTeam, TPvPOccupationTeamScoreInfo &TeamScore) = 0;	
	//
	virtual void						OnDamage( DnActorHandle hActor, DnActorHandle hHitter, const INT64 iDamage ) = 0;
	virtual void						OnDie( DnActorHandle hActor, DnActorHandle hHitter ) = 0;
	virtual void						OnCmdAddStateEffect( const CDnSkill::SkillInfo* pSkillInfo ) = 0;
	virtual void						OnLeaveUser( DnActorHandle hActor ) = 0;
	virtual void						OnNotify( DnActorHandle hActor, const UINT uiScoreType, const UINT uiScore ) = 0;
	virtual void						OnStartRound() = 0;
	virtual UINT						GetCaptainKillCount( DnActorHandle hActor ){ return 0; }
	virtual bool OnTryAcquirePoint(DnActorHandle hActor, int AreaID, LOCAL_TIME Localtime) = 0;
	virtual bool OnAcquirePoint(int nTeam, const WCHAR * pName, bool bSnatched = false, bool bBreakInto = false) = 0;
	virtual void OnOccupationTeamScore(int nTeam, int nType) = 0;

	struct ScoreType
	{
		enum
		{
			KillScore = 0,
			AssistScore,
		};
	};

protected:

	IPVPGameModeScoreSystem*	m_pGameModeScoreSystem;

	UINT						m_uiGameMode;
	UINT						m_uiVictoryExp;
	UINT						m_uiVictoryBonusRate;
	UINT						m_uiDefeatExp;
	UINT						m_uiDefeatBonusRate;
};

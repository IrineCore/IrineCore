#include "stdafx.h"
#include "DNGuildWar.h"
#include "DNDivisionManager.h"
#include "DNVillageConnection.h"
#include "DNGuildWarManager.h"

extern TMasterConfig g_Config;

CDNGuildWar::CDNGuildWar()
{
	m_cEventStep = GUILDWAR_STEP_NONE;
	m_wScheduleID = GUILDWARSCHEDULE_DEF;
	m_nDBJobSeq = 0;
	m_nDBJobSendTick = 0;
	m_bDBJobSend = false;
	m_bDBJobSuccess = false;
	memset(m_bEventComplete, 0x00, sizeof(m_bEventComplete));
	m_tCurrent = 0;
	memset(m_tEventMoment, 0x00, sizeof(m_tEventMoment));
	m_wWinersWeightRate = 0;
}

CDNGuildWar::~CDNGuildWar()
{

}

void CDNGuildWar::Reset()
{
	m_wScheduleID = GUILDWARSCHEDULE_DEF;
	memset (m_tEventMoment, 0, sizeof(m_tEventMoment));
	memset (m_bEventComplete, 0, sizeof(m_bEventComplete));
	m_nDBJobSeq = 0;
	m_nDBJobSendTick = 0;
	m_bDBJobSend = false;
	m_bDBJobSuccess = false;
}

void CDNGuildWar::Process()
{
	time(&m_tCurrent);

	if (m_tEventMoment[GUILDWAR_EVENT_START] < m_tCurrent && !m_bEventComplete[GUILDWAR_EVENT_START])
	{
		OnStartEvent();
	}
	
	if (m_tEventMoment[GUILDWAR_EVENT_END] < m_tCurrent && !m_bEventComplete[GUILDWAR_EVENT_END])
	{
		OnEndEvent();
	}

/*
	//If winning guild is set, fetch whether reward was granted.. roughly once per minute..
	// Initialize here.
	DWORD dwTick = timeGetTime();
	static DWORD SendTick = 60*1000;
	if( g_pGuildWarManager->GetPreWinGuild().IsSet() && !g_pGuildWarManager->GetFinalWinGuildReward() && SendTick+(60*1000) < dwTick)
	{
		//CDNVillageConnection *pVillageConnection = g_pDivisionManager->GetFirstEnableVillageServer();	
		CDNVillageConnection* pVillageConnection = g_pDivisionManager->GetVillageConnectionByVillageID( g_pDivisionManager->GetPvPLobbyVillageID() );
		if( pVillageConnection )
		{			
			pVillageConnection->SendGetGuildWarPreWindGuildReward(g_pGuildWarManager->GetPreWinScheduleID(), g_pGuildWarManager->GetPreWinGuild().nDBID);			
		}
		SendTick = dwTick;
	}
*/
}

void CDNGuildWar::OnStartEvent()
{
	m_bEventComplete[GUILDWAR_EVENT_START] = true;	
	MAChangeGuildWarStep ChangeGuildWarStep;
	memset(&ChangeGuildWarStep, 0, sizeof(ChangeGuildWarStep));
	ChangeGuildWarStep.wScheduleID = m_wScheduleID;
	ChangeGuildWarStep.cEventStep = m_cEventStep;
	ChangeGuildWarStep.cEventType = GUILDWAR_EVENT_START;
	ChangeGuildWarStep.wWinersWeightRate = m_wWinersWeightRate;

	if (g_pGuildWarManager->GetCheatFlag())
	{
		ChangeGuildWarStep.bCheatFlag = true;
		g_pGuildWarManager->SetCheatFlag(false);
	}
	
	g_pDivisionManager->SendChangeGuildWarStep(&ChangeGuildWarStep);

	g_Log.Log(LogType::_GUILDWAR, g_Config.nWorldSetID, 0, 0, 0, L"[GUILDWAR] GuildWar OnStartEvent [ScheduleID:%d][Step:%d] \r\n", m_wScheduleID, m_cEventStep);
	m_nDBJobSendTick = timeGetTime();
}

void CDNGuildWar::OnEndEvent()
{
	m_bEventComplete[GUILDWAR_EVENT_END] = true;
	MAChangeGuildWarStep ChangeGuildWarStep;
	memset(&ChangeGuildWarStep, 0, sizeof(ChangeGuildWarStep));
	ChangeGuildWarStep.wScheduleID = m_wScheduleID;
	ChangeGuildWarStep.cEventStep = m_cEventStep;
	ChangeGuildWarStep.cEventType = GUILDWAR_EVENT_END;
	ChangeGuildWarStep.wWinersWeightRate = m_wWinersWeightRate;

	g_pDivisionManager->SendChangeGuildWarStep(&ChangeGuildWarStep);

	g_Log.Log(LogType::_GUILDWAR, g_Config.nWorldSetID, 0, 0, 0, L"[GUILDWAR] GuildWar OnEndEvent [ScheduleID:%d][Step:%d] \r\n", m_wScheduleID, m_cEventStep);
	m_nDBJobSendTick = 0;
}

bool CDNGuildWar::UpdateTime(short wScheduleID, time_t tStart, time_t tEnd, short wWinersWeightRate)
{
	m_wScheduleID = wScheduleID;
	m_tEventMoment[GUILDWAR_EVENT_START] = tStart;
	m_tEventMoment[GUILDWAR_EVENT_END] = tEnd;
	m_wWinersWeightRate = wWinersWeightRate;

	return true;
}

bool CDNGuildWar::IsValidPeriod()
{
	time(&m_tCurrent);

	if (m_tEventMoment[GUILDWAR_EVENT_START] < m_tCurrent && 
		m_tCurrent < m_tEventMoment[GUILDWAR_EVENT_END])
		return true;

	return false;
}

bool CDNGuildWar::IsFinishPeriod()
{
	if (m_bEventComplete[GUILDWAR_EVENT_START] && m_bEventComplete[GUILDWAR_EVENT_END])
		return true;

	return false;
}


time_t CDNGuildWar::GetEventTime(char cType)
{
	if (CheckEventType(cType))
		return m_tEventMoment[cType];

	return 0;
}

void CDNGuildWar::SetEventComplete(char cType, bool bComplete)
{
	if (CheckEventType(cType))
		m_bEventComplete[cType] = bComplete;
}

bool CDNGuildWar::IsEventComplete(char cType)
{
	if (CheckEventType(cType))
		return m_bEventComplete[cType];

	return false;
}

bool CDNGuildWar::CheckEventType(char cType)
{
	if (GUILDWAR_EVENT_START <= cType && cType < GUILDWAR_EVENT_MAX)
		return true;

	return false;
}


///////////////////////////////////////////////////////////////////
//						 Application period
///////////////////////////////////////////////////////////////////
CDNGuildWarPreparation::CDNGuildWarPreparation()
{
	m_cEventStep = GUILDWAR_STEP_PREPARATION;
}

CDNGuildWarPreparation::~CDNGuildWarPreparation()
{
	
}

void CDNGuildWarPreparation::Process()
{
	CDNGuildWar::Process();
	// Initialize here.
	DWORD dwTick = timeGetTime();
	// Check if Job was executed..
	// On first entry, reset the previous round.
	if( m_bEventComplete[GUILDWAR_EVENT_START] && !m_bDBJobSend && !m_nDBJobSeq && m_nDBJobSendTick>0 && m_nDBJobSendTick+GUILDWAR_DB_JOB_SYSTEM_RESULT_REQ_MAX < dwTick)
	{
		//CDNVillageConnection *pVillageConnection = g_pDivisionManager->GetFirstEnableVillageServer();	
		CDNVillageConnection* pVillageConnection = g_pDivisionManager->GetVillageConnectionByVillageID( g_pDivisionManager->GetPvPLobbyVillageID() );
		if( pVillageConnection )
		{
			pVillageConnection->SendAddDBJobSystemReserve(DBJOB_GUILDWAR_INIT);
			g_Log.Log(LogType::_GUILDWAR, g_Config.nWorldSetID, 0, 0, 0, L"[GUILDWAR] SendAddDBJobSystemReserve Init\r\n");
			SetDBJobSend(true);
		}
		else
		{
			// We are screwed if this happens...
			g_Log.Log(LogType::_GUILDWAR, g_Config.nWorldSetID, 0, 0, 0, L"[GUILDWAR] SendGetDBJobSystemReserve Not Village !!!\r\n");
		}
	}	
	// If Job was executed, check Job request every 1 minute.
	if( m_bDBJobSend && m_nDBJobSeq && m_nDBJobSendTick+GUILDWAR_DB_JOB_SYSTEM_RESULT_REQ_MAX < dwTick)
	{			
		//CDNVillageConnection *pVillageConnection = g_pDivisionManager->GetFirstEnableVillageServer();	
		CDNVillageConnection* pVillageConnection = g_pDivisionManager->GetVillageConnectionByVillageID( g_pDivisionManager->GetPvPLobbyVillageID() );
		if( pVillageConnection )
		{
			pVillageConnection->SendGetDBJobSystemReserve(m_nDBJobSeq);
			g_Log.Log(LogType::_GUILDWAR, g_Config.nWorldSetID, 0, 0, 0, L"[GUILDWAR] SendGetDBJobSystemReserve Init \r\n");
		}
		else
		{
			// We are screwed if this happens...
			g_Log.Log(LogType::_GUILDWAR,g_Config.nWorldSetID, 0, 0, 0,  L"[GUILDWAR] SendGetDBJobSystemReserve Init Not Village !!!\r\n");
		}
		m_nDBJobSendTick = dwTick;
	}
	// Check if Job request succeeded
	if( m_bDBJobSend && m_nDBJobSeq && m_bDBJobSuccess )
	{
		m_bDBJobSend = false;
	}
}

///////////////////////////////////////////////////////////////////
//						 Preliminary period
///////////////////////////////////////////////////////////////////
CDNGuildWarTrial::CDNGuildWarTrial()
{
	m_cEventStep = GUILDWAR_STEP_TRIAL;
	m_nTrialPointSendTick = 0;	
}

CDNGuildWarTrial::~CDNGuildWarTrial()
{

}

void CDNGuildWarTrial::Process()
{
	CDNGuildWar::Process();
	// Fetch and broadcast guild prelim scores every 10 minutes.
	DWORD dwTick = timeGetTime();
	if( m_bEventComplete[GUILDWAR_EVENT_START] && m_nTrialPointSendTick+GUILDWAR_TRIAL_POINT_REFRESH_TICK_MAX < dwTick)
	{
		//CDNVillageConnection *pVillageConnection = g_pDivisionManager->GetFirstEnableVillageServer();	
		CDNVillageConnection* pVillageConnection = g_pDivisionManager->GetVillageConnectionByVillageID( g_pDivisionManager->GetPvPLobbyVillageID() );
		if( pVillageConnection )
		{
			pVillageConnection->SendGetGuildWarPointRunning();
			//g_Log.Log(LogType::_GUILDWAR, g_Config.nWorldSetID, 0, 0, 0, L"[GUILDWAR] SendGetGuildWarPointRunning\r\n");			
		}
		else
		{
			// We are screwed if this happens...
			g_Log.Log(LogType::_GUILDWAR, g_Config.nWorldSetID, 0, 0, 0, L"[GUILDWAR] SendGetGuildWarPointRunning Not Village !!!\r\n");
		}
		m_nTrialPointSendTick = dwTick;
	}
}

///////////////////////////////////////////////////////////////////
//						 Finals and reward period
///////////////////////////////////////////////////////////////////
CDNGuildWarReward::CDNGuildWarReward()
{
	m_cEventStep = GUILDWAR_STEP_REWARD;
	Reset();	
}

CDNGuildWarReward::~CDNGuildWarReward()
{

}

void CDNGuildWarReward::Reset()
{
	CDNGuildWar::Reset();

	m_cCurrentFinals = GUILDWAR_FINALPART_16;	
	//m_cCurrentFinals = GUILDWAR_FINALPART_NONE;	
	m_bTrialStatsRequest = false;
	m_bFinalSchedule = false;
	memset(m_bFinalsComplete, 0, sizeof(m_bFinalsComplete));
	memset(m_tFinalsStartMoment, 0, sizeof(m_tFinalsStartMoment));
	memset(m_tFinalsEndMoment, 0, sizeof(m_tFinalsEndMoment));
}

void CDNGuildWarReward::Process()
{
	CDNGuildWar::Process();	
	DWORD dwTick = timeGetTime();

	// If Job was executed, check Job request every 1 minute.
	if( !m_bTrialStatsRequest && m_bDBJobSend && m_nDBJobSeq && m_nDBJobSendTick+GUILDWAR_DB_JOB_SYSTEM_RESULT_REQ_MAX < dwTick)
	{			
		//CDNVillageConnection *pVillageConnection = g_pDivisionManager->GetFirstEnableVillageServer();	
		CDNVillageConnection* pVillageConnection = g_pDivisionManager->GetVillageConnectionByVillageID( g_pDivisionManager->GetPvPLobbyVillageID() );
		if( pVillageConnection )
		{
			pVillageConnection->SendGetDBJobSystemReserve(m_nDBJobSeq);
			g_Log.Log(LogType::_GUILDWAR, g_Config.nWorldSetID, 0, 0, 0, L"[GUILDWAR] SendGetDBJobSystemReserve Stats \r\n");
		}
		else
		{
			// We are screwed if this happens...
			g_Log.Log(LogType::_GUILDWAR, g_Config.nWorldSetID, 0, 0, 0, L"[GUILDWAR] SendGetDBJobSystemReserve Stats Not Village !!!\r\n");
		}
		m_nDBJobSendTick = timeGetTime();
	}
	// Check if Job request succeeded
	if( m_bDBJobSend && m_nDBJobSeq && m_bDBJobSuccess )
	{	
		//CDNVillageConnection *pVillageConnection = g_pDivisionManager->GetFirstEnableVillageServer();	
		CDNVillageConnection* pVillageConnection = g_pDivisionManager->GetVillageConnectionByVillageID( g_pDivisionManager->GetPvPLobbyVillageID() );
		if( pVillageConnection )
		{
			pVillageConnection->SendGetGuildWarFinalTeam();				
			g_Log.Log(LogType::_GUILDWAR, g_Config.nWorldSetID, 0, 0, 0, L"[GUILDWAR] SendGetGuildWarFinalTeam \r\n");
		}
		else
		{
			// We are screwed if this happens...
			g_Log.Log(LogType::_GUILDWAR, g_Config.nWorldSetID, 0, 0, 0, L"[GUILDWAR] Reward Process Not Village !!!\r\n");
		}		
		g_pDivisionManager->SendGetGuildWarTrialStats();
		m_bTrialStatsRequest = true;
		m_bDBJobSend = false;		
	}
	if( m_bFinalSchedule && g_pGuildWarManager->IsFinalTeamSetting() && g_pGuildWarManager->GetFinalProgress() )
	{
		// Proceed with each round..
		for (int i=GUILDWAR_FINALPART_MAX-1; i>GUILDWAR_FINALPART_NONE; --i)
		{			
			if(m_tCurrent >= m_tFinalsStartMoment[i] && m_tCurrent < m_tFinalsEndMoment[i] && !m_bFinalsComplete[i] )
			{
				// Match start
				m_bFinalsComplete[i] = true;
				m_cCurrentFinals = i;
				g_pDivisionManager->SendSetGuildwarFinalProcess(i, 0); // Notify village and game servers of match start.

				if( m_tCurrent - m_tFinalsStartMoment[i] > 60*2 ) // Do not create room if creation time already passed 2 minutes ago (assume Master restarted)
					continue;

				// Create room by tier..
				g_pGuildWarManager->GuildWarCreateRoom();
				g_pGuildWarManager->SetFinalStart(true);
			}
			else if( m_tCurrent > m_tFinalsEndMoment[i] && m_bFinalsComplete[i] && m_cCurrentFinals == i )
			{
				// Match over.. proceed to next round..
				g_pDivisionManager->SendSetGuildwarFinalProcess(i-1, m_tFinalsStartMoment[i-1]);

				// For matches not finished by now, decide winner based on interim or preliminary score...
				g_pGuildWarManager->CalcGuildWarTournamentResult();				
				g_pDivisionManager->SendGuildWarAllStop();
				m_cCurrentFinals = i-1;								
				// Recreate tournament group..
				if( m_cCurrentFinals != GUILDWAR_FINALPART_NONE)
					g_pGuildWarManager->SetGuildWarTournamentGroup();
			}
		}
	}
}

void CDNGuildWarReward::OnStartEvent()
{
	CDNGuildWar::OnStartEvent();
	//m_nDBJobSendTick = timeGetTime();
	// When finals start, query whether finals-qualifying teams are saved
	// Request to Colosseum village
	CDNVillageConnection* pVillageConnection = g_pDivisionManager->GetVillageConnectionByVillageID( g_pDivisionManager->GetPvPLobbyVillageID() );
	if( pVillageConnection == NULL || pVillageConnection->GetActive() == false )
	{
		g_Log.Log(LogType::_GUILDWAR, g_Config.nWorldSetID, 0, 0, 0, L"[GUILDWAR] SendGetGuildWarTournamentInfo Not Village !!!\r\n");
		return;
	}
	else	
		pVillageConnection->SendGetGuildWarTournamentInfo(m_wScheduleID);		

	if( g_pGuildWarManager->GetFinalProgress() )
	{
		// Check if there is already a round in progress.
		for (int i=GUILDWAR_FINALPART_MAX-1; i>GUILDWAR_FINALPART_NONE; --i)
		{			
			if(m_tCurrent >= m_tFinalsStartMoment[i] && m_tCurrent >= m_tFinalsEndMoment[i] )
			{			
				m_bFinalsComplete[i] = true;
				m_cCurrentFinals = i-1;
				continue;
			}
			if( m_tCurrent > m_tFinalsStartMoment[i] && m_tCurrent <= m_tFinalsEndMoment[i] )
			{
				// Match currently in progress..
				m_cCurrentFinals = i;
				g_pGuildWarManager->SetFinalStart(true);
				return;
			}
		}
		// If it reached here, the next round is being prepared.
		g_pDivisionManager->SendSetGuildwarFinalProcess(m_cCurrentFinals, m_tFinalsStartMoment[m_cCurrentFinals]);
	}
}

void CDNGuildWarReward::SetFinalScheduleInfo(MASetGuildWarFinalSchedule* pData)
{
	// Finals setup
	for( int i=m_cCurrentFinals; i>GUILDWAR_FINALPART_NONE; --i)
	{
		m_tFinalsStartMoment[i] = pData->GuildWarFinalSchedule[i].tBeginTime;
		m_tFinalsEndMoment[i] = pData->GuildWarFinalSchedule[i].tEndTime;
	}
	m_bFinalSchedule = true;
}

char CDNGuildWarReward::GetCurFinalPart()
{
	return m_cCurrentFinals;
}
time_t CDNGuildWarReward::GetCurFinalPartBeginTime()
{
	if( m_bFinalsComplete[m_cCurrentFinals] ) // If in progress..
		return 0;
	return m_tFinalsStartMoment[m_cCurrentFinals];
}

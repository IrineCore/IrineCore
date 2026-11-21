#include "stdafx.h"
#include "MAAiScript.h"
#include "DnMonsterActor.h"
#include "DnWorld.h"
#include "DnWeapon.h"
#include "lua_tinker.h"
#include "DnSkill.h"
#include "DnBlow.h"
#include "MAAiChecker.h"
#include "MAAiCommand.h"
#include "DNAggroSystem.h"
#include "DNMonsterAggroSystem.h"
#include "MAAiProcessor.h"
#include "DnPartsMonsterActor.h"

#if defined(PRE_ADD_64990)
#include "DNGameRoom.h"
#endif // PRE_ADD_64990

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

AiScriptLoader g_AiScriptLoader;

AiScriptLoader::AiScriptLoader()
{
	
}
AiScriptLoader::~AiScriptLoader()
{
	for( std::map<std::string, ScriptData>::iterator itor=m_ScriptMap.begin() ; itor!=m_ScriptMap.end() ; ++itor )
	{
		for( UINT i=0 ; i<(*itor).second.m_AITable.size() ; ++i )
		{
			for( UINT j=0 ; j<(*itor).second.m_AITable[i].VecActionTable.size() ; ++j )
			{
				SAFE_DELETE( (*itor).second.m_AITable[i].VecActionTable[j].pMAAiCheckerManager );
			}
		}
#if defined(PRE_ADD_64990)
		for (int nIndex = 0; nIndex < 2; ++nIndex)
		{
			for( DWORD i=0; i<(*itor).second.m_SkillTable[nIndex].size(); i++ ) 
			{
				SAFE_DELETE( (*itor).second.m_SkillTable[nIndex][i].pMAAiCheckerManager );
			}
		}
#else
		for( DWORD i=0; i<(*itor).second.m_SkillTable.size(); i++ ) 
		{
			SAFE_DELETE( (*itor).second.m_SkillTable[i].pMAAiCheckerManager );
		}
#endif // PRE_ADD_64990
		for( UINT i=0 ; i<(*itor).second.m_AISkillProcessorTable.size() ; ++i )
		{
			SAFE_DELETE( (*itor).second.m_AISkillProcessorTable[i].pMAAiProcessorManager );
		}
		for( UINT i=0; i<(*itor).second.m_NonDownMeleeDamageActionTable.size(); ++i ) 
		{
			SAFE_DELETE( (*itor).second.m_NonDownMeleeDamageActionTable[i].pMAAiCheckerManager );
		}
		for( UINT i=0; i<(*itor).second.m_NonDownRangeDamageActionTable.size(); ++i ) 
		{
			SAFE_DELETE( (*itor).second.m_NonDownRangeDamageActionTable[i].pMAAiCheckerManager );
		}
		for( UINT i=0; i<(*itor).second.m_MeleeDefenseActionTable.size(); ++i ) 
		{
			SAFE_DELETE( (*itor).second.m_MeleeDefenseActionTable[i].pMAAiCheckerManager );
		}
		for( UINT i=0; i<(*itor).second.m_RangeDefenseActionTable.size(); ++i ) 
		{
			SAFE_DELETE( (*itor).second.m_RangeDefenseActionTable[i].pMAAiCheckerManager );
		}
		for( UINT i=0; i<(*itor).second.m_BeHitSkillActionTable.size(); ++i ) 
		{
			SAFE_DELETE( (*itor).second.m_BeHitSkillActionTable[i].pMAAiCheckerManager );
		}
		for( UINT i=0; i<(*itor).second.m_AssaultActionTable.size(); ++i ) 
		{
			SAFE_DELETE( (*itor).second.m_AssaultActionTable[i].pMAAiCheckerManager );
		}
	}
}

bool AiScriptLoader::AllLoadScript()
{
	m_ScriptMap.clear();
	std::vector<CFileNameString> FileList;
	CEtResourceMng::GetInstance().FindFileListAll_IgnoreExistFile( "MonsterAI", "*.lua", FileList );
	//CEtResourceMng::GetInstance().FindFileListAll_IgnoreExistFile( "MonsterAI", "MultiTarget_Test.lua", FileList );
	
	for ( int i = 0 ; i < (int)FileList.size() ; i++ )
	{
		bool bResult = LoadScript(FileList[i].c_str());
 		if ( !bResult )
		{
			std::string szLog = FormatA("Ai File Load Failed : %s\r\n", FileList[i].c_str());
			std::wstring wszLog;
			ToWideString(szLog, wszLog);
			g_Log.Log(LogType::_FILELOG, L"%s", wszLog.c_str());

			return false;
		}
	}

	return true;
}

bool AiScriptLoader::GetScript(const char* szFileName, OUT ScriptData& data)
{
	std::string szTemp = szFileName;
	ToLowerA( szTemp );
	std::map<std::string, ScriptData>::iterator it = m_ScriptMap.find( szTemp );
	if ( it == m_ScriptMap.end() )
		return false;

	data = it->second;
	return true;

}

bool AiScriptLoader::LoadActionTable(lua_tinker::table& t, int nIdx, OUT ActionTable& at)
{
	ActionTableCommon::eType Type = ActionTableCommon::Max;
	const char* szActionName = t.get<const char*>("action_name");
	if ( szActionName )
		Type = ActionTableCommon::Action;
	else
	{
		const char* szSkilIndex = t.get<const char*>("lua_skill_index");
		if( szSkilIndex )
			Type = ActionTableCommon::Skill;
	}

	if( Type == ActionTableCommon::Max )
		return false;

	std::string sz;
	if( szActionName )
		sz = szActionName;
	int		nRate					= t.get<int>("rate");
	int		nLoop					= t.get<int>("loop");
	int		nCoolTime				= t.get<int>("cooltime");
	int		nCancelLook				= t.get<int>("cancellook");
	float	fApproach				= t.get<float>("approach");
	int		nMinAttackMissRadian	= t.get<int>("min_missradian");
	int		nMaxAttackMissRadian	= t.get<int>("max_missradian");
	float	fRandomTarget			= t.get<float>("randomtarget");
#ifdef PRE_ADD_MONSTER_NORMAL_ACTION_PRIORITY
	int     nPriority               = t.get<int>("priority");
#endif

#if defined (PRE_MOD_AIMULTITARGET)
	int		nMultipleTarget			= 0;
	int		nSummonerTarget			= 0;
#else
	bool	bMultipleTarget			= false;
#endif
	int		iMultipleMaxCount		= 0;
	bool	bIsExecptCannonPlayer	= false;

	bool	bAnyMultipleTarget	= false;
	const char* pAnyMultipleTarget = t.get<const char*>("anymultipletarget");
	if( pAnyMultipleTarget )
		bAnyMultipleTarget = true;

	const char* pTemp = t.get<const char*>( bAnyMultipleTarget ? "anymultipletarget" : "multipletarget" );
	if( pTemp )
	{
		std::string					strString( pTemp );
		std::vector<std::string>	vSplit;
		boost::algorithm::split( vSplit, strString, boost::algorithm::is_any_of(",") );

#if defined (PRE_MOD_AIMULTITARGET)
		nMultipleTarget = boost::lexical_cast<int>(vSplit[0]);
		if( vSplit.size() > 1 )
		{
			iMultipleMaxCount = boost::lexical_cast<int>(vSplit[1]);
		}

		if( vSplit.size() > 2 )
		{
			if( stricmp( vSplit[2].c_str(), "exceptcannon" ) == 0 )
				bIsExecptCannonPlayer = true;
		}

		if( vSplit.size() > 3 )
			nSummonerTarget = boost::lexical_cast<int>(vSplit[3]);	
#else
		bMultipleTarget = boost::lexical_cast<int>(vSplit[0]) ? true : false;
		if( vSplit.size() == 2 )
		{
			iMultipleMaxCount = boost::lexical_cast<int>(vSplit[1]);
		}
		else if( vSplit.size() == 3 )
		{
			if( stricmp( vSplit[2].c_str(), "exceptcannon" ) == 0 )
				bIsExecptCannonPlayer = true;
		}
#endif
	}

	if ( fApproach < 1.0f)
		fApproach = 100.0f;

	const char* szCondition			= t.get<const char*>("target_condition");

	at.Type						= Type;
	at.szActionName				= sz;
	at.nIdx						= (int)nIdx;
	at.nRate					= nRate;
	at.nLoop					= nLoop;
	at.nCoolTime				= nCoolTime;
	at.nCancelLook				= nCancelLook;
	at.fApproach				= fApproach;
	at.nMinAttackMissRadian		= nMinAttackMissRadian;
	at.nMaxAttackMissRadian		= nMaxAttackMissRadian;
	at.fRandomTarget			= fRandomTarget;
#if defined (PRE_MOD_AIMULTITARGET)
	at.nMultipleTarget			= nMultipleTarget;
	at.nSummonerTarget			= nSummonerTarget;
#else
	at.bMultipleTarget			= bMultipleTarget;
#endif
	at.iMaxMultipleTargetCount	= iMultipleMaxCount;
	at.bIsAnyMultipleTarget		= bAnyMultipleTarget;
	at.bIsExceptCannonPlayer	= bIsExecptCannonPlayer;
#ifdef PRE_ADD_MONSTER_NORMAL_ACTION_PRIORITY
	at.nActionPriority             = nPriority;
#endif

#if defined( PRE_FIX_68096 )	
	if( t.get<const char*>("globalcooltime") )
	{
		at.iGlobalCoolTimeIndex[0] = t.get<int>("globalcooltime")-1;
		_ASSERT( at.iGlobalCoolTimeIndex[0] >= 0 );
	}
	for( int i=1 ; i<ActionTableCommon::eCommon::MaxGlobalCoolTime ; ++i )
	{
		char szBuf[MAX_PATH];
		sprintf( szBuf, "globalcooltime%d", i+1 );
		at.iGlobalCoolTimeIndex[i] = t.get<int>(szBuf)-1;
		if(at.iGlobalCoolTimeIndex[i] == -1)
			break;		
	}
#else
	if( t.get<const char*>("globalcooltime") )
	{
		at.iGlobalCoolTimeIndex = t.get<int>("globalcooltime")-1;
		_ASSERT( at.iGlobalCoolTimeIndex >= 0 );
	}
#endif

	if( Type == ActionTableCommon::Skill )
	{
		at.nLuaSkillIndex = t.get<int>("lua_skill_index");
		at.nBeHitSkillIndex = t.get<int>("skill_index");
	}

	// LoadChecker
	at.pMAAiCheckerManager = new CMAAiActionCheckerManager;
	if( !at.pMAAiCheckerManager->bLoadChecker( t ) )
		return false;

	if ( szCondition )
		at.szTargetCondition = szCondition;
	
	return true;
}

bool AiScriptLoader::LoadTable( lua_State* pLua, const char* pTableName, std::vector<ActionTable>& vActionTable )
{
	vActionTable.clear();

	lua_tinker::table	luaTable	= lua_tinker::get<lua_tinker::table>( pLua, pTableName );
	lua_tinker::table	t			= luaTable.get<lua_tinker::table>(1);

	if( t.get<const char*>("action_name") || t.get<const char*>("lua_skill_index") )
	{
		for( int j=1 ; j<=luaTable.getSize() ; ++j )
		{
			lua_tinker::table LocalT = luaTable.get<lua_tinker::table>(j);
			ActionTable at;
			if( !LoadActionTable( LocalT, (int)vActionTable.size(), at ) )
				return false;
			vActionTable.push_back(at);
		}
	}

	return true;
}

bool AiScriptLoader::LoadSkillProcessor( lua_State* pLua, std::vector<AISkillProcessor>& vSkillProcessor )
{
	vSkillProcessor.clear();

	lua_tinker::table	luaTable	= lua_tinker::get<lua_tinker::table>( pLua, "g_Lua_SkillProcessor" );
	lua_tinker::table	t			= luaTable.get<lua_tinker::table>(1);

	if( t.get<const char*>("skill_index") )
	{
		for( int i=1 ; i<=luaTable.getSize() ; ++i )
		{
			lua_tinker::table LocalT = luaTable.get<lua_tinker::table>(i);
			
			AISkillProcessor at;

			at.nSkillIndex				= LocalT.get<int>("skill_index");
			at.pMAAiProcessorManager	= new CMAAiProcessorManager();
			if( !at.pMAAiProcessorManager->bLoad( LocalT ) )
				return false;

			vSkillProcessor.push_back(at);
		}
	}

	return true;
}

#if defined( PRE_ADD_LOTUSGOLEM )

bool AiScriptLoader::LoadPartsProcessor( lua_State* pLua, ScriptData& data )
{
	// OnlyParts
	data.m_iOnlyPartsDamage = lua_tinker::get<int>(pLua, "g_Lua_OnlyPartsDamage");

	// Parts
	std::vector<AIPartsProcessor>& vPartsProcessor = data.m_AIPartsProcessor;

	vPartsProcessor.clear();

	lua_tinker::table	luaTable	= lua_tinker::get<lua_tinker::table>( pLua, "g_Lua_PartsProcessor" );
	lua_tinker::table	t			= luaTable.get<lua_tinker::table>(1);

	if( t.get<const char*>("hp") )
	{
		for( int i=1 ; i<=luaTable.getSize() ; ++i )
		{
			lua_tinker::table LocalT = luaTable.get<lua_tinker::table>(i);

			AIPartsProcessor at;
			

			// hp
			{
				const char* pszHP = LocalT.get<const char*>("hp");
				if( pszHP == NULL )
				{
					_ASSERT(0);
					return false;
				}

				std::string					strString( pszHP );
				std::vector<std::string>	vSplit;
				std::vector<int>			vTableID;

				boost::algorithm::split( vSplit, strString, boost::algorithm::is_any_of(",") );

				for( UINT i=0 ; i<vSplit.size() ; ++i )
					vTableID.push_back( atoi(vSplit[i].c_str()) );

				if( vTableID.size() != 2 )
				{
					_ASSERT(0);
					return false;
				}

				at.iMinHP	= vTableID[0];
				at.iMaxHP	= vTableID[1];
			}

			// checkskill
			{
				at.iCheckSkillIndex = 0;
				const char* pszCheckSkillIndex = LocalT.get<const char*>("checkskill");
				if( pszCheckSkillIndex )
					at.iCheckSkillIndex = atoi( pszCheckSkillIndex );
			}

			// checkblow
			{
				at.iCheckBlowIndex = 0;
				const char* pszCheckBlowIndex = LocalT.get<const char*>("checkblow");
				if( pszCheckBlowIndex )
					at.iCheckBlowIndex = atoi( pszCheckBlowIndex );
			}

			// checkaction
			{
				const char* pszCheckAction = LocalT.get<const char*>("checkaction");
				if( pszCheckAction )
					at.strCheckActionName = pszCheckAction;
			}

			// ignore
			{
				const char* pszIgnore = LocalT.get<const char*>("ignore");
				if( pszIgnore )
				{
					std::string					strString( pszIgnore );
					std::vector<std::string>	vSplit;
					std::vector<int>			vTableID;

					boost::algorithm::split( vSplit, strString, boost::algorithm::is_any_of(",") );

					for( UINT i=0 ; i<vSplit.size() ; ++i )
						vTableID.push_back( atoi(vSplit[i].c_str()) );

					at.vIgnorePartsIndex = vTableID;
				}
			}

			// nodamage
			{
				const char* pszNoDamage = LocalT.get<const char*>("nodamage");
				if( pszNoDamage )
				{
					std::string					strString( pszNoDamage );
					std::vector<std::string>	vSplit;
					std::vector<int>			vTableID;

					boost::algorithm::split( vSplit, strString, boost::algorithm::is_any_of(",") );

					for( UINT i=0 ; i<vSplit.size() ; ++i )
						vTableID.push_back( atoi(vSplit[i].c_str()) );

					at.vNoDamagePartsIndex = vTableID;
				}
			}

			// active
			{
				const char* pszActive = LocalT.get<const char*>("active");
				if( pszActive )
				{
					std::string					strString( pszActive );
					std::vector<std::string>	vSplit;
					std::vector<int>			vTableID;

					boost::algorithm::split( vSplit, strString, boost::algorithm::is_any_of(",") );

					for( UINT i=0 ; i<vSplit.size() ; ++i )
						vTableID.push_back( atoi(vSplit[i].c_str()) );

					at.vActivePartsIndex = vTableID;
				}
			}

			vPartsProcessor.push_back(at);
		}
	}

	return true;
}

#endif // #if defined( PRE_ADD_LOTUSGOLEM )

bool AiScriptLoader::LoadAggroRange( lua_State* pLua, std::vector<AIAggroRange>& vAggroRange )
{
	vAggroRange.clear();

	lua_tinker::table	luaTable	= lua_tinker::get<lua_tinker::table>( pLua, "g_Lua_ThreatRange" );
	lua_tinker::table	t			= luaTable.get<lua_tinker::table>(1);

	if( t.get<const char*>("selfhppercent") )
	{
		for( int i=1 ; i<=luaTable.getSize() ; ++i )
		{
			lua_tinker::table LocalT = luaTable.get<lua_tinker::table>(i);

			AIAggroRange at;

			at.iSelfHPPercent			= LocalT.get<int>("selfhppercent");
			at.iThreatRange				= LocalT.get<int>("ThreatRange");
			at.iCognizanceGentleRange	= LocalT.get<int>("CognizanceGentleRange");
			at.iCognizanceThreatRange	= LocalT.get<int>("CognizanceThreatRange");

			vAggroRange.push_back(at);
		}
	}

	return true;
}

bool AiScriptLoader::LoadGlobalCoolTime( lua_State* pLua, std::vector<AIGlobalCoolTime>& vGlobalCoolTime )
{
	vGlobalCoolTime.clear();

	for( int i=1 ; i<INT_MAX ; ++i )
	{
		char szBuf[MAX_PATH];
		sprintf( szBuf, "g_Lua_GlobalCoolTime%d", i );
		int iCoolTime = lua_tinker::get<int>( pLua, szBuf );
		if( iCoolTime <= 0 )
			break;

		AIGlobalCoolTime at;
		at.dwCoolTime = static_cast<DWORD>(iCoolTime);
		vGlobalCoolTime.push_back( at );
	}

	return true;
}

bool AiScriptLoader::LoadSetPartsRebirthTable( lua_State* pLua, ScriptData& data )
{
	data.m_vSetPartsRebirthTable.clear();

	std::string			strTableName	= boost::io::str( boost::format( "g_Lua_RebirthParts" ) );
	lua_tinker::table	luaTable		= lua_tinker::get<lua_tinker::table>( pLua, strTableName.c_str() );
	if( !luaTable.m_obj->m_pointer )
		return true;

	for( int i=1 ; i<=luaTable.getSize() ; ++i )
	{
		AI_SETPARTSREBIRTH_TABLE PartsTable;

		lua_tinker::table t = luaTable.get<lua_tinker::table>(i);

		// PartsTableID
		std::string					strString( t.get<const char*>("tableID") );
		std::vector<std::string>	vSplit;
		boost::algorithm::split( vSplit, strString, boost::algorithm::is_any_of(",") );
		for( UINT j=0 ; j<vSplit.size() ; ++j )
			PartsTable.vPartsTableID.push_back( boost::lexical_cast<UINT>(vSplit[j]) );
		// Tick
		PartsTable.uiRebirthTick = t.get<int>("tick");

		data.m_vSetPartsRebirthTable.push_back( PartsTable );
	}

	return true;
}

bool AiScriptLoader::LoadScript(const char* szFilename)
{
	std::map<std::string, ScriptData>::iterator it = m_ScriptMap.find(std::string(szFilename));
	if ( it != m_ScriptMap.end() )
		return false;

	ScriptData data;
	std::string szFullName;
	szFullName = CEtResourceMng::GetInstance().GetFullName( szFilename ).c_str();

	char szTemp[256] = { 0, };
	_GetFullFileName( szTemp, _countof(szTemp), szFilename );
	_strlwr_s( szTemp );
	data.m_szSettingFile = szTemp;

	lua_State* pLua =  lua_open();
	luaL_openlibs(pLua); 

	if ( lua_tinker::dofile(pLua, szFullName.c_str()) != 0 )
	{
		OutputDebug("Cannot open file : %s\n" , szFullName.c_str());
		lua_close(pLua);
		return true;
	}

	data.m_nNearTableCnt	= lua_tinker::get<int>(pLua, "g_Lua_NearTableCount");
	data.m_nPatrolBaseTime	= lua_tinker::get<int>(pLua, "g_Lua_PatrolBaseTime");
	data.m_nPatrolRandTime	= lua_tinker::get<int>(pLua, "g_Lua_PatrolRandTime");
	data.m_fApproachValue	= lua_tinker::get<float>(pLua, "g_Lua_ApproachValue");
	data.m_nAssaultTime		= lua_tinker::get<int>(pLua, "g_Lua_AssaultTime");
	data.m_Type				=  ScriptData::None;
	if( lua_tinker::get<int>(pLua, "g_Lua_NoAggroOwnerFollow") > 0 )
		data.m_Type |= ScriptData::NoAggroOwnerFollow;
#ifdef PRE_ADD_AI_NOAGGRO_STAND
	bool bIsNoAggroStand = (lua_tinker::get<int>(pLua, "g_Lua_NoAggroStand") > 0);
	if (bIsNoAggroStand)
		data.m_Type |= ScriptData::NoAggroStand;
#endif

	if ( data.m_nAssaultTime < 100 )
		data.m_nAssaultTime = 3000;

	if ( data.m_fApproachValue < 10.0f )
		data.m_fApproachValue = 100.0f;

	data.m_AITable.clear();
	data.m_CustomActionMap.clear();

	// SET parts monster resurrection table
	if( !LoadSetPartsRebirthTable( pLua, data ) )
		return false;

	// MeleeDefenseTable
	if( !LoadTable( pLua, "g_Lua_MeleeDefense", data.m_MeleeDefenseActionTable ) )
		return false;
	// RangeDefenseTable
	if( !LoadTable( pLua, "g_Lua_RangeDefense", data.m_RangeDefenseActionTable ) )
		return false;
	// NonDownMeleeDamageTable
	if( !LoadTable( pLua, "g_Lua_NonDownMeleeDamage", data.m_NonDownMeleeDamageActionTable ) )
		return false;
	// NonDownRangeDamageTable
	if( !LoadTable( pLua, "g_Lua_NonDownRangeDamage", data.m_NonDownRangeDamageActionTable ) )
		return false;
	// BeHitSkillTable
	if( !LoadTable( pLua, "g_Lua_BeHitSkill", data.m_BeHitSkillActionTable ) )
		return false;
	// AssultTable
	if( !LoadTable( pLua, "g_Lua_Assault", data.m_AssaultActionTable ) )
		return false;
	if( !LoadSkillProcessor( pLua, data.m_AISkillProcessorTable ) )
		return false;
#if defined( PRE_ADD_LOTUSGOLEM )
	if( !LoadPartsProcessor( pLua, data ) )
		return false;
#endif // #if defined( PRE_ADD_LOTUSGOLEM )
	if( !LoadAggroRange( pLua, data.m_AIAggroRange ) )
		return false;
	if( !LoadGlobalCoolTime( pLua, data.m_AIGlobalCoolTime ) )
		return false;

	// If the skill table exists.
#if defined(PRE_ADD_64990)
	vector<string> tableNames;
	MonsterSkillAI::CheckSkillTable(pLua, tableNames);

	string PvESkillTableName;
	string PvPSkillTableName;

	if (tableNames[2].empty() == false)
		PvPSkillTableName = tableNames[2];
	else
	{
		//If existing "g_Lua_Skill" exists and there is no PvE
		if (tableNames[0].empty() == false && tableNames[1].empty() == true)
			PvPSkillTableName = tableNames[0];
	}

	if (tableNames[1].empty() == false)
		PvESkillTableName = tableNames[1];
	else
	{
		if (tableNames[0].empty() == false)
			PvESkillTableName = tableNames[0];
	}

	if( !MonsterSkillAI::LoadSkillTable(pLua, data.m_SkillTable[0], PvESkillTableName) )
		return false;

	if( !MonsterSkillAI::LoadSkillTable(pLua, data.m_SkillTable[1], PvPSkillTableName) )
		return false;
#else
	if( !MonsterSkillAI::LoadSkillTable(pLua, data.m_SkillTable) )
		return false;
#endif // PRE_ADD_64990
	
	for ( int i = 0 ; i < data.m_nNearTableCnt ; i++ )
	{
		AI_TABLE table;
		char buff[512] = {0,};
		ZeroMemory(buff, sizeof(char)*512);
		sprintf_s(buff, "g_Lua_NearValue%d", i+1 );
		table.fNearValue	= (float)lua_tinker::get<float>(pLua, buff);
		table.fNearValueSq	= table.fNearValue * table.fNearValue;

		ZeroMemory(buff, sizeof(char)*512);
		sprintf_s(buff, "g_Lua_Near%d", i+1 );
		lua_tinker::table neartable = lua_tinker::get<lua_tinker::table>(pLua, buff);
		if( !neartable.m_obj->m_pointer )
			return false;

		table.nRateSum = 0;

		for ( int j = 1 ; j <= neartable.getSize() ; j++ )
		{
			lua_tinker::table t = neartable.get<lua_tinker::table>(j);

			ActionTable at;
			at.nDistanceState = i;
			if( !LoadActionTable(t, (int)table.VecActionTable.size(), at) )
				return false;

			table.VecActionTable.push_back(at);
			table.nRateSum += at.nRate;
		}

		data.m_AITable.push_back(table);
	}

	lua_tinker::table custom_table = lua_tinker::get<lua_tinker::table>(pLua, "g_Lua_CustomAction");

	// fuck.. -_-
	bool bHave = false;
	lua_tinker::table t = custom_table.get<lua_tinker::table>("CustomAction1");
	lua_tinker::table t2 = t.get<lua_tinker::table>(1);
	const char* szCustomActionName = t2.get<const char*>(1);
	if ( szCustomActionName == NULL )	bHave = false;
	else	bHave = true;

	if ( bHave )
	{
		char buff[512] = {0,} ;
		for ( int i = 1 ; i <= custom_table.getSize() ; i++ )
		{
			CustomActionTable	custom;
			ZeroMemory(buff, sizeof(char)*512);
			sprintf_s(buff, "CustomAction%d", i );
			custom.szCustomActionName = buff;

			lua_tinker::table custom_action = custom_table.get<lua_tinker::table>(buff);
			for ( int j = 1 ; j <= custom_action.getSize() ; j++ )
			{
				lua_tinker::table action_info = custom_action.get<lua_tinker::table>(j);

				CustomAction info;
				info.szActionName	= action_info.get<const char*>(1);
				if( stricmp( info.szActionName.c_str(), "useskill" ) == 0 )
				{
					info.nLuaSkillIndex = action_info.get<int>( "lua_skill_index" );
				}
				else
				{
					info.nLoop = action_info.get<int>(2);
				}
				custom.m_ActionList.push_back(info);
			}
			data.m_CustomActionMap.insert(make_pair(custom.szCustomActionName, custom));
		}

	}

	lua_tinker::table customAttackTable = lua_tinker::get<lua_tinker::table>(pLua, "g_Lua_CustomAttack");

	//-----------------------------------------------------------------------------------------------

	{
		lua_tinker::table state_table = lua_tinker::get<lua_tinker::table>(pLua, "g_Lua_State");
		bHave = false;
		lua_tinker::table LocalT = state_table.get<lua_tinker::table>("State1");
		const char* szStateName = LocalT.get<const char*>(1);
		if ( szStateName == NULL ) { bHave = false; 	}
		else { 	bHave = true;	}

		if ( bHave )
		{
			char buff[512] = {0,};
			for ( int i = 1 ; i <= state_table.getSize() ; i++ )
			{
				ZeroMemory(buff, sizeof(char)*512);
				sprintf_s(buff, "State%d", i );
				StateCondition condition;

				lua_tinker::table __state = state_table.get<lua_tinker::table>(buff);
				for ( int j = 1 ; j <= __state.getSize() ; j++ )
				{
					OrOperator oper;
					const char* _szState = __state.get<const char*>(j);
					std::string szState = _szState;

					std::vector<std::string> orOper;
					TokenizeA(szState, orOper, "|");

					for ( int k = 0 ; k < (int)orOper.size() ; k++ )
					{
						StateOperation stateOper;
						stateOper.nState = CDnActorState::None;
						stateOper.bNOT = false;

						stateOper.szString = orOper[k];
						
						std::string::size_type npos = stateOper.szString.find("!");
						if ( npos < stateOper.szString.size() )
						{
							stateOper.bNOT = true;
						}
						std::string szLocalTemp = orOper[k];
						RemoveStringA(szLocalTemp, std::string("!"));
						stateOper.nState = CDnActorState::String2ActorStateEnum(szLocalTemp.c_str());
					
						oper.OrOperatorArray.push_back(stateOper);
					}
					condition.AndOperatorArray.push_back(oper) ;
				}
				data.m_StateContidionMap.insert(make_pair(buff, condition));
			}
		}
	}

	//---------------------------------------------------------------------------
	m_ScriptMap.insert(make_pair(data.m_szSettingFile, data));

	lua_close(pLua);
	return true;
}

//---------------------------------------------------------------------------------------------------------

MAAiScript::MAAiScript( DnActorHandle hActor, MAAiReceiver *pReceiver )
: MAAiBase( hActor, pReceiver )
{
	m_Data.m_nNearTableCnt = 0;
	m_Data.m_nPatrolBaseTime = 5000;
	m_Data.m_nPatrolRandTime = 3000;
	m_Data.m_fApproachValue = 100.0f;
	m_Data.m_nCurrentCustomActionPlayIndex = -1;
	m_Data.m_nAssaultTime = 5000;
	m_Data.m_Type = ScriptData::eTypeBit::None;
	m_nState = AT_Normal;
	m_nSeletedAssaultActionIndex = -1;
	m_bValidAI = false;

	m_pDestinationOBB	= NULL;
	m_uiMoveFrontRate	= 100;	// Default MOVE_FRONT
	m_dwDestinationOBB	= 0;
	m_ChangeStateTime = 0;
	m_pAICommand		= new MAAiCommand;
}

MAAiScript::~MAAiScript()
{
	SAFE_DELETE( m_pAICommand );
}

void MAAiScript::Process( LOCAL_TIME LocalTime, float fDelta )
{
	MAAiBase::Process( LocalTime, fDelta );
	m_SkillAI.Process(LocalTime, fDelta);

	std::string szAction = m_hActor->GetCurrentAction();
	if ( szAction == ""  )	
		m_hActor->CmdAction( "Stand", 0, -1.f );
	
	OnAI("", LocalTime);
#if defined( PRE_ADD_LOTUSGOLEM )
	_ProcessParts();
#endif // #if defined( PRE_ADD_LOTUSGOLEM )
}

void MAAiScript::OnChangeAction(const char* szPrevAction)
{
	if( bIsAILook() && !m_strAILookAction.empty() )
	{
		if( strcmp( m_strAILookAction.c_str(), szPrevAction ) == 0 )
		{
			ResetAILook();
			m_strAILookAction.clear();
		}
		else
		{
			return;
		}
	}

	if( bIsAILook() && !m_hActor->IsProcessSkill() )
		ResetAILook();

#if defined( PRE_FIX_MOVEBACK )	
	if(m_hActor && m_hActor->IsMonsterActor() && m_pMonsterActor )
		m_pMonsterActor->SetNearMoveBack();
#endif
	// Reset the actor set by the projectile target reservation signal when the action changes.
	m_hReservedProjectileTarget = CDnActor::Identity();
}

void MAAiScript::OnFinishAction(const char* szPrevAction, LOCAL_TIME time)
{
	if( bIsAILook() && !m_strAILookAction.empty() )
	{
		if( strcmp( m_strAILookAction.c_str(), szPrevAction ) == 0 )
		{
			ResetAILook();
			m_strAILookAction.clear();
		}
	}

	// Reset MultipleTarget if the action has finished.
#if defined (PRE_MOD_AIMULTITARGET)
	if( m_cMultipleTarget.GetMultipleTarget() > 0 && m_cMultipleTarget.GetType() == CMAAiMultipleTarget::Skill )
#else
	if( m_cMultipleTarget.bIsMultipleTarget() && m_cMultipleTarget.GetType() == CMAAiMultipleTarget::Skill )
#endif
	{
		if( m_hActor->IsProcessSkill() == false )
			m_cMultipleTarget.Reset();
	}
	else
	{
		m_cMultipleTarget.Reset();
	}
#if defined( PRE_FIX_MOVEBACK )	
	if(m_hActor && m_hActor->IsMonsterActor() && m_pMonsterActor )
		m_pMonsterActor->SetPrevMoveBack( false );
#endif

	OnAI(szPrevAction, time);
}

void MAAiScript::OnDie()
{

}

void MAAiScript::OnInitNaviDestination( SOBB* pOBB, UINT uiMoveFrontRate )
{
	m_pDestinationOBB	= pOBB;
	m_uiMoveFrontRate	= uiMoveFrontRate;
}

#ifndef _FINAL_BUILD

void MAAiScript::OnTestNaviAI( LOCAL_TIME time )
{
	if ( IsDelay(AIDelayType::Patrol) ) 
		return;

	if ( m_hTarget )
	{
		EtVector3*	pvPos	= m_hActor->GetPosition();
		float		fLength = 0.0f;

		if( m_hTarget ) 
		{
			fLength = EtVec3Length( &( *pvPos - *m_hTarget->GetPosition() ) );
		}

		// Step back if too close. 
		if ( fLength <= 50.0f && m_hActor->IsMovable() /*&& !m_hActor->IsHit() && !m_hActor->IsAttack() && !m_hActor->IsDown()*/ )
		{
			std::string szAction;
			int nBack = _rand(m_hActor->GetRoom())%2;
			switch( nBack )
			{
			case 0: szAction = "Move_Back"; break;
			case 1: szAction = "Walk_Back"; break;
			}

			m_hActor->CmdAction( szAction.c_str(), 0, g_fBendFrame  );
			return;
		}
		if ( m_hActor->IsStay() && m_hActor->IsMovable()   )
		{
			m_hActor->CmdMoveNavi( m_hTarget, 50.0f, "Move_Front", -1, g_fBendFrame );
			//m_hActor->CmdMove( m_hTarget, 50.0f, "Move_Front", -1, g_fBendFrame );
			return;
		}
	}

	SetDelay( AIDelayType::Patrol, 10000);
}

void MAAiScript::OnTestAttackAI( LOCAL_TIME time )
{
	static int nTestCount = 0;
		
	if ( IsDelay( AIDelayType::Patrol ) ) 
		return;

	if( m_hTarget )
	{
		int state = 0;

		if( m_hActor->IsStay() && m_hActor->IsMovable() )
		{
			int nIndex = SelectAction(m_Data.m_AITable[state].VecActionTable, time);
			if( nIndex < 0 )
				return;

			int nLoop = m_Data.m_AITable[state].VecActionTable[nIndex].nLoop;
			if( nLoop > 0 ) nLoop = _rand(m_hActor->GetRoom())%nLoop;
			std::string szAction = m_Data.m_AITable[state].VecActionTable[nIndex].szActionName;

			if( strstr( szAction.c_str(), "Attack" ) ) 
			{
				m_vAILook	= EtVec3toVec2( *m_hTarget->GetPosition() - *m_hActor->GetPosition() );

				int nAdd = (_rand(m_hActor->GetRoom())%10)+0;
				if( nTestCount++%2 )
					nAdd *= -1;

				m_bIsAILook	= true;
				m_vAILook.x = m_vAILook.x * cos( D3DXToRadian(nAdd)) - m_vAILook.y * sin( D3DXToRadian(nAdd) );
				m_vAILook.y = m_vAILook.x * sin( D3DXToRadian(nAdd)) + m_vAILook.y * cos( D3DXToRadian(nAdd) );
				EtVec2Normalize( &m_vAILook, &m_vAILook );

				m_hActor->CmdAction( szAction.c_str(), nLoop, g_fBendFrame );

				SetDelay( AIDelayType::Patrol, 5000 );
				//g_Log.LogA( "Attack angle=%d\r\n", nAdd );
				return;
			}
		}
	}
}

#endif // #ifndef _FINAL_BUILD

void MAAiScript::OnAI(const char* szPrevAction, LOCAL_TIME time)
{
	if( !m_bValidAI || m_hActor->IsDie() ) 
		return;

	EtVector3* pvPos = m_hActor->GetPosition();
	FindTarget();

#ifndef _FINAL_BUILD
	//return OnTestNaviAI( time );
	//return OnTestAttackAI( time );
#endif

	if ( m_pMonsterActor )
	{
		if ( m_pMonsterActor->OnPreAiProcess(szPrevAction, time) )
			return;
	}

	int		state	= 0;
	float	fLength = 0.0f;

	if( m_hTarget ) 
	{
		state	= GetTargetDistanceState( m_hTarget );

#ifdef PRE_FIX_PARTSMONSTER_AI_TARGETTING
		fLength = EtVec3Length( &( *pvPos - m_hTarget->FindAutoTargetPos() ) );
#else
		fLength = EtVec3Length( &( *pvPos - *m_hTarget->GetPosition() ) );
#endif

	}

	if ( m_nState == AT_CustomAction )
	{
		if ( m_hActor->IsStay() && m_hActor->IsMovable() )
		{
//			if( m_hActor->GetLookTarget() != m_hTarget ) m_hActor->CmdLook(m_hTarget);
			OnCustomAction(state, fLength, time);
		}
		return;
	}
	else if( m_nState == AT_Assault )
	{
		if( m_hActor->IsMove() )
		{
			// If charging and AssaultTime has elapsed, stop
			if( time - m_ChangeStateTime >= m_Data.m_nAssaultTime )
			{
				m_hActor->CmdStop( "Stand" );
				//g_Log.LogA( "AssaultTime ended, stopping movement!!!\r\n" );
			}
		}
	}

	if( m_hTarget ) 
	{
		// If the target is dead or too far away, cancel targeting and wander.
		if( m_hTarget->IsDie() || (m_pMonsterActor && fLength > m_pMonsterActor->GetCognizanceThreatRange()) )
		{
			SetTarget( CDnActor::Identity() );
//			m_hActor->CmdLook(m_hTarget);
			SetDelay( AIDelayType::Patrol, 0 );
			return;
		}
		else
		{
			if ( state < 0 )
			{
				// g_Log.Log(_WARNING, L"Aggro distance and distance table values do not match..\n"); // Temporarily commented out
				SetTarget( CDnActor::Identity() );
//				m_hActor->CmdLook(m_hTarget);
				SetDelay( AIDelayType::Patrol, 0 );
				return;
			}
		}

		// Step back if too close. 
		if ( !(m_hActor->IsNaviMode()) && fLength <= 50.0f && m_hActor->IsMovable() )
		{
			const char *szCurAction = m_hActor->GetCurrentAction();
			if( strcmp( szCurAction, "Move_Back" ) == NULL || strcmp( szCurAction, "Walk_Back" ) == NULL ) 
				return;

			std::string szAction;
			int nBack = _rand(m_hActor->GetRoom())%2;
			switch( nBack )
			{
				case 0: szAction = "Move_Back"; break;
				case 1: szAction = "Walk_Back"; break;
			}

			if( m_hActor->IsExistAction( szAction.c_str() ) )
			{
#if defined( PRE_FIX_MOVEBACK )								
				if ( m_hActor && m_hActor->IsMonsterActor() && m_pMonsterActor )
					m_pMonsterActor->SetPrevMoveBack( true );
#endif
				m_hActor->CmdAction( szAction.c_str(), 0, g_fBendFrame  );
				return;
			}
		}

		// If blocked/defended
		if ( strstr( m_hActor->GetCurrentAction(), "Defense" ) )
		{
			m_nState = AT_Defense;
			return;
		}

		// If damaged and damage table exists 
		if ( m_hActor->IsHit() && m_hActor->IsStiff() )
		{
			if ( m_Data.m_NonDownMeleeDamageActionTable.empty() == false || m_Data.m_NonDownRangeDamageActionTable.empty() == false )	
			{
				m_nState = AT_Damage;
				return;
			}
		}

		bool bResult = m_hActor->IsProcessSkill();
		if ( bResult == false )
		{
			for( UINT i=0 ; i<m_Data.m_AISkillProcessorTable.size() ; ++i )
			{
				const AISkillProcessor& at = m_Data.m_AISkillProcessorTable.at(i);
				at.pMAAiProcessorManager->End();
			}
		}
		else
		{
			for( UINT i=0 ; i<m_Data.m_AISkillProcessorTable.size() ; ++i )
			{
				const AISkillProcessor& at = m_Data.m_AISkillProcessorTable.at(i);

				if( at.nSkillIndex == m_hActor->GetProcessSkill()->GetClassID() )
				{
					at.pMAAiProcessorManager->Process( m_hActor );		
				}
			}
		}

		if ( m_hActor->IsStay() && m_hActor->IsMovable() )
		{
//			if( m_hActor->GetLookTarget() != m_hTarget )  
//				m_hActor->CmdLook(m_hTarget);
			// This is the AI branch point. 
			OnDistanceState(state, fLength, time);
		}
	}
	// Wander if no target
	else
	{
		if( m_bEnableAggroProcess && m_pMonsterActor->GetAggroSystem() )
		{
			if( !static_cast<CDNMonsterAggroSystem*>(m_pMonsterActor->GetAggroSystem())->bIsInit() )
				return;
		}

#ifdef PRE_ADD_AI_NOAGGRO_STAND
		if (m_Data.m_Type & ScriptData::eTypeBit::NoAggroStand)
		{
			if (m_hActor && m_hActor->IsMonsterActor())
			{
				if (m_hActor->IsMove())
					m_hActor->CmdStop("Stand");
				return;
			}
		}
#endif

		if( m_pReceiver->GetAIState() == MAAiReceiver::Disable ) return;
		if( _bDestinationProcess() )
			return;

		if ( IsDelay(AIDelayType::Patrol) ) 
			return;

		// NonTargetAI
		bool bPatrol = false;
		if( m_pMonsterActor )
			bPatrol = m_pMonsterActor->OnAINonTarget();

		if( bPatrol )
		{
			if ( m_Data.m_nPatrolBaseTime < 1 ||  m_Data.m_nPatrolRandTime < 1)
				SetDelay( AIDelayType::Patrol, 5000 + _rand(m_hActor->GetRoom())%3000 );
			else
				SetDelay( AIDelayType::Patrol, m_Data.m_nPatrolBaseTime + _rand(m_hActor->GetRoom())%m_Data.m_nPatrolRandTime );
		}
	}
}

int MAAiScript::GetTargetDistanceState( DnActorHandle hTarget )
{
	if( !hTarget )
		return -1;

	EtVector3 *pvPos = m_hActor->GetPosition();

#ifdef PRE_FIX_PARTSMONSTER_AI_TARGETTING
	float fLengthSq = EtVec3LengthSq( &( *pvPos - hTarget->FindAutoTargetPos() ) );
#else
	float fLengthSq = EtVec3LengthSq( &( *pvPos - *hTarget->GetPosition() ) );
#endif

#if defined( PRE_ADD_ACADEMIC )
	if( m_hActor->IsMonsterActor() )
	{
		CDnMonsterActor* pMonster = static_cast<CDnMonsterActor*>(m_hActor.GetPointer());
		if( pMonster->GetSummonerPlayerActor() )
		{
			fLengthSq -= pow((m_hActor->GetUnitSize()+hTarget->GetUnitSize())*1.f,2);
			if( fLengthSq < 0.f )
				fLengthSq = 0.f;
		}
	}
#endif // #if defined( PRE_ADD_ACADEMIC )

	int nSize = (int)m_Data.m_AITable.size();
	if ( nSize < 2 )
		return -1;

	if ( fLengthSq >= 0.0f && fLengthSq < m_Data.m_AITable[0].fNearValueSq )
		return 0;

	for ( int i = 0 ; i < nSize-1 ; i++ )
	{
		if ( fLengthSq >= m_Data.m_AITable[i].fNearValueSq && fLengthSq < m_Data.m_AITable[i+1].fNearValueSq )
			return i+1;
	}

	return nSize - 1;
}

void MAAiScript::OnDistanceState(int state, float fLength, LOCAL_TIME time)
{
	// Initialize Target
	m_cMultipleTarget.Reset();

	switch(m_nState)
	{
	case AT_Normal:			OnNormal(state, fLength, time);			break;
	case AT_Assault:		OnAssault(state, fLength, time);		break;
	case AT_CustomAction:	OnCustomAction(state, fLength, time);	break;
	case AT_Defense:		OnDefense(state, fLength, time);		break;
	case AT_Damage:			OnDamage(state, fLength, time);			break;
	case AT_UseSkill:		OnUseSkill(state, fLength, time);		break;
	}
		

}

void MAAiScript::_CmdActionTable( const ActionTable* pTable, float fLength, int iLoop, ActionType State/*=AT_Normal*/ )
{
	if( pTable->Type == ActionTableCommon::eType::Skill )
	{
		bool bResult = m_SkillAI.UseSkill( pTable->nLuaSkillIndex );
		if ( bResult )
		{
			m_nState = AT_UseSkill;
			return;
		}
	}
	else
	{
		std::string szAction = pTable->szActionName;
		_CmdAction(szAction.c_str(), fLength, iLoop);

	}

	m_nState = State;
}

void MAAiScript::_CmdFinishProcessSkill( LOCAL_TIME time )
{
	DnSkillHandle hSkill = m_hActor->GetProcessSkill();
	if( !hSkill )
		return;

	int nSkillID = hSkill->GetClassID();

	m_hActor->ResetProcessSkill();

	// Packet
	BYTE pBuffer[128];
	CPacketCompressStream Stream( pBuffer, 128 );

	Stream.Write( &nSkillID, sizeof(int) );

	m_hActor->Send( eActor::SC_FINISH_PROCESSSKILL, &Stream );
}

void MAAiScript::OnAssault(int state, float fLength, LOCAL_TIME time)
{
	int nIndex = m_nSeletedAssaultActionIndex;

	if ( m_Data.m_AssaultActionTable.empty() || nIndex < 0 || nIndex >= (int)m_Data.m_AssaultActionTable.size() )
	{
		m_nState = AT_Normal;
		return;
	}
		
	// When close during charge, pick an action from the charge table and perform it.
	int nLoop = m_Data.m_AssaultActionTable[nIndex].nLoop;
	if( nLoop > 0 ) nLoop = _rand(m_hActor->GetRoom())%nLoop;

	_CmdActionTable( &m_Data.m_AssaultActionTable[nIndex], fLength, nLoop );
}

int MAAiScript::SelectAction_Random(std::vector<ActionTable*>& AtArray, LOCAL_TIME time)
{
	if ( AtArray.empty() )
		return -1;

	int nRateSum = 0;
	int nCount	 = static_cast<int>(AtArray.size());
	for( int i=0 ; i<nCount ; ++i )
		nRateSum += AtArray[i]->nRate;

	if( nRateSum == 0 )
		return -1;

	int nRand	= _rand(m_hActor->GetRoom())%nRateSum;
	int nSum	= AtArray[0]->nRate;
	int nIndex	= (nCount-1);
	
	for( int i=1 ; i<nCount ; ++i )
	{
		if( nRand < nSum )
		{
			nIndex = i-1;
			break;
		}

		nSum += AtArray[i]->nRate;
	}

#ifdef PRE_ADD_MONSTER_NORMAL_ACTION_PRIORITY
	int nPriorityValue = 0;
	for( int i=0 ; i<nCount ; ++i )
	{
		if( AtArray[i]->nActionPriority > nPriorityValue )
		{
			nPriorityValue = AtArray[i]->nActionPriority;
			nIndex = i;
		}
	}
#endif
	
	AtArray[nIndex]->nLastTime = time;
	
	return AtArray[nIndex]->nIdx;
}

// Returns -1 if no action is available.
int MAAiScript::SelectAction(std::vector<ActionTable>& AtArray, LOCAL_TIME time)
{
	std::vector<ActionTable*> UsableAT;
	UsableAT.reserve( AtArray.size() );

	for( UINT i=0 ; i<AtArray.size() ; ++i )
	{
		if( !m_hActor->IsLimitAction( AtArray[i].szActionName.c_str() ) && _bIsTargetActorChecker( AtArray[i] ) && IsPassedCoolTime(AtArray[i], time) && IsCheckedTargetCondition(AtArray[i], this->GetTarget()) && _bIsActorChecker( AtArray[i] ) )
		{ 
			UsableAT.push_back( &(AtArray[i]) );
		}
	}

	return SelectAction_Random( UsableAT, time );
}

bool MAAiScript::_bIsActorChecker( const ActionTable& AT )
{
	return AT.pMAAiCheckerManager->bIsActorChecker( m_hActor );
}

bool MAAiScript::_bIsTargetActorChecker( const ActionTable& AT )
{
	return AT.pMAAiCheckerManager->bIsTargetActorChecker( m_hActor, m_hTarget );
}

// return true if destination is set, otherwise return false;
bool MAAiScript::_bDestinationProcess()
{
	SOBB	ChangeObb;
	UINT	uiMoveFrontRate = m_uiMoveFrontRate;
	SOBB*	pObb			= m_pDestinationOBB;

#if defined( PRE_ADD_ACADEMIC )
	if( m_Data.m_Type&ScriptData::eTypeBit::NoAggroOwnerFollow )
	{
		if( m_hActor->IsMonsterActor() )
		{
			CDnMonsterActor* pMonster = static_cast<CDnMonsterActor*>(m_hActor.GetPointer());
			DnActorHandle hSummonerPlayerActor = pMonster->GetSummonerPlayerActor();
			if( hSummonerPlayerActor )
			{
				ChangeObb.Center = *hSummonerPlayerActor->GetPosition();
				ChangeObb.Axis[0] = EtVector3( 1.f, 0.f, 0.f );
				ChangeObb.Axis[1] = EtVector3( 0.f, 1.f, 0.f );
				ChangeObb.Axis[2] = EtVector3( 0.f, 0.f, 1.f );
				ChangeObb.Extent[0] = ChangeObb.Extent[1] = ChangeObb.Extent[2] = 100.f;

				// Change to SummonerPlayer position
				pObb			= &ChangeObb;
				uiMoveFrontRate = 100;

				DWORD dwCurTime = timeGetTime();
				if( dwCurTime-m_dwDestinationOBB < 1000 )
					return true;
				m_dwDestinationOBB = dwCurTime;

			}
		}
	}
#endif // #if defined( PRE_ADD_ACADEMIC )

	if( pObb )
	{
		if( pObb->IsInside( *m_hActor->GetPosition() ) )
		{
			if( m_hActor->IsMove() )
				m_hActor->CmdStop( "Stand" );
			return true;
		}

		if( !m_hActor->IsMove() && m_hActor->IsMovable() )
		{
			UINT uiRandVal = _rand( m_hActor->GetRoom() )%100;
			m_hActor->CmdMoveNavi( pObb->Center, 50.0f, (uiMoveFrontRate<=uiRandVal) ? "Walk_Front" : "Move_Front", -1, g_fBendFrame );
		}
		
		return true;
	}

	return false;
}

bool MAAiScript::IsPassedCoolTime(ActionTable& at, LOCAL_TIME time)
{
	if( at.nLastTime == 0 || at.nLastTime + at.nCoolTime <= time )
		return true;

	return false;
}

bool MAAiScript::IsCheckedTargetCondition(ActionTable& at, DnActorHandle hTarget)
{
	if ( at.szTargetCondition.empty() )
		return true;

	std::map<std::string, StateCondition>::iterator it = m_Data.m_StateContidionMap.find(at.szTargetCondition);
	if ( it == m_Data.m_StateContidionMap.end() )
		return true;

	if ( !GetTarget() )
		return false;

	return (*it).second.Check( GetTarget() );
}

// Get alive users within distance (state) excluding m_hTarget
void MAAiScript::GetTargetDistance( const int state,  DNVector(DnActorHandle)& vTarget, bool bExceptAggroTarget/*=true*/ )
{
	CDNAggroSystem* pAggroSystem = m_pMonsterActor->GetAggroSystem();
	_ASSERT( pAggroSystem );
	pAggroSystem->Convert( vTarget );
	
	for( DNVector(DnActorHandle)::iterator itor=vTarget.begin() ; itor!=vTarget.end() ;  )
	{
		DnActorHandle hTarget = (*itor);
		if( !hTarget || (bExceptAggroTarget && hTarget == m_hTarget) || hTarget->IsDie() || GetTargetDistanceState( hTarget ) != state )
		{
			itor = vTarget.erase( itor );
			continue;
		}

		++itor;
	}
}

// Get alive users within distance (Min~Max) excluding m_hTarget
void MAAiScript::GetTargetDistance( const int iMin, const int iMax, DNVector(DnActorHandle)& vTarget, bool bExceptAggroTarget/*=true*/ )
{
	CDNAggroSystem* pAggroSystem = m_pMonsterActor->GetAggroSystem();
	_ASSERT( pAggroSystem );
	pAggroSystem->Convert( vTarget );

	for( DNVector(DnActorHandle)::iterator itor=vTarget.begin() ; itor!=vTarget.end() ;  )
	{
		DnActorHandle hTarget = (*itor);
		if( !hTarget || (bExceptAggroTarget && hTarget == m_hTarget) || hTarget->IsDie() || !bIsTargetDistance( m_hActor, hTarget, iMin, iMax ) )
		{
			itor = vTarget.erase( itor );
			continue;
		}

		++itor;
	}
}

bool MAAiScript::bIsTargetDistance( DnActorHandle hActor, DnActorHandle hTarget, const int iMin, const int iMax )
{
	if( !hActor || !hTarget )
		return false;

	bool bFindPosition = true;
	float fLengthSq = EtVec3LengthSq( &( *hActor->GetPosition() - *hTarget->GetPosition() ) );

	if( fLengthSq < iMin*iMin || fLengthSq > iMax*iMax )
		bFindPosition = false;

#if defined( PRE_ADD_ACADEMIC )
	if( hActor->IsMonsterActor() )
	{
		CDnMonsterActor* pMonster = static_cast<CDnMonsterActor*>(hActor.GetPointer());

		// PartsMonster detects distance using Collision.
		if( pMonster->GetSummonerPlayerActor() && hTarget->IsPartsMonsterActor() )
		{
			if( hTarget->GetObjectHandle()->FindCollisionPrimitiveCenterInRange( (*hActor->GetPosition()), static_cast<float>(iMin), static_cast<float>(iMax) ) == true )
				bFindPosition = true;
		}
	}
#endif // #if defined( PRE_ADD_ACADEMIC )

	return bFindPosition;
}

// Set random target within AIState range
void MAAiScript::_SelectRandomTarget( const int state, const float fRandomTarget )
{
	if( fRandomTarget == 0.f )
		return;

	DNVector(DnActorHandle) vTarget;
	GetTargetDistance( state, vTarget );
	if( vTarget.empty() )
		return;

	DnActorHandle hPrevTarget = m_hTarget;

	size_t uiRand = _rand(m_hActor->GetRoom())%vTarget.size();
	ChangeTarget( vTarget[uiRand] );
	
	if( fRandomTarget > 0.f )
	{
		CDNAggroSystem* pAggroSystem = m_hActor->GetAggroSystem();
		CDNAggroSystem::AggroStruct* pPrevStruct = pAggroSystem->GetAggroStruct( hPrevTarget );
		if( pPrevStruct )
		{
			CDNAggroSystem::AggroStruct* pStruct = pAggroSystem->GetAggroStruct( vTarget[uiRand] );
			if( pStruct )
			{
				pStruct->iAggro = static_cast<int>(pPrevStruct->iAggro * fRandomTarget);
			}
		}
	}
}

#if defined( PRE_ADD_LOTUSGOLEM )
void MAAiScript::_ProcessParts()
{
	if( m_Data.m_AIPartsProcessor.empty() )
		return;

	if( m_hActor->IsPartsMonsterActor() == false )
	{
		_ASSERT(0);
		return;
	}

	static_cast<CDnPartsMonsterActor*>(m_hActor.GetPointer())->ProcessPartsAI( m_Data.m_AIPartsProcessor );
}
#endif // #if defined( PRE_ADD_LOTUSGOLEM )

void MAAiScript::OnNormal(int state, float fLength, LOCAL_TIME time)
{
	if( m_Data.m_AITable.empty() || state < 0 || state >= static_cast<int>(m_Data.m_AITable.size()) )
		return;

	// Check if the skill is usable, and if so, switch to the skill-using state.

	bool bResult = m_SkillAI.UseSkill();
	if ( bResult )
	{
		m_nState = AT_UseSkill;
		return;
	}

	int nIndex = SelectAction(m_Data.m_AITable[state].VecActionTable, time);
	if( nIndex < 0 )
		return;

	_SelectRandomTarget( state, m_Data.m_AITable[state].VecActionTable[nIndex].fRandomTarget );

	int nLoop = m_Data.m_AITable[state].VecActionTable[nIndex].nLoop;
	if( nLoop > 0 ) nLoop = _rand(m_hActor->GetRoom())%nLoop;
	
	std::string szAction = m_Data.m_AITable[state].VecActionTable[nIndex].szActionName;
	
	m_Data.m_nMinAttackMissRadian	= m_Data.m_AITable[state].VecActionTable[nIndex].nMinAttackMissRadian;
	m_Data.m_nMaxAttackMissRadian	= m_Data.m_AITable[state].VecActionTable[nIndex].nMaxAttackMissRadian;

	if ( szAction == "Assault" )
	{
		if ( m_Data.m_AssaultActionTable.empty() )
			return;

		nIndex = SelectAction(m_Data.m_AssaultActionTable, time);
		if( nIndex < 0 )
			return;

		m_nState						= AT_Assault;
		m_ChangeStateTime				= time;
		m_nSeletedAssaultActionIndex	= nIndex;

		float fApproach = m_Data.m_AssaultActionTable[m_nSeletedAssaultActionIndex].fApproach;

		// Even in charge mode, charge only by the approach value for now.
		m_hActor->CmdMove( m_hTarget, fApproach, "Move_Front", -1, g_fBendFrame );			

		//g_Log.LogA( "Monster assault!!!\r\n" );
		return;
	}

	if ( szAction.find("CustomAction") < szAction.size() )
	{
		// If it is a custom action 
		if ( _FindCustomAction(szAction.c_str(), m_Data.m_CurrentCustomActionTable ) )
		{
//			OutputDebug("CustomAction : %s\n", szAction.c_str());
			m_Data.m_nCurrentCustomActionPlayIndex = 0;
			m_nState = AT_CustomAction;
			return;
		}
	}


	// Set Multiple target action
	m_cMultipleTarget.SetMultipleTargetAction( &m_Data.m_AITable[state].VecActionTable[nIndex] );

	_CmdAction(szAction.c_str(), fLength, nLoop);

#if defined( PRE_FIX_68096 )
	for( int i=0;i<ActionTableCommon::eCommon::MaxGlobalCoolTime;i++ )
	{
		int iGlobalCoolTimeIndex = m_Data.m_AITable[state].VecActionTable[nIndex].iGlobalCoolTimeIndex[i];
		DWORD dwTime = timeGetTime();

		if( iGlobalCoolTimeIndex >= 0 )
		{
			if( iGlobalCoolTimeIndex < static_cast<int>(GetScriptData().m_AIGlobalCoolTime.size()) )
				m_GlobalCoolTime.AddCoolTime( iGlobalCoolTimeIndex, dwTime+(GetScriptData().m_AIGlobalCoolTime[iGlobalCoolTimeIndex].dwCoolTime ) );
			else
				_ASSERT(0);
		}
	}
#else
	int iGlobalCoolTimeIndex = m_Data.m_AITable[state].VecActionTable[nIndex].iGlobalCoolTimeIndex;
	DWORD dwTime = timeGetTime();

	if( iGlobalCoolTimeIndex >= 0 )
	{
		if( iGlobalCoolTimeIndex < static_cast<int>(GetScriptData().m_AIGlobalCoolTime.size()) )
			m_GlobalCoolTime.AddCoolTime( iGlobalCoolTimeIndex, dwTime+(GetScriptData().m_AIGlobalCoolTime[iGlobalCoolTimeIndex].dwCoolTime ) );
		else
			_ASSERT(0);
	}
#endif
}

void MAAiScript::_CmdAction(const char* szAction, float fLength, int nLoop)
{
	// if( m_hActor->GetLookTarget() != m_hTarget ) m_hActor->CmdLook( m_hTarget );

	if( strstr( szAction, "Front" ) ) 
	{
		if ( !m_hTarget )
			return;

		float fAttackDist = m_hTarget->GetUnitSize() + m_hActor->GetUnitSize() + ( ( m_hActor->GetWeapon() ) ? m_hActor->GetWeapon()->GetWeaponLength() : 0.f );
		if( fLength <=  fAttackDist )
			return;
		
		//m_hActor->CmdLook( m_hTarget, false );
		m_hActor->CmdMove( m_hTarget, m_Data.m_fApproachValue, szAction, nLoop, g_fBendFrame);

	}
	else if( strstr( szAction, "Attack" ) ) 
	{
		// MissAttack
		if( m_hTarget && m_Data.m_nMaxAttackMissRadian > 0 )
		{
			m_vAILook = EtVec3toVec2( *m_hTarget->GetPosition() - *m_hActor->GetPosition() );

			// Set Miss angle
			int nRadian = (_rand(m_hActor->GetRoom())%(m_Data.m_nMaxAttackMissRadian-m_Data.m_nMinAttackMissRadian+1))+m_Data.m_nMinAttackMissRadian;
			if( _rand(m_hActor->GetRoom())%2 )
				nRadian *= -1;
		
			// Set AILook
			m_bIsAILook	= true;
			m_vAILook.x = m_vAILook.x * cos( D3DXToRadian(nRadian)) - m_vAILook.y * sin( D3DXToRadian(nRadian) );
			m_vAILook.y = m_vAILook.x * sin( D3DXToRadian(nRadian)) + m_vAILook.y * cos( D3DXToRadian(nRadian) );
			EtVec2Normalize( &m_vAILook, &m_vAILook );

			m_strAILookAction = szAction;
			//g_Log.LogA( "MissAttack : %s Radian=%d\r\n", szAction, nRadian );
		}

		m_hActor->CmdAction( szAction, nLoop, g_fBendFrame );
	}
	else
	{
//		if( m_hActor->GetLookTarget() != m_hTarget ) 
//			m_hActor->CmdLook( m_hTarget );

		m_hActor->CmdAction(szAction, nLoop , g_fBendFrame);
	}
}

void MAAiScript::OnCustomAction(int state, float fLength, LOCAL_TIME time)
{
	if ( m_Data.m_nCurrentCustomActionPlayIndex == -1 )
		return;

	// Once the custom action finishes playing, switch to normal state.
	if ( m_Data.m_nCurrentCustomActionPlayIndex >= (int)m_Data.m_CurrentCustomActionTable.m_ActionList.size() )
	{
		m_nState = AT_Normal;
		m_Data.m_nCurrentCustomActionPlayIndex = -1;
		return;
	}

	std::string szCurAction = m_Data.m_CurrentCustomActionTable.m_ActionList[m_Data.m_nCurrentCustomActionPlayIndex].szActionName;
	int nCount = m_Data.m_CurrentCustomActionTable.m_ActionList[m_Data.m_nCurrentCustomActionPlayIndex].nLoop;
	
	//m_hActor->CmdAction(szCurAction.c_str(), nCount);
	if( stricmp( szCurAction.c_str(), "useskill") == 0 )
	{
		m_SkillAI.UseSkill( m_Data.m_CurrentCustomActionTable.m_ActionList[m_Data.m_nCurrentCustomActionPlayIndex].nLuaSkillIndex );
	}
	else
	{
		_CmdAction(szCurAction.c_str(), fLength, nCount);
	}
	m_Data.m_nCurrentCustomActionPlayIndex++;

	/*
	for ( int i = 0 ; i < (int)m_Data.m_CurrentCustomActionTable.m_ActionList.size() ; i++ )
	{
OutputDebug("CustomAction : %s, #%d ", m_Data.m_CurrentCustomActionTable.m_ActionList[i].szActionName.c_str(),
			m_Data.m_CurrentCustomActionTable.m_ActionList[i].nLoop );
	}
	OutputDebug("\n");
	OutputDebug("Play OnCustomAction : %s[%d] \n", szCurAction.c_str(), nCount);
	*/

}


void MAAiScript::OnDefense(int state, float fLength, LOCAL_TIME time)
{
	if ( m_Data.m_MeleeDefenseActionTable.empty() && m_Data.m_RangeDefenseActionTable.empty() )
	{
		m_nState = AT_Normal;
		return;
	}

	CDnDamageBase::SHitParam* pHitParam = m_hActor->GetHitParam();
	if( !pHitParam )
	{
		m_nState = AT_Normal;
		return;
	}

	std::vector<ActionTable>* pDefenseTable = NULL;
	if ( pHitParam->DistanceType == CDnDamageBase::DistanceTypeEnum::Range )
		pDefenseTable = &(m_Data.m_RangeDefenseActionTable);
	else
		pDefenseTable = &(m_Data.m_MeleeDefenseActionTable);

	if( pDefenseTable->empty() )
	{
		m_nState = AT_Normal;
		return;
	}

	std::vector<ActionTable>& DefenseTable = *pDefenseTable;
	int nIndex = SelectAction(DefenseTable, time);
	if( nIndex < 0 )
	{
		m_nState = AT_Normal;
		return;
	}

	int nLoop = DefenseTable[nIndex].nLoop;
	if( nLoop > 0 ) nLoop = _rand(m_hActor->GetRoom())%nLoop;

	_CmdActionTable( &DefenseTable[nIndex], fLength, nLoop );
}

void MAAiScript::OnDamage(int state, float fLength, LOCAL_TIME time)
{
	if ( m_Data.m_NonDownMeleeDamageActionTable.empty() && m_Data.m_NonDownRangeDamageActionTable.empty() )
	{
		m_nState = AT_Normal;
		return;
	}

	CDnDamageBase::SHitParam* pHitParam = m_hActor->GetHitParam();
	if( !pHitParam )
	{
		m_nState = AT_Normal;
		return;
	}

	std::vector<ActionTable>* pDefenseTable = NULL;
	if ( pHitParam->DistanceType == CDnDamageBase::DistanceTypeEnum::Range )
		pDefenseTable = &(m_Data.m_NonDownRangeDamageActionTable);
	else
		pDefenseTable = &(m_Data.m_NonDownMeleeDamageActionTable);

	if ( pDefenseTable->empty() )
	{
		m_nState = AT_Normal;
		return;
	}

	std::vector<ActionTable>& DefenseTable = *pDefenseTable;
	int nIndex = SelectAction(DefenseTable, time);
	if( nIndex < 0 )
	{
		m_nState = AT_Normal;
		return;
	}

	int nLoop = DefenseTable[nIndex].nLoop;
	if( nLoop > 0 ) nLoop = _rand(m_hActor->GetRoom())%nLoop;

	_CmdActionTable( &DefenseTable[nIndex], fLength, nLoop );
}

void MAAiScript::OnHitFinish( LOCAL_TIME LocalTime, HitStruct *pStruct )
{
	// So far, BeHitSkill is the only thing handled by AI when Hit succeeds
	// Return if not currently using a skill.
	if( !m_hActor->IsProcessSkill() )
		return;
	
	std::vector<ActionTable>* pBeHitSkillTable = &m_Data.m_BeHitSkillActionTable;
	if( pBeHitSkillTable->empty() )
		return;

	std::vector<ActionTable> SelectTable;
	for( UINT i=0 ; i<pBeHitSkillTable->size() ; ++i )
	{
		 const ActionTable& t = pBeHitSkillTable->at(i);

		 if( t.nBeHitSkillIndex == m_hActor->GetProcessSkill()->GetClassID() )
			 SelectTable.push_back( t );
	}

	if( SelectTable.empty() )
		return;

	int nIndex = SelectAction( SelectTable, LocalTime );
	if( nIndex < 0 )
		return;

#if defined( PRE_FIX_HITFINISH )
	for( UINT i=0 ; i<SelectTable.size() ; ++i )
	{
		if( nIndex == SelectTable[i].nIdx )
		{
			nIndex = i;
			break;
		}
	}
	if( nIndex >= SelectTable.size() )
		return;
#endif
	if( SelectTable[nIndex].Type == ActionTableCommon::eType::Skill )
		m_hActor->OnFinishAction( m_hActor->GetCurrentAction(), LocalTime );

	_CmdFinishProcessSkill( LocalTime );
	_CmdActionTable( &SelectTable[nIndex], 0.f, 0, AT_UseSkill );
}

void MAAiScript::OnUseSkill(int state, float fLength, LOCAL_TIME time)
{
	bool bResult = m_hActor->IsProcessSkill();
	if ( bResult == false )
	{
		for( UINT i=0 ; i<m_Data.m_AISkillProcessorTable.size() ; ++i )
		{
			const AISkillProcessor& at = m_Data.m_AISkillProcessorTable.at(i);

			at.pMAAiProcessorManager->End();
		}

		// If there is a chained skill to execute, run it immediately.
		if( m_SkillAI.GetNextLuaSkillIndex() >= 0 )
		{
			m_SkillAI.UseSkill( m_SkillAI.GetNextLuaSkillIndex() );
			return;
		}

		m_nState = AT_Normal;
	}
	else
	{
		for( UINT i=0 ; i<m_Data.m_AISkillProcessorTable.size() ; ++i )
		{
			const AISkillProcessor& at = m_Data.m_AISkillProcessorTable.at(i);

			if( at.nSkillIndex == m_hActor->GetProcessSkill()->GetClassID() )
			{
				at.pMAAiProcessorManager->Process( m_hActor );		
			}
		}
	}
}

void MAAiScript::OnBeginNaviMode( const char* szPrevAction, const bool bPrevIsMove )
{
	DnActorHandle hTarget = GetTarget();

	if( !m_hActor->IsNaviMode() && hTarget && bPrevIsMove && (strstr( szPrevAction, "Front" )||strstr( szPrevAction, "Back")) )
	{
		const char* pszActionName = "Walk_Front";
		if( m_hActor->IsExistAction( pszActionName) )
		{
			m_hActor->CmdMoveNavi( hTarget, 50.0f, pszActionName, -1, g_fBendFrame );
		}
		else
		{
			_ASSERT(0);
		}
	}
}

void MAAiScript::AICmdMsgProc( const UINT uiMsg, void* pParam )
{
	if( !m_pAICommand )
	{
		_DANGER_POINT();
		return;
	}

	m_pAICommand->MsgProc( this, uiMsg, pParam );
}

bool MAAiScript::_FindCustomAction(const char* szCustomActionName, OUT CustomActionTable& table)
{
	std::map<std::string, CustomActionTable>::iterator it = m_Data.m_CustomActionMap.find(std::string(szCustomActionName));
	if ( it == m_Data.m_CustomActionMap.end() )
		return false;

	table = it->second;
	return true;
}


bool MAAiScript::LoadScript(const char* szFilename)
{
	m_bValidAI = g_AiScriptLoader.GetScript(szFilename, m_Data);
	if ( !m_bValidAI )
		return false;

#if defined(PRE_ADD_64990)
	int nType = 0;

	CDNGameRoom *pRoom = m_hActor->GetGameRoom();
	
	if( pRoom && pRoom->bIsPvPRoom() )
		nType = 1;
	
	m_SkillAI.Initialize(&(m_Data.m_SkillTable[nType]), m_hActor);
#else
	m_SkillAI.Initialize(&(m_Data.m_SkillTable), m_hActor);
#endif // PRE_ADD_64990
	return m_bValidAI;

}

void MAAiScript::ReloadAllScript()
{
	g_AiScriptLoader.AllLoadScript();
}

// Called when STE_ProjectileTargetPosition signal is invoked
void MAAiScript::_OnSetProjectileTarget()
{
	m_cMultipleTarget.CalcTarget( m_hActor, this );
}

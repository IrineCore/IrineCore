#include "stdafx.h"
#include "DnDataManager.h"

#include "TaskManager.h"
#include "DnQuestTask.h"
#include "DnTableDB.h"
#include "DnActor.h"
#include "DnInterface.h"
#include "DnUIString.h"
#include "DnItemTask.h"
#include "DnCharInventory.h"
#include "DnCashInventory.h"
#include "DnQuestTask.h"
#include "SyncTimer.h"
#include "DnInterfaceString.h"
#include "Timeset.h"
#include "DnNPCActor.h"
#include "DnCommonTask.h"
#include "DnPlayerActor.h"
#include "DnWorldData.h"

#ifdef PRE_FIX_MEMOPT_EXT
#include "DnCommonUtil.h"
#endif

#ifdef PRE_ADD_LOWLEVEL_QUEST_HIDE
#include "GameOption.h"
#endif 

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnDataManager g_DataManager;


CDnDataManager::CDnDataManager()
{

}

CDnDataManager::~CDnDataManager()
{
	ClearAll();
}

void CDnDataManager::ClearAll()
{
	m_TalkMap.clear();
	_RemoveAllJournalData();
	_RemoveAllQuestCondition();

	for (TMapActorData::iterator iterActor = m_pActorData.begin(); iterActor != m_pActorData.end(); ++iterActor){
		SAFE_DELETE(iterActor->second);
	}
	m_pActorData.clear();

	for (TMapNpcData::iterator iterNpc = m_pNpcData.begin(); iterNpc != m_pNpcData.end(); ++iterNpc){
		SAFE_DELETE(iterNpc->second);
	}
	m_pNpcData.clear();

}

bool CDnDataManager::LoadData()
{
	LoadActorData();
	LoadNpcData();

	_LoadTalkParamTable();
	_LoadQuestList();
	_LoadPeriodQuestList();

	LoadQuestRecompense();
	LoadQuestLevelCapRecompense();

#if defined(PRE_OPT_CLIENT_QUEST_SCRIPT)
#else
	_LoadTalkData();
#endif

	return true;
}

bool CDnDataManager::_LoadQuestList()
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TQUEST);

	if ( !pSox )
		return false;

	for( int i=0; i<pSox->GetItemCount(); i++ ) 
	{
		char szTemp[256] = {0,};
		memset(&szTemp, 0, sizeof(szTemp));

		int nTableID = pSox->GetItemID(i);
		std::string questname = pSox->GetFieldFromLablePtr( nTableID, "_Name" )->GetString();
		std::string questtalk = pSox->GetFieldFromLablePtr( nTableID, "_QuestTalk" )->GetString();
		std::string questscript = pSox->GetFieldFromLablePtr( nTableID, "_QuestScript" )->GetString();

		QuestInfo info;
		info.nQuestIndex = nTableID;
		ToWideString(questname, info.szQuestName);
		ToWideString(questtalk, info.szQuestTalk);
		ToWideString(questscript, info.szQuestScript);
		ToLowerW(info.szQuestTalk);
		ToLowerW(info.szQuestScript);

		info.nSortIndex = pSox->GetFieldFromLablePtr( nTableID, "_QSortIndex" )->GetInteger();
		info.cQuestType = pSox->GetFieldFromLablePtr( nTableID, "_QuestType" )->GetInteger();
		info.nMapIndex = pSox->GetFieldFromLablePtr( nTableID, "_Map" )->GetInteger();
		info.bLevelCapReward = pSox->GetFieldFromLablePtr( nTableID, "_LevelCapReward" )->GetInteger() == 1 ? true : false;

		m_QuestInfoMap.insert(make_pair(info.szQuestTalk, info));
	}

	return true;
}

bool CDnDataManager::_LoadPeriodQuestList()
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TQUESTPERIOD );

	if ( !pSox )
		return false;

	for( int itr = 0; itr < pSox->GetItemCount(); ++itr ) 
	{
		PeriodQuestInfo periodQuestInfo;
		char * szTemp;
		std::vector< string > vQuestID;

		int nTableID = pSox->GetItemID( itr );

		periodQuestInfo.nType = pSox->GetFieldFromLablePtr( nTableID, "_DailyQuestType" )->GetInteger();
		szTemp = pSox->GetFieldFromLablePtr( nTableID, "_RandomQuestID" )->GetString();

		TokenizeA( szTemp, vQuestID, ":" );

		for( std::vector<string>::iterator itor = vQuestID.begin(); itor != vQuestID.end(); ++itor )
			periodQuestInfo.vecQuestID.push_back( atoi( (*itor).c_str() ) );

		m_vPeriodQuestInfo.push_back( periodQuestInfo );
	}

	return true;
}

void CDnDataManager::InitNpcTalkData( int nMapTableID )
{
	// Manually register the start index called first
	std::wstring _szStart( NpcTalkStringGroup[NPCTALK_START] );
	DWORD nHashCode = GetStringHashCode(_szStart.c_str());
	m_XMLIndexMap.insert( make_pair(nHashCode, _szStart ) );
	
#ifdef _DEBUG
	// Log the filename of the non-existent NpcFile.
	wchar_t fileName[32] = L"log\\NpcFile.log";	
	m_fp = _wfopen( fileName, L"w" );
#endif

	CDnWorldData *pWorldData = CDnWorldDataManager::GetInstance().GetWorldData( nMapTableID );
	if( pWorldData )
		pWorldData->LoadNpcTalk();
	
#ifdef _DEBUG
	fclose( m_fp );
#endif

}

void CDnDataManager::ClearTalkData()
{
	m_XMLFileMap.clear();
	m_XMLIndexMap.clear();
	m_TalkMap.clear();

	_RemoveAllJournalData();
	_RemoveAllQuestCondition();
}

bool CDnDataManager::LoadNpcTalkData( int nNpcIndex )
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TNPC );

	if( !pSox )
		return false;

	char szTalkFileName[256] = {0,};
	WCHAR wszTalkFileName[256] = {0,};
	_strcpy(szTalkFileName, _countof(szTalkFileName), pSox->GetFieldFromLablePtr(nNpcIndex, "_TalkFile")->GetString(), (int)strlen(pSox->GetFieldFromLablePtr(nNpcIndex, "_TalkFile")->GetString()));
	MultiByteToWideChar(CP_ACP, 0, szTalkFileName, -1, wszTalkFileName, 256 );

	return _AddTalk( wszTalkFileName );
}

bool CDnDataManager::LoadNpcQuestData( int nNpcIndex )
{
	TNpcData * pNpc = GetNpcData( nNpcIndex );

	if( NULL == pNpc )
		return false;

	for(int itr = 0; itr < QUEST_MAX_CNT ; itr++)
	{
		QuestInfo* pInfo = FindQuestInfo( pNpc->QuestIndexArr[itr] );
		if ( pInfo )
		{
			if( false == _AddTalk( pInfo->szQuestTalk.c_str() ) )
			{
				OutputDebug( "CDnDataManager::LoadNpcQuestData - Failed!!" );
				return false;
			}
		}

		pInfo = FindQuestInfo( pNpc->ReputeQuestIndexArr[itr] );
		if ( pInfo )
		{
			if( false == _AddTalk( pInfo->szQuestTalk.c_str() ) )
			{
				OutputDebug( "CDnDataManager::LoadNpcQuestData - Failed!!" );
				return false;
			}
		}
	}

	return true;
}

bool CDnDataManager::LoadQuestData( int nQuestID )
{
	QuestInfo* pInfo = FindQuestInfo( nQuestID );
	if ( pInfo )
	{
		if( false == _AddTalk( pInfo->szQuestTalk.c_str() ) )
		{
			OutputDebug( "CDnDataManager::LoadNpcQuestData - Failed!!" );
			return false;
		}
	}

	return true;
}

bool CDnDataManager::_LoadTalkData()
{
	m_XMLFileMap.clear();
	m_XMLIndexMap.clear();

	// Manually register the start index called first
	std::wstring _szStart(L"start");
	DWORD nHashCode = GetStringHashCode(_szStart.c_str());
	m_XMLIndexMap.insert( make_pair(nHashCode, _szStart ) );

	/*
	std::string sss;
	std::wstring wsss;

	sss = CEtResourceMng::GetInstance().GetFullPath(CFileNameString("Talk_Npc")).c_str();

	ToWideString(sss, wsss);
	LogWnd::Log(3, L"Load Npc Folder : %s ", wsss.c_str());

	CEtResourceMng::GetInstance().FindFileListInFolder( sss.c_str(), "*.xml", FileList);
	*/

	std::vector<CFileNameString> FileList;
	CEtResourceMng::GetInstance().FindFileListAll_IgnoreExistFile( "Talk_Npc", "*.xml", FileList );

	for ( size_t i = 0 ; i < FileList.size() ; i++ )
	{
		WCHAR wszFile[512];
		ZeroMemory(&wszFile, sizeof(WCHAR)*512);
		MultiByteToWideChar(CP_ACP, 0, FileList[i].c_str(), -1, wszFile, 512 );

		// 20100624 Added extraction of filename only from full file path (b4nfter)
		{
			TCHAR szFileName[MAX_PATH] = { _T('\0'), };
			TCHAR szExecName[MAX_PATH] = { _T('\0'), };

			errno_t nRetVal = _wsplitpath_s(wszFile, NULL, 0, NULL, 0, szFileName, _countof(szFileName), szExecName, _countof(szExecName));
			if (nRetVal) {
				// Error occurred
				return false;
			}

			_sntprintf_s(wszFile, _countof(wszFile), _T("%s%s"), szFileName, szExecName);
		}

		_AddTalk(wszFile);
	}

	FileList.clear();

	/*
	sss.clear();
	sss = CEtResourceMng::GetInstance().GetFullPath(CFileNameString("Talk_Quest")).c_str();

	ToWideString(sss, wsss);
	LogWnd::Log(3, L"Load Quest Folder : %s ", wsss.c_str());

	CEtResourceMng::GetInstance().FindFileListInFolder( sss.c_str(), "*.xml", FileList);
	*/

	CEtResourceMng::GetInstance().FindFileListAll_IgnoreExistFile( "Talk_Quest", "*.xml", FileList );

	for ( size_t i = 0 ; i < FileList.size() ; i++ )
	{
		WCHAR wszFile[512];
		ZeroMemory(&wszFile, sizeof(WCHAR)*512);
		MultiByteToWideChar(CP_ACP, 0, FileList[i].c_str(), -1, wszFile, 512 );

		// 20100624 Added extraction of filename only from full file path (b4nfter)
		{
			TCHAR szFileName[MAX_PATH] = { _T('\0'), };
			TCHAR szExecName[MAX_PATH] = { _T('\0'), };

			errno_t nRetVal = _wsplitpath_s(wszFile, NULL, 0, NULL, 0, szFileName, _countof(szFileName), szExecName, _countof(szExecName));
			if (nRetVal) {
				// Error occurred
				return false;
			}

			_sntprintf_s(wszFile, _countof(wszFile), _T("%s%s"), szFileName, szExecName);
		}

		_AddTalk(wszFile);
	}

	return true;
}

bool CDnDataManager::_LoadTalkParamTable()
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TTALKPARAM );

	if ( !pSox )
		return false;

	for( int i=0; i<pSox->GetItemCount(); i++ ) 
	{
		int nTableID = pSox->GetItemID(i);
		char *pString = pSox->GetFieldFromLablePtr( nTableID, "String" )->GetString();
		std::string str = (pString == NULL ) ? "" : pString;
		if ( str.empty() == false )
		{
			std::wstring wstr;
			ToWideString(str, wstr);

			m_TalkParamMap.insert(make_pair(nTableID, wstr));
		}
	}

	m_StaticTalkParamList.clear();
	m_StaticTalkParamList.push_back( __StaticTalkParamInfo( __StaticTalkParamInfo::user_nick,	L"user_nick") );
	m_StaticTalkParamList.push_back( __StaticTalkParamInfo( __StaticTalkParamInfo::user_class,	L"user_class") );
	m_StaticTalkParamList.push_back( __StaticTalkParamInfo( __StaticTalkParamInfo::monster,		L"monster") );
	m_StaticTalkParamList.push_back( __StaticTalkParamInfo( __StaticTalkParamInfo::npc,			L"npc") );
	m_StaticTalkParamList.push_back( __StaticTalkParamInfo( __StaticTalkParamInfo::item,		L"item") );
	m_StaticTalkParamList.push_back( __StaticTalkParamInfo( __StaticTalkParamInfo::skill,		L"skill") );
	m_StaticTalkParamList.push_back( __StaticTalkParamInfo( __StaticTalkParamInfo::map,			L"map") );
	m_StaticTalkParamList.push_back( __StaticTalkParamInfo( __StaticTalkParamInfo::action,		L"action") );
	m_StaticTalkParamList.push_back( __StaticTalkParamInfo( __StaticTalkParamInfo::sound,		L"sound") );
	m_StaticTalkParamList.push_back( __StaticTalkParamInfo( __StaticTalkParamInfo::uistring,	L"ui_string") );
	m_StaticTalkParamList.push_back( __StaticTalkParamInfo( __StaticTalkParamInfo::repair_cost,	L"repair_price") );
	m_StaticTalkParamList.push_back( __StaticTalkParamInfo( __StaticTalkParamInfo::repair_cost_all,	L"repair_all_price") );
	m_StaticTalkParamList.push_back( __StaticTalkParamInfo( __StaticTalkParamInfo::money,		L"money"));
	m_StaticTalkParamList.push_back( __StaticTalkParamInfo( __StaticTalkParamInfo::quest_level,	L"quest_level"));


	return true;
}

std::wstring CDnDataManager::_GetTalkParam(int nIndex)
{
	TALK_PARAM_MAP_IT iter = m_TalkParamMap.find(nIndex);
	if ( iter != m_TalkParamMap.end() )
	{
		std::wstring wstr = iter->second;
		return wstr;
	}
	return std::wstring(L"");
}

QuestInfo* CDnDataManager::FindQuestInfo(std::wstring& szTalkFilename)
{
	QUEST_INFO_MAP_IT iter = m_QuestInfoMap.find(szTalkFilename);
	if ( iter != m_QuestInfoMap.end() )
	{
		QuestInfo* pQuestInfo = &(iter->second);
		return pQuestInfo;
	}
	return NULL;
}

QuestInfo* CDnDataManager::FindQuestInfo(UINT nQuestID)
{
	QUEST_INFO_MAP_IT iter = m_QuestInfoMap.begin();
	for ( iter ; iter != m_QuestInfoMap.end() ; ++iter )
	{
		QuestInfo* pQuestInfo = &(iter->second);
		if ( pQuestInfo )
		{
			if (pQuestInfo->nQuestIndex == nQuestID )
			{
				return pQuestInfo;
			}
		}
	}
	return NULL;
}

bool CDnDataManager::IsPeriodQuest( const int nQuestID ) const
{
	std::vector<int>::const_iterator questIdItor;
	for( PERIOD_QUEST_INFO_VEC_IT periodQuestInfoitor = m_vPeriodQuestInfo.begin(); periodQuestInfoitor != m_vPeriodQuestInfo.end(); ++periodQuestInfoitor )
	{
		questIdItor = find( (*periodQuestInfoitor).vecQuestID.begin(), (*periodQuestInfoitor).vecQuestID.end(), nQuestID );

		if( questIdItor != (*periodQuestInfoitor).vecQuestID.end() )
			return true;
	}

	return false;
}

bool CDnDataManager::_AddTalk(const WCHAR* _wszTalkFileName)
{
 	TALK_PARAGRAPH_MAP TalkParagraphMap;

	std::wstring wszTalkFileName;
	wszTalkFileName = _wszTalkFileName;

	std::transform(wszTalkFileName.begin(), wszTalkFileName.end(), wszTalkFileName.begin(), towlower); 

	// If already exists
	if ( _GetTalk(wszTalkFileName.c_str(), TalkParagraphMap) == true ) 
	{
		LogWnd::Log(1, L"File already loaded : %s" , wszTalkFileName.c_str() );
		return true;
	}

	char szTalkFileName[512] = "";
	ZeroMemory(szTalkFileName, sizeof(char)*512);
	WideCharToMultiByte( CP_ACP, 0, _wszTalkFileName, -1, szTalkFileName, 512, NULL, NULL );

	CResMngStream Stream( szTalkFileName );

	if ( Stream.IsValid() == false )
	{
		LogWnd::Log(1, L"Cannot open file : %s" , wszTalkFileName.c_str() );
		
#ifdef _DEBUG
		if( m_fp )
			fwprintf( m_fp, L"%s\n", wszTalkFileName.c_str() );
#endif

		return false;
	}

	// Generate a hash code from the filename and store it in the map.
	std::wstring __wsz(_wszTalkFileName);
	ToLowerW(__wsz);
	DWORD nHashCode = GetStringHashCode(__wsz.c_str());
	TXMLFileMap::iterator it = m_XMLFileMap.find(nHashCode);
	if ( it != m_XMLFileMap.end() )
	{
		LogWnd::Log(1, L"Already added hash code. code : %d file : %s\n" , nHashCode, wszTalkFileName.c_str());
		return false;
	}
	m_XMLFileMap.insert( make_pair(nHashCode, __wsz));

	int nLen = Stream.Size();

	char* pBuffer = new char[nLen+100];
	ZeroMemory(pBuffer, nLen+100);

	class __Temp
	{
	public:
		__Temp(char* p) { m_p = p; }
		~__Temp() { delete[] m_p;  }
		char* m_p;
	};
	__Temp ScopeDelete(pBuffer);


	int nReadSize = Stream.Read(pBuffer, nLen);

	TiXmlDocument doc;
	doc.Parse(pBuffer, 0, TIXML_ENCODING_UTF8 );
	if ( doc.Error() )
	{
		std::wstring msg;
		msg = L"Failed to parse xml file : ";
		msg += wszTalkFileName;
		msg += L" error msg : ";
		const char* errmsg = doc.ErrorDesc();
		std::wstring wszError;
		ToWideString(std::string(errmsg), wszError );

		msg += wszError;
		LogWnd::Log(5, L"%s id : %d col:%d row:%d", msg.c_str(), doc.ErrorId(), doc.ErrorCol(), doc.ErrorRow());

		return false;
	}

	TiXmlElement* pElement;
	pElement = doc.RootElement();

	if ( !pElement )
	{
		std::wstring msg;
		msg = L"Failed to parse xml file. No root element";
		msg += wszTalkFileName;
		msg += L" error msg : ";
		const char* errmsg = doc.ErrorDesc();
		WCHAR buff[512] = L"";
		ZeroMemory(&buff, sizeof(WCHAR)*512);
		MultiByteToWideChar(CP_ACP, 0, errmsg, -1, buff, 512 );
		msg += buff;
		LogWnd::Log(5, L"%s", msg.c_str());
		return false;
	}


	//pElement = pElement->FirstChildElement();

	TiXmlNode* pNode = pElement->FirstChild("talk_paragraph");

	if ( !pNode )		
	{
		std::wstring msg;
		msg = L"Cannot find talk_paragraph !!!! ";
		msg += _wszTalkFileName;
		LogWnd::Log(5, L"%s", msg.c_str());
		return false;
	}


	pElement = pNode->ToElement();

	if (!pElement) return false;

	const int ___BUF_SIZE = 4096;
	WCHAR buff[___BUF_SIZE] = L"";
	const char* sz = NULL;

	for ( pElement ; pElement != NULL ; pElement = pElement->NextSiblingElement() )
	{
		sz = pElement->Value();
		ZeroMemory(&buff, sizeof(WCHAR)*___BUF_SIZE);
		MultiByteToWideChar(CP_UTF8, 0, sz, -1, buff, ___BUF_SIZE );
		std::wstring szString(buff);

		// question
		if ( szString != L"talk_paragraph" )
		{
			continue;
		}

		TALK_PARAGRAPH talk;
		const char* szTalkIndex = pElement->Attribute("index");

		ZeroMemory(&buff, sizeof(WCHAR)*___BUF_SIZE);
		MultiByteToWideChar(CP_UTF8, 0, szTalkIndex, -1, buff, ___BUF_SIZE );
		talk.szIndex = buff;

		const char* szImageName = pElement->Attribute("image");
		if ( szImageName )
			talk.szImageName = szImageName;
		else
			talk.szImageName = "";

		const char* szItemIndex= pElement->Attribute("itemindex");
		if ( szItemIndex )
			talk.nItemIndex = atoi( szItemIndex );
		else
			talk.nItemIndex = 0;


		TiXmlElement* pTalkElement = pElement->FirstChildElement();

		for ( pTalkElement ; pTalkElement != NULL ; pTalkElement = pTalkElement->NextSiblingElement() )
		{
			sz = pTalkElement->Value();
			ZeroMemory(&buff, sizeof(WCHAR)*___BUF_SIZE);
			MultiByteToWideChar(CP_UTF8, 0, sz, -1, buff, ___BUF_SIZE );
			std::wstring szString(buff);

			// question
			if ( szString == L"talk_question" )
			{
				sz = pTalkElement->GetText();
				ZeroMemory(&buff, sizeof(WCHAR)*___BUF_SIZE);
				MultiByteToWideChar(CP_UTF8, 0, sz, -1, buff, ___BUF_SIZE );
				talk.Question.szQuestion = buff;

			}
			// talk_answer
			else if ( szString == L"talk_answer" )
			{
				TALK_ANSWER answer;


				sz = pTalkElement->Attribute("link_index");
				ZeroMemory(&buff, sizeof(WCHAR)*___BUF_SIZE);
				MultiByteToWideChar(CP_UTF8, 0, sz, -1, buff, ___BUF_SIZE );
				answer.szLinkIndex = buff;

				// Create a hash code for each index.
				std::wstring _wsz;
				_wsz = answer.szLinkIndex;
				ToLowerW(_wsz);
				DWORD nHashCode = GetStringHashCode(_wsz.c_str());
				m_XMLIndexMap.insert( make_pair(nHashCode, _wsz) );

				sz = pTalkElement->Attribute("link_target");
				ZeroMemory(&buff, sizeof(WCHAR)*___BUF_SIZE);
				MultiByteToWideChar(CP_UTF8, 0, sz, -1, buff, ___BUF_SIZE );
				answer.szLinkTarget = buff;

				
				if ( answer.szLinkTarget.empty() )
				{
					answer.szLinkTarget = wszTalkFileName;
					answer.bOtherTargetLink = false;
				}
				else if ( answer.szLinkTarget != wszTalkFileName )
				{
					answer.bOtherTargetLink = true;
				}
				else
				{
					answer.bOtherTargetLink = true;
				}

				sz = pTalkElement->GetText();
				ZeroMemory(&buff, sizeof(WCHAR)*___BUF_SIZE);
				MultiByteToWideChar(CP_UTF8, 0, sz, -1, buff, ___BUF_SIZE );
				answer.szAnswer = buff;

				char szTalkFileName[512] = "";
				ZeroMemory(szTalkFileName, sizeof(char)*512);
				WideCharToMultiByte( CP_ACP, 0, answer.szLinkTarget.c_str() , -1, szTalkFileName, 512, NULL, NULL );

				CResMngStream Stream( szTalkFileName );
				if ( Stream.IsValid() == true )
				{
					talk.Answers.push_back(answer);
				}

			///	talk.Answers.push_back(answer);
			}
			else
			{
				//return false;
			}
		}

		TALK_PARAGRAPH_MAP_IT iter = TalkParagraphMap.find(talk.szIndex);
		if ( iter != TalkParagraphMap.end() )
		{
			std::wstring msg;
			msg = L"Duplicate index found in dialogue file : ";
			msg += _wszTalkFileName;
			msg += L" ->";
			msg += talk.szIndex;

			LogWnd::Log(5, L"%s", msg.c_str());
			return false;
		}
		else
		{
			TalkParagraphMap.insert(make_pair(talk.szIndex, talk));

			// Create a hash code for each index.
			ToLowerW(talk.szIndex);
			DWORD nHashCode = GetStringHashCode(talk.szIndex.c_str());
			m_XMLIndexMap.insert( make_pair(nHashCode, talk.szIndex) );
		}
	}

	// If it is a quest dialogue file
	QuestInfo* pInfo = FindQuestInfo(std::wstring(wszTalkFileName));
	if ( pInfo )
	{
		// Also load additional journals and quest conditions 
		_LoadJournalData(doc.RootElement(), pInfo->nQuestIndex);
		_LoadQuestCondition(doc.RootElement(), pInfo->nQuestIndex);

	}

	m_TalkMap.insert(make_pair(std::wstring(wszTalkFileName), TalkParagraphMap));

	std::wstring msg;
	msg = L"Dialogue file added :";
	msg += wszTalkFileName;
	LogWnd::Log(2, L"%s", msg.c_str());

	return true;
}

bool CDnDataManager::_RemoveTalk(const WCHAR* _wszTalkFileName)
{
	std::wstring wszTalkFileName(_wszTalkFileName);
	std::transform(wszTalkFileName.begin(), wszTalkFileName.end(), wszTalkFileName.begin(), towlower); 

	TALK_MAP_IT iter = m_TalkMap.find( wszTalkFileName );
	if ( iter != m_TalkMap.end() )
	{
		m_TalkMap.erase( iter );
		return true;
	}

	return false;
}


bool CDnDataManager::_GetTalk(const WCHAR* _wszTalkFileName, OUT TALK_PARAGRAPH_MAP& TalkParagraphMap )
{
	std::wstring wszTalkFileName(_wszTalkFileName);
	std::transform(wszTalkFileName.begin(), wszTalkFileName.end(), wszTalkFileName.begin(), towlower); 

	TALK_MAP_IT iter = m_TalkMap.find(	wszTalkFileName );
	if ( iter != m_TalkMap.end() )
	{
		TalkParagraphMap = iter->second;
		return true;
	}

	return false;
}

bool CDnDataManager::GetTalkParagraph(IN std::wstring& wszIndex, IN std::wstring& wszTarget,  IN std::vector<TalkParam>& talkParam, OUT TALK_PARAGRAPH& talk, IN StaticParamCallback* pCallback /* = NULL */)
{
	TALK_MAP_IT iter = m_TalkMap.find(wszTarget);
	if ( iter == m_TalkMap.end() )
	{
		return false;
	}

	TALK_PARAGRAPH_MAP& paragraph_map = iter->second;
	TALK_PARAGRAPH_MAP_IT it = paragraph_map.find(wszIndex);
	if ( it == paragraph_map.end() )
	{
		return false;
	}

	TALK_PARAGRAPH ori_talk = it->second;

	std::vector<TALK_ANSWER> vecTalk_Priority[eQuestTalkPriorityType::Max];  // Set to hold dialogue priority: <Main, SubComplete, SubInProgressNow, SubInProgressGray, Available>
	for(int i=0;i<eQuestTalkPriorityType::Max;i++)
		vecTalk_Priority[i].clear();

	talk = ori_talk;
	talk.Answers.clear();
	

	CDnQuestTask* pQuestTask = (CDnQuestTask*)CTaskManager::GetInstance().GetTask("QuestTask");
	CDnCommonTask* pCommonTask = (CDnCommonTask*)CTaskManager::GetInstance().GetTask("CommonTask");
	if(!pQuestTask || !pCommonTask )
		return false;

	int nCurrentTalkNPCID = pCommonTask->GetCurrentTalkNpcID();
	bool bIsQuestSkipped = false; 	// For checking if a quest has ever been skipped

	for ( size_t i = 0 ; i < ori_talk.Answers.size() ; i++ )
	{
		QuestInfo* pQuestInfo = FindQuestInfo(ori_talk.Answers[i].szLinkTarget);

		bool bSkip = false;
		int nTalkPriority = eQuestTalkPriorityType::Text;

		// When link target is a quest and links to another talk file (quest talk) 
		if ( pQuestInfo && ori_talk.Answers[i].bOtherTargetLink )
		{
			QuestCondition* pCondition = GetQuestCondition(pQuestInfo->nQuestIndex);

			if( pCondition && pCondition->Check() )
			{
				nTalkPriority =  eQuestTalkPriorityType::Available;
				bSkip = false;

#ifdef PRE_FIX_40328
				if( pQuestTask->FindPlayingQuest(pQuestInfo->nQuestIndex) == -1 && !IsExistStartQuestFromNpc(nCurrentTalkNPCID , pQuestInfo->nQuestIndex ) ) 
					bSkip = true;
#endif

			}
			else
			{
				bSkip = true;
				if( pQuestTask->FindPlayingQuest(pQuestInfo->nQuestIndex) > -1)
					bSkip = false;
			}


#ifdef PRE_ADD_LOWLEVEL_QUEST_HIDE
			if( CGameOption::GetInstance().m_bHideQuestMarkByLevel )
			{
				Journal* pJournalData = g_DataManager.GetJournalData(pQuestInfo->nQuestIndex);
				if( pJournalData && ! bSkip )
				{	
					if( IsLowLevelQuest( pJournalData ) && pQuestTask->FindPlayingQuest(pQuestInfo->nQuestIndex) == -1 ) // Do not skip quests in progress. 
					{
						bSkip = true;
					}
					else
					{
						bSkip = false;
					}
				}
			}
#endif 

			if ( pQuestInfo && pQuestTask->IsClearQuest(pQuestInfo->nQuestIndex) == true )
			{
				bSkip = true;
				bIsQuestSkipped = true;
			}

			// Append 'in progress' label here if currently in progress.
			std::wstring str;
			const TQuestGroup* pQuestGroup = pQuestTask->GetQuestGroup();

			if( pQuestGroup && pQuestTask->FindPlayingQuest(pQuestInfo->nQuestIndex) > -1 ) // Only when in progress or higher.
			{
				Journal* pJournal = g_DataManager.GetJournalData(pQuestInfo->nQuestIndex);
				bool bExistMarkType = false;
				int nMarkType = 0;

				if ( pJournal )
				{
					for ( int k = 0 ; k < MAX_PLAY_QUEST ; k++ )
					{
						if ( pQuestGroup->Quest[k].nQuestID == pQuestInfo->nQuestIndex )
						{
							int nQuestIdx = pQuestGroup->Quest[k].nQuestID;
							int nJournalIdx = (int)pQuestGroup->Quest[k].cQuestJournal;

							Journal* pJournalData = g_DataManager.GetJournalData(pQuestGroup->Quest[k].nQuestID);
							if ( pJournalData )
							{
								for ( int j = 0 ; j < (int)pJournalData->JounalPages.size() ; j++ )
								{
									if ( pJournalData->JounalPages[j].nJournalPageIndex == nJournalIdx &&
										pJournalData->JounalPages[j].nStateMarkNpcID == nCurrentTalkNPCID)
									{
										nMarkType = pJournalData->JounalPages[j].nStateMarkType;
										bExistMarkType = true;
									}
								}
							}

						}

					}
				}


				if(bExistMarkType) // If the NPC currently being spoken to is the same as the quest target NPC
				{
					switch(nMarkType)
					{
					case CDnNPCActor::QuestPlaying_Main:
						{
							str = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1200012);
							nTalkPriority = eQuestTalkPriorityType::Main;
						}
						break;
					case CDnNPCActor::QuestRecompense_Main:
						{
							str = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1200013 );
							nTalkPriority =  eQuestTalkPriorityType::Main;;
						}
						break;
					case CDnNPCActor::QuestRecompense_Sub:
					case CDnNPCActor::CrossFavorComplete:
					case CDnNPCActor::CrossReQuestComplete:
#ifdef PRE_ADD_LOWLEVEL_QUEST_HIDE
					case CDnNPCActor::CrossQuestGlobalComplete:
#endif
						{
							str = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1200013 );
							nTalkPriority =  eQuestTalkPriorityType::Complete;
						}
						break;
					case CDnNPCActor::QuestPlaying_Sub:
					case CDnNPCActor::CrossFavorNow:
					case CDnNPCActor::CrossReQuestNow:
#ifdef PRE_ADD_LOWLEVEL_QUEST_HIDE
					case CDnNPCActor::CrossQuestGlobalNow:
#endif
						{
							str = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1200012 );
							nTalkPriority =  eQuestTalkPriorityType::Playing;
						}
						break;
					default:
						break;
					}
				}
				else // Talking to another NPC where quest doesn't progress -> In Progress [Gray] : No distinction between Main / Sub
				{
					str = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1200011);
					nTalkPriority =  eQuestTalkPriorityType::LowPriority;
				}
			
				ori_talk.Answers[i].szAnswer += str;
			}
		}

		if ( bSkip == false )
		{
			vecTalk_Priority[nTalkPriority].push_back(ori_talk.Answers[i]);
		}
	}

	for(int i=0;i< eQuestTalkPriorityType::Max;i++)
	{
		if(!vecTalk_Priority[i].empty())
		{
			for(int k=0;k<(int)vecTalk_Priority[i].size();k++)
				talk.Answers.push_back(vecTalk_Priority[i][k]);
		}
	}

	// If a quest skip occurs and there is no answer at all, force-set the paragraph to _no_quest.
	if( talk.Answers.empty() ) {
		if ( bIsQuestSkipped || !ori_talk.Answers.empty() )		// Even if there are multiple answers, if none can be done due to restrictions, go to no_quest.
		{
			static std::wstring __noquest(L"_no_quest");
			bool bResult = GetTalkParagraph( __noquest , wszTarget, talkParam, talk);

			if ( !bResult )
			{
#ifndef _FINAL_BUILD
				std::wstring wszLog = FormatW(L"GetTalkParagraph Fail [%s:%s], ", __noquest.c_str(), wszTarget.c_str());
				GetInterface().AddChatMessage(CHATTYPE_NORMAL, L"", wszLog.c_str());
#endif // _FINAL_BUILD

			}
			return bResult;

		}
	}

	// Replace torque parameter if present.
	if ( talkParam.size() )
	{
		for ( int i = 0 ; i < (int)talkParam.size() ; i++ )
		{
			std::wstring wszKey;
			std::wstring wsz;
			if ( talkParam[i].cType == TalkParam::INT )
			{
				wsz =FormatW( L"%d", talkParam[i].nValue );
			}
			else if ( talkParam[i].cType == TalkParam::STRING )
			{
				wsz = _GetTalkParam(talkParam[i].nValue);
			}
	

			if ( !wsz.empty() )
				AllReplaceW( talk.Question.szQuestion, std::wstring(talkParam[i].wszKey), wsz);
		}

	}

	ReplaceStaticTalkParam(talk.Question.szQuestion, pCallback);

	for ( int i = 0 ; i < (int)talk.Answers.size() ; i++ )
		ReplaceStaticTalkParam(talk.Answers[i].szAnswer, pCallback);

	return true;
}


void CDnDataManager::ReplaceStaticTalkParam(std::wstring& str, IN StaticParamCallback* pCallback /* = NULL */)
{
	std::vector<std::wstring> tokens;
	TokenizeW(str, tokens, std::wstring(L"{"), std::wstring(L"}"), false);

	for ( int j = 0 ; j < (int)tokens.size() ; j++ )
	{
		std::vector<std::wstring> child_tokens;
		TokenizeW(tokens[j], child_tokens, L":");
		//for ( int k = 0 ; k < (int)child_tokens.size() ; k++ )
		//{
		for ( int i = 0 ; i < (int)m_StaticTalkParamList.size() ; i++ )
		{
			if ( child_tokens[0] == m_StaticTalkParamList[i].szKey )
			{
				std::wstring wsz = _GetString(m_StaticTalkParamList[i].nType, child_tokens, pCallback);
				AllReplaceW(str, tokens[j], wsz);
			}
		}
		//}
	}

	RemoveStringW(str, std::wstring(L"{"));
	RemoveStringW(str, std::wstring(L"}"));

}

std::wstring CDnDataManager::_GetString(int nType, std::vector<std::wstring>& tokens, IN StaticParamCallback* pCallback /* = NULL */)
{
	switch( nType )
	{
	case __StaticTalkParamInfo::user_nick:
		{
			if ( CDnActor::s_hLocalActor )
				return CDnActor::s_hLocalActor->GetName();
		}
		break;
	case __StaticTalkParamInfo::user_class:
		{
			if(!CDnActor::s_hLocalActor)
				return L"";
			CDnPlayerActor *pPlayer = dynamic_cast<CDnPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());
			if(!pPlayer)
				return L"";

			return pPlayer->GetJobName();
		}
		break;
	case __StaticTalkParamInfo::monster:
		{
			DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMONSTER );
			int nMonID = _wtoi(tokens[1].c_str());
			std::wstring wszMonsterName;
			int nStringTableID = pSox->GetFieldFromLablePtr( nMonID, "_NameID" )->GetInteger();
			char *szParam = pSox->GetFieldFromLablePtr( nMonID, "_NameIDParam" )->GetString();
			MakeUIStringUseVariableParam( wszMonsterName, nStringTableID, szParam );
			return wszMonsterName;
		}
		break;
	case __StaticTalkParamInfo::npc:	
		{
			DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TNPC );
			int nID = _wtoi(tokens[1].c_str());
			int nStringTableID = pSox->GetFieldFromLablePtr( nID, "_NameID" )->GetInteger();
			std::wstring wszName = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nStringTableID );
			return wszName;
		}
		break;
	case __StaticTalkParamInfo::item:
		{
			DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TITEM );
			int nID = _wtoi(tokens[1].c_str());
			int nStringTableID = pSox->GetFieldFromLablePtr( nID, "_NameID" )->GetInteger();
			std::wstring wszName = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nStringTableID );
			return wszName;
		}
		break;
	case __StaticTalkParamInfo::skill:	
		{
			DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TSKILL );
			int nID = _wtoi(tokens[1].c_str());
			int nStringTableID = pSox->GetFieldFromLablePtr( nID, "_NameID" )->GetInteger();
			std::wstring wszName = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nStringTableID );
			return wszName;
		}
		break;
	case __StaticTalkParamInfo::map:
		{
			DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMAP );
			int nID = _wtoi(tokens[1].c_str());
			int nStringTableID = pSox->GetFieldFromLablePtr( nID, "_MapNameID" )->GetInteger();
			std::wstring wszName = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nStringTableID );
			return wszName;
		}
		break;
	case __StaticTalkParamInfo::action:
		{
			if ( pCallback )
			{
				std::string szAction;
				ToMultiString(tokens[1], szAction);
				pCallback->OnAction(szAction);
			}
		}
		break;
	case __StaticTalkParamInfo::sound:
		{
			if ( pCallback )
			{
				std::string szSoundFileName;
				ToMultiString(tokens[1], szSoundFileName);
				pCallback->OnSound(szSoundFileName);
			}
		}
		break;

	case __StaticTalkParamInfo::uistring:
		{
			std::wstring wszUIStringMsg;
			wszUIStringMsg = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, _wtoi(tokens[1].c_str()));
			return wszUIStringMsg;
		}
		break;
	case __StaticTalkParamInfo::repair_cost:
		{
#ifdef PRE_ADD_REPAIR_NPC
			if( CDnItemTask::IsActive() )
			{
				int nPrice = GetItemTask().CalcRepairEquipPrice();
				if( CDnQuestTask::IsActive() )
				{
					GetQuestTask().CheckAndCalcStoreBenefit( NpcReputation::StoreBenefit::RepairFeeDiscount, nPrice );
				}
	
				int nGold = nPrice/10000;
				int nSilver = (nPrice%10000)/100;
				int nBronze = nPrice%100;

				return FormatW( L"%d%s %d%s %d%s" , 
					nGold , GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 507 ),
					nSilver , GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 508 ),
					nBronze , GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 509 ) ).c_str();
			}
#endif
		}
		break;

	case __StaticTalkParamInfo::repair_cost_all:
		{
#ifdef PRE_ADD_REPAIR_NPC
			if( CDnItemTask::IsActive() )
			{
				int nPrice = GetItemTask().CalcRepairEquipPrice() + GetItemTask().CalcRepairCharInvenPrice();
				if( CDnQuestTask::IsActive() )
				{
					GetQuestTask().CheckAndCalcStoreBenefit( NpcReputation::StoreBenefit::RepairFeeDiscount, nPrice );
				}

				int nGold = nPrice/10000;
				int nSilver = (nPrice%10000)/100;
				int nBronze = nPrice%100;

				return FormatW( L"%d%s %d%s %d%s" , 
					nGold , GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 507 ),
					nSilver , GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 508 ),
					nBronze , GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 509 ) ).c_str();

			}
#endif
		}
		break;

	case __StaticTalkParamInfo::money:
		{
#ifdef PRE_ADD_REPAIR_NPC
			if( CDnItemTask::IsActive() )
			{
				INT64 nCoin = GetItemTask().GetCoin();
				INT64 nGold = nCoin/10000;
				INT64 nSilver = (nCoin%10000)/100;
				INT64 nBronze = nCoin%100;

				return FormatW( L"%I64d%s %I64d%s %I64d%s" , 
					nGold , GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 507 ),
					nSilver , GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 508 ),
					nBronze , GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 509 ) ).c_str();
			}
#endif
		}
		break;

	//case __StaticTalkParamInfo::quest_level:
	//{
	//	DNTableFileFormat* pSox = GetDNTable(CDnTableDB::TQUEST);
	//	int nID = _wtoi(tokens[1].c_str());
	//	int nStringTableID = pSox->GetFieldFromLablePtr(nID, "_MapNameID")->GetInteger();
	//	std::wstring wszName = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, nStringTableID);
	//	return wszName;
	//}
	//break;
	}

	return std::wstring(L"");
}

void CDnDataManager::_LoadJournalData(TiXmlElement* pRootElement, UINT nQuestID)
{
	TiXmlNode* pNode = pRootElement->FirstChild("journal_data");

	if ( !pNode )		return;


	TiXmlElement* pElement = pNode->ToElement();

	if (!pElement) return;


	const int ___BUF_SIZE = 4096;
	WCHAR buff[___BUF_SIZE] = {0,};
	const char* sz = NULL;

	Journal* pJournal = new Journal;

	pJournal->nQuestIndex = nQuestID;

	// Get the quest title.
	// Using the method below retrieves strings from parts of XML that cannot be translated.
	// So in overseas versions, titles in the notification window and some titles in the quest window tree are not visible.
	//sz = pElement->Attribute("quest_title");
	//ZeroMemory(&buff, sizeof(WCHAR)*___BUF_SIZE);
	//MultiByteToWideChar(CP_UTF8, 0, sz, -1, buff, ___BUF_SIZE );
	//pJournal->wszQuestTitle = buff;
	//
	// Getting it from the quest info struct returns the quest table name like "Black Crest I (Warrior)", which should not be used.
	//QuestInfo *pQuestInfo = FindQuestInfo(pJournal->nQuestIndex);
	//
	// In the quest xml, the proper title value is set in qkname above.
	// Since it will always be present, load without exception handling.
	TiXmlElement* pQuestDescElement = pRootElement->FirstChildElement("quest_desc");
	TiXmlElement* pQkNameElement = pQuestDescElement->FirstChildElement("qkname");
	TiXmlElement* pQkTypeElement = pQuestDescElement->FirstChildElement("qtype");

	sz = pQkNameElement->GetText();
	ZeroMemory(&buff, sizeof(WCHAR)*___BUF_SIZE);
	MultiByteToWideChar(CP_UTF8, 0, sz, -1, buff, ___BUF_SIZE );
	pJournal->wszQuestTitle = buff;

	// Get chapter title.
	sz = pElement->Attribute("chapterid");
	ZeroMemory(&buff, sizeof(WCHAR)*___BUF_SIZE);
	MultiByteToWideChar(CP_UTF8, 0, sz, -1, buff, ___BUF_SIZE );
	pJournal->nChapterIndex = _wtoi(buff);

	// Reward
	sz = pElement->Attribute("recompense");
	ZeroMemory(&buff, sizeof(WCHAR)*___BUF_SIZE);
	MultiByteToWideChar(CP_UTF8, 0, sz, -1, buff, ___BUF_SIZE );

	std::vector<std::wstring> recompense_tokens;
	TokenizeW( buff, recompense_tokens, L"," );
	for ( int i = 0 ; i < (int)recompense_tokens.size() ; ++i )
	{
		std::vector<std::wstring> child_tokens;
		TokenizeW(recompense_tokens[i], child_tokens, L":" );
		if ( child_tokens.size() == 2 )
		{
			int nClass = _wtoi(child_tokens[0].c_str());
			int nTableID = _wtoi(child_tokens[1].c_str());
			ASSERT( nClass >= 0 && nClass <= CLASSKINDMAX );
			if( false == (nClass >= 0 && nClass <= CLASSKINDMAX) )
				continue;

			if( nClass == 0 ) pJournal->nRecompenseTableID = nTableID;
			else pJournal->nRecompenseTableIDForClass[ nClass-1 ] = nTableID; // #35786 Verified source is written based on the 4 existing classes. < Class-specific rewards > 
		}
		else if( recompense_tokens.size() == 1 && child_tokens.size() == 1 ) {		// For Old Format Compatible
			pJournal->nRecompenseTableID = _wtoi( child_tokens[ 0 ].c_str() );
		}
		else {
			ASSERT( false && "Recompense Table Error ");
		}
	}

	// Quest level 
	sz = pElement->Attribute("quest_level");
	ZeroMemory(&buff, sizeof(WCHAR)*___BUF_SIZE);
	MultiByteToWideChar(CP_UTF8, 0, sz, -1, buff, ___BUF_SIZE );
	pJournal->nQuestLevel = _wtoi(buff);

	// Quest type
	sz = pQkTypeElement->GetText();
	ZeroMemory(&buff, sizeof(WCHAR)*___BUF_SIZE);
	MultiByteToWideChar(CP_UTF8, 0, sz, -1, buff, ___BUF_SIZE );
	if( strstr( "mainquest", sz ) )
		pJournal->nQuestType = Journal::emQuestType_Main;
	else if( strstr( sz, "sub" ) )
		pJournal->nQuestType = Journal::emQuestType_Sub;
	else if( strstr( sz, "venturer" ) )
		pJournal->nQuestType = Journal::emQuestType_Sub;
	else if( strstr( sz, "day" ) )
		pJournal->nQuestType = Journal::emQuestType_Day;
	else if( strstr( sz, "favorMalice" ) )
		pJournal->nQuestType = Journal::emQuestType_Rep;

	if( Journal::emQuestType_Main != pJournal->nQuestType )
	{
		QuestInfo * pQuestInfo = FindQuestInfo( nQuestID );
		int nTypeID = -1;
		if( Journal::emQuestType_Sub == pJournal->nQuestType )
			nTypeID = 430;	// Normal
		else if( Journal::emQuestType_Rep == pJournal->nQuestType )
			nTypeID = 431;	// Favor
		else if( Journal::emQuestType_Day == pJournal->nQuestType )
			nTypeID = 429;	// Daily

#ifdef PRE_MOD_LIKEABILITY_QUEST_TEXT
#ifdef PRE_MOD_CONSTANT_TEXT_CONVERT_TO_TABLE_TEXT
		pJournal->wszQuestLevelTitle = FormatW(L"[%s %d] %s",GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7888 ), pJournal->nQuestLevel, pJournal->wszQuestTitle.c_str());	 // Current Quest Journal display format: Category + Quest Lv + Quest Name
#else 
		pJournal->wszQuestLevelTitle = FormatW(L"[Lv. %d] %s",pJournal->nQuestLevel, pJournal->wszQuestTitle.c_str());	 // Current Quest Journal display format: Category + Quest Lv + Quest Name
#endif 
#else
#ifdef PRE_MOD_CONSTANT_TEXT_CONVERT_TO_TABLE_TEXT
		pJournal->wszQuestLevelTitle = FormatW(L"%s [%s %d] %s", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nTypeID ), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7888 ), pJournal->nQuestLevel, pJournal->wszQuestTitle.c_str());
#else
		pJournal->wszQuestLevelTitle = FormatW(L"%s [Lv. %d] %s", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nTypeID ), pJournal->nQuestLevel, pJournal->wszQuestTitle.c_str());
#endif 
#endif
	}
	else
	{
#ifdef PRE_MOD_CONSTANT_TEXT_CONVERT_TO_TABLE_TEXT
		pJournal->wszQuestLevelTitle = FormatW(L"[%s %d] %s", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7888 ), pJournal->nQuestLevel, pJournal->wszQuestTitle.c_str());
#else
		pJournal->wszQuestLevelTitle = FormatW(L"[Lv. %d] %s", pJournal->nQuestLevel, pJournal->wszQuestTitle.c_str());
#endif 
	}

	// Quest mark type
	sz = pElement->Attribute("quest_mark_type");
	ZeroMemory(&buff, sizeof(WCHAR)*___BUF_SIZE);
	MultiByteToWideChar(CP_UTF8, 0, sz, -1, buff, ___BUF_SIZE );
	pJournal->nQuestMarkType = _wtoi(buff);

	pNode  = pElement->FirstChild("journal");
	if ( !pNode )		
	{
		delete pJournal;
		return;
	}

	pElement = pNode->ToElement();


	for ( pElement ; pElement != NULL ; pElement = pElement->NextSiblingElement() )
	{
		sz = pElement->Value();
		ZeroMemory(&buff, sizeof(WCHAR)*___BUF_SIZE);
		MultiByteToWideChar(CP_UTF8, 0, sz, -1, buff, ___BUF_SIZE );
		std::wstring szString(buff);

		// question
		if ( szString != L"journal" )
		{
			continue;
		}


		JournalPage journal_page;

		const char* szJournalIndex = pElement->Attribute("index");
		ZeroMemory(&buff, sizeof(WCHAR)*___BUF_SIZE);
		MultiByteToWideChar(CP_UTF8, 0, szJournalIndex, -1, buff, ___BUF_SIZE );
		journal_page.nJournalPageIndex = _wtoi(buff);

		const char* szChapterIndex = pElement->Attribute("chapterid");
		ZeroMemory(&buff, sizeof(WCHAR)*___BUF_SIZE);
		MultiByteToWideChar(CP_UTF8, 0, szChapterIndex, -1, buff, ___BUF_SIZE );
		journal_page.nChapterIndex = _wtoi(buff);

		TiXmlElement* pChildElement = pElement->FirstChildElement();

		journal_page.nStateMarkNpcID = 0;
		journal_page.nStateMarkType = 0;
		journal_page.szTodoMsg = L"Todo message.";

		for ( pChildElement ; pChildElement != NULL ; pChildElement = pChildElement->NextSiblingElement() )
		{
			sz = pChildElement->Value();
			ZeroMemory(&buff, sizeof(WCHAR)*___BUF_SIZE);
			MultiByteToWideChar(CP_UTF8, 0, sz, -1, buff, ___BUF_SIZE );
			std::wstring szString(buff);

			// title
			if ( szString == L"title" )
			{
				sz = pChildElement->GetText();
				ZeroMemory(&buff, sizeof(WCHAR)*___BUF_SIZE);
				MultiByteToWideChar(CP_UTF8, 0, sz, -1, buff, ___BUF_SIZE );
				journal_page.szTitle = buff;
			}
			else if ( szString == L"title_image" )
			{
				sz = pChildElement->GetText();
				ZeroMemory(&buff, sizeof(WCHAR)*___BUF_SIZE);
				MultiByteToWideChar(CP_UTF8, 0, sz, -1, buff, ___BUF_SIZE );
				journal_page.szTitleImage = buff;
			}
			else if ( szString == L"contents" )
			{
				sz = pChildElement->GetText();
				ZeroMemory(&buff, sizeof(WCHAR)*___BUF_SIZE);
				MultiByteToWideChar(CP_UTF8, 0, sz, -1, buff, ___BUF_SIZE );
				journal_page.szContents = buff;
			}			
			else if ( szString == L"contents_image" )
			{
				sz = pChildElement->GetText();
				ZeroMemory(&buff, sizeof(WCHAR)*___BUF_SIZE);
				MultiByteToWideChar(CP_UTF8, 0, sz, -1, buff, ___BUF_SIZE );
				journal_page.szContentsImage = buff;
				if ( journal_page.szContentsImage.empty() )
				{
					journal_page.szContentsImage = L"QuestDescDefault.dds";
				}
			}			
			else if ( szString == L"need_itemlist" )
			{
				sz = pChildElement->GetText();
				ZeroMemory(&buff, sizeof(WCHAR)*___BUF_SIZE);
				MultiByteToWideChar(CP_UTF8, 0, sz, -1, buff, ___BUF_SIZE );

				//@ Parse separately
				std::wstring wszString;
				wszString = buff;
				_ParseJournalItemlist( wszString, journal_page.NeedItemList);

			}			

			else if ( szString == L"destination" )
			{
				sz = pChildElement->GetText();
				ZeroMemory(&buff, sizeof(WCHAR)*___BUF_SIZE);
				MultiByteToWideChar(CP_UTF8, 0, sz, -1, buff, ___BUF_SIZE );
				journal_page.szDestination = buff;

			}			
			else if ( szString == L"todo_msg" )
			{

				sz = pChildElement->GetText();
				ZeroMemory(&buff, sizeof(WCHAR)*___BUF_SIZE);
				MultiByteToWideChar(CP_UTF8, 0, sz, -1, buff, ___BUF_SIZE );

				journal_page.szTodoMsg = buff;
			}

			else if ( szString == L"destination_pos" )
			{
				sz = pChildElement->GetText();
				ZeroMemory(&buff, sizeof(WCHAR)*___BUF_SIZE);
				MultiByteToWideChar(CP_UTF8, 0, sz, -1, buff, ___BUF_SIZE );

				//@ Parse separately
				std::wstring wszString;
				wszString = buff;
				// First remove all whitespace from the string.
				RemoveSpaceW(wszString);
				std::vector<std::wstring> tokens;
				TokenizeW(wszString, tokens, L",");

				if ( tokens.size() == 3 )
				{
					journal_page.nDestnationMapIndex = (int)_wtoi(tokens[0].c_str());
					journal_page.vDestinationPos.x = (float)_wtof(tokens[1].c_str());
					journal_page.vDestinationPos.y = (float)_wtof(tokens[2].c_str());
				}
				else if ( tokens.size() == 2 )
				{
					journal_page.nDestnationMapIndex = (int)_wtoi(tokens[0].c_str());
					ASSERT(tokens[1].c_str()[0] == 'N' || tokens[1].c_str()[0] == 'n' );
					journal_page.nDestinationNpc = (int)_wtoi( tokens[1].c_str() + 1 );
				}
			}			

			else if ( szString == L"destination_mode" )
			{
				sz = pChildElement->GetText();
				ZeroMemory(&buff, sizeof(WCHAR)*___BUF_SIZE);
				MultiByteToWideChar(CP_UTF8, 0, sz, -1, buff, ___BUF_SIZE );

				//@ Parse separately
				std::wstring wszString;
				wszString = buff;

				_ParseDestMode(wszString, journal_page);

			}			
			else if ( szString == L"only_minimap" )
			{
				sz = pChildElement->GetText();
				ZeroMemory(&buff, sizeof(WCHAR)*___BUF_SIZE);
				MultiByteToWideChar(CP_UTF8, 0, sz, -1, buff, ___BUF_SIZE );
				std::wstring szbool;
				szbool = buff;
				if ( szbool == L"1" )
					journal_page.bOnlyMinimap = true;
				else
					journal_page.bOnlyMinimap = false;

			}

			else if ( szString == L"statemark_npcid" )
			{
				sz = pChildElement->GetText();
				ZeroMemory(&buff, sizeof(WCHAR)*___BUF_SIZE);
				MultiByteToWideChar(CP_UTF8, 0, sz, -1, buff, ___BUF_SIZE );

				journal_page.nStateMarkNpcID = _wtoi(buff);
			}		
			else if ( szString == L"statemark_type" )
			{
				sz = pChildElement->GetText();
				ZeroMemory(&buff, sizeof(WCHAR)*___BUF_SIZE);
				MultiByteToWideChar(CP_UTF8, 0, sz, -1, buff, ___BUF_SIZE );
				journal_page.nStateMarkType = _wtoi(buff);

			}		

			else
			{

			}
		}


		pJournal->JounalPages.push_back(journal_page);
	}

	m_JournalMap.insert(make_pair( pJournal->nQuestIndex, pJournal));

}


void CDnDataManager::_ParseJournalItemlist(std::wstring& wszString, std::vector<JournalItem>& j_item_list)
{
	// First remove all whitespace from the string.
	RemoveSpaceW(wszString);

	std::vector<std::wstring> tokens;
	TokenizeW(wszString, tokens, L",");

	for ( size_t i = 0 ; i < tokens.size() ; i++ )
	{
		std::vector<std::wstring> child_tokens;
		TokenizeW(tokens[i], child_tokens, L":" );

		JournalItem j_item;

		if ( child_tokens.size() == 3 && child_tokens[0] == L"item" )
		{
			j_item.cType = JournalItem::Item;
			j_item.nValue = _wtoi(child_tokens[1].c_str());
			j_item.nCount = _wtoi(child_tokens[2].c_str());
			j_item_list.push_back(j_item);
		}

		else if ( child_tokens.size() == 3 && child_tokens[0] == L"monster" )
		{
			j_item.cType = JournalItem::Monster;
			j_item.nValue = _wtoi(child_tokens[1].c_str());
			j_item.nCount = _wtoi(child_tokens[2].c_str());
			j_item_list.push_back(j_item);
		}

		else if ( child_tokens.size() == 2 && child_tokens[0] == L"gold" )
		{
			j_item.cType = JournalItem::Gold;
			j_item.nValue = _wtoi(child_tokens[1].c_str());
			j_item.nCount = 0;
			j_item_list.push_back(j_item);
		}
		else
		{
			// Ugh
		}
	}
}

void CDnDataManager::_ParseDestMode(std::wstring& wszString, JournalPage&  page)
{
	//wszString = L"custom:1@0,2@1,3@2";
	// First remove all whitespace from the string.
	RemoveSpaceW(wszString);

	std::vector<std::wstring> tokens;
	TokenizeW(wszString, tokens, L":");

	std::wstring wszMode;
	if ( tokens.empty()  )
	{
		wszMode = wszString;
	}
	else
	{
		wszMode = tokens[0];
	}

	ToLowerW(wszMode);

	if ( wszMode == L"none" )						page.nDestnationMode = JournalPage::none;
	else if ( wszMode == L"hide" )					page.nDestnationMode = JournalPage::hide;
	else if ( wszMode == L"arrival" )				page.nDestnationMode = JournalPage::arrival;
	else if ( wszMode == L"stage_hide" )			page.nDestnationMode = JournalPage::stage_hide;
	else if ( wszMode == L"stage_arrival" )			page.nDestnationMode = JournalPage::stage_arrival;
	else if ( wszMode == L"select_hide" )			page.nDestnationMode = JournalPage::select_hide;
	else if ( wszMode == L"select_arrival" )		page.nDestnationMode = JournalPage::select_arrival;
	else if ( wszMode == L"stage_hide_except" )		page.nDestnationMode = JournalPage::stage_hide_except;
	else if ( wszMode == L"custom" )				page.nDestnationMode = JournalPage::custom;
	else											page.nDestnationMode = JournalPage::none;

	if ( tokens.size() > 1 )
	{
		// Special handling is needed when it is custom.
		if ( page.nDestnationMode == JournalPage::custom )
		{
			std::vector<std::wstring> child_tokens;
			TokenizeW(tokens[1], child_tokens, L"," );

			for ( int i = 0 ; i < (int)child_tokens.size() ; ++i )
			{
				std::vector<std::wstring> temp_tokens;
				TokenizeW(child_tokens[i], temp_tokens, L"@" );
				if ( temp_tokens.size() == 2 )
				{
					JournalPage::DestinationMark mark;
					mark.nMapIdx = _wtoi(temp_tokens[0].c_str());
					mark.nMarkIdx = _wtoi(temp_tokens[1].c_str());
					page.CustomDestnationMark.push_back(mark);
				}
			}

		}
		else
		{
			std::vector<std::wstring> child_tokens;
			TokenizeW(tokens[1], child_tokens, L"," );

			page.DestnationModeValue.reserve(child_tokens.size());
			for ( int i = 0 ; i < (int)child_tokens.size() ; ++i )
			{
				page.DestnationModeValue.push_back(_wtoi(child_tokens[i].c_str()));
			}
		}

	}
}

void CDnDataManager::_RemoveAllJournalData()
{

	JOURNAL_DATA_MAP_IT iter;
	iter = m_JournalMap.begin();

	for ( ; iter != m_JournalMap.end() ; iter++ )
	{
		Journal* pJournal = iter->second;
		SAFE_DELETE(pJournal);
	}

	m_JournalMap.clear();
}

Journal* CDnDataManager::GetJournalData(UINT nQuestIndex)
{
	JOURNAL_DATA_MAP_IT iter = m_JournalMap.find(nQuestIndex);
	if ( iter != m_JournalMap.end() )
	{
		Journal* pJournal = iter->second;
		return pJournal;
	}
	return NULL;
}

//--------------------------------------------------------------------------------------------------------
// Quest condition related code

QuestCondition::QuestCondition()
{

}

QuestCondition::~QuestCondition()
{
	for ( size_t i = 0 ; i < ConditionList.size() ; i++ )
	{
		ConditionBase* pCB = ConditionList[i];
		SAFE_DELETE(pCB);
	}
	ConditionList.clear();
}

struct __QuestDate : public ConditionBase
{
	bool DateCheck;
	int nMinDate;
	int nMaxDate;

	virtual void Init(std::wstring& data) 
	{
		nMinDate = 0;
		nMaxDate = 0;
		DateCheck = true;

		if ( data.empty() )
		{
			bUse = false;
			return;
		}

		std::vector<std::wstring> tokens;
		TokenizeW(data, tokens, L":");
		if ( tokens.size() != 2 )
		{
			bUse = false;
			return;
		}
		
		if(!tokens.empty())
		{
			nMinDate = _wtoi(tokens[0].c_str());
			nMaxDate = _wtoi(tokens[1].c_str());
		}

		bUse = true;
	}
	virtual bool Check()
	{
		if(!CSyncTimer::GetInstance().IsStarted())
			_ASSERT(0&&"Sync Timer failed to receive time info [Quest].");

		const __time64_t tCurTime = CSyncTimer::GetInstance().GetCurTime();   // Time received from server.
		DBTIMESTAMP DbTime;  
		CTimeSet::ConvertTimeT64ToDbTimeStamp_LC( tCurTime, &DbTime );        // Important: convert must be done with LC.

		int nDate;
		nDate =  (DbTime.year * 10000) + (DbTime.month * 100) + (DbTime.day); 
		

		if( nMinDate <= nDate &&  nMaxDate >= nDate  ) // If today's date "nData" falls between the min/max dates specified in xml
		{
			
			DateCheck = true; // True

		}
		else
		{
			DateCheck = false; // Otherwise false
		}


		if ( !bUse )
			return true;

		if( !CDnActor::s_hLocalActor ) 
			return false;

		if(DateCheck) return true;

		return false;
	}
};


struct __QuestDay : public ConditionBase
{
	bool DayOfWeekCheck;
	WORD wDayOfWeek;

	std::vector<std::wstring> tokens;

	virtual void Init(std::wstring& data) 
	{
		DayOfWeekCheck = true;
		wDayOfWeek = -1;

		if ( data.empty() )
		{
			bUse = false;
			return;
		}

		TokenizeW(data, tokens, L":");
		bUse = true;

	}
	virtual bool Check()
	{

		if(!CSyncTimer::GetInstance().IsStarted())
			_ASSERT(0&&"Sync Timer failed to receive time info [Quest].");

		const __time64_t tCurTime = CSyncTimer::GetInstance().GetCurTime(); // Receives time from the server.
		DBTIMESTAMP DbTime;  
		CTimeSet::ConvertTimeT64ToDbTimeStamp_LC( tCurTime, &DbTime );

		CTimeSet cTime;
		cTime.Set(DbTime);

		wDayOfWeek = cTime.GetDayOfWeek();
		
		for(int i=0;i < (int)tokens.size() ; i++)
		{
			if((WORD)_wtoi(tokens[i].c_str()) == -1)
			{
				DayOfWeekCheck = false;
				break;
			}
			else if((WORD)_wtoi(tokens[i].c_str()) == -2)
			{
				DayOfWeekCheck = true;
				break;
			}
			else if((WORD)_wtoi(tokens[i].c_str()) == wDayOfWeek) // Day of week is received as 1~7. (If it matches the token, stop checking and return true)
			{
				DayOfWeekCheck = true; // Return True if any token matches.
				break;
			}
			else
			{
				DayOfWeekCheck = false; // Otherwise false
			}
		}


		if ( !bUse )
			return true;

		if( !CDnActor::s_hLocalActor ) 
			return false;

		if(DayOfWeekCheck)
		{
			return true;
		}

		return false;
	}

};

struct __HaveNormalItem : public ConditionBase
{
	struct ConditionItem
	{
		int nItemTableNumber;
		int nItemNumber;
	};

	std::vector<ConditionItem> ConditionItemList;

	virtual void Init(std::wstring& data) 
	{
		ConditionItemList.clear();

		if( data.empty() )
		{
			bUse = false;
			return;
		}

		std::vector<std::wstring> tokenList;
		TokenizeW(data, tokenList, L",");

		if(!tokenList.empty())
		{
			for(DWORD i=0; i<tokenList.size(); i++)
			{
				std::vector<std::wstring> tokens;
				TokenizeW( tokenList[i] , tokens, L":");
				if ( tokens.size() != 2 )
				{
					bUse = false;
					return;
				}

				ConditionItem TempInfo;
				TempInfo.nItemTableNumber = _wtoi(tokens[0].c_str()); // Required item number
				TempInfo.nItemNumber = _wtoi(tokens[1].c_str());       // Required item count

				ConditionItemList.push_back(TempInfo);
			}
		}
		else
		{
			bUse = false;
			return;
		}

		bUse = true;
	}

	virtual bool Check()
	{
		if ( !bUse )
			return true;

		if( !CDnActor::s_hLocalActor ) 
			return false;

		if(ConditionItemList.empty())
			return false;

		for(DWORD i=0;i<ConditionItemList.size(); i++)
		{
			if( !(GetItemTask().GetCharInventory().GetItemCount( ConditionItemList[i].nItemTableNumber ) >= ConditionItemList[i].nItemNumber) )			// Check how many of that item I have
			{
				return false;
			}
		}

		return true;
	}

};

struct __HaveCashItem : public ConditionBase
{
	int nItemTableNumber;
	int nItemNumber;
	virtual void Init(std::wstring& data) 
	{
		nItemTableNumber = 0;
		nItemNumber = 0;

		if ( data.empty() )
		{
			bUse = false;
			return;
		}

		std::vector<std::wstring> tokens;
		TokenizeW(data, tokens, L":");
		if ( tokens.size() != 2 )
		{
			bUse = false;
			return;
		}

		nItemTableNumber = _wtoi(tokens[0].c_str()); // Required item number
		nItemNumber = _wtoi(tokens[1].c_str());       // Required item count
		bUse = true;
	}
	virtual bool Check()
	{
		if ( !bUse )
			return true;

		if( !CDnActor::s_hLocalActor ) 
			return false;


		if(GetItemTask().GetCashInventory().GetItemCount( nItemTableNumber ) >= nItemNumber)			// Check how many of that item I have
		{
			return true;
		}

		return false;
	}

};


struct __Reputation_Favor : public ConditionBase // Likeability value
{
	int nNpcID;
	int nReputationValue_Min;
	int nReputationValue_Max;
	
	virtual void Init(std::wstring& data) 
	{
		nNpcID = 0;
		nReputationValue_Min = 0;
		nReputationValue_Max = 0;

		if ( data.empty() )
		{
			bUse = false;
			return;
		}

		std::vector<std::wstring> tokens;
		TokenizeW(data, tokens, L":");
		if ( tokens.size() != 3 )
		{
			bUse = false;
			return;
		}

		nNpcID = _wtoi(tokens[0].c_str()); // Required item number
		nReputationValue_Min = _wtoi(tokens[1].c_str());       // Required item count
		nReputationValue_Max = _wtoi(tokens[2].c_str());
		bUse = true;
	}
	virtual bool Check()
	{
		if ( !bUse )
			return true;

		if( !CDnActor::s_hLocalActor ) 
			return false;

		CDnQuestTask* pQuestTask = static_cast<CDnQuestTask*>(CTaskManager::GetInstance().GetTask( "QuestTask" ));
		if( !pQuestTask )
			return false;

		if(pQuestTask)			// Check how many of that item I have
		{
#if defined(PRE_ADD_NPC_REPUTATION_SYSTEM)
			CReputationSystemRepository* pReputationRepos = pQuestTask->GetReputationRepository();
			REPUTATION_TYPE iFavorValue = pReputationRepos->GetNpcReputation( nNpcID, IReputationSystem::NpcFavor ); // Receive the favorability value of the target NPC.

			if(nReputationValue_Min <= iFavorValue && iFavorValue <=nReputationValue_Max) // Operates only when favorability is at or above the specified value.
				return true;
			else if(nReputationValue_Min <= iFavorValue && nReputationValue_Max == -1)
				return true;
			else if(iFavorValue <=nReputationValue_Max && nReputationValue_Min == -1)
				return true;
#endif	// #if defined(PRE_ADD_NPC_REPUTATION_SYSTEM)
		}

		return false;
	}

};


struct __Reputation_Hatred : public ConditionBase // Dislike value
{
	int nNpcID;
	int nReputationValue_Min;
	int nReputationValue_Max;

		virtual void Init(std::wstring& data) 
	{
		nNpcID = 0;
		nReputationValue_Min = 0;
		nReputationValue_Max = 0;

		if ( data.empty() )
		{
			bUse = false;
			return;
		}

		std::vector<std::wstring> tokens;
		TokenizeW(data, tokens, L":");
		if ( tokens.size() != 3 )
		{
			bUse = false;
			return;
		}

		nNpcID = _wtoi(tokens[0].c_str()); // Required item number
		nReputationValue_Min = _wtoi(tokens[1].c_str());       // Required item count
		nReputationValue_Max = _wtoi(tokens[2].c_str());
		bUse = true;
	}
	virtual bool Check()
	{
		if ( !bUse )
			return true;

		if( !CDnActor::s_hLocalActor ) 
			return false;

		CDnQuestTask* pQuestTask = static_cast<CDnQuestTask*>(CTaskManager::GetInstance().GetTask( "QuestTask" ));
		if( !pQuestTask )
			return false;

		if(pQuestTask)			// Check how many of that item I have
		{
#if defined(PRE_ADD_NPC_REPUTATION_SYSTEM)
			CReputationSystemRepository* pReputationRepos = pQuestTask->GetReputationRepository();
			REPUTATION_TYPE iFavorValue = pReputationRepos->GetNpcReputation( nNpcID, IReputationSystem::NpcMalice ); // Receive the favorability value of the target NPC.

			if(nReputationValue_Min <= iFavorValue && iFavorValue <=nReputationValue_Max) // Operates only when favorability is at or above the specified value.
				return true;
			else if(nReputationValue_Min <= iFavorValue && nReputationValue_Max == -1)
				return true;
			else if(iFavorValue <=nReputationValue_Max && nReputationValue_Min == -1)
				return true;
#endif	// #if defined(PRE_ADD_NPC_REPUTATION_SYSTEM)
		}

		return false;
	}

};

struct __UserLevel : public ConditionBase 
{
	int nMinUserLevel;
	int nMaxUserLevel;
	virtual void Init(std::wstring& data) 
	{
		nMinUserLevel = 0;
		nMaxUserLevel = 0;

		if ( data.empty() )
		{
			bUse = false;
			return;
		}

		std::vector<std::wstring> tokens;
		TokenizeW(data, tokens, L":");
		if ( tokens.size() != 2 )
		{
			bUse = false;
			return;
		}

		nMinUserLevel = _wtoi(tokens[0].c_str());
		nMaxUserLevel = _wtoi(tokens[1].c_str());
		bUse = true;
	}
	virtual bool Check()
	{
		if ( !bUse )
			return true;

		if( !CDnActor::s_hLocalActor ) 
			return false;

		int nPlayerLevel = CDnActor::s_hLocalActor->GetLevel();

		if ( nMinUserLevel <= nPlayerLevel && nPlayerLevel <= nMaxUserLevel )
			return true;

		if ( nMinUserLevel <= nPlayerLevel && nMaxUserLevel == -1 )
			return true; 

		return false;
	}
	
};

/*
struct __WorldZoneLevel : public ConditionBase 
{
int nMinUserWorldZoneLevel;
int nMaxUserWorldZoneLevel;
virtual void Init(std::wstring& data) 
{
if ( data.empty() )
{
bUse = false;
return;
}

std::vector<std::wstring> tokens;
TokenizeW(data, tokens, L":");
if ( tokens.size() != 2 )
{
bUse = false;
return;
}

nMinUserWorldZoneLevel = _wtoi(tokens[0].c_str());
nMaxUserWorldZoneLevel = _wtoi(tokens[1].c_str());
bUse = true;
}

//@ World zone level does not exist yet.
virtual bool Check()
{
return true; 
}
};
*/

struct __UserClass : public ConditionBase 
{
	std::vector<int> ClassIDList;
	virtual void Init(std::wstring& data) 
	{
		if ( data.empty() )
		{
			bUse = false;
			return;
		}

		ClassIDList.clear();

		std::vector<std::wstring> tokens;
		TokenizeW(data, tokens, L":");
		for ( size_t i = 0 ; i < tokens.size() ; i++ )
		{
			ClassIDList.push_back( _wtoi(tokens[i].c_str()));
		}
		bUse = true;
	}
	virtual bool Check()
	{
		if ( !bUse )
			return true;

		if ( ClassIDList.empty() )
			return true;

		if( !CDnActor::s_hLocalActor )
			return false;

		for ( size_t i  = 0 ; i < ClassIDList.size() ; i++ )
		{
			if ( ClassIDList[i] == CDnActor::s_hLocalActor->GetClassID() )
				return true;
		}

		return false; 
	}
};

struct __PrevQuest : public ConditionBase  // List of quests that must unconditionally be held. AND condition
{
	std::vector<UINT> PrevQuestList;
	virtual void Init(std::wstring& data) 
	{
		if ( data.empty() )
		{
			bUse = false;
			return;
		}

		PrevQuestList.clear();

		std::vector<std::wstring> tokens;
		TokenizeW(data, tokens, L":");
		for ( size_t i = 0 ; i < tokens.size() ; i++ )
		{
			PrevQuestList.push_back( _wtoi(tokens[i].c_str()));
		}
		bUse = true;
	}
	virtual bool Check()
	{
		if ( !bUse )
			return true;

		CDnQuestTask* pQuestTask = (CDnQuestTask*)CTaskManager::GetInstance().GetTask("QuestTask");
		if ( !pQuestTask ) return false;

		for ( size_t i  = 0 ; i < PrevQuestList.size() ; i++ )
		{
			if ( pQuestTask->IsClearQuest(PrevQuestList[i]) == false )
				return false;
		}

		return true; 
	}
};

struct __HaveQuest : public ConditionBase  // Activate if having even one of the configured quests (OR condition)
{
	std::vector<UINT> QuestList;
	virtual void Init(std::wstring& data) 
	{
		if ( data.empty() )
		{
			bUse = false;
			return;
		}

		QuestList.clear();

		std::vector<std::wstring> tokens;
		TokenizeW(data, tokens, L",");
		for ( size_t i = 0 ; i < tokens.size() ; i++ )
		{
			QuestList.push_back( _wtoi(tokens[i].c_str()));
		}
		bUse = true;
	}
	virtual bool Check()
	{
		if ( !bUse )
			return true;

		CDnQuestTask* pQuestTask = (CDnQuestTask*)CTaskManager::GetInstance().GetTask("QuestTask");
		if ( !pQuestTask ) return false;

		for ( size_t i  = 0 ; i < QuestList.size() ; i++ )
		{
			if ( pQuestTask->IsClearQuest(QuestList[i]))
				return true;
		}

		return false; 
	}
};

struct __HaveSymbolItem : public ConditionBase 
{
	struct ___item 
	{
		UINT nItemID;
		int nCnt;
	};

	std::vector<___item>	HaveSymbolItemList;

	virtual void Init(std::wstring& data) 
	{
		if ( data.empty() )
		{
			bUse = false;
			return;
		}

		RemoveSpaceW(data);
		std::vector<std::wstring> tokens;
		TokenizeW(data, tokens, L",");

		for ( size_t i = 0 ; i < tokens.size() ; i++ )
		{
			std::vector<std::wstring> child_tokens;
			TokenizeW(tokens[i], child_tokens, L":");
			if ( child_tokens.size() != 2 )
				continue;

			___item item;
			item.nItemID = _wtoi(child_tokens[0].c_str());
			item.nCnt = _wtoi(child_tokens[1].c_str());
			HaveSymbolItemList.push_back(item);
		}

		bUse = true;
	}

	virtual bool Check()
	{
		return true; 
	}
};


struct __CompleteMission : public ConditionBase 
{
	std::vector<int> MissionIDList;
	virtual void Init(std::wstring& data) 
	{
		if ( data.empty() )
		{
			bUse = false;
			return;
		}

		MissionIDList.clear();

		std::vector<std::wstring> tokens;
		TokenizeW(data, tokens, L":");
		for ( size_t i = 0 ; i < tokens.size() ; i++ )
		{
			MissionIDList.push_back( _wtoi(tokens[i].c_str()));
		}
		bUse = true;
	}
	virtual bool Check()
	{
		if ( !bUse )
			return true;

		if ( MissionIDList.empty() )
			return true;

		if( !CDnMissionTask::IsActive() )
			return false;

		for ( size_t i  = 0 ; i < MissionIDList.size() ; i++ )
		{
			int nMissionArrayIndex = MissionIDList[ i ] - 1;		// ArrayIndex is the ItemID written in the table minus 1.
			CDnMissionTask::MissionInfoStruct* pMissionStruct = CDnMissionTask::GetInstance().GetMissionFromArrayIndex( nMissionArrayIndex );
			ASSERT( pMissionStruct != NULL && "QuestCondition : Invalid Mission Index!!" );
			if ( !pMissionStruct || !pMissionStruct->bAchieve ) {
				return false;
			}
		}

		return true;
	}
};

struct __UserPcCafe : public ConditionBase 
{
	char cPcBangGrade;

	virtual void Init(std::wstring& data) 
	{
		cPcBangGrade = 0;

		if ( data.empty() )
		{
			bUse = false;
			return;
		}

		cPcBangGrade = _wtoi(data.c_str());
		bUse = true;
	}
	virtual bool Check()
	{
		if ( !bUse )
			return true;

		CDnCommonTask* pCommonTask = (CDnCommonTask*)CTaskManager::GetInstance().GetTask("CommonTask");
		if(!pCommonTask )
			return false;

		if( pCommonTask->GetPCBangGrade() >= cPcBangGrade )
			return true;

		return false; 
	}
};


void CDnDataManager::_LoadQuestCondition(TiXmlElement* pRootElement, UINT nQuestID)
{
	TiXmlNode* pNode = pRootElement->FirstChild("condition_data");

	if ( !pNode )		return;


	TiXmlElement* pElement = pNode->ToElement();

	if (!pElement) return;

	const int ___BUF_SIZE = 4096;
	WCHAR buff[___BUF_SIZE] = L"";
	const char* sz = NULL;

	QuestCondition* pQuestCondition = new QuestCondition;

	pQuestCondition->nQuestIndex = nQuestID;

	TiXmlElement* pChildElement = pElement->FirstChildElement();

	for ( pChildElement ; pChildElement != NULL ; pChildElement = pChildElement->NextSiblingElement() )
	{
		sz = pChildElement->Value();
		ZeroMemory(&buff, sizeof(WCHAR)*___BUF_SIZE);
		MultiByteToWideChar(CP_UTF8, 0, sz, -1, buff, ___BUF_SIZE );
		std::wstring szString(buff);


		// Quest Date
		if ( szString == L"quest_date" )
		{
			sz = pChildElement->GetText();
			ZeroMemory(&buff, sizeof(WCHAR)*___BUF_SIZE);
			MultiByteToWideChar(CP_UTF8, 0, sz, -1, buff, ___BUF_SIZE );

			__QuestDate* pConBase = new __QuestDate;
			pConBase->Init(std::wstring(buff));
			pQuestCondition->ConditionList.push_back(pConBase);
		}


		// Quest Day
		if( szString == L"quest_day" )
		{
			sz = pChildElement->GetText();
			ZeroMemory(&buff, sizeof(WCHAR)*___BUF_SIZE);
			MultiByteToWideChar(CP_UTF8, 0, sz, -1, buff, ___BUF_SIZE );

			__QuestDay* pConBase = new __QuestDay;
			pConBase->Init(std::wstring(buff));
			pQuestCondition->ConditionList.push_back(pConBase);

		}

		// Quest Item
		if( szString == L"have_normal_item" )
		{
			sz = pChildElement->GetText();
			ZeroMemory(&buff, sizeof(WCHAR)*___BUF_SIZE);
			MultiByteToWideChar(CP_UTF8, 0, sz, -1, buff, ___BUF_SIZE );

			__HaveNormalItem* pConBase = new __HaveNormalItem;
			pConBase->Init(std::wstring(buff));
			pQuestCondition->ConditionList.push_back(pConBase);
		}
		
		// Have Cash Item
		if( szString == L"have_cash_item" )
		{
			sz = pChildElement->GetText();
			ZeroMemory(&buff, sizeof(WCHAR)*___BUF_SIZE);
			MultiByteToWideChar(CP_UTF8, 0, sz, -1, buff, ___BUF_SIZE );

			__HaveCashItem* pConBase = new __HaveCashItem;
			pConBase->Init(std::wstring(buff));
			pQuestCondition->ConditionList.push_back(pConBase);
		}

		// Favorability
		if( szString == L"npc_favor" )
		{
			sz = pChildElement->GetText();
			ZeroMemory(&buff, sizeof(WCHAR)*___BUF_SIZE);
			MultiByteToWideChar(CP_UTF8, 0, sz, -1, buff, ___BUF_SIZE );

			__Reputation_Favor* pConBase = new __Reputation_Favor;
			pConBase->Init(std::wstring(buff));
			pQuestCondition->ConditionList.push_back(pConBase);
		}

		// Negative affinity
		if( szString == L"npc_malice" )
		{
			sz = pChildElement->GetText();
			ZeroMemory(&buff, sizeof(WCHAR)*___BUF_SIZE);
			MultiByteToWideChar(CP_UTF8, 0, sz, -1, buff, ___BUF_SIZE );

			__Reputation_Hatred* pConBase = new __Reputation_Hatred;
			pConBase->Init(std::wstring(buff));
			pQuestCondition->ConditionList.push_back(pConBase);
		}

		// user_level
		if ( szString == L"user_level" )
		{
			sz = pChildElement->GetText();
			ZeroMemory(&buff, sizeof(WCHAR)*___BUF_SIZE);
			MultiByteToWideChar(CP_UTF8, 0, sz, -1, buff, ___BUF_SIZE );

			__UserLevel* pConBase = new __UserLevel;
			pConBase->Init(std::wstring(buff));
			pQuestCondition->ConditionList.push_back(pConBase);
		}
		/*
		// user_worldzone_level
		else if ( szString == L"user_worldzone_level" )
		{
		sz = pChildElement->GetText();
		ZeroMemory(&buff, sizeof(WCHAR)*___BUF_SIZE);
		MultiByteToWideChar(CP_UTF8, 0, sz, -1, buff, ___BUF_SIZE );

		__WorldZoneLevel* pConBase = new __WorldZoneLevel;
		pConBase->Init(std::wstring(buff));
		pQuestCondition->ConditionList.push_back(pConBase);
		}
		*/
		
		// user_class
		if ( szString == L"user_class" )
		{
			sz = pChildElement->GetText();
			ZeroMemory(&buff, sizeof(WCHAR)*___BUF_SIZE);
			MultiByteToWideChar(CP_UTF8, 0, sz, -1, buff, ___BUF_SIZE );

			__UserClass* pConBase = new __UserClass;
			pConBase->Init(std::wstring(buff));
			pQuestCondition->ConditionList.push_back(pConBase);
		}
	
		// prev_quest
		if ( szString == L"prev_quest" )
		{
			sz = pChildElement->GetText();
			ZeroMemory(&buff, sizeof(WCHAR)*___BUF_SIZE);
			MultiByteToWideChar(CP_UTF8, 0, sz, -1, buff, ___BUF_SIZE );

			__PrevQuest* pConBase = new __PrevQuest;
			pConBase->Init(std::wstring(buff));
			pQuestCondition->ConditionList.push_back(pConBase);
		}
		
		// have_quest
		if ( szString == L"have_quest" )
		{
			sz = pChildElement->GetText();
			ZeroMemory(&buff, sizeof(WCHAR)*___BUF_SIZE);
			MultiByteToWideChar(CP_UTF8, 0, sz, -1, buff, ___BUF_SIZE );

			__HaveQuest* pConBase = new __HaveQuest;
			pConBase->Init(std::wstring(buff));
			pQuestCondition->ConditionList.push_back(pConBase);
		}
		
		// have_symbol_item
		if ( szString == L"have_symbol_item" )
		{
			sz = pChildElement->GetText();
			ZeroMemory(&buff, sizeof(WCHAR)*___BUF_SIZE);
			MultiByteToWideChar(CP_UTF8, 0, sz, -1, buff, ___BUF_SIZE );

			__HaveSymbolItem* pConBase = new __HaveSymbolItem;
			pConBase->Init(std::wstring(buff));
			pQuestCondition->ConditionList.push_back(pConBase);
		}
		
		// Complete Mission
		if ( szString == L"complete_mission" )
		{
			sz = pChildElement->GetText();
			ZeroMemory(&buff, sizeof(WCHAR)*___BUF_SIZE);
			MultiByteToWideChar(CP_UTF8, 0, sz, -1, buff, ___BUF_SIZE );

			__CompleteMission* pConBase = new __CompleteMission;
			pConBase->Init(std::wstring(buff));
			pQuestCondition->ConditionList.push_back(pConBase);
		}
		
		// Pc_Cafe_Grade
		if ( szString == L"pc_cafe" )
		{
			sz = pChildElement->GetText();
			ZeroMemory(&buff, sizeof(WCHAR)*___BUF_SIZE);
			MultiByteToWideChar(CP_UTF8, 0, sz, -1, buff, ___BUF_SIZE );

			__UserPcCafe* pConBase = new __UserPcCafe;
			pConBase->Init(std::wstring(buff));
			pQuestCondition->ConditionList.push_back(pConBase);
		}
	}

	m_QuestConditionMap.insert( make_pair(pQuestCondition->nQuestIndex, pQuestCondition ));
}


void CDnDataManager::_RemoveAllQuestCondition()
{

	QUEST_CONDITION_MAP_IT iter;
	iter = m_QuestConditionMap.begin();

	for ( ; iter != m_QuestConditionMap.end() ; iter++ )
	{
		QuestCondition* pQuestCondition = iter->second;
		SAFE_DELETE(pQuestCondition);
	}

	m_QuestConditionMap.clear();
}

QuestCondition* CDnDataManager::GetQuestCondition(UINT nQuestIndex)
{
	QUEST_CONDITION_MAP_IT iter = m_QuestConditionMap.find(nQuestIndex);
	if ( iter != m_QuestConditionMap.end() )
	{
		QuestCondition* pQuestCondition = iter->second;
		return pQuestCondition;
	}
	return NULL;
}


bool QuestCondition::Check()
{
	for ( size_t i = 0 ; i < ConditionList.size() ; i++ )
	{

		ConditionBase* _Condition = (ConditionList[i]);

		if ( _Condition->bUse && _Condition->Check() == false )
		{
			return false;
		}
	}
	return true;
}

bool QuestCondition::ShowCheck()
{
	for ( size_t i = 0 ; i < ConditionList.size() ; i++ )
	{
		ConditionBase* _Condition = (ConditionList[i]);

		if ( _Condition->bUse && _Condition->ShowCheck() == false )
		{
			return false;
		}
	}
	return true;
}



//---------------------------------------------------------------------------------
// Actor (ActorTable.ext - TActorData)
//---------------------------------------------------------------------------------
bool CDnDataManager::LoadActorData()
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TACTOR );
	if (!pSox){
		return false;
	}

	if (pSox->GetItemCount() <= 0){
		return false;
	}

	TActorData *pActorData = NULL;

	for (int i = 0; i < pSox->GetItemCount(); i++){
		pActorData = new TActorData;
		memset(pActorData, 0, sizeof(TActorData));

		pActorData->nActorID = pSox->GetItemID(i);
		m_pActorData.insert(make_pair(pActorData->nActorID, pActorData));
	}

	return true;
}

TActorData* CDnDataManager::GetActorData(int nActorIndex)
{
	if (nActorIndex <= 0) return NULL;
	if (m_pActorData.empty()) return NULL;

	TMapActorData::iterator iter = m_pActorData.find(nActorIndex);
	if (iter != m_pActorData.end()){
		return iter->second;
	}

	return NULL;
}
//---------------------------------------------------------------------------------
// Npc (NPCTable.ext - TNpcData, TActorData)
//---------------------------------------------------------------------------------
bool CDnDataManager::LoadNpcData()
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TNPC);

	if (!pSox){
		return false;
	}

	if (pSox->GetItemCount() <= 0){
		return false;
	}

	TActorData *pActorData = NULL;
	TNpcData *pNpcData = NULL;
	char szTemp[256];
	memset(&szTemp, 0, sizeof(szTemp));
	int ActorIndex = 0;

	for (int i = 0; i < pSox->GetItemCount(); i++){
		pNpcData = new TNpcData;
		memset(pNpcData, 0, sizeof(TNpcData));

		pNpcData->nNpcID = pSox->GetItemID(i);
		//wcscpy_s( pNpcData->wszName, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, pSox->GetFieldFromLablePtr(pNpcData->nNpcID, "_NameID")->GetInteger() ) );
		_wcscpy( pNpcData->wszName, _countof(pNpcData->wszName), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, pSox->GetFieldFromLablePtr(pNpcData->nNpcID, "_NameID")->GetInteger() ),
			(int)wcslen(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, pSox->GetFieldFromLablePtr(pNpcData->nNpcID, "_NameID")->GetInteger() )));
		ActorIndex = pSox->GetFieldFromLablePtr(pNpcData->nNpcID, "_ActorIndex")->GetInteger();
		pActorData = GetActorData(ActorIndex);
		if (pActorData) pNpcData->ActorData = *pActorData;

		DNTableCell *pShopIconCell = pSox->GetFieldFromLablePtr(pNpcData->nNpcID, "_ShopIcon");
		pNpcData->bShopIcon = (pShopIconCell && pShopIconCell->GetInteger() != 0);

		pNpcData->nParam[0] = pSox->GetFieldFromLablePtr(pNpcData->nNpcID, "_Param1")->GetInteger();
		pNpcData->nParam[1] = pSox->GetFieldFromLablePtr(pNpcData->nNpcID, "_Param2")->GetInteger();
		_strcpy(pNpcData->szTalkFile, _countof(pNpcData->szTalkFile), pSox->GetFieldFromLablePtr(pNpcData->nNpcID, "_TalkFile")->GetString(), (int)strlen(pSox->GetFieldFromLablePtr(pNpcData->nNpcID, "_TalkFile")->GetString()));
		_strcpy(pNpcData->szScriptFile, _countof(pNpcData->szScriptFile), pSox->GetFieldFromLablePtr(pNpcData->nNpcID, "_ScriptFile")->GetString(), (int)strlen(pSox->GetFieldFromLablePtr(pNpcData->nNpcID, "_ScriptFile")->GetString()));

		for (int j = 0; j < QUEST_MAX_CNT ; j++){
			sprintf_s(szTemp, "_Quest%d", j + 1);
			pNpcData->QuestIndexArr[j] = pSox->GetFieldFromLablePtr(pNpcData->nNpcID, szTemp)->GetInteger();
		}

		char * szQuestUnmarked = pSox->GetFieldFromLablePtr(pNpcData->nNpcID, "_QuestUnmarked" )->GetString();

		if( szQuestUnmarked[0] != NULL )
		{
			std::vector<std::string> tokens;
			TokenizeA(std::string(szQuestUnmarked), tokens, ",");

			for( DWORD itr = 0; itr < tokens.size(); ++itr )
			{
				if( QUEST_MAX_CNT <= itr )
					break;

				std::string & szString = tokens[itr];

				pNpcData->ReputeQuestIndexArr[itr] = atoi( szString.c_str() );
			}
		}

		m_pNpcData.insert(make_pair(pNpcData->nNpcID, pNpcData));
	}

	return true;
}


TNpcData* CDnDataManager::GetNpcData( int nNpcIndex )
{
	if (m_pNpcData.empty()) 
		return NULL;

	TMapNpcData::iterator iter = m_pNpcData.find(nNpcIndex);
	if (iter != m_pNpcData.end()){
		return iter->second;
	}

	return NULL;

}

bool CDnDataManager::IsExistStartQuestFromNpc( int nNpcIndex, int nQuestIndex )
{
	const TNpcData* pNpcData = GetNpcData(nNpcIndex);
	if(pNpcData)
	{
		for( int i=0; i<QUEST_MAX_CNT; i++ )
		{
			if( pNpcData->QuestIndexArr[i] == nQuestIndex )
				return true;
		}

		for( int n=0; n<QUEST_MAX_CNT; n++ )
		{
			if( pNpcData->ReputeQuestIndexArr[n] == nQuestIndex )
				return true;
		}
	}

	return false;
}

std::vector<std::wstring> CDnDataManager::GetTalkFileList()
{
	std::vector<std::wstring> _list;

	TALK_MAP_IT iter ;
	for ( iter = m_TalkMap.begin() ; iter != m_TalkMap.end() ; iter++ )
	{
		_list.push_back(iter->first);
	}
	return _list;

}

DWORD CDnDataManager::GetStringHashCode(const WCHAR* pString)
{
	std::wstring __wsz(pString);
	ToLowerW(__wsz);
	const WCHAR* pChar = __wsz.c_str();

	DWORD ch;
	DWORD len = (DWORD)wcslen( pChar );
	DWORD result = 5381;
	for( DWORD i = 0 ; i < len ; i++ )
	{
		ch = (unsigned long)pChar[i];
		result = ((result<< 5) + result) + ch; // hash * 33 + ch
	}	  
	return result;
}

bool CDnDataManager::GetTalkFileName(DWORD nHashCode, OUT std::wstring& wszIndex)
{
	TXMLFileMap::iterator it = m_XMLFileMap.find(nHashCode);
	if ( it ==  m_XMLFileMap.end() )
	{
		wszIndex = L"Error";
		return false;
	}

	wszIndex = it->second;
	return true;
}

bool CDnDataManager::GetTalkIndexName(DWORD nHashCode, OUT std::wstring& wszIndex)
{
	TXMLIndexMap::iterator it = m_XMLIndexMap.find(nHashCode);
	if ( it ==  m_XMLIndexMap.end() )
	{
		wszIndex = L"Error";
		return false;
	}
	wszIndex = it->second;
	return true;
}

bool CDnDataManager::LoadQuestRecompense()
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TQUESTREWARD );
	if ( !pSox )
		return false;

	m_QuestRecompenseMap.clear();
	for ( int i = 0; i < pSox->GetItemCount() ; i++ )
	{
		TQuestRecompense recompenseData;
		ZeroMemory(&recompenseData, sizeof(TQuestRecompense));

		int nIndex = pSox->GetItemID(i);
		recompenseData.nIndex = nIndex;

		recompenseData.nQuestIndex = pSox->GetFieldFromLablePtr(nIndex, "_questID")->GetInteger();
		recompenseData.nCoin = pSox->GetFieldFromLablePtr(nIndex, "_Gold")->GetInteger();
		recompenseData.nStringIndex = pSox->GetFieldFromLablePtr(nIndex, "_StringIndex")->GetInteger();
		recompenseData.nExp = pSox->GetFieldFromLablePtr(nIndex, "_Exp")->GetInteger();

		recompenseData.cType = (char)pSox->GetFieldFromLablePtr(nIndex, "_Type")->GetInteger();
		recompenseData.cSelectMax = (char)pSox->GetFieldFromLablePtr(nIndex, "_SelectMax")->GetInteger();

		recompenseData.nOperator = pSox->GetFieldFromLablePtr(nIndex, "_StageLevelOperator")->GetInteger();
		recompenseData.nDifficult = pSox->GetFieldFromLablePtr(nIndex, "_StageLevel")->GetInteger();

		recompenseData.nLevelCapStringIndex = pSox->GetFieldFromLablePtr(nIndex, "_LevelCabStringIndex")->GetInteger();
		recompenseData.nRewardFatigue = pSox->GetFieldFromLablePtr(nIndex, "_GiveFTG")->GetInteger();

		if ( recompenseData.cType == 2 && ( recompenseData.cSelectMax < 1 || recompenseData.cSelectMax > RECOMPENSE_ITEM_MAX ) )
		{
			_ASSERT( false && "Quest recompense table Error");
			LogWnd::Log(1, L"Quest recompense table Error at index [%d]\n", nIndex);
			return false;
		}
		char szLabel[512] = {0,};
		int nMailID = pSox->GetFieldFromLablePtr(nIndex, "_MailID")->GetInteger();
		for ( int j = 0 ; j < RECOMPENSE_ITEM_MAX ; j++ )
		{
			sprintf_s( szLabel, "_ItemIndex%d", j + 1 );
			recompenseData.ItemArray[j].nItemID = pSox->GetFieldFromLablePtr(nIndex, szLabel)->GetInteger();

			sprintf_s( szLabel, "_ItemCount%d", j + 1);
			recompenseData.ItemArray[j].nItemCount = pSox->GetFieldFromLablePtr(nIndex, szLabel)->GetInteger();
		}
		for ( int j = 0 ; j < MAILATTACHITEMMAX ; j++ )
		{
			recompenseData.CashItemSNArray[j] = CDnTableDB::GetInstance().GetMailInfo(nMailID, CDnTableDB::MailInfoRewardItemID, true, j);
		}

		m_QuestRecompenseMap.insert(make_pair(nIndex, recompenseData));
		UINT nQuestIndex = pSox->GetFieldFromLablePtr(nIndex, "_questID")->GetInteger();
		m_QuestIndexRecompenseMap.insert(make_pair(nQuestIndex,nIndex));
	}
	return true;
}

bool CDnDataManager::GetQuestRecompenseByQuestIndex(UINT nQuestID,OUT TQuestRecompense& recom)
{
	QUEST_INDEX_RECOMPENSE_IT it = m_QuestIndexRecompenseMap.find(nQuestID);
	UINT nRewardIndex = it->second;

	if(it == m_QuestIndexRecompenseMap.end())
		return false;
	
	QUEST_RECOMPENSE_MAP_IT reward_it = m_QuestRecompenseMap.find(nRewardIndex);
	
	if ( reward_it == m_QuestRecompenseMap.end() )
		return false;

	CopyMemory(&recom, &(reward_it->second), sizeof(TQuestRecompense));

	return true;
}

bool CDnDataManager::GetQuestRecompense(UINT nRecompenseIndex, OUT TQuestRecompense& recom)
{
	QUEST_RECOMPENSE_MAP_IT it = m_QuestRecompenseMap.find(nRecompenseIndex);
	if ( it == m_QuestRecompenseMap.end() )
	{
		return false;
	}

	CopyMemory(&recom, &(it->second), sizeof(TQuestRecompense));

	return true;
}

bool CDnDataManager::LoadQuestLevelCapRecompense()
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TQUEST_LEVELCAP_REWARD );
	if ( !pSox )
		return false;

	char szLabel[512] = {0,};
	for( int itr = 0; itr < pSox->GetItemCount(); ++itr )
	{
		TQuestLevelCapRecompense levelCapRecompenseData;
		ZeroMemory( &levelCapRecompenseData, sizeof(TQuestLevelCapRecompense) );

		int nIndex = pSox->GetItemID( itr );

		levelCapRecompenseData.cType = pSox->GetFieldFromLablePtr(nIndex, "_QuestType")->GetInteger();
		levelCapRecompenseData.cClassID = pSox->GetFieldFromLablePtr(nIndex, "_class")->GetInteger();

		for( int jtr = 1; jtr <= MAX_QUEST_LEVEL_CAP_REWARD; ++jtr )
		{
			sprintf_s( szLabel, _countof(szLabel), "_ItemIndex%d", jtr );
			levelCapRecompenseData.RecompenseItem[jtr - 1].nItemID = pSox->GetFieldFromLablePtr(nIndex, szLabel)->GetInteger();

			sprintf_s( szLabel, _countof(szLabel), "_ItemCount%d", jtr );
			levelCapRecompenseData.RecompenseItem[jtr - 1].nItemCount = pSox->GetFieldFromLablePtr(nIndex, szLabel)->GetInteger();
		}

		m_QuestLevelCapRecompenseMap.insert( QUEST_LEVEL_CAP_RECOMPENSE_MAP::value_type(std::make_tuple( levelCapRecompenseData.cType, levelCapRecompenseData.cClassID ), levelCapRecompenseData ) );
	}

	return true;
}

bool CDnDataManager::GetQuestLevelCapRecompense( const char cQuestType, const char cClassID, OUT TQuestLevelCapRecompense & questLevelCapRecompense )
{
	QUEST_LEVEL_CAP_RECOMPENSE_IT it = m_QuestLevelCapRecompenseMap.find( std::make_tuple(cQuestType, cClassID) );

	if( it == m_QuestLevelCapRecompenseMap.end() )
		return false;

	CopyMemory(&questLevelCapRecompense, &(it->second), sizeof(TQuestLevelCapRecompense));

	return true;
}

std::wstring CDnDataManager::GetChapterString(int nChapterIdx)
{
	const static std::wstring __gsError(L"TableError");
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TQUESTCHAPTER );
	if ( !pSox )
		return __gsError;


	for ( int i = 0; i < pSox->GetItemCount() ; i++ )
	{
		int nItemID = pSox->GetItemID(i);
		int __nChapterIndex = pSox->GetFieldFromLablePtr(nItemID, "_ChapterIndex")->GetInteger();
		int nChapterStringIndex = pSox->GetFieldFromLablePtr(nItemID, "_ChapterStringIndex")->GetInteger();
		if ( nChapterIdx == __nChapterIndex )
		{
			return GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nChapterStringIndex ) ;
		}
	}

	return __gsError;
}


std::string CDnDataManager::GetChapterImageFileName(int nChapterIdx)
{
	const static std::string __gsError("TableError");
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TQUESTCHAPTER );
	if ( !pSox )
		return __gsError;


	for ( int i = 0; i < pSox->GetItemCount() ; i++ )
	{
		int nItemID = pSox->GetItemID(i);
		int __nChapterIndex = pSox->GetFieldFromLablePtr(nItemID, "_ChapterIndex")->GetInteger();
		if ( nChapterIdx == __nChapterIndex )
		{
			std::string szFileName = pSox->GetFieldFromLablePtr(nItemID, "_ChapterImageFile")->GetString();
			return szFileName;
		}
	}

	return __gsError;
}


std::string CDnDataManager::GetQuestImageFileName(int nQuestIndex)
{
	const static std::string __gsError("TableError");
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TQUEST );
	if ( !pSox )
		return __gsError;

#ifdef PRE_FIX_MEMOPT_EXT
	DNTableFileFormat*  pFileNameSox = GetDNTable( CDnTableDB::TFILE );
	if (!pFileNameSox)
		return __gsError;
#endif

	for ( int i = 0; i < pSox->GetItemCount() ; i++ )
	{
		int nItemID = pSox->GetItemID(i);
		if ( nItemID == nQuestIndex )
		{
#ifdef PRE_FIX_MEMOPT_EXT
			std::string szFileName;
			CommonUtil::GetFileNameFromFileEXT(szFileName, pSox, nItemID, "_QuestImageTitle", pFileNameSox);
#else
			std::string szFileName = pSox->GetFieldFromLablePtr(nItemID, "_QuestImageTitle")->GetString();
#endif
			return szFileName;
		}
	}

	return __gsError;
}

char CDnDataManager::GetQuestType(int nQuestIndex) const
{
	DN_ASSERT(CHECK_RANGE(nQuestIndex, 1, MAX_QUEST_INDEX),	"Check!");

	DNTableFileFormat*  pSox = GetDNTable( CDnTableDB::TQUEST );
	DN_ASSERT(NULL != pSox,	"Invalid!");

	DNTableCell* pSoxField = pSox->GetFieldFromLablePtr(nQuestIndex, "_QuestType");
	if (!pSoxField) {
		DN_RETURN(QuestType_Min);
	}

	char ret = (static_cast<char>(pSoxField->GetInteger()));

	if( QuestType_SubQuest == ret )
	{
		if( IsPeriodQuest( nQuestIndex ) )
			ret = QuestType_PeriodQuest;
	}

	return ret;
}

bool CDnDataManager::GetQuestHideType(int nQuestIndex)
{
	bool IsQuestHide = false;

	DN_ASSERT(CHECK_RANGE(nQuestIndex, 1, MAX_QUEST_INDEX),	"Check!");
	DNTableFileFormat*  pSox = GetDNTable( CDnTableDB::TQUEST );
	DN_ASSERT(NULL != pSox,	"Invalid!");

	if(pSox)
	{
		IsQuestHide = ( pSox->GetFieldFromLablePtr( nQuestIndex, "_QuestMarkHide" )->GetInteger() == TRUE ) ? true : false;
	}

	return IsQuestHide;
}

#ifdef PRE_ADD_LOWLEVEL_QUEST_HIDE
bool CDnDataManager::IsEventQuest( int nQuestIndex )
{
	DN_ASSERT(CHECK_RANGE(nQuestIndex, 1, MAX_QUEST_INDEX),	"Check!");
	DNTableFileFormat*  pSox = GetDNTable( CDnTableDB::TQUEST );
	DN_ASSERT(NULL != pSox,	"Invalid!");

	return ( pSox->GetFieldFromLablePtr( nQuestIndex, "_IsEvent" )->GetInteger() == TRUE ) ? true : false;
}

bool CDnDataManager::IsLowLevelQuest( Journal* pJournal )
{
	if( pJournal->nQuestType == Journal::emQuestType_Main || pJournal->nQuestType == Journal::emQuestType_Rep )
		return false;

	if( IsEventQuest( pJournal->nQuestIndex ) )
		return false;
		
	if( GetQuestHideType( pJournal->nQuestIndex ) == false ) // Event and job-change quest filtering condition 
		return false;
	
	return ( CDnActor::s_hLocalActor && CDnActor::s_hLocalActor->GetLevel() >= pJournal->nQuestLevel + 10 );
}	

#endif 


#include "stdafx.h"
#include "EtTrigger.h"
#include "EtTriggerObject.h"
#include "EtTriggerElement.h"
#include <map>
using namespace std;

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif // #if !defined( USE_BOOST_MEMPOOL )


CEtTrigger::CEtTrigger( CEtWorldSector *pSector )
{
	m_bEnable = true;
	m_pSector = pSector;

	EventCallbackStruct Struct;
	Struct.szFuncName = "CEtTrigger::Process";
	m_VecEventTrigger.push_back( Struct );

	m_LocalTime = 0;

#ifdef PRE_FIX_62845
	m_vecSzErrLuaFile.clear();
#endif
}

CEtTrigger::~CEtTrigger()
{
	SAFE_DELETE_PVEC( m_pVecTriggerList );
	SAFE_DELETE_VEC( m_VecEventTrigger );
	SAFE_DELETE_PVEC( m_pVecDefineValueList );
}

bool CEtTrigger::Load( const char *szFileName )
{
	CResMngStream Stream( szFileName );
	if( !Stream.IsValid() ) return true;

	int nCount;
	Stream.Read( &nCount, sizeof(int) );
	for( int i=0; i<nCount; i++ ) {
		CEtTriggerObject *pObject = AllocTriggerObject();
		if( pObject->Load( &Stream ) == false ) {
			SAFE_DELETE( pObject );
			continue;
		}
		pObject->SetCreateIndex( i );
		CalcEventCallback( pObject );
		m_pVecTriggerList.push_back( pObject );
		ThreadDelay();
	}

	return true;
}

bool CEtTrigger::LoadDefine( const char *szFileName )
{
	CResMngStream Stream( szFileName );
	if( !Stream.IsValid() ) return true;

	int nCount;
	Stream.Read( &nCount, sizeof(int) );

	std::string szStr;
	ParamTypeEnum ParamType;
	int nUniqueID;
	DefineValue *pValue;
	for( int i=0; i<nCount; i++ ) {
		Stream.Read( &nUniqueID, sizeof(int) );
		ReadStdString( szStr, &Stream );
		Stream.Read( &ParamType, sizeof(int) );

		pValue = new DefineValue;
		pValue->nUniqueID = nUniqueID;
		pValue->ParamType = ParamType;
		pValue->szName = szStr;

		switch( pValue->ParamType ) {
			case CEtTrigger::Integer:
				Stream.Read( &pValue->nValue, sizeof(int) );
				break;
			case CEtTrigger::Float:
				Stream.Read( &pValue->fValue, sizeof(float) );
				break;
			case CEtTrigger::String:
				{
					ReadStdString( szStr, &Stream );
					pValue->szValue = new char[szStr.size()+1];
					memset( pValue->szValue, 0, szStr.size()+1 );
					memcpy( pValue->szValue, szStr.c_str(), szStr.size() );
				}
				break;
		}
		m_pVecDefineValueList.push_back( pValue );
	}
	return true;
}

CEtTriggerObject *CEtTrigger::AllocTriggerObject()
{
	return new IBoostPoolEtTriggerObject( this );
}

void CEtTrigger::InsertEventCallback( const char *szFuncName, CEtTriggerObject *pObject )
{
	for( DWORD i=0; i<m_VecEventTrigger.size(); i++ ) {
		if( strcmp( szFuncName, m_VecEventTrigger[i].szFuncName.c_str() ) == NULL ) {
			m_VecEventTrigger[i].pVecObject.push_back( pObject );
			return;
		}
	}
}

void CEtTrigger::CalcEventCallback( CEtTriggerObject *pObject )
{
	if( pObject->GetEventCount() == 0 ) {
		pObject->SetTick( 1000 );
		InsertEventCallback( "CEtTrigger::Process", pObject );
		return;
	}
	for( DWORD j=0; j<pObject->GetEventCount(); j++ ) {
		CEtTriggerElement *pElement = pObject->GetEventFromIndex(j);
		char *szFuncName = pElement->GetLuaVariable<char *>( "g_CallbackFunc" );
		if( szFuncName == NULL ) continue;
		int nID = pElement->GetLuaVariable<int>( "g_CallbackDefineID" );
		lua_tinker::table EventValueStore = pElement->GetLuaVariable<lua_tinker::table>( "g_CallbackStore" );

		for( int i=1; i<=EventValueStore.getSize(); i++ ) {
			pObject->InsertEventValueStore( EventValueStore.get<char *>(i), -1 );
		}

		int nIndex = -1;
		for( DWORD i=0; i<m_VecEventTrigger.size(); i++ ) {
			if( strcmp( m_VecEventTrigger[i].szFuncName.c_str(), szFuncName ) == NULL ) {
				nIndex = i;
				break;
			}
		}
		if( nIndex == -1 ) {
			EventCallbackStruct Struct;
			Struct.szFuncName = szFuncName;
			Struct.pVecObject.push_back( pObject );

			for( int i=1; i<=EventValueStore.getSize(); i++ ) {
				Struct.szMapStoreList.insert( make_pair( EventValueStore.get<char *>(i), true ) );
			}

			m_VecEventTrigger.push_back( Struct );
		}
		else {
			m_VecEventTrigger[nIndex].pVecObject.push_back( pObject );
		}
		RegisterEventParam( szFuncName, nID, pObject, pElement );
	}
}

CEtTriggerObject *CEtTrigger::GetTriggerFromName( const char *szTriggerName )
{
	for( DWORD i=0; i<m_pVecTriggerList.size(); i++ ) {
		if( strcmp( m_pVecTriggerList[i]->GetTriggerName(), szTriggerName ) == NULL ) {
			return m_pVecTriggerList[i];
		}
	}
	return NULL;
}

CEtTriggerObject *CEtTrigger::GetTriggerFromIndex( DWORD dwIndex )
{
	if( dwIndex >= m_pVecTriggerList.size() ) return NULL;
	return m_pVecTriggerList[dwIndex];
}


void CEtTrigger::Process( LOCAL_TIME LocalTime, float fDelta )
{
	if( !m_bEnable ) return;
	m_LocalTime = LocalTime;
	OnEventCallback( "CEtTrigger::Process", LocalTime, fDelta );

	for( DWORD i=0; i<m_pVecActionTriggerQueue.size(); i++ ) {
		CEtTriggerObject *pObject = m_pVecActionTriggerQueue[i];
		if( pObject->ProcessQueue( LocalTime, fDelta ) == true ) {
			m_pVecActionTriggerQueue.erase( m_pVecActionTriggerQueue.begin() + i );
			if( !pObject->IsRepeat() ) {
				FindAndRemoveEventCallbackObject( pObject );
			}
			i--;
		}
	}
}

void CEtTrigger::OnEventCallback( const char *szFuncName, LOCAL_TIME LocalTime, float fDelta, bool bProcessQueue )
{
	if( !m_bEnable ) return;
	for( DWORD i=0; i<m_VecEventTrigger.size(); i++ ) {
		EventCallbackStruct *pStruct = &m_VecEventTrigger[i];
		if( strcmp( szFuncName, pStruct->szFuncName.c_str() ) == NULL )  {
			for( DWORD j=0; j<pStruct->pVecObject.size(); j++  ) {
				CEtTriggerObject *pObject = pStruct->pVecObject[j];
				if( !pObject->IsCanProcess( LocalTime, fDelta ) ) continue;
				bool bProcess = pObject->Process( LocalTime, fDelta );
				if( !bProcessQueue && bProcess ) {
					if( pObject->ProcessQueue( LocalTime, fDelta ) == true ) {
						std::vector<CEtTriggerObject *>::iterator it = std::find( m_pVecActionTriggerQueue.begin(), m_pVecActionTriggerQueue.end(), pObject );
						if( it != m_pVecActionTriggerQueue.end() ) m_pVecActionTriggerQueue.erase( it );
						if( !pObject->IsRepeat() ) {
							FindAndRemoveEventCallbackObject( pObject );
							j--;
							continue;
						}
					}
				}
			}
		}
	}
}

void CEtTrigger::RegisterEventParam( const char *szFuncName, int nDefineID, CEtTriggerObject *pObject, CEtTriggerElement *pEvent )
{
	if( strcmp( szFuncName, "CEtTrigger::Process" ) == NULL ) {
		CEtTriggerElement::ParamStruct *pStruct = pEvent->GetParameterStruct(0);
		if( !pStruct ) return;
		pObject->SetTick( pStruct->nValue * 1000 );
	}
}

void CEtTrigger::FindAndRemoveEventCallbackObject( CEtTriggerObject *pObject )
{
	for( DWORD i=0; i<m_VecEventTrigger.size(); i++ ) {
		for( DWORD j=0; j<m_VecEventTrigger[i].pVecObject.size(); j++ ) {
			if( m_VecEventTrigger[i].pVecObject[j] == pObject ) {
				m_VecEventTrigger[i].pVecObject.erase( m_VecEventTrigger[i].pVecObject.begin() + j );
				j--;
			}
		}
	}
	
	for( DWORD i=0; i<m_pVecActionTriggerQueue.size(); i++ ) {
		if( m_pVecActionTriggerQueue[i] == pObject ) {
			m_pVecActionTriggerQueue.erase( m_pVecActionTriggerQueue.begin() + i );
			i--;
		}
	}

	for( DWORD i=0; i<m_pVecTriggerList.size(); i++ ) {
		if( m_pVecTriggerList[i] == pObject ) {
			SAFE_DELETE( m_pVecTriggerList[i] );
			m_pVecTriggerList.erase( m_pVecTriggerList.begin() + i );
			break;
		}
	}
}

void CEtTrigger::EnableEventCallback( bool bEnable )
{
	m_bEnable = bEnable;
}


void CEtTrigger::RegisterDefineValue( lua_State *pState )
{
	lua_tinker::table table( pState );
	for( int i=0; i<256; i++ ) {
		table.set<int>( i, 0 );
	}
	for( DWORD i=0; i<m_pVecDefineValueList.size(); i++ ) {
		DefineValue *pValue = m_pVecDefineValueList[i];

		switch( pValue->ParamType ) {
			case Integer: table.set<int>( pValue->nUniqueID, pValue->nValue ); break;
			case Float:   table.set<float>( pValue->nUniqueID, pValue->fValue ); break;
			case String:  table.set<char *>( pValue->nUniqueID, pValue->szValue ); break;
		}
	}

	lua_tinker::set<lua_tinker::table>( pState, "g_value", table );

	// Let's put just 5 custom tables usable directly from Script itself
	lua_tinker::table CustomTable( pState );
	for( int i=0; i<5; i++ ) {
		CustomTable.set<int>( i, 0 );
	}
	lua_tinker::set<lua_tinker::table>( pState, "g_customvalue", CustomTable );
}

void CEtTrigger::InsertActionQueue( CEtTriggerObject *pObject )
{
	m_pVecActionTriggerQueue.push_back( pObject );
}

bool CEtTrigger::IsProcessActionQueue( CEtTriggerObject *pObject )
{
	if( std::find( m_pVecActionTriggerQueue.begin(), m_pVecActionTriggerQueue.end(), pObject ) != m_pVecActionTriggerQueue.end() ) return true;
	return false;
}

void CEtTrigger::InsertTriggerEventStore( const char *szValueStr, int nValue )
{
	for( DWORD i=0; i<m_VecEventTrigger.size(); i++ ) {
		EventCallbackStruct *pStruct = &m_VecEventTrigger[i];
		if( pStruct->szMapStoreList.find( szValueStr ) == pStruct->szMapStoreList.end() ) continue;

		for( DWORD j=0; j<pStruct->pVecObject.size(); j++  ) {
			CEtTriggerObject *pObject = pStruct->pVecObject[j];
			pObject->InsertEventValueStore( szValueStr, nValue );
		}

	}
}

#ifdef PRE_FIX_62845
void CEtTrigger::SetErrLuaName( std::string szFileName )
{
	if( !m_vecSzErrLuaFile.empty() )
	{
		std::vector<std::string>::iterator it = m_vecSzErrLuaFile.begin();
		for( ; it != m_vecSzErrLuaFile.end() ; ++it )
		{
			if( strcmp( it->c_str() , szFileName.c_str() ) == 0 )
			{
				return;
			}
		}
	}

	m_vecSzErrLuaFile.push_back(szFileName);
}

void CEtTrigger::ShowErrLuaName()
{
	if(!m_vecSzErrLuaFile.empty())
	{
		char pBuff[256] = { 0 , };
		std::string szFinalMessage;
		std::vector<std::string>::iterator it = m_vecSzErrLuaFile.begin();
		for( ; it != m_vecSzErrLuaFile.end() ; ++it )
		{
			sprintf_s( pBuff, 256, "%s\n", it->c_str() );
			szFinalMessage += pBuff;
		}
		
		char*	pBuff2 = NULL;
		int		nBuff2Size = (int)szFinalMessage.length() + 256;
		pBuff2 = new char[nBuff2Size];
		sprintf_s( pBuff2, nBuff2Size, "Failed to read trigger lua file.\n\n%s\nPlease check the lua files.", szFinalMessage.c_str() );
		MessageBoxA(NULL, pBuff2, "Error", MB_ICONWARNING | MB_OK);

		delete[] pBuff2;
		pBuff2 = NULL;
	}
}
#endif
#include "stdafx.h"
#include "DNChatRoom.h"
#include "DNChatRoomManager.h"

CDNChatRoomManager * g_pChatRoomManager = NULL;

CDNChatRoomManager::CDNChatRoomManager()
{
	ScopeLock<CSyncLock> Lock(m_Sync);

	m_pVecChatRoomList.clear();
	m_pMapChatRoomList.clear();

	m_nLastRoomID = 0;
}

CDNChatRoomManager::~CDNChatRoomManager()
{
	if (m_pVecChatRoomList.empty() && m_pMapChatRoomList.empty()) return;

	ScopeLock<CSyncLock> Lock(m_Sync);

	if (!m_pVecChatRoomList.empty())
	{
		std::vector <CDNChatRoom*>::iterator ii;
		for (ii = m_pVecChatRoomList.begin(); ii != m_pVecChatRoomList.end(); ii++)
			SAFE_DELETE((*ii));
	}

	m_pVecChatRoomList.clear();
	m_pMapChatRoomList.clear();
}

CDNChatRoom * CDNChatRoomManager::GetChatRoom( UINT ChatRoomID )
{
	if( ChatRoomID <= 0 )		return NULL;

	ScopeLock<CSyncLock> Lock(m_Sync);

	std::map <UINT, CDNChatRoom*>::iterator ii = m_pMapChatRoomList.find( ChatRoomID );
	if (ii == m_pMapChatRoomList.end() )		return NULL;
	
	return (*ii).second;
}

int CDNChatRoomManager::CreateChatRoom( UINT uLeaderAID, WCHAR * pwzRoomName, BYTE nRoomType, USHORT uEnterAllow, WCHAR * pwzPassword,
									    WCHAR * PRLine1, WCHAR * PRLine2, WCHAR * PRLine3 )
{
	if( uLeaderAID <= 0 )				return 0;
	if( wcslen( pwzRoomName ) <= 0 )	return 0;

	ScopeLock<CSyncLock> Lock(m_Sync);

	// Create new room
	CDNChatRoom * pNewChatRoom = new CDNChatRoom( _GetNewRoomID(), pwzRoomName, nRoomType, uEnterAllow, pwzPassword, PRLine1, PRLine2, PRLine3 );
	if( pNewChatRoom == NULL )		return 0;
	m_pVecChatRoomList.push_back( pNewChatRoom );
	m_pMapChatRoomList[pNewChatRoom->GetChatRoomID()] = pNewChatRoom;

	return pNewChatRoom->GetChatRoomID();
}

void CDNChatRoomManager::DestroyChatRoom( UINT nChatRoomID )
{
	CDNChatRoom * pChatRoom = GetChatRoom( m_nLastRoomID );
	if( pChatRoom == NULL )			return;

	std::vector <CDNChatRoom*>::iterator ih;
	for (ih = m_pVecChatRoomList.begin(); ih != m_pVecChatRoomList.end(); ih++)
	{
		if ((*ih)->GetChatRoomID() == nChatRoomID)
		{
			m_pVecChatRoomList.erase( ih );
			m_pMapChatRoomList.erase( nChatRoomID );
			break;
		}
	}
}

UINT CDNChatRoomManager::_GetNewRoomID()
{
	m_nLastRoomID++;

	if( m_nLastRoomID > 100000000 )		m_nLastRoomID = 1;

	CDNChatRoom * pChatRoom = GetChatRoom( m_nLastRoomID );
	while( pChatRoom != NULL )
	{
		m_nLastRoomID++;
		pChatRoom = GetChatRoom( m_nLastRoomID );
	}

	return m_nLastRoomID;
}
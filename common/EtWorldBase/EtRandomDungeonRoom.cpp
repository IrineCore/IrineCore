#include "stdafx.h"
#include "EtRandomDungeonRoom.h"
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CEtRandomDungeonRoom::CEtRandomDungeonRoom( RoomType Type )
{
	m_RoomType = Type;
	m_Position = IntVec3( -1, -1, -1 );
	m_nWidth = m_nHeight = -1;
}

CEtRandomDungeonRoom::~CEtRandomDungeonRoom()
{
	SAFE_DELETE_VEC( m_VecDoorList );
}

CEtRandomDungeonRoom::RoomType CEtRandomDungeonRoom::GetRoomType()
{
	return m_RoomType;
}

void CEtRandomDungeonRoom::Initialize( int nX, int nY, int nZ, int nWidth, int nHeight )
{
	m_Position = IntVec3( nX, nY, nZ );
	m_nWidth = nWidth;
	m_nHeight = nHeight;
}

bool CEtRandomDungeonRoom::AddDoor( int nX, int nY, DoorDirection Direction )
{
	// Doors can only be placed on the side.
	if( !( nX == 0 || nX == m_nWidth - 1 || nY == 0 || nY == m_nHeight - 1 ) ) return false;

	for( DWORD i=0; i<m_VecDoorList.size(); i++ ) {
		if( m_VecDoorList[i].nX == nX && m_VecDoorList[i].nY == nY && m_VecDoorList[i].Direction == Direction ) return false;
	}

	DoorStruct Struct;
	Struct.nX = nX;
	Struct.nY = nY;
	Struct.Direction = Direction;
	m_VecDoorList.push_back( Struct );

	return true;
}


IntVec3 *CEtRandomDungeonRoom::GetPosition()
{
	return &m_Position;
}

int CEtRandomDungeonRoom::GetWidth()
{
	return m_nWidth;
}

int CEtRandomDungeonRoom::GetHeight()
{
	return m_nHeight;
}

int CEtRandomDungeonRoom::GetDoorCount()
{
	return (int)m_VecDoorList.size();
}

CEtRandomDungeonRoom::DoorStruct *CEtRandomDungeonRoom::GetDoor( int nIndex )
{
	if( nIndex < 0 || nIndex >= (int)m_VecDoorList.size() ) return NULL;
	return &m_VecDoorList[nIndex];
}

std::vector<CEtRandomDungeonRoom::DoorStruct> *CEtRandomDungeonRoom::GetDoorList()
{
	return &m_VecDoorList;
}
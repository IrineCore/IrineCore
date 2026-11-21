#pragma once

#include "Common.h"
class CEtRandomDungeonRoom {
public:
	enum RoomType {
		RT_CONSTRAINT,
		RT_RANDOM,
	};

	enum DoorDirection {
		DD_NONE = 0,
		DD_WEST = 0x0001,
		DD_EAST = 0x0002,
		DD_NORTH = 0x0004,
		DD_SOUTH = 0x0008,
//		DD_UP = 0x0010,
//		DD_DOWN = 0x0020,
	};

	struct DoorStruct {
		int nX;
		int nY;
		DoorDirection Direction;
	};

public:
	CEtRandomDungeonRoom( RoomType Type = RT_CONSTRAINT );
	virtual ~CEtRandomDungeonRoom();

protected:
	RoomType m_RoomType;
	IntVec3 m_Position;
	int m_nWidth;
	int m_nHeight;
	std::vector<DoorStruct> m_VecDoorList;

public:
	RoomType GetRoomType();

	void Initialize( int nX, int nY, int nZ, int nWidth, int nHeight );
	bool AddDoor( int nX, int nY, DoorDirection Direction );

	IntVec3 *GetPosition();
	int GetWidth();
	int GetHeight();
	int GetDoorCount();
	DoorStruct *GetDoor( int nIndex );
	std::vector<DoorStruct> *GetDoorList();
};
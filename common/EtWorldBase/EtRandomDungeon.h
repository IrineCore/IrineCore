#pragma once

#include "Common.h"
#include "Timer.h"

class CEtMaze;
class CEtMazeMask;
class CEtRandomDungeonRoom;
class CEtRandomDungeon {
public:
	CEtRandomDungeon();
	virtual ~CEtRandomDungeon();

	enum BlockAttribute {
		BA_NONE = 0,
		BA_BLOCK = 0x0001,
		BA_PASSAGE = 0x0002,
		BA_ROOM = 0x0004,
		BA_DOOR = 0x0008,

		// Direction
		BA_WEST = 0x0010,
		BA_EAST = 0x0020,
		BA_NORTH = 0x0040,
		BA_SOUTH = 0x0080,
		BA_UP = 0x0100,
		BA_DOWN = 0x0200,
	};

protected:
	struct WEIGHTEDLIST {
		long nData;
		int nWeight;
		WEIGHTEDLIST* pNext;
	};

	struct RandomRoomPossibleStruct {
		int nWidth;
		int nHeight;
		int nWeight;
	};

	CEtMaze *m_pMaze;
	CEtMazeMask *m_pMazeMask;
	IntVec3 m_DungeonSize;
	EtVector3 m_vOffset;
	int ***m_pDungeonArray;
	float m_fBlockSize;

	int m_nSeed;

	std::vector<CEtRandomDungeonRoom *> m_pVecRoomList;

	int m_nRandomRoomCount;
	int m_nTotalRandomRoomWeight;
	std::vector<RandomRoomPossibleStruct> m_VecRandomRoomList;

protected:
	void CreateMazeMask( int nWidth, int nHeight, int nLevel );
	bool GenerateMaze( int nWidth, int nHeight, int nLevel, int nSparseness, int nRandomness, int nDeadendRemove );

	int AddConstraintRoom( CEtRandomDungeonRoom *pRoom, bool bCheckRect = false );
	bool AddRandomRoom( CEtRandomDungeonRoom *pRoom );
	void AddRandomRoomPossibleSize( int nWidth, int nHeight, int nWeight );
	void SetRandomRoomCount( int nValue );

	void InitializeSeed( int nSeed );


protected:
	bool CalcDungeonArray();
	void AllocDungeonArray();
	void FreeDungeonArray();

	bool GenerateRandomRoom( int nRoomCount, int nMinWidth, int nMaxWidth, int nMinHeight, int nMaxHeight );
	bool GenerateRandomRoom();
	void SetMaskRoom( int nX, int nY, int nZ, int nWidth, int nHeight );
	void FindSideDoor( int nX, int nY, int nZ, int nWidth, int nHeight, CEtRandomDungeonRoom *pRoom );

	bool FindOptimalRoomPlacement( int &nWidth, int &nHeight, int nLevel, int &nResultX, int &nResultY );

	int AddToWeightedList( WEIGHTEDLIST** list, long data, int weight );
	long GetWeightedItem( WEIGHTEDLIST** list, int index, int* count );
	void DestroyWeightedList( WEIGHTEDLIST** list );
	int RollDice( int count, int sides );

	int RollRandomRoom();

public:
	virtual bool Initialize( int nSeed = 0 );
	virtual void Destroy();

	virtual void Render( LOCAL_TIME LocalTime );

	bool Load( const char *szFileName );
	int GetDungeonAt( int nX, int nY, int nZ );

	int GetRoomCount();
	CEtRandomDungeonRoom *GetRoom( int nIndex );

	EtVector3 *GetOffset();
	float GetBlockSize();
	IntVec3 *GetDungeonSize();

	int GetCurSeed() { return m_nSeed; }
};
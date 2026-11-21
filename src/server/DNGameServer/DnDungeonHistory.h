#pragma once

#include "Common.h"
#include <bitset>
using namespace std;

class CDnGameTask;
class CDnDungeonHistory {
public:
	CDnDungeonHistory( CDnGameTask *pTask );
	virtual ~CDnDungeonHistory();

protected:
	CDnGameTask *m_pGameTask;
	struct SectorStruct {
		SectorIndex Index;
		std::map<DWORD,DWORD> dwMapLivePropList;
	};
	int m_nRandomSeed;
	std::map<DWORD,DWORD> m_dwMapLiveUnitList;
	std::vector<SectorStruct> m_VecLiveSectorList;

public:
	bool Backup();
	bool Recovery();

	int GetRandomSeed() { return m_nRandomSeed; }
	void SetRandomSeed( int nValue ) { m_nRandomSeed = nValue; }
};

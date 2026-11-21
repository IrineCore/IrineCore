#pragma once

#include "KdTree.h"
#include "MultiSingleton.h"
#include "MultiCommon.h"
#include "MemPool.h"

class CEtCollisionMng : public CMultiSingleton< CEtCollisionMng, MAX_SESSION_COUNT >, public TBoostMemoryPool<CEtCollisionMng>
{
public:
	CEtCollisionMng( CMultiRoom *pRoom );
	virtual ~CEtCollisionMng(void);
	void Clear();

	struct SKdTreeData
	{
		TKdTree< SKdTreeCollisionPrimitive > *pKdTree;
		int nRefCount;
	};

protected:
	TKdTree< SKdTreeCollisionPrimitive > *m_pCurKdTree;

	static std::map< long, SKdTreeData > s_mapKdTree;
	static CSyncLock s_Lock;

public:
	bool FindCollision( std::vector< SCollisionPrimitive * > &vecPrimitive, SAABox &BoundingBox, EtVector3 &vMove, DNVector(SCollisionResponse) &vecResponse, bool bCalcContactTime );

	void Load( CStream *pStream );

	static void PreLoad( CStream *pStream );
};

#pragma once

#include "DNBaseObject.h"
#include "Util.h"

/*---------------------------------------------------------------------------------------
									CDNBaseObjectManager

	m_KeyObjects(TMapConnections): first - ObjectUID
	m_NameKeyObjects(TMapNameObjects): first - CharacterName
---------------------------------------------------------------------------------------*/

typedef map<std::wstring, CDNBaseObject*> TMapNameObjects;
typedef vector<CDNBaseObject*> TVecObjects;

class CDNBaseObjectManager
{
private:
protected:
	TMapObjects m_KeyObjects;
	TMapNameObjects m_NameKeyObjects;
	TVecObjects m_VecObjects;	

	CSyncLock m_KeyLock;
	CSyncLock m_NameKeyLock;
	CSyncLock m_VecLock;

	int m_nProcessCount;

	virtual void Clear();

public:
	CDNBaseObjectManager(void);
	virtual ~CDNBaseObjectManager(void);

	virtual void DoUpdate(DWORD CurTick);

	bool AddObject(CDNBaseObject *pObj);
	bool DelObject(CDNBaseObject *pObj);

	CDNBaseObject* FindObject(UINT nUID);
	CDNBaseObject* FindObjectByName(const WCHAR *pwszName);

	int GetObjCount();
	bool IsEmptyObject();	// Check if map or vector is empty
};

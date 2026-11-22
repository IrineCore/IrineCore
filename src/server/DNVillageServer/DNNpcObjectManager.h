#pragma once

#include "DNBaseObjectManager.h"

class CDNNpcObject;

class CDNNpcObjectManager: public CDNBaseObjectManager
{
public:
	CDNNpcObjectManager(void);
	virtual ~CDNNpcObjectManager(void);

	CDNNpcObject* FindNpcByNpcID(int nNpcID);
};

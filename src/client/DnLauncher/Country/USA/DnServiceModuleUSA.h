#pragma once


#include "../../DnServiceModule.h"


class CDnServiceModuleUSA : public IServiceModule
{
public:
	CDnServiceModuleUSA() {}
	virtual ~CDnServiceModuleUSA() {}

public:
	virtual BOOL Initialize();
	virtual void Destroy();
};

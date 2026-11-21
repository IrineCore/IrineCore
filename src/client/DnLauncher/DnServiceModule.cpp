#include "stdafx.h"
#include "DnServiceModule.h"
#include "DnServiceModuleUSA.h"


IServiceModule* g_pServiceModule = NULL;


IServiceModule* DnServiceModule::CreateServiceModule()
{
	return ( new CDnServiceModuleUSA() );
}

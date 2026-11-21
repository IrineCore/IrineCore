#include "stdafx.h"
#include "DnServiceSetup.h"

IServiceSetup*	g_pServiceSetup = NULL;


IServiceSetup* DnServiceSetup::Create()
{
	return (new IServiceSetup());
	return NULL;
}




#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif




int IServiceSetup::PreInitialize(void* pCustomData)
{
	return 0;
}
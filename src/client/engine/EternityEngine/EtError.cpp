#include "Stdafx.h"
#include "EtError.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

int g_nLastError = ET_OK;

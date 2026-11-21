#include "Stdafx.h"
#ifdef PRE_MOD_ITEM_COMPOUND_DIALOG_RENEWAL
#include "DnCompoundBase.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CDnCompoundBase::CDnCompoundBase( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback, bool bAutoCursor )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback, bAutoCursor )
{

}

CDnCompoundBase::~CDnCompoundBase()
{

}

#endif 

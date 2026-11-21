#include "StdAfx.h"
#include "DnWorldSkillUserProp.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif



CDnWorldSkillUserProp::CDnWorldSkillUserProp(void)
{

}

CDnWorldSkillUserProp::~CDnWorldSkillUserProp(void)
{
}


bool CDnWorldSkillUserProp::InitializeTable(int nTableID )
{
	if( CDnWorldActProp::InitializeTable( nTableID ) == false ) return false;

	if( GetData() )
	{
		// Not much to do on the client side.
		//SkillUserStruct* pStruct = (SkillUserStruct*)GetData();
		//int iSkillID = pStruct->SkillTableID;
		//int iSkillLevel = pStruct->SkillLevel;
		//m_fCheckRadius = pStruct->fRadius;
		//m_iUseType = pStruct->UseType;
	}

	return true;
}

bool CDnWorldSkillUserProp::Initialize( CEtWorldSector *pParentSector, const char *szPropName, EtVector3 &vPos, EtVector3 &vRotate, EtVector3 &vScale )
{
	bool bResult = CDnWorldActProp::Initialize( pParentSector, szPropName, vPos, vRotate, vScale );
	if( bResult && m_hMonster ) {
		*m_hMonster->GetMatEx() = *GetMatEx();
		m_hMonster->SetActionSignalScale( *GetActionSignalScale() );
	}
	return bResult;
}


void CDnWorldSkillUserProp::Process( LOCAL_TIME LocalTime, float fDelta )
{
	//switch( m_iUseType )
	//{
	//	case ONCE:
	//		
	//		break;

	//	case CONTINUOUS:
	//		
	//		break;
	//}
}



void CDnWorldSkillUserProp::OnSignal( SignalTypeEnum Type, void* pPtr, LOCAL_TIME LocalTime, LOCAL_TIME SignalStartTime, LOCAL_TIME SignalEndTime, int nSignalIndex )
{
	// Effects defined on props will also play automatically from ActionBase, so there isn't much to do here either.
}
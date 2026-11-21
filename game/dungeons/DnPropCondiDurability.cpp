#include "stdafx.h"
#include "DnPropCondiDurability.h"
#include "DnWorldActProp.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


CDnPropCondiDurability::CDnPropCondiDurability( DnPropHandle hEntity ) : 
												CDnPropCondition(hEntity),
												m_iStandardDurability( 0 ),
												m_iOperation( 0 )
{

}

CDnPropCondiDurability::~CDnPropCondiDurability(void)
{

}


void CDnPropCondiDurability::Initialize( int iStandardDurability, int iOperation )
{
	m_iStandardDurability = iStandardDurability;
	m_iOperation = iOperation;
}


bool CDnPropCondiDurability::IsSatisfy( void )
{
	bool bResult = false;

	if( !m_hEntity )
		return false;

	CDnWorldActProp* pActProp = static_cast<CDnWorldActProp*>(m_hEntity.GetPointer());
	if( NULL == pActProp )
	{
#ifdef ENABLE_PROP_CONDITION_LOG
		OutputDebug( "[?? Prop FSM Condition Log] CDnPropCondiDurability::IsSatisfy - Entity Prop is NULL!\n" );
#endif
		return false;
	}

	int iNowDurability = pActProp->GetDurability();

	switch( m_iOperation )
	{
		case EQUAL:
			bResult = (iNowDurability == m_iStandardDurability);
			break;

		case GREATER:
			bResult = (iNowDurability > m_iStandardDurability);
			break;

		case LESS:
			bResult = (iNowDurability < m_iStandardDurability);
			break;

		case GREATER_EQUAL:
			bResult = (iNowDurability >= m_iStandardDurability);
			break;

		case LESS_EQUAL:
			bResult = (iNowDurability <= m_iStandardDurability);
			break;
	}

#ifdef ENABLE_PROP_CONDITION_LOG
	char* pOperation = NULL;
	switch( m_iOperation )
	{
		case EQUAL:
			pOperation = "==";
			break;

		case GREATER:
			pOperation = ">";
			break;

		case LESS:
			pOperation = "<";
			break;

		case GREATER_EQUAL:
			pOperation = ">=";
			break;

		case LESS_EQUAL:
			pOperation = "<=";
			break;
	}

	char* pResult = NULL;
	if( bResult )
		pResult = "true";
	else
		pResult = "false";

	OutputDebug( "[Prop(%d) FSM Condition Log] CDnPropCondiDurability::IsSatisfy (Now:)%d %s %d Result: \"%s\"\n", 
				  m_hEntity->GetUniqueID(), iNowDurability, pOperation, m_iStandardDurability, pResult );
#endif

	return bResult;
}
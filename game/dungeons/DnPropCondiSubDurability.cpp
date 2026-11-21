#include "stdafx.h"
#include "DnPropCondiSubDurability.h"
#include "DnWorldActProp.h"

CDnPropCondiSubDurability::CDnPropCondiSubDurability( DnPropHandle hEntity ) : CDnPropCondition( hEntity )
{
	m_pProp = dynamic_cast<CDnWorldActProp*>(hEntity.GetPointer());
	m_iDurabilityIndex = 0;
	m_iOperation = 0;
	m_iStandardDurability = 0;
}

CDnPropCondiSubDurability::~CDnPropCondiSubDurability(void)
{
}


void CDnPropCondiSubDurability::Initialize( int iStandardDurability, int iDurabilityIndex, int iOperation )
{
	m_iStandardDurability = iStandardDurability;
	m_iDurabilityIndex = iDurabilityIndex;
	m_iOperation = iOperation;
}


bool CDnPropCondiSubDurability::IsSatisfy( void )
{
	bool bResult = false;

	if( !m_hEntity )
		return false;

	if( NULL == m_pProp )
		return false;

	int iNowDurability = m_pProp->GetSubDurability( m_iDurabilityIndex );

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

	return bResult;
}
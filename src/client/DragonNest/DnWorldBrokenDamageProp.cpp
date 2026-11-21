#include "StdAfx.h"
#include "DnWorldBrokenDamageProp.h"
#include "DnTableDB.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


CDnWorldBrokenDamageProp::CDnWorldBrokenDamageProp(void)
{

}

CDnWorldBrokenDamageProp::~CDnWorldBrokenDamageProp(void)
{
	ReleasePostCustomParam();
}


bool CDnWorldBrokenDamageProp::InitializeTable( int nTableID )
{
	if( CDnWorldActProp::InitializeTable( nTableID ) == false ) return false;

	LoadHitEffectData( nTableID );
	m_bIsStaticCollision = false;

	if( GetData() ) 
	{
		BrokenDamageStruct* pStruct = (BrokenDamageStruct*)GetData();
		bool				bResult = false;

//#61146 Structure allowing Trap-type prop attack power to be set differently by difficulty.
#ifdef PER_ADD_PROP_TRAP_DIFFICULT_SET
		bResult = InitializeMonsterActorProp( pStruct->nMonsterTableID, pStruct->MonsterWeight );
#else
		bResult = InitializeMonsterActorProp( pStruct->nMonsterTableID );
#endif	

		_ASSERT( bResult );
		if( bResult )
		{
			m_nDurability = pStruct->nDurability;

			if( -1 != m_nDurability )
			{
				if( 0 == m_nDurability ) m_nDurability = 1;
				m_nItemDropGroupTableID = pStruct->nItemDropGroupTableID;
				if( m_nItemDropGroupTableID > 0 ) {
					CDnDropItem::CalcDropItemList( -1, m_nItemDropGroupTableID, m_VecDropItemList );

					for( DWORD i=0; i<m_VecDropItemList.size(); i++ ) {
						if( CDnDropItem::PreInitializeItem( m_VecDropItemList[i].nItemID ) == false ) {
							m_VecDropItemList.erase( m_VecDropItemList.begin() + i );
							i--;
						}
					}
				}
			}
		}
		else
			return false;
	}

	return true;
}

bool CDnWorldBrokenDamageProp::Initialize( CEtWorldSector *pParentSector, const char *szPropName, EtVector3 &vPos, EtVector3 &vRotate, EtVector3 &vScale )
{
	bool bResult = CDnWorldBrokenProp::Initialize( pParentSector, szPropName, vPos, vRotate, vScale );
	if( bResult && m_hMonster ) {
		*m_hMonster->GetMatEx() = *GetMatEx();
		m_hMonster->SetActionSignalScale( *GetActionSignalScale() );
	}
	return bResult;
}

#pragma once
#include "DnWorldBrokenProp.h"
#include "MAActorProp.h"
#include "DnDamageBase.h"


// #24282 Broken prop with multiple durability values
// All primary control is handled on the server, and the client only receives action packets and performs actions..
class CDnWorldMultiDurabilityProp : public CDnWorldBrokenProp,
								  public MAActorProp,
								  public CDnDamageBase
{
private:


protected:
	bool InitializeTable( int nTableID );
	void ReleasePostCustomParam( void ) 
	{ 
		if( m_pData && m_bPostCreateCustomParam )
		{
			MultiDurabilityBrokenPropStruct* pMultiDurabilityStruct = static_cast<MultiDurabilityBrokenPropStruct*>( m_pData );
			SAFE_DELETE( pMultiDurabilityStruct );
			m_pData = NULL;		// Must be set to NULL so it will not be deleted again in the destructor of a derived class.
		}
	};
	bool Initialize( CEtWorldSector *pParentSector, const char *szPropName, EtVector3 &vPos, EtVector3 &vRotate, EtVector3 &vScale );

public:
	CDnWorldMultiDurabilityProp(void);
	virtual ~CDnWorldMultiDurabilityProp(void);

	DnActorHandle GetActorHandle( void ) { return m_hMonster; };
	//void OnSignal( SignalTypeEnum Type, void *pPtr, LOCAL_TIME LocalTime, LOCAL_TIME SignalStartTime, LOCAL_TIME SignalEndTime, int nSignalIndex );
};

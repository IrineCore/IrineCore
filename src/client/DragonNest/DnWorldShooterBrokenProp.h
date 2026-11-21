#pragma once
#include "DnWorldBrokenProp.h"
#include "MAActorProp.h"
#include "DnDamageBase.h"


// Prop that gets hit while shooting something and becomes Broken when durability runs out.
class CDnWorldShooterBrokenProp : public CDnWorldBrokenProp,
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
			ShooterBrokenStruct* pShooterBrokenStruct = static_cast<ShooterBrokenStruct*>( m_pData );
			SAFE_DELETE( pShooterBrokenStruct );
			m_pData = NULL;		// Must be set to NULL so it will not be deleted again in the destructor of a derived class.
		}
	};

public:
	CDnWorldShooterBrokenProp(void);
	virtual ~CDnWorldShooterBrokenProp(void);

	bool Initialize( CEtWorldSector *pParentSector, const char *szPropName, EtVector3 &vPos, EtVector3 &vRotate, EtVector3 &vScale );
	DnActorHandle GetActorHandle( void ) { return m_hMonster; };
	void OnSignal( SignalTypeEnum Type, void *pPtr, LOCAL_TIME LocalTime, LOCAL_TIME SignalStartTime, LOCAL_TIME SignalEndTime, int nSignalIndex );
};

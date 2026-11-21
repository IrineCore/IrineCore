#pragma once
#include "DnWorldBrokenProp.h"
#include "MAActorProp.h"


class CDnWorldBrokenBuffProp : public CDnWorldBrokenProp, public MAActorProp
{
private:

protected:
	void ReleasePostCustomParam( void )
	{ 
		if( m_pData && m_bPostCreateCustomParam )
		{
			BuffBrokenStruct* pBuffBrokenStruct = static_cast<BuffBrokenStruct*>( m_pData );
			SAFE_DELETE( pBuffBrokenStruct ); 
			m_pData = NULL;		// Must be set to NULL so it will not be deleted again in the destructor of a derived class.
		}
	};

public:
	CDnWorldBrokenBuffProp(void);
	virtual ~CDnWorldBrokenBuffProp(void);

	virtual bool Initialize( CEtWorldSector *pParentSector, const char *szPropName, EtVector3 &vPos, EtVector3 &vRotate, EtVector3 &vScale );
	bool InitializeTable( int nTableID );
};

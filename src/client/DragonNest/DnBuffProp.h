#pragma once
#include "DnWorldActProp.h"
#include "MAActorProp.h"


class CDnBuffProp : public CDnWorldActProp,
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
			BuffStruct* pBuffStruct = static_cast<BuffStruct*>( m_pData );
			SAFE_DELETE( pBuffStruct );
			m_pData = NULL;		// Must be set to NULL so it will not be deleted again in the destructor of a derived class.
		}
	};

	// Buff props do not need to run signals separately... everything runs on the server.

public:
	CDnBuffProp(void);
	virtual ~CDnBuffProp(void);

	virtual bool Initialize( CEtWorldSector *pParentSector, const char *szPropName, EtVector3 &vPos, EtVector3 &vRotate, EtVector3 &vScale );
	virtual DnActorHandle GetActorHandle( void ) { return m_hMonster; };
};

#pragma once
#include "DnWorldBrokenProp.h"
#include "MAActorProp.h"


// Prop that deals damage on explosion. Nothing special to do on the client.
class CDnWorldBrokenDamageProp : public CDnWorldBrokenProp,
								 public CDnDamageBase,
								 public MAActorProp
{
private:
	


protected:
	virtual bool InitializeTable( int nTableID );
	void ReleasePostCustomParam( void ) 
	{ 
		if( m_pData && m_bPostCreateCustomParam )
		{
			BrokenDamageStruct* pBrokenDamageStruct = static_cast<BrokenDamageStruct*>( m_pData );
			SAFE_DELETE( pBrokenDamageStruct );
			m_pData = NULL;		// Must be set to NULL so it will not be deleted again in the destructor of a derived class.
		}
	};

public:
	CDnWorldBrokenDamageProp(void);
	virtual ~CDnWorldBrokenDamageProp(void);

	virtual bool Initialize( CEtWorldSector *pParentSector, const char *szPropName, EtVector3 &vPos, EtVector3 &vRotate, EtVector3 &vScale );
	// from CDnDamageBase
	virtual DnActorHandle GetActorHandle( void ) { return m_hMonster; };
};

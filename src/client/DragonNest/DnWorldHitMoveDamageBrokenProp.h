#pragma once
#include "DnWorldBrokenDamageProp.h"

// Prop that moves opposite to the hit direction when struck and explodes dealing damage when durability runs out
// Not shared between client and server; each runs independently.
class CDnWorldHitMoveDamageBrokenProp : public CDnWorldBrokenDamageProp
{
private:
	enum
	{
		MOVE_AXIS_NONE,
		MOVE_PROPLOCAL_AXIS_X,
		MOVE_PROPLOCAL_AXIS_Y,
		MOVE_PROPLOCAL_AXIS_Z,
	};

	//EtVector3 m_vDest;
	float m_fMoveDistance;
	bool m_bHitMove;
	
	float m_fWholeMoveTime;
	float m_fMovedTime;
	float m_fVelocity;
	float m_fResist;
	EtVector3 m_vHitPosition;
	EtVector3 m_vVelocity;
	EtVector3 m_vResist;
	EtVector3 m_vDir;

	NavigationCell* m_pCurCell;

	// When axis-locked with default movement property (crusher trap)
	int m_iMovingAxis;
	float m_fAxisMovingSpeed;
	float m_fAxisMaxMoveDistance;
	EtVector3 m_vInitialPos;
	EtVector3 m_vDestPosition;
	bool m_bArrived;
	float m_fHitDurability;
	int m_iActivateElementIndex;
	bool m_bActivate;

protected:
	virtual bool InitializeTable( int nTableID );
	virtual bool Initialize( CEtWorldSector *pParentSector, const char *szPropName, EtVector3 &vPos, EtVector3 &vRotate, EtVector3 &vScale );
	virtual bool CreateObject( void );
	void ReleasePostCustomParam( void ) 
	{
		if( m_pData && m_bPostCreateCustomParam )
		{
			HitMoveDamageBrokenStruct* pHitMoveDamageBrokenStruct = static_cast<HitMoveDamageBrokenStruct*>( m_pData );
			SAFE_DELETE( pHitMoveDamageBrokenStruct );
			m_pData = NULL;		// Must be set to NULL so it will not be deleted again in the destructor of a derived class.
		}
	};

	virtual void OnDamageReadAdditionalPacket( CDnDamageBase::SHitParam &HitParam, CPacketCompressStream *pStream );
public:
	CDnWorldHitMoveDamageBrokenProp(void);
	virtual ~CDnWorldHitMoveDamageBrokenProp(void);

	// from CDnWorldProp
	virtual void OnDamage( CDnDamageBase *pHitter, CDnActor::SHitParam &HitParam );
	virtual void Process( EtMatrix *pmatWorld, LOCAL_TIME LocalTime, float fDelta );

	void _UpdatePropPos();
	void _GetPosProperly( EtVector3 &vPrevPos );
};

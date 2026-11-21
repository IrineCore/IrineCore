#pragma once

#include "EtMatrixEx.h"
#include "EtWorldProp.h"
#include "DNTableFile.h"

// Implement basic Prop.
class CDnCutSceneWorldProp : public CEtWorldProp
{
public:
	//enum PropType
	//{
	//	STATIC,
	//	ACTION,
	//	BROKEN,
	//	OPERATION,
	//	//LIGHT,

	//	PROP_TYPE_COUNT,
	//};

protected:
	EtMatrix			m_matWorld;

protected:
	virtual bool _CreateObject( void );
	virtual void* AllocPropData(int& usingCount);

public:
	MatrixEx		m_matExWorld;

public:
	CDnCutSceneWorldProp(void);
	virtual ~CDnCutSceneWorldProp(void);

	// from CEtWorldProp
	virtual bool Initialize( CEtWorldSector *pParentSector, const char *szPropName, EtVector3 &vPos, EtVector3 &vRotate, EtVector3 &vScale );
	virtual bool InitializeTable( DNTableFileFormat*  pPropTable, int iTableID );
	void EnableCastShadow( bool bEnable );
	void EnableReceiveShadow( bool bEnable );
	
	void UpdateMatrix( void );

	void Show( bool bShow ) { m_Handle->ShowObject( bShow ); };

	// Provided to derived classes. Not inherited.
	virtual void Process( LOCAL_TIME LocalTime, float fDelta ) {};
};

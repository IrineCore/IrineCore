#pragma once

#include "DnWorldOperationProp.h"
#include "DnActor.h"
#include "DnDropItem.h"

class CDnWorldChestProp : public CDnWorldOperationProp
{
public:

	//enum ChestStateEnum {
	//	None = 0,
	//	Close = 0x01,
	//	Open = 0x02,
	//	CanOpen = 0x04,
	//	CantOpen = 0x08,
	//	Opening = 0x10,
	//};
	enum
	{
		CLOSED,
		TRY_OPEN,
		OPEN_SUCCESS,		// If OPENNING timing is needed later, implement it then.
		OPEN_FAILED,
	};

	CDnWorldChestProp();
	virtual ~CDnWorldChestProp();

protected:
	//int m_nCloseActionIndex;
	int m_iNeedKeyID;
	int m_iItemDropGroupTableID;
	bool m_bRemoveNeedKeyItem;
	int m_iNumNeedKeyItem;

	int m_iServerOperateResult;

	// #37265 Reward value per level range.
	// Even if m_iItemDropGroupTableID is set, if these range values are properly configured, they take priority.
	struct S_LEVEL_BOUNDS_REWARD
	{
		int iLevelStart;
		int iLevelEnd;
		int iItemDropGroupTableID;
		S_LEVEL_BOUNDS_REWARD( void ) : iLevelStart( 0 ), iLevelEnd( 0 ), iItemDropGroupTableID( 0 ) {};
	};
	vector<S_LEVEL_BOUNDS_REWARD> m_vlLevelBoundsReward;
	vector<CDnItem::DropItemStruct> m_VecDropItemList;

protected:
	virtual bool Initialize( CEtWorldSector *pParentSector, const char *szPropName, EtVector3 &vPos, EtVector3 &vRotate, EtVector3 &vScale );
	virtual bool InitializeTable( int nTableID );
	void ReleasePostCustomParam( void ) 
	{
		if( m_pData && m_bPostCreateCustomParam )
		{
			ChestStruct* pChestStruct = static_cast<ChestStruct*>( m_pData );
			SAFE_DELETE( pChestStruct );
			m_pData = NULL;		// Must be set to NULL so it will not be deleted again in the destructor of a derived class.
		}
	};
	//void DropItems();

public:
	//virtual bool CreateObject();

	virtual void CmdOperation();

	virtual void OnDispatchMessage( DWORD dwProtocol, BYTE *pPacket );

	virtual int GetOperateState( void ) { return m_iServerOperateResult; };

	bool CanOpen( void );

	virtual int GetCrosshairType();
	//ChestStateEnum GetChestState();
};
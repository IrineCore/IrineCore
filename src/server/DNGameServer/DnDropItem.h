#pragma once

#include "DnUnknownRenderObject.h"
#include "DnBaseTransAction.h"
#include "EtMatrixEx.h"
#include "DnActionBase.h"
#include "DnRenderBase.h"
#include "DnItem.h"

class CDnWorld;
class CDNGameRoom;
class NavigationCell;

class CDnDropItem : public CDnUnknownRenderObject< CDnDropItem, MAX_SESSION_COUNT >, public CDnActionBase, public CDnRenderBase, public CDnBaseTransAction, public TBoostMemoryPool< CDnDropItem >
{
public:
	CDnDropItem( CMultiRoom *pRoom, DWORD dwUniqueID, bool bProcess );
	virtual ~CDnDropItem();

	enum DROPITEM_PROTOCOL {
		DPT_PICKUP = 0,
		DPT_NOPICKUP,
		DPT_PICKUPINSTANT,
	};

	STATIC_DECL( DWORD s_dwUniqueCount );

protected:

	STATIC_DECL( std::vector<DnDropItemHandle> s_hVecPreLoadItemList );
	STATIC_DECL( bool s_bActive );

	MatrixEx m_Cross;

	int m_nItemID;
	int m_nOverlapCount;
	int m_nRandomSeed;
	char m_cOption;
	DWORD m_dwUniqueID;
	float m_fLifeTime;
	int	m_nRotate;
	float m_fDistance;
	float m_fDisappearTime;
	bool m_bDisappear;
	NavigationCell *m_pCurCell;
	DWORD m_dwOwnerUniqueID;
	eItemRank m_Rank;
	bool m_bReversionItem;
	bool m_bReversionLock;
#if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
	int m_nEnchantID;
#endif // #if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
//	std::vector<DropItemStruct> m_VecDropItemList;
protected:
	void ProcessLifeTime( LOCAL_TIME LocalTime, float fDelta );
	void ProcessAttributeMovement( float fDistance );

	static int GetRoomRand( CMultiRoom* pRoom );

	static void CalcCoinLump( CMultiRoom *pRoom, int nCoin, int nMin, int nMax, std::vector<int> &nVecResult );
#if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
	static void CalcDropItems( CMultiRoom *pRoom, int nDropItemTableID, DNVector(CDnItem::DropItemStruct) &VecResult, int &nDepth, bool bIncreaseUniqueID = true, int nBaseDropItemTableID = 0 );
#else	// #if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
	static void CalcDropItems( CMultiRoom *pRoom, int nDropItemTableID, DNVector(CDnItem::DropItemStruct) &VecResult, int &nDepth, bool bIncreaseUniqueID = true );
#endif	// #if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
public:
#if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
	virtual bool				Initialize( EtVector3 &vPos, int nItemID, int nRandomSeed, char cOption, int nCount, int nRotate, bool bPreInitialize = false, int nEnchantID = 0 );
#else	// #if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
	virtual bool				Initialize( EtVector3 &vPos, int nItemID, int nRandomSeed, char cOption, int nCount, int nRotate, bool bPreInitialize = false );
#endif	// #if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
	virtual void				Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void				SyncClassTime( LOCAL_TIME LocalTime );
	virtual DnDropItemHandle	GetDropItemHandle() { return GetMySmartPtr(); };

	int GetItemID() { return m_nItemID; }
	int GetOverlapCount() { return m_nOverlapCount; }
	int GetRandomSeed() { return m_nRandomSeed; }
	char GetOption() { return m_cOption; }
	void SetUniqueID( DWORD dwUniqueID ) { m_dwUniqueID = dwUniqueID; }
	DWORD GetUniqueID() const { return m_dwUniqueID; }
	int	GetRotate(){ return m_nRotate; }
	float	GetLifeTime(){ return m_fLifeTime; }
	float	GetDistance(){ return m_fDistance; }

	void SetOwnerUniqueID( DWORD dwUniqueID ) { m_dwOwnerUniqueID = dwUniqueID; }
	DWORD GetOwnerUniqueID() { return m_dwOwnerUniqueID; };

	EtVector3 *GetPosition() { return &m_Cross.m_vPosition; }

	// CDnBaseTransAction
	virtual void Send( DWORD dwProtocol, DnActorHandle hActor, CMemoryStream *pStream, GlobalEnum::ClientSessionTypeEnum SessionType = GlobalEnum::ClientSessionTypeEnum::Auto );
	virtual void Send( DWORD dwProtocol, CMemoryStream *pStream, GlobalEnum::ClientSessionTypeEnum SessionType = GlobalEnum::ClientSessionTypeEnum::Auto, bool bImmediate = true );
	virtual void OnDispatchMessage( CDNUserSession *pSession, DWORD dwProtocol, BYTE *pPacket ) {}

	void InsertDropItem();
	void RemoveDropItem();

	virtual void GetBoundingSphere( SSphere &Sphere, bool bActorSize = false );
//	virtual void GetBoundingBox( SAABox &Box );

	// CDnActionBase
	virtual void OnSignal( SignalTypeEnum Type, void *pPtr, LOCAL_TIME LocalTime, LOCAL_TIME SignalStartTime, LOCAL_TIME SignalEndTime, int nSignalIndex );

	// Static functions
	static bool IsActive( CMultiRoom *pRoom );
	static bool InitializeClass( CMultiRoom *pRoom );
	static void ProcessClass( CMultiRoom *pRoom, LOCAL_TIME LocalTime, float fDelta );
	static void ReleaseClass( CMultiRoom *pRoom );
	static DWORD AllocUniqueID( CMultiRoom *pRoom ) { return STATIC_INSTANCE_( s_dwUniqueCount )++; }
#if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
	static bool	PreInitializeItem( CMultiRoom *pRoom, int nItemID, int nEnchantID = 0 );
	static DnDropItemHandle DropItem( CMultiRoom *pRoom, EtVector3 &vPos, DWORD dwUniqueID, int nItemID, int nSeed, char cOption, int nOverlapCount, int nRotate, UINT nOwnerUniqueID = -1, int nEnchantID = 0);
#else	// #if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
	static bool PreInitializeItem( CMultiRoom *pRoom, int nItemID );
	static DnDropItemHandle DropItem( CMultiRoom *pRoom, EtVector3 &vPos, DWORD dwUniqueID, int nItemID, int nSeed, char cOption, int nOverlapCount, int nRotate, UINT nOwnerUniqueID = -1 );
#endif	// #if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
	
	static void CalcDropItemList( CMultiRoom *pRoom, int nItemDropTableID, DNVector(CDnItem::DropItemStruct) &VecList, bool bIncreaseUniqueID = true );
	static void CalcDropItemList( CMultiRoom *pRoom, TDUNGEONDIFFICULTY Difficulty, int nItemDropTableID, DNVector(CDnItem::DropItemStruct) &VecList, bool bIncreaseUniqueID = true );
	static void CalcDropControlList( CMultiRoom *pRoom, int nDropControlGroupID, DNVector(CDnItem::DropItemStruct) &VecList, bool bIncreaseUniqueID = true );
#if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
	int GetEnchantID(void){ return m_nEnchantID; }
	static int GetDropItemEnchantID( CMultiRoom *pRoom, int nItemDropTableID );
#endif	// #if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
	void		SetReversionItem(bool bSet);
	void		LockReversionItem(bool bLock);
	bool		IsReversionItem() const { return m_bReversionItem; }
	bool		IsReversionLocked() const { return m_bReversionLock; }

	void MakeItemStruct( TItem &ItemInfo );
};

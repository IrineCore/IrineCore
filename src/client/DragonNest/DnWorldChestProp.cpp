#include "StdAfx.h"
#include "DnWorldChestProp.h"
#include "DNProtocol.h"
#include "DnPartyTask.h"
#include "TaskManager.h"
#include "DnItemTask.h"
#include "DNProtocol.h"
#include "DnPropState.h"
#include "DnPropCondition.h"
#include "DnPropStateDoAction.h"
#include "DnPropOperatedCondition.h"
#include "DnLocalPlayerActor.h"
#include "DnItemTask.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


CDnWorldChestProp::CDnWorldChestProp()
{
	//m_nCloseActionIndex = -1;
	m_iNeedKeyID = 0;
	m_iItemDropGroupTableID = 0;
	m_bRemoveNeedKeyItem = false;
	m_iNumNeedKeyItem = 0;

	m_iServerOperateResult = CLOSED;
}

CDnWorldChestProp::~CDnWorldChestProp()
{
	ReleasePostCustomParam();
}

bool CDnWorldChestProp::InitializeTable( int nTableID )
{
	if( CDnWorldActProp::InitializeTable( nTableID ) == false ) return false;

	if( GetData() )
	{
		ChestStruct *pStruct = (ChestStruct*)GetData();
		m_iNeedKeyID = pStruct->nNeedKeyID;
		m_iItemDropGroupTableID = pStruct->nItemDropGroupTableID;
		m_bRemoveNeedKeyItem = (pStruct->bRemoveKeyItem == TRUE);
		m_iNumNeedKeyItem = pStruct->nNumNeedItem;

		string strLevelBoundDescription;
		string strLevelBoundRewardItemDropGroupID;

		if( pStruct->strLevelBoundDescription )
			strLevelBoundDescription.assign( pStruct->strLevelBoundDescription );

		if( pStruct->strLevelBoundRewardItemDropGroupID )
			strLevelBoundRewardItemDropGroupID.assign( pStruct->strLevelBoundRewardItemDropGroupID );

		//// For testing.
		//strLevelBoundDescription.assign( "1~20;21~30;31~40;41~50" );
		//strLevelBoundRewardItemDropGroupID.assign( "1;2;3;4" );

		bool bValidLevelBoundSetting = true;
		vector<string> vlLevelPair;
		TokenizeA( strLevelBoundDescription, vlLevelPair, ";" );
		vector<string> vlRewards;
		TokenizeA( strLevelBoundRewardItemDropGroupID, vlRewards, ";" );
		if( (false == vlLevelPair.empty()) &&
			(vlLevelPair.size() == vlRewards.size()) )
		{
			for( int i = 0; i < (int)vlLevelPair.size(); ++i )
			{
				string& strLevelBound = vlLevelPair.at( i );
				vector<string> vlLevelBound;
				TokenizeA( strLevelBound, vlLevelBound, "~" );
				if( 2 == (int)vlLevelBound.size() )
				{
					S_LEVEL_BOUNDS_REWARD Reward;
					Reward.iLevelStart = atoi( vlLevelBound.at(0).c_str() );
					Reward.iLevelEnd = atoi( vlLevelBound.at(1).c_str() );
					Reward.iItemDropGroupTableID = atoi( vlRewards.at(i).c_str() );

					m_vlLevelBoundsReward.push_back( Reward );
				}
				else
				{
					bValidLevelBoundSetting = false;
					break;
				}
			}

			if( false == bValidLevelBoundSetting )
				m_vlLevelBoundsReward.clear();
		}


		if( m_vlLevelBoundsReward.empty() &&
			m_iItemDropGroupTableID > 0 )
		{
			// #38984 Created only on the client and not actually rendered. 
			// Used in _VisibleProp of the table during prop creation or inside CDnDropItem::CalcDropItemList
			// Since _rand does not resync seed values mid-way, the call count must match between server and client.
			CDnDropItem::CalcDropItemList( -1, m_iItemDropGroupTableID, m_VecDropItemList );

			for( DWORD i=0; i<m_VecDropItemList.size(); i++ ) {
				if( CDnDropItem::PreInitializeItem( m_VecDropItemList[i].nItemID ) == false ) {
					m_VecDropItemList.erase( m_VecDropItemList.begin() + i );
					i--;
				}
			}
		}
	}

	return true;
}


bool CDnWorldChestProp::Initialize( CEtWorldSector *pParentSector, const char *szPropName, EtVector3 &vPos, EtVector3 &vRotate, EtVector3 &vScale )
{
	CDnPropState* pNormalState = CDnPropState::Create( GetMySmartPtr(), CDnPropState::NORMAL );
	CDnPropState* pOpenActionState = CDnPropState::Create( GetMySmartPtr(), CDnPropState::DO_ACTION );
	CDnPropState* pItemDropState = CDnPropState::Create( GetMySmartPtr(), CDnPropState::ITEM_DROP );
	static_cast<CDnPropStateDoAction*>(pOpenActionState)->AddActionName( "Close_Open" );
	
	CDnPropCondition* pOpenOperateCondition = CDnPropCondition::Create( GetMySmartPtr(), CDnPropCondition::OPERATED );
	CDnPropCondition* pCloseOperateCondition = CDnPropCondition::Create( GetMySmartPtr(), CDnPropCondition::OPERATED );
	CDnPropCondition* pNULLCondition = CDnPropCondition::Create( GetMySmartPtr(), CDnPropCondition::NULL_CONDITION );
	static_cast<CDnPropOperatedCondition*>(pOpenOperateCondition)->SetRequireState( OPEN_SUCCESS );
	static_cast<CDnPropOperatedCondition*>(pCloseOperateCondition)->SetRequireState( OPEN_FAILED );

	m_pFSM->AddCondition( pOpenOperateCondition );
	m_pFSM->AddCondition( pCloseOperateCondition );
	m_pFSM->AddCondition( pNULLCondition );

	m_pFSM->AddState( pNormalState );
	m_pFSM->AddState( pOpenActionState );
	m_pFSM->AddState( pItemDropState );

	pNormalState->AddTransitState( pOpenActionState, pOpenOperateCondition );
	pOpenActionState->AddTransitState( pItemDropState, pNULLCondition );

	m_pFSM->SetEntryState( pNormalState );

	return CDnWorldActProp::Initialize( pParentSector, szPropName, vPos, vRotate, vScale );
}


// Called when the mouse clicks on a prop.
void CDnWorldChestProp::CmdOperation()
{
	Send( eProp::CS_CMDOPERATION, NULL );
}


bool CDnWorldChestProp::CanOpen( void )
{
	// If a key item is defined, its required count must also be met to open it.
	if( 0 < m_iNeedKeyID )
	{
		CDnCharInventory& Inventory = GetItemTask().GetCharInventory();
		int iHasItemCount = Inventory.GetItemCount( m_iNeedKeyID );
		if( m_iNumNeedKeyItem <= iHasItemCount )
		{
			return true;
		}

		return false;
	}
	
	return true;
}

void CDnWorldChestProp::OnDispatchMessage( DWORD dwProtocol, BYTE *pPacket )
{
	switch( dwProtocol ) {
		case eProp::SC_CMDOPERATION:
			{
				CPacketCompressStream Stream( pPacket, 128 );

				//ChestStateEnum State;
				Stream.Read( &m_iServerOperateResult, sizeof(int), CPacketCompressStream::INTEGER_SHORT );

				// If opening succeeds, set drop items according to level
				if( OPEN_SUCCESS == m_iServerOperateResult )
				{
					// Update drop item value if level-bracket reward is configured.
					if( false == m_vlLevelBoundsReward.empty() )
					{
						// Set reward drop item ID matching the level of the interacting actor.
						m_iItemDropGroupTableID = 0;
						int iAccessActorLevel = CDnLocalPlayerActor::s_hLocalActor->GetLevel();
						for( int i = 0; i < (int)m_vlLevelBoundsReward.size(); ++i )
						{
							const S_LEVEL_BOUNDS_REWARD LevelBoundsReward = m_vlLevelBoundsReward.at( i );
							if( LevelBoundsReward.iLevelStart <= iAccessActorLevel &&
								iAccessActorLevel <= LevelBoundsReward.iLevelEnd )
							{
								m_iItemDropGroupTableID = LevelBoundsReward.iItemDropGroupTableID;
							}
						}

						// If interval cannot be found, process as the last interval reward.
						if( 0 == m_iItemDropGroupTableID )
							m_iItemDropGroupTableID = m_vlLevelBoundsReward.back().iItemDropGroupTableID;


						m_VecDropItemList.clear();

						// #38984 Created only on the client and not actually rendered. 
						// Used in _VisibleProp of the table during prop creation or inside CDnDropItem::CalcDropItemList
						// Since _rand does not resync seed values mid-way, the call count must match between server and client.
						CDnDropItem::CalcDropItemList( -1, m_iItemDropGroupTableID, m_VecDropItemList );

						for( DWORD i=0; i<m_VecDropItemList.size(); i++ ) {
							if( CDnDropItem::PreInitializeItem( m_VecDropItemList[i].nItemID ) == false ) {
								m_VecDropItemList.erase( m_VecDropItemList.begin() + i );
								i--;
							}
						}
					}
				}
			}
			break;
	}
	CDnWorldOperationProp::OnDispatchMessage( dwProtocol, pPacket );
}

int CDnWorldChestProp::GetCrosshairType()
{
	switch( GetOperateState() ) {
		case CDnWorldChestProp::CLOSED:
			if( CanOpen() ) return CDnLocalPlayerActor::CrossHairType::Chest_UnLock;
			else return CDnLocalPlayerActor::CrossHairType::Chest_Lock;
			break;

		case CDnWorldChestProp::OPEN_SUCCESS:
			break;
	}
	return CDnLocalPlayerActor::CrossHairType::Normal;
}
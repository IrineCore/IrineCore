#include "Stdafx.h"
#include "DnLifeSkillPlantTask.h"
#include "DnAcceptRequestDlg.h"
#include "DnInterface.h"
#include "DnLocalPlayerActor.h"
#include "DnWorld.h"
#include "EtWorldSector.h"
#include "EtWorldEventControl.h"
#include "EtWorldEventArea.h"
#include "DnWorldOperationProp.h"
#include "FarmSendPacket.h"
#include "DnItemTask.h"
#include "DnWorldSector.h"
#include "DnPlayerCamera.h"
#include "DnLifeHarvestIcon.h"
#include "DnMinimap.h"
#include "DnUIString.h"
#include "DnMainDlg.h"
#include "DnLifeTooltipDlg.h"
#include "DnMainMenuDlg.h"
#include "DnCharStatusDlg.h"
#include "DnCharVehicleDlg.h"
#include "GlobalValue.h"
#include "DnWorldData.h"
#include "SyncTimer.h"
#include "DnTableDB.h"
#include "DnSkillTask.h"
#include "SecondarySkill.h"
#include "TaskManager.h"
#include "SystemSendPacket.h"
#include "DnGuildTask.h"
#include "DnLifeConditionDlg.h"


#define PROGRESS_BAR_TIME	Farm::Max::PLANTING_TICK / 1000.0f
#define TARGET_LENGTH 300.0f
#define QUESTION_WATER_BOTTLE 100
#define QUESTION_WATER_CHOICE 101
#define QUESTION_REJECT_PLANT 102

CDnLifeSkillPlantTask::CDnLifeSkillPlantTask()
: CTaskListener( false )
, m_eAcceptType( ePlant_Seed )
, m_nWareHouseItemCount( 0 )
, m_bLocalBattle( false )
, m_nTotalAreaCount( 0 )
, m_nPlantAreaCount( 0 )
, m_pLifeHarvestIcon( NULL )
{
	m_PlantInfo.Clear();
}

CDnLifeSkillPlantTask::~CDnLifeSkillPlantTask()
{
	SAFE_DELETE( m_pLifeHarvestIcon );
}

bool CDnLifeSkillPlantTask::Initialize()
{
	m_pLifeHarvestIcon = new CDnLifeHarvestIcon;

	return true;
}

void CDnLifeSkillPlantTask::FinalizeStage()
{
	for( std::map<int, SFarmArea *>::iterator itor = m_mFarmArea.begin(); itor != m_mFarmArea.end(); ++itor )
		SAFE_DELETE( (*itor).second );

	m_mFarmArea.clear();
	m_vGrowingArea.clear();

	if( m_pLifeHarvestIcon )
		m_pLifeHarvestIcon->Finalize();
}

void CDnLifeSkillPlantTask::OnDispatchMessage( int nMainCmd, int nSubCmd, char *pData, int nSize )
{
	switch( nMainCmd ) {
		case SC_FARM : OnRecvFarmSystemMessage(nSubCmd, pData, nSize); break;
	}
}

void CDnLifeSkillPlantTask::OnRecvFarmSystemMessage( int nSubCmd, char *pData, int nSize )
{
	switch( nSubCmd ){
		case eFarm::SC_BEGIN_AREASTATE:	OnRecvAreaState( (SCFarmBeginAreaState *)pData);break;
		case eFarm::SC_PLANT:				OnRecvPlant( (SCFarmPlant *)pData );			break;
		case eFarm::SC_HARVEST:				OnRecvHarvest( (SCFarmHarvest *)pData );		break;
		case eFarm::SC_ADDWATER:			OnRecvAddWater( (SCFarmAddWater *)pData );		break;
		case eFarm::SC_AREAINFO:			OnRecvAreaInfo( (SCAreaInfo *)pData );			break;
		case eFarm::SC_AREAINFO_LIST:		OnRecvSeedInfoList( (SCAreaInfoList *)pData );	break;
		case eFarm::SC_WAREHOUSE_LIST:		OnRecvWareHouseList( (SCFarmWareHouseList *)pData );			break;
		case eFarm::SC_TAKE_WAREHOUSE_ITEM:	OnRecvTakeWareHouseItem( (SCFarmTakeWareHouseItem *)pData );	break;
		case eFarm::SC_FIELDCOUNTIFNO :		OnRecvFieldCountInfo( (SCFarmFieldCountInfo *)pData );			break;
		case eFarm::SC_WAREHOUSE_ITEMCOUNT :OnRecvWareHouseItemCount( (SCFarmWareHouseItemCount*)pData );	break;
		case eFarm::SC_ADDWATER_ANOTHERUSER : OnRecvAddWaterAnotheruser( (SCFarmAddWaterAnotherUser*)pData );	break;
		case eFarm::SC_CHANGE_OWNERNAME :	OnRecvChangeOwnerName( (SCChangeOwnerName *)pData );	break;
	}
}

void CDnLifeSkillPlantTask::InitializeGrowingArea()
{
	m_nTotalAreaCount = 0;

	CEtWorldEventControl*	pControl	= NULL;
	CEtWorldSector*			pSector		= NULL;
	CEtWorldEventArea*		pArea		= NULL;

	std::vector<CEtWorldEventArea*> vRespawnArea;

	CDnWorld * pWorld = CDnWorld::GetInstancePtr();

	for( DWORD i=0; i<pWorld->GetGrid()->GetActiveSectorCount(); i++ ) 
	{
		pSector	 = pWorld->GetGrid()->GetActiveSector(i);
		pControl = pSector->GetControlFromUniqueID( ETE_FarmGrowingArea );
		if( !pControl ) 
			continue;

		for( DWORD j=0; j<pControl->GetAreaCount(); j++ ) 
		{
			pArea = pControl->GetAreaFromIndex(j);
			if( pArea )
			{
				FarmGrowingAreaStruct * pFarmGrowingItemArea = (FarmGrowingAreaStruct*)pArea->GetData();

				std::string					strString(pArea->GetName());
				std::vector<std::string>	vSplit;

				boost::algorithm::split( vSplit, strString, boost::algorithm::is_any_of(" ") );

				_ASSERT( vSplit.size() == 3 );
				if( vSplit.size() < 3 )
					continue;

				int iHarvestAreaIndex = boost::lexical_cast<int>( vSplit[2] );

				// Currently temporary; need to look up ground props per map from the table and insert them
				CDnWorldOperationProp * pProp = AddProp( "FarmPlants_Basic.skn", pArea->GetOBB()->Center, 0, true );

				SFarmArea * sFarmArea = new SFarmArea( pArea->GetOBB(), Farm::AreaState::NONE, pProp );

				if( JOINT_FARM_AREA == pFarmGrowingItemArea->nFarmType )
					++m_nTotalAreaCount;
				else
					sFarmArea->m_bPrivate = true;

				m_mFarmArea.insert( std::make_pair(iHarvestAreaIndex, sFarmArea ) );
			}
		}
	}

	CDnLifeConditionDlg * pLifeCondition = GetInterface().GetLifeConditionDlg();

	if( pLifeCondition && pLifeCondition->IsShow() )
		pLifeCondition->SetEmptyCount( m_nTotalAreaCount, m_nTotalAreaCount );
}

void CDnLifeSkillPlantTask::InitializeProp()
{
	// Load the entire prop list.
	if( 0 == CDnWorld::GetInstance().GetGateCount() )
	{
		CDnWorld::GetInstance().GetGateStruct( 0 );
	}
}

void CDnLifeSkillPlantTask::SeedPlant( BYTE cInvenIndex, BYTE cCount, INT64 * nCashSerial, BYTE * pCount )
{
	if( NULL == m_PlantInfo.m_pFarmArea )	
	{
		m_PlantInfo.Clear();
		return;
	}

	m_PlantInfo.m_cInvenIndex = cInvenIndex;
	m_PlantInfo.m_cCashItemCount = cCount;
	if( nCashSerial && pCount )
	{
		memcpy( m_PlantInfo.m_nCashBoostItemSerials, nCashSerial, sizeof(INT64)*cCount );
		memcpy( m_PlantInfo.m_pCashBoostItemCount, pCount, sizeof(BYTE)*cCount );
	}

	BattleAndVehicleMode( false );
	RequestPlant( m_PlantInfo.m_nSelectAreaIndex, Farm::ActionType::START, m_PlantInfo.m_cInvenIndex, cCount, m_PlantInfo.m_nCashBoostItemSerials, m_PlantInfo.m_pCashBoostItemCount );
}

CDnWorldOperationProp * CDnLifeSkillPlantTask::AddProp( const char *szPropName, EtVector3 & vPos, INT64 nSeedValue, bool bInitialize )
{
	CEtWorldSector * pSector = CDnWorld::GetInstance().GetSector( vPos.x, vPos.z );
	CDnWorldOperationProp * pProp = new CDnWorldOperationProp;

	// When props are first created in the area... needed only at initialization.
	if( bInitialize )
	{
		vPos.x += ( pSector->GetTileWidthCount() * pSector->GetTileSize() ) / 2.f;
		vPos.z += ( pSector->GetTileHeightCount() * pSector->GetTileSize() ) / 2.f;
	}

	float fSize = 1.0f;

	if( 0 != nSeedValue )
	{
		fSize = (float)(nSeedValue%60) / 100.0f + 1.f;
	}

	pProp->Initialize( pSector, szPropName, vPos, EtVector3(0.0f, 0.0f, 0.0f), EtVector3( fSize, fSize, fSize ) );

	pProp->SetCrosshairType( CDnLocalPlayerActor::CrossHairType::Seed );

	pSector->InsertProp( pProp );

	return pProp;
}

CDnWorldOperationProp * CDnLifeSkillPlantTask::DeleteProp( CDnWorldOperationProp * pProp, EtVector3 & vPos )
{
	if( !pProp )
		return false;

	CDnWorldSector * pSector = (CDnWorldSector *)(CDnWorld::GetInstance().GetSector( vPos.x, vPos.z ));

	pSector->DeleteProp( reinterpret_cast<CEtWorldProp *>(pProp) );
	pSector->RemoveProcessProp( pProp );
	SAFE_DELETE( pProp );

	return pProp;
}

bool CDnLifeSkillPlantTask::ChangeState( Farm::AreaState::eState eAreaState, int nAreaIndex, SFarmArea * sFarmArea )
{
	if( NULL == sFarmArea )
		return false;

	if( Farm::AreaState::NONE == eAreaState )
	{
		if( Farm::AreaState::GROWING == sFarmArea->m_eState || Farm::AreaState::ADDWATER == sFarmArea->m_eState )
		{
			if( !CDnActor::s_hLocalActor ) return false;
			CDnLocalPlayerActor * localActor = ((CDnLocalPlayerActor *)(CDnActor::s_hLocalActor.GetPointer()));
			// Show system message when my crop dies
			if( __wcsicmp_l( localActor->GetName(), sFarmArea->m_wszCharName ) == 0 )
			{
				DNTableFileFormat* pItemSox = GetDNTable( CDnTableDB::TITEM );
				CDnWorldData *pWorldData = CDnWorldDataManager::GetInstance().GetWorldData( CGlobalInfo::GetInstance().m_nCurrentMapIndex );
				if( pWorldData && pItemSox )
				{
					std::wstring wszItemName;
					std::wstring wszMapName;
					WCHAR wszString[256];

					pWorldData->GetMapName( wszMapName );
					wszItemName = CDnItem::GetItemFullName( sFarmArea->m_iItemID );
					swprintf_s( wszString, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7488 ), wszMapName.c_str(), wszItemName.c_str() );

					GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", wszString, false );
				}
			}

			for( std::vector<SFarmArea *>::iterator itor = m_vGrowingArea.begin(); itor != m_vGrowingArea.end();  )
			{
				if( (*itor)->m_nSelectAreaIndex == sFarmArea->m_nSelectAreaIndex )
					itor = m_vGrowingArea.erase( itor );
				else
					++itor;
			}
		}

		sFarmArea->m_pWaterProp = DeleteProp( sFarmArea->m_pWaterProp, sFarmArea->m_pOBB->Center );
		sFarmArea->m_pProp = DeleteProp( sFarmArea->m_pProp, sFarmArea->m_pOBB->Center );
		sFarmArea->m_pProp = AddProp( "FarmPlants_Basic.skn", sFarmArea->m_pOBB->Center, 0 );
		sFarmArea->SetGrowLevel( GROW_LEVEL1 );

		sFarmArea->DeleteGauge();
		DeleteHarvest( nAreaIndex );

		GetInterface().ChangeState( nAreaIndex, sFarmArea->m_iItemID, eAreaState, (float)sFarmArea->m_fTimeBar, (float)sFarmArea->m_fWaterBar, sFarmArea->m_szHarvestIconName );

		sFarmArea->m_eState = eAreaState;
	}
	else if( Farm::AreaState::GROWING == eAreaState && Farm::AreaState::ADDWATER != sFarmArea->m_eState )
	{
		if( sFarmArea->m_pLifeGaugeDlg )
			return false; 

		DNTableFileFormat* pFarmListSox = GetDNTable( CDnTableDB::TFARMSKIN );
		DNTableFileFormat* pFarmSox = GetDNTable( CDnTableDB::TFARMCULTIVATE );

		int nPropID = pFarmSox->GetFieldFromLablePtr( sFarmArea->m_iItemID, "_SkinID" )->GetInteger();
		char * strPropName = pFarmListSox->GetFieldFromLablePtr( nPropID, "_SkinFileName" )->GetString();

		sFarmArea->m_pProp = DeleteProp( sFarmArea->m_pProp, sFarmArea->m_pOBB->Center );
		sFarmArea->m_pProp = AddProp( strPropName, sFarmArea->m_pOBB->Center, sFarmArea->m_nRandSeed );
		sFarmArea->SetGrowLevel( GROW_LEVEL1 );

		m_vGrowingArea.push_back( sFarmArea );

		sFarmArea->CreateGauge();

		if( !CDnActor::s_hLocalActor || sFarmArea->m_wszCharName == NULL ) return false;
		CDnLocalPlayerActor * localActor = ((CDnLocalPlayerActor *)(CDnActor::s_hLocalActor.GetPointer()));

		if( __wcsicmp_l( localActor->GetName(), sFarmArea->m_wszCharName ) == 0 )
			GetInterface().ChangeState( nAreaIndex, sFarmArea->m_iItemID, eAreaState, (float)sFarmArea->m_fTimeBar, (float)sFarmArea->m_fWaterBar, sFarmArea->m_szHarvestIconName );

		// Set info if the owner of the growing farm crop is in the same guild.
		TGuildMember * pMember = CDnGuildTask::GetInstance().GetGuildMemberFromName(sFarmArea->m_wszCharName);
		if( pMember )
			sFarmArea->m_pLifeGaugeDlg->SetGuildInfo( true, localActor->GetGuildSelfView(), sFarmArea->m_wszCharName );
		else
			sFarmArea->m_pLifeGaugeDlg->SetGuildInfo( false, localActor->GetGuildSelfView(), sFarmArea->m_wszCharName );

		sFarmArea->m_eState = eAreaState;
	}
	else if( Farm::AreaState::COMPLETED == eAreaState && Farm::AreaState::HARVESTING != sFarmArea->m_eState && Farm::AreaState::COMPLETED != sFarmArea->m_eState )
	{
		sFarmArea->m_pWaterProp = DeleteProp( sFarmArea->m_pWaterProp, sFarmArea->m_pOBB->Center );
		if( GROW_LEVEL2 == sFarmArea->m_eGrowLevel )
			sFarmArea->SetGrowLevel( GROW_LEVEL3 );
		else
		{
			DNTableFileFormat* pFarmListSox = GetDNTable( CDnTableDB::TFARMSKIN );
			DNTableFileFormat* pFarmSox = GetDNTable( CDnTableDB::TFARMCULTIVATE );

			int nPropID = pFarmSox->GetFieldFromLablePtr( sFarmArea->m_iItemID, "_SkinID" )->GetInteger();
			char * strPropName = pFarmListSox->GetFieldFromLablePtr( nPropID, "_SkinFileName" )->GetString();;

			sFarmArea->m_pProp = DeleteProp( sFarmArea->m_pProp, sFarmArea->m_pOBB->Center );
			sFarmArea->m_pProp = AddProp( strPropName, sFarmArea->m_pOBB->Center, sFarmArea->m_nRandSeed );
			sFarmArea->SetGrowLevel( GROW_LEVEL3 );
		}

		sFarmArea->DeleteGauge();

		for( std::vector<SFarmArea *>::iterator itor = m_vGrowingArea.begin(); itor != m_vGrowingArea.end(); ++itor )
		{
			if( (*itor)->m_nSelectAreaIndex == sFarmArea->m_nSelectAreaIndex )
			{
				m_vGrowingArea.erase( itor );
				break;
			}
		}

		InsertHarvest( nAreaIndex, sFarmArea );

		if( !CDnActor::s_hLocalActor || sFarmArea->m_wszCharName == NULL ) return false;
		CDnLocalPlayerActor * localActor = ((CDnLocalPlayerActor *)(CDnActor::s_hLocalActor.GetPointer()));

		if( __wcsicmp_l( localActor->GetName(), sFarmArea->m_wszCharName ) == 0 )
			GetInterface().ChangeState( nAreaIndex, sFarmArea->m_iItemID, eAreaState, (float)sFarmArea->m_fTimeBar, (float)sFarmArea->m_fWaterBar, sFarmArea->m_szHarvestIconName );

		sFarmArea->m_eState = eAreaState;
	}

	CDnLifeConditionDlg * pLifeCondition = GetInterface().GetLifeConditionDlg();

	if( pLifeCondition && pLifeCondition->IsShow() )
	{
		m_nPlantAreaCount = 0;
		std::map<int, SFarmArea *>::iterator Itor = m_mFarmArea.begin();
		for( ; Itor != m_mFarmArea.end(); ++Itor )
		{
			if( !(*Itor).second->m_bPrivate && Farm::AreaState::NONE != (*Itor).second->m_eState )
				++m_nPlantAreaCount;
		}

		pLifeCondition->SetEmptyCount( m_nTotalAreaCount - m_nPlantAreaCount, m_nTotalAreaCount );
	}

	return true;
}

CDnLifeSkillPlantTask::SFarmArea * CDnLifeSkillPlantTask::GetFarmArea( int nIndex )
{
	std::map<int, SFarmArea *>::iterator Itor = m_mFarmArea.find( nIndex );
	if( Itor != m_mFarmArea.end() )
		return (*Itor).second;

	return NULL;
}

CDnLifeSkillPlantTask::SFarmArea * CDnLifeSkillPlantTask::GetFarmArea( DnPropHandle hProp )
{
	std::map<int, SFarmArea *>::iterator Itor = m_mFarmArea.begin();

	for( ; Itor != m_mFarmArea.end(); ++Itor )
	{
		if( (*Itor).second->m_pProp->GetMySmartPtr().GetHandle() == hProp->GetMySmartPtr().GetHandle()	)
			return (*Itor).second;
	}

	return NULL;
}

EtVector3 CDnLifeSkillPlantTask::GetFarmAreaPos( int nAreaIndex )
{
	EtVector3 vPos;

	SFarmArea * pFarmArea = GetFarmArea( nAreaIndex );

	if( pFarmArea )
	{
		vPos = pFarmArea->m_pProp->GetMatEx()->m_vPosition;

		SAABox box;
		pFarmArea->m_pProp->GetBoundingBox(box);
		float fHeight = box.Max.y - box.Min.y + 5.f;

		vPos.y += fHeight;
	}

	return vPos;
}

void CDnLifeSkillPlantTask::GetEmptyAreaInfo( std::vector< std::pair<int, EtVector3> > & vecEmptyAreaInfo )
{
	std::map<int, SFarmArea *>::iterator itor = m_mFarmArea.begin();

	for( ; itor != m_mFarmArea.end(); ++itor )
	{
		if( Farm::AreaState::NONE == (*itor).second->m_eState )
			vecEmptyAreaInfo.push_back( std::pair<int, EtVector3>( (*itor).first, GetFarmAreaPos((*itor).first) ) );
	}
}

Farm::AreaState::eState CDnLifeSkillPlantTask::GetPropState( DnPropHandle hProp )
{
	SFarmArea * pFarmArea = GetFarmArea( hProp );

	if( pFarmArea )
		return pFarmArea->m_eState;

	return Farm::AreaState::NONE;
}

CDnLifeSkillPlantTask::SPlantTooltip CDnLifeSkillPlantTask::PlantInfo( SFarmArea * pFarmArea )
{
	SPlantTooltip sInfo;

	sInfo.m_wszName = std::wstring( pFarmArea->m_wszPlantName );
	sInfo.m_eState = pFarmArea->m_eState;

	if( Farm::AreaState::GROWING == pFarmArea->m_eState )
	{
		sInfo.m_wszState = std::wstring( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7425 ) );

		int deltaSecond = (int)((pFarmArea->m_dwElapsedTime >= pFarmArea->m_dwTotalTime) ? 0 : (pFarmArea->m_dwTotalTime - pFarmArea->m_dwElapsedTime) / 1000);
		int deltaMinute = deltaSecond / 60;
		int deltaHour = deltaMinute / 60;
		int deltaDay = deltaHour / 24;
		deltaSecond = deltaSecond % 60;
		deltaMinute = deltaMinute % 60;
		deltaHour = deltaHour % 24;

		WCHAR wszTemp[256];
		WCHAR wszSecond[256];

		swprintf_s( wszTemp, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7437 ), deltaDay, deltaHour, deltaMinute, deltaSecond );
		sInfo.m_wszTime = std::wstring( wszTemp );

		swprintf_s( wszSecond, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7438 ), pFarmArea->m_nConsumePerSecond );

		swprintf_s( wszTemp, L"%d / %d", pFarmArea->m_dwRemainWater <= 0 ? 0 : (DWORD)pFarmArea->m_dwRemainWater / 1000 , (DWORD)pFarmArea->m_dwTotalWater / 1000 );
		sInfo.m_wszWater = std::wstring( wszTemp );
		sInfo.m_wszWaterCount = wszSecond;

		deltaSecond = (int)(pFarmArea->m_dwRemainWater > 0 ?  pFarmArea->m_dwRemainWater / 10000 : 0);
		deltaMinute = deltaSecond / 60;
		deltaHour = deltaMinute / 60;
		deltaDay = deltaHour / 24;
		deltaSecond = deltaSecond % 60;
		deltaMinute = deltaMinute % 60;
		deltaHour = deltaHour % 24;

		swprintf_s( wszTemp, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7437 ), deltaDay, deltaHour, deltaMinute, deltaSecond );
		sInfo.m_wszWaterTime = std::wstring( wszTemp );
		sInfo.m_wszPlayerName = std::wstring( pFarmArea->m_wszCharName );
	}
	else if( Farm::AreaState::COMPLETED == pFarmArea->m_eState )
	{
		sInfo.m_wszState = std::wstring( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7426 ) );
		sInfo.m_wszPlayerName = std::wstring( pFarmArea->m_wszCharName );
	}

	return sInfo;
}

CDnLifeSkillPlantTask::SPlantTooltip CDnLifeSkillPlantTask::PlantInfo( int nAreaIndex )
{
	SPlantTooltip sInfo;

	SFarmArea * pFarmArea = GetFarmArea( nAreaIndex );

	if( pFarmArea && pFarmArea->m_eState != Farm::AreaState::NONE )
		sInfo = PlantInfo( pFarmArea );

	return sInfo;
}

CDnLifeSkillPlantTask::SPlantTooltip CDnLifeSkillPlantTask::PlantInfo( DnPropHandle hProp )
{
	SPlantTooltip sInfo;

	SFarmArea * pFarmArea = GetFarmArea( hProp );

	if( pFarmArea && pFarmArea->m_eState != Farm::AreaState::NONE )
		sInfo = PlantInfo( pFarmArea );

	return sInfo;
}

void CDnLifeSkillPlantTask::PropOperation( DnPropHandle hProp )
{
	if( !CDnActor::s_hLocalActor ) return;
	CDnLocalPlayerActor * pLocalPlayer = dynamic_cast<CDnLocalPlayerActor *>( CDnActor::s_hLocalActor.GetPointer() );
	if( !pLocalPlayer ) return;

#if defined(PRE_ADD_VIP_FARM)
	if( Farm::Attr::Vip & CDnBridgeTask::GetInstance().GetAttr() )
	{
		bool bVip = pLocalPlayer->IsVipFarm();

		if( !bVip )
		{
			GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7534 ), MB_OK, QUESTION_REJECT_PLANT, this );	//Farm entry item has expired. Press [OK] to move from the farm to the farm village.
			return;
		}
	}
#endif	//#if defined(PRE_ADD_VIP_FARM)

	std::map<int, SFarmArea *>::iterator Itor = m_mFarmArea.begin();

	for( ; Itor != m_mFarmArea.end(); ++Itor )
	{
		if( (*Itor).second->m_pProp->GetMySmartPtr().GetHandle() == hProp->GetMySmartPtr().GetHandle()	)
		{
			EtVector3 vPos = (*Itor).second->m_pProp->GetMatEx()->m_vPosition;
			EtVector2 vDir;

			vDir.x = vPos.x - pLocalPlayer->GetPosition()->x;
			vDir.y = vPos.z - pLocalPlayer->GetPosition()->z;
			EtVec2Normalize( &vDir, &vDir );

			pLocalPlayer->CmdLook( vDir );

			m_PlantInfo.m_pFarmArea = (*Itor).second;
			m_PlantInfo.m_nSelectAreaIndex = (*Itor).first;

			if( Farm::AreaState::NONE == m_PlantInfo.m_pFarmArea->m_eState )
				CDnInterface::GetInstance().OpenLifeSlotDialog();			
			else if( Farm::AreaState::GROWING == m_PlantInfo.m_pFarmArea->m_eState && m_PlantInfo.m_pFarmArea->m_fWaterBar > 0.f)
			{
#if defined( PRE_REMOVE_FARM_WATER )
				return;
#endif	// PRE_REMOVE_FARM_WATER

				int eWater = UseWater(hProp);

				if( eWater_Disable_Enough == eWater )
				{
					GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7501 ) );	// The plant is full of water and cannot be watered any more.
					return;
				}
				else if( eWater_Disable_Item == eWater )
				{
					GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7480 ) );	// No watering can in inventory.
					return;
				}
				else if( eWater_Disable_Skill == eWater )
				{
					return;
				}

				m_vWaterItem.clear();
				m_vWaterSlotIndex.clear();
				m_vWaterBottle.clear();

				GetItemTask().GetCharInventory().FindItemFromItemType( ITEMTYPE_WATER, m_vWaterItem, m_vWaterSlotIndex );

				GetItemTask().FindItemFromItemType( ITEMTYPE_WATERBOTTLE, ITEM_SLOT_TYPE::ST_INVENTORY_CASH, m_vWaterBottle);
				
				if( m_vWaterItem.empty() && m_vWaterBottle.empty() )
				{
					GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7480 ), false );	// No watering can in inventory.
					return;
				}
				else if( !m_vWaterItem.empty() && !m_vWaterBottle.empty() )
				{
					GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7462 ), MB_ABORTRETRYIGNORE, QUESTION_WATER_BOTTLE, this );
				}
				else if( !m_vWaterItem.empty() )
				{
					WaterSizeChoice();
				}
				else if( !m_vWaterBottle.empty() )
				{
					m_PlantInfo.m_nUseWaterItemID = m_vWaterBottle[0]->GetClassID();
					m_PlantInfo.m_nCashWaterItemSerial = m_vWaterBottle[0]->GetSerialID();
					BattleAndVehicleMode( false );
					RequestAddWater( m_PlantInfo.m_nSelectAreaIndex, Farm::ActionType::START, ITEMPOSITION_CASHINVEN, 0, m_vWaterBottle[0]->GetSerialID() );
				}
			}
			else if( Farm::AreaState::COMPLETED == m_PlantInfo.m_pFarmArea->m_eState || Farm::AreaState::HARVESTING == m_PlantInfo.m_pFarmArea->m_eState )
			{
				if( eHarvest_Enable != UseHarvest( hProp ) )
					return;

				int iSlotIndex = GetItemTask().GetCharInventory().FindItemSlotIndex( (*Itor).second->m_iHarvestItemID );

				if( -1 != iSlotIndex )
				{
					m_PlantInfo.m_cInvenIndex = iSlotIndex;
					BattleAndVehicleMode( false );
					RequestHarvest( m_PlantInfo.m_nSelectAreaIndex, Farm::ActionType::START, iSlotIndex );
				}
				else
					GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7485 ), false );	// No sickle in inventory.
			}

			return;
		}
	}

	m_PlantInfo.Clear();
}

void CDnLifeSkillPlantTask::InsertHarvest( int nAreaIndex, SFarmArea * pFarmArea )
{
	EtVector3 vPos = pFarmArea->m_pProp->GetMatEx()->m_vPosition;

	SAABox box;
	pFarmArea->m_pProp->GetBoundingBox(box);
	float fHeight = box.Max.y - box.Min.y + 10.f;
	
	// Root-type icons sit low, so add a bit more offset.
	DNTableFileFormat* pFarmSox = GetDNTable( CDnTableDB::TFARMCULTIVATE );
	int nPropID = pFarmSox->GetFieldFromLablePtr( pFarmArea->m_iItemID, "_SkinID" )->GetInteger();

	if( 4 == nPropID )
		fHeight += 10.0f;

	vPos.y += fHeight;

	if( m_pLifeHarvestIcon )
		m_pLifeHarvestIcon->InsertList( nAreaIndex, vPos, pFarmArea->m_szHarvestIconName );
}

void CDnLifeSkillPlantTask::DeleteHarvest( int nAreaIndex )
{
	if( m_pLifeHarvestIcon )
		m_pLifeHarvestIcon->DeleteList( nAreaIndex );
}

void CDnLifeSkillPlantTask::GrowingProcess( float fElapedTime )
{
	float fTimeBar = 50.f;
	float fWaterBar = 50.f;

	if( !CDnActor::s_hLocalActor ) return;
	CDnLocalPlayerActor * localActor = ((CDnLocalPlayerActor *)(CDnActor::s_hLocalActor.GetPointer()));

	INT64 dwConsumeWater;

	for( std::vector<SFarmArea *>::iterator itor = m_vGrowingArea.begin(); itor != m_vGrowingArea.end(); ++itor )
	{
		(*itor)->m_dwElapsedTime += (INT64)(fElapedTime * 1000);
		dwConsumeWater = (*itor)->m_nConsumePerSecond * (*itor)->m_dwElapsedTime;

		(*itor)->m_dwRemainWater = (*itor)->m_dwWater + (*itor)->m_dwWaterBottle + (*itor)->m_dwTotalWater;
		if( (*itor)->m_dwRemainWater > dwConsumeWater )
			(*itor)->m_dwRemainWater -= dwConsumeWater;
		else
			(*itor)->m_dwRemainWater = 0;

		(*itor)->m_fTimeBar = (float)( (double)(*itor)->m_dwElapsedTime / (double)(*itor)->m_dwTotalTime );
		(*itor)->m_fWaterBar = (float)( (double)(*itor)->m_dwRemainWater / (double)(*itor)->m_dwTotalWater );

		if( (*itor)->m_fTimeBar >= 0.5f && (*itor)->m_eGrowLevel == GROW_LEVEL1 )
			(*itor)->SetGrowLevel( GROW_LEVEL2 );

		if( (*itor)->m_fWaterBar < 0.f )
		{
			(*itor)->m_fWaterBar = 0.f;
			(*itor)->m_dwRemainWater = 0;
		}

		if( (*itor)->m_pLifeGaugeDlg == NULL )
			continue;
		else if( dwConsumeWater > (*itor)->m_dwWaterBottle )
			(*itor)->m_pWaterProp = DeleteProp( (*itor)->m_pWaterProp, (*itor)->m_pOBB->Center );

		(*itor)->m_pLifeGaugeDlg->SetTimeBar( (float)((*itor)->m_fTimeBar * 100.0f) );
		(*itor)->m_pLifeGaugeDlg->SetWaterBar( (float)((*itor)->m_fWaterBar * 100.0f) );
		
		(*itor)->m_pLifeGaugeDlg->UpdateGauge( (*itor)->m_iItemID, (*itor)->m_eGrowLevel );

		if( __wcsicmp_l( localActor->GetName(), (*itor)->m_wszCharName ) == 0 )
			GetInterface().ChangeState( (*itor)->m_nSelectAreaIndex, (*itor)->m_iItemID, Farm::AreaState::GROWING, (float)((*itor)->m_fTimeBar * 100.0f), (float)((*itor)->m_fWaterBar * 100.0f), (*itor)->m_szHarvestIconName );
	}
}

void CDnLifeSkillPlantTask::WaterSizeChoice()
{
	bool bWaterChoice = false;
	int nIndex = 0, nWater = m_vWaterItem[0]->GetTypeParam(0);
	for( int itr = 1 ; itr < (int)m_vWaterItem.size(); ++itr )
	{
		if( nWater != m_vWaterItem[itr]->GetTypeParam(0) )
			bWaterChoice = true;

		if( nWater > m_vWaterItem[itr]->GetTypeParam(0) )
		{
			nIndex = itr;
			nWater = m_vWaterItem[itr]->GetTypeParam(0);
		}
	}

	if( bWaterChoice )
	{
		if( !CDnActor::s_hLocalActor )	return;
		CDnLocalPlayerActor * pLocalPlayer = dynamic_cast<CDnLocalPlayerActor *>(CDnActor::s_hLocalActor.GetPointer() );
		if( !pLocalPlayer ) return;

		pLocalPlayer->LockInput( true );

		m_PlantInfo.m_nUseWaterItemID = m_vWaterItem[nIndex]->GetClassID();
		m_PlantInfo.m_cInvenIndex = m_vWaterSlotIndex[nIndex];

		WCHAR wszMsg[512];
		swprintf_s( wszMsg, _countof(wszMsg), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7525 ), m_vWaterItem[nIndex]->GetName() );

		GetInterface().MessageBox( wszMsg, MB_YESNO, QUESTION_WATER_CHOICE, this );
	}
	else
	{
		m_PlantInfo.m_nUseWaterItemID = m_vWaterItem[0]->GetClassID();
		m_PlantInfo.m_cInvenIndex = m_vWaterSlotIndex[0];
		BattleAndVehicleMode( false );
		RequestAddWater( m_PlantInfo.m_nSelectAreaIndex, Farm::ActionType::START, ITEMPOSITION_INVEN, m_vWaterSlotIndex[0], 0 );
	}	
}

int CDnLifeSkillPlantTask::UseWater( DnPropHandle hProp )
{
	CSecondarySkill * pSecondarySkill = GetSkillTask().GetSecondarySkill( SecondarySkill::SubType::CultivationSkill );
	if( !pSecondarySkill || pSecondarySkill->GetGrade() == 0  )
		return eWater_Disable_Skill;

	SFarmArea * pFarmArea = GetFarmArea( hProp );

	if( !pFarmArea )
		return eWater_Disable_Enough;

	std::vector<CDnItem *> vecItems, vecCashItems;

	GetItemTask().FindItemFromItemType( ITEMTYPE_WATER, ITEM_SLOT_TYPE::ST_INVENTORY, vecItems );
	GetItemTask().FindItemFromItemType( ITEMTYPE_WATERBOTTLE, ITEM_SLOT_TYPE::ST_INVENTORY_CASH, vecCashItems );

	if( vecItems.empty() && vecCashItems.empty() )
		return eWater_Disable_Item;

	if( pFarmArea->m_dwRemainWater > pFarmArea->m_dwTotalWater && pFarmArea->m_dwWaterBottle > pFarmArea->m_nConsumePerSecond * pFarmArea->m_dwElapsedTime )
		return eWater_Disable_Enough;

	if( !vecItems.empty() )
	{
		if( pFarmArea->m_dwRemainWater < pFarmArea->m_dwTotalWater )
			return eWater_Enable;
	}
	if( !vecCashItems.empty() )
	{
		if( pFarmArea->m_dwWaterBottle < pFarmArea->m_nConsumePerSecond * pFarmArea->m_dwElapsedTime )
			return eWater_Enable;
	}

	return eWater_Disable_Enough;
}

int CDnLifeSkillPlantTask::UseHarvest( DnPropHandle hProp )
{	
	CSecondarySkill * pSecondarySkill = GetSkillTask().GetSecondarySkill( SecondarySkill::SubType::CultivationSkill );
	if( !pSecondarySkill || pSecondarySkill->GetGrade() == 0  )
		return eHarvest_Disable_Skill;

	SFarmArea * pFarmArea = GetFarmArea( hProp );

	if( !pFarmArea )
		return eHarvest_Disable_Item;

	int iSlotIndex = GetItemTask().GetCharInventory().FindItemSlotIndex( pFarmArea->m_iHarvestItemID );

	if( -1 != iSlotIndex )
		return eHarvest_Enable;

	return eHarvest_Disable_Item;
}

void CDnLifeSkillPlantTask::OpenBaseDialog()
{
	for(int itr = 0; itr < (int)m_vGrowingArea.size(); ++itr )
		m_vGrowingArea[itr]->m_pLifeGaugeDlg->Show( true );
}

void CDnLifeSkillPlantTask::Process(LOCAL_TIME LocalTime, float fDelta)
{
	GrowingProcess( fDelta );
}

void CDnLifeSkillPlantTask::OnUICallbackProc(int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg)
{
	switch( nID ) 
	{
	case ACCEPT_REQUEST_DIALOG:
		{
			if( nCommand == EVENT_BUTTON_CLICKED )
			{
				if( strcmp( pControl->GetControlName(), "ID_PROCESS_COMPLETE") == 0 )
				{
					if( ePlant_Seed == m_eAcceptType )
						RequestPlant( m_PlantInfo.m_nSelectAreaIndex, Farm::ActionType::COMPLETE, m_PlantInfo.m_cInvenIndex, m_PlantInfo.m_cCashItemCount, m_PlantInfo.m_nCashBoostItemSerials, m_PlantInfo.m_pCashBoostItemCount );
					else if( ePlant_Water == m_eAcceptType )
					{
						if( !m_vWaterBottle.empty() && m_PlantInfo.m_nCashWaterItemSerial )
							RequestAddWater( m_PlantInfo.m_nSelectAreaIndex, Farm::ActionType::COMPLETE, ITEMPOSITION_CASHINVEN, 0, m_PlantInfo.m_nCashWaterItemSerial );
						else if( !m_vWaterItem.empty() )
							RequestAddWater( m_PlantInfo.m_nSelectAreaIndex, Farm::ActionType::COMPLETE, ITEMPOSITION_INVEN, m_PlantInfo.m_cInvenIndex, 0 );
					}
					else if( ePlant_Harvest == m_eAcceptType )
						RequestHarvest( m_PlantInfo.m_nSelectAreaIndex, Farm::ActionType::COMPLETE, m_PlantInfo.m_cInvenIndex);
				}
				else if( strcmp( pControl->GetControlName(), "ID_BUTTON_CANCEL") == 0 )
				{
					if( ePlant_Seed == m_eAcceptType )
					{
						m_PlantInfo.m_pFarmArea->m_eState = Farm::AreaState::NONE;
						RequestPlant( m_PlantInfo.m_nSelectAreaIndex, Farm::ActionType::CANCEL, m_PlantInfo.m_cInvenIndex, m_PlantInfo.m_cCashItemCount, m_PlantInfo.m_nCashBoostItemSerials, m_PlantInfo.m_pCashBoostItemCount );
					}
					else if( ePlant_Water == m_eAcceptType )
					{
						m_PlantInfo.m_pFarmArea->m_eState = Farm::AreaState::GROWING;
						RequestAddWater( m_PlantInfo.m_nSelectAreaIndex, Farm::ActionType::CANCEL, ITEMPOSITION_INVEN, m_PlantInfo.m_cInvenIndex, m_PlantInfo.m_nCashWaterItemSerial );
					}
					else if( ePlant_Harvest == m_eAcceptType )
					{
						m_PlantInfo.m_pFarmArea->m_eState = Farm::AreaState::COMPLETED;
						RequestHarvest( m_PlantInfo.m_nSelectAreaIndex, Farm::ActionType::CANCEL, m_PlantInfo.m_cInvenIndex );
					}
					GetInterface().CloseMovieProcessDlg(true);
				}

				CDnLocalPlayerActor::LockInput( false );
			}
		}
		break;
	case QUESTION_WATER_BOTTLE:
		{
			if( nCommand == EVENT_BUTTON_CLICKED )
			{
				if( strcmp( pControl->GetControlName(), "ID_BT_BUCKET" ) == 0 ) 
				{
					if( !m_vWaterBottle.empty() )
					{
						m_PlantInfo.m_nUseWaterItemID = m_vWaterBottle[0]->GetClassID();
						m_PlantInfo.m_nCashWaterItemSerial = m_vWaterBottle[0]->GetSerialID();
						m_PlantInfo.m_cInvenIndex = 0;
						BattleAndVehicleMode( false );
						RequestAddWater( m_PlantInfo.m_nSelectAreaIndex, Farm::ActionType::START, ITEMPOSITION_CASHINVEN, 0, m_vWaterBottle[0]->GetSerialID() );
					}
				}
				else if( strcmp( pControl->GetControlName(), "ID_BT_USEPOT" ) == 0 ) 
				{
					if( !m_vWaterSlotIndex.empty() )
					{
						WaterSizeChoice();
					}
				}					
			}
		}
		break;
	case QUESTION_WATER_CHOICE:
		{
			if( nCommand == EVENT_BUTTON_CLICKED )
			{
				if( strcmp( pControl->GetControlName(), "ID_YES" ) == 0 ) 
				{
					BattleAndVehicleMode( false );
					RequestAddWater( m_PlantInfo.m_nSelectAreaIndex, Farm::ActionType::START, ITEMPOSITION_INVEN, m_PlantInfo.m_cInvenIndex, 0 );
				}
			}

			if( !CDnActor::s_hLocalActor )	return;
			CDnLocalPlayerActor * pLocalPlayer = dynamic_cast<CDnLocalPlayerActor *>(CDnActor::s_hLocalActor.GetPointer() );
			if( !pLocalPlayer ) return;

			pLocalPlayer->LockInput( false );
		}
		break;

	case QUESTION_REJECT_PLANT:
		{
			if( CDnLocalPlayerActor::IsLockInput() ) break;

			GetInterface().TerminateStageClearWarpStandBy();

			// Note : Move to town
			CDnGameTask* pGameTask = (CDnGameTask *) CTaskManager::GetInstance().GetTask( "GameTask" );
			if( pGameTask ) {
				switch( pGameTask->GetGameTaskType() ) {
					case GameTaskType::Normal:
					case GameTaskType::DarkLair:
					case GameTaskType::Farm:
					{
#ifdef PRE_FIX_GAMEQUIT_TO_VILLAGE
						SendAbandonStage(false, false);
#else
						SendAbandonStage(false);
#endif
					}
					break;
					case GameTaskType::PvP:
						SendMovePvPGameToPvPLobby();
						break;
				}
			}
		}
		break;
	}
}

void CDnLifeSkillPlantTask::Open(int ePlant)
{
	m_eAcceptType = ePlant;

	WCHAR wszString[256];
	if( ePlant_Seed == m_eAcceptType )
		swprintf_s( wszString, _countof(wszString), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7467 ) );	// Planting seed...
	else if( ePlant_Water == m_eAcceptType )
		swprintf_s( wszString, _countof(wszString), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7468 ) );	// Watering...
	else if( ePlant_Harvest == m_eAcceptType )
		swprintf_s( wszString, _countof(wszString), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7469 ) );	// Harvesting...

	GetInterface().OpenMovieProcessDlg( NULL, PROGRESS_BAR_TIME, wszString, ACCEPT_REQUEST_DIALOG, this );

	CDnLocalPlayerActor::LockInput( true );
	CDnMouseCursor::GetInstance().ShowCursor( true, true );
}

void CDnLifeSkillPlantTask::BattleAndVehicleMode( bool bBattle )
{
	// Remember whether own character is in battle mode, then switch own battle mode according to the argument.
	// When tending the farm, argument is false; when farm work ends, switch to the remembered battle mode.
	// This function is called before starting farm work and after finishing farm work (cancel, complete).

	if( !CDnActor::s_hLocalActor ) return;
	CDnLocalPlayerActor * localActor = ((CDnLocalPlayerActor *)(CDnActor::s_hLocalActor.GetPointer()));

	m_bLocalBattle = localActor->IsBattleMode();

	if( m_bLocalBattle != bBattle && !localActor->IsCallingVehicle())
		localActor->CmdToggleBattle( bBattle );

	if( localActor->IsVehicleMode() )
	{
		//localActor->UnRideVehicle(true);
		CDnCharStatusDlg * pCharStatusDlg = (CDnCharStatusDlg *)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::CHARSTATUS_DIALOG );
		CDnCharVehicleDlg * pCharVehicleDlg = pCharStatusDlg->GetVehicleDlg();

		if( pCharVehicleDlg )
			pCharVehicleDlg->ForceUnRideVehicle();
	}
}

void CDnLifeSkillPlantTask::SetFarmAreaInfo( CDnLifeSkillPlantTask::SFarmArea * pFarmArea, TFarmAreaInfo * pPacket )
{
	DNTableFileFormat* pFarmSkinSox = GetDNTable( CDnTableDB::TFARMSKIN );
	DNTableFileFormat* pFarmSox = GetDNTable( CDnTableDB::TFARMCULTIVATE );
	DNTableFileFormat* pItemSox = GetDNTable( CDnTableDB::TITEM );
	if( !pFarmSox || !pItemSox || !pFarmSkinSox || !pFarmArea )	return;

	int nPropID = pFarmSox->GetFieldFromLablePtr( pPacket->iItemID, "_SkinID" )->GetInteger();
	char * szHarvestIconName = pFarmSkinSox->GetFieldFromLablePtr( nPropID, "_HarvestIcon" )->GetString();;
	_strcpy( pFarmArea->m_szHarvestIconName, _countof(pFarmArea->m_szHarvestIconName), szHarvestIconName, (int)strlen(szHarvestIconName) );
	pFarmArea->m_nSelectAreaIndex = pPacket->iAreaIndex;
	pFarmArea->m_iHarvestItemID = pFarmSox->GetFieldFromLablePtr( pPacket->iItemID, "_HarvestItemID" )->GetInteger();
	if( Farm::AreaState::NONE != pPacket->State )
	{
		_wcscpy( pFarmArea->m_wszCharName, _countof(pFarmArea->m_wszCharName), pPacket->wszCharName, (int)wcslen(pPacket->wszCharName) );
		pFarmArea->m_iItemID = pPacket->iItemID;
	}

	std::wstring wszItemName = CDnItem::GetItemFullName( pPacket->iItemID );

	_wcscpy( pFarmArea->m_wszPlantName, _countof(pFarmArea->m_wszPlantName), wszItemName.c_str(), (int)wcslen(wszItemName.c_str()) );

	pFarmArea->m_nRandSeed = pPacket->iRandSeed;

	if( Farm::AreaState::GROWING == pPacket->State )
	{
		pFarmArea->m_dwElapsedTime = pPacket->iElapsedTick;
		pFarmArea->m_dwTotalTime = pFarmSox->GetFieldFromLablePtr( pPacket->iItemID, "_CultivateMaxTime" )->GetInteger() * 1000;
		pFarmArea->m_nConsumePerSecond = pFarmSox->GetFieldFromLablePtr( pPacket->iItemID, "_ConsumeWater" )->GetInteger();
		pFarmArea->m_dwTotalWater = pFarmSox->GetFieldFromLablePtr( pPacket->iItemID, "_MaxWater" )->GetInteger() * 1000;

		pFarmArea->m_dwWaterBottle = 0;
		pFarmArea->m_dwWater = 0;
		bool bIsWaterBottle = false;
		INT64 dwDecreaseTime = 0;
		for( BYTE itr = 0; itr < pPacket->cAttachCount; ++itr )
		{
			if( GROWING_BOOST_ITEM == pPacket->AttachItems[itr].iItemID 	//Growth accelerator
#if defined(PRE_ADD_VIP_FARM)
				|| Farm::Common::VIP_GROWING_BOOST_ITEMID == pPacket->AttachItems[itr].iItemID	// Set by default if VIP.
#endif
#if defined( PRE_ADD_TOTAL_LEVEL_SKILL )
				|| ITEMTYPE_GROWING_BOOST ==  pItemSox->GetFieldFromLablePtr( pPacket->AttachItems[itr].iItemID, "_Type" )->GetInteger()	// Add type check
#endif
				)
			{
				DWORD dwPer = pItemSox->GetFieldFromLablePtr( pPacket->AttachItems[itr].iItemID, "_TypeParam1" )->GetInteger() * pPacket->AttachItems[itr].iCount;

				dwDecreaseTime += pFarmArea->m_dwTotalTime * dwPer / 100;	// Calculate percentage
			}
			else	//Water, water bucket
			{
				if( pItemSox->GetFieldFromLablePtr( pPacket->AttachItems[itr].iItemID, "_IsCash" )->GetInteger() == TRUE )
					pFarmArea->m_dwWaterBottle += pItemSox->GetFieldFromLablePtr( pPacket->AttachItems[itr].iItemID, "_TypeParam1" )->GetInteger() * pPacket->AttachItems[itr].iCount * 1000;
				else
					pFarmArea->m_dwWater += (INT64)pItemSox->GetFieldFromLablePtr( pPacket->AttachItems[itr].iItemID, "_TypeParam1" )->GetInteger() * (INT64)pPacket->AttachItems[itr].iCount * 1000;
			}
		}
		pFarmArea->m_dwTotalTime -= dwDecreaseTime;

		//If the water container has more water than consumed, spawn the water container prop.
		if( pFarmArea->m_dwElapsedTime * pFarmArea->m_nConsumePerSecond < pFarmArea->m_dwWaterBottle )
		{
			EtVector3 etVector = pFarmArea->m_pOBB->Center;
			etVector += pFarmArea->m_pOBB->Axis[0] * pFarmArea->m_pOBB->Extent[0] * 0.8f;
			etVector -= pFarmArea->m_pOBB->Axis[2] * pFarmArea->m_pOBB->Extent[2] * 0.8f;

			pFarmArea->m_pWaterProp = AddProp( "SH_D_BlackmineSwitch01.skn", etVector, 0 );
		}
	}
}

int CDnLifeSkillPlantTask::GetExtendPrivateFarmFieldCount()
{
#if defined( PRE_ADD_PRIVATEFARM_EXTEND_CASH )
	std::vector< CDnItem *> pList;
	CDnItemTask::GetInstance().GetCashInventory().FindItemFromItemType( ITENTYPE_FARM_PRIVATE_EXTNED, pList );

	if( pList.empty() )
		return 0;

	int iExtend = 0;
	for( UINT itr = 0; itr < pList.size(); ++itr )
		iExtend += pList[itr]->GetTypeParam( 0 );

	return iExtend;
#else
	return 0;
#endif	// #if defined( PRE_ADD_PRIVATEFARM_EXTEND_CASH )
}

void CDnLifeSkillPlantTask::PrivateFarmErrorMessage()
{
#ifdef PRE_FIX_FARMERRORMSG_USA
	std::wstring wszString;
	int iAvailCount = Farm::Max::PRIVATEPLANTCOUNT + GetExtendPrivateFarmFieldCount();
	wszString = FormatW(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7541 ), iAvailCount);

	GetInterface().MessageBox( wszString.c_str() );
#else
	WCHAR wszString[128];
	int iAvailCount = Farm::Max::PRIVATEPLANTCOUNT + GetExtendPrivateFarmFieldCount();
	swprintf_s( wszString, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7541 ), iAvailCount );	//  Cannot plant any more seeds in your personal farm. [Max seeds currently allowed in personal farm: %d.]

	GetInterface().MessageBox( wszString );
#endif
}

void CDnLifeSkillPlantTask::OnRecvSeedInfoList( SCAreaInfoList * pPacket )
{
	SFarmArea * pFarmArea;

	for( USHORT itr = 0; itr < pPacket->unCount; ++itr )
	{
		std::map<int, SFarmArea *>::iterator itor = m_mFarmArea.find( pPacket->AreaInfos[itr].iAreaIndex );

		if( itor == m_mFarmArea.end() )
			continue;

		pFarmArea = (*itor).second;

		SetFarmAreaInfo( pFarmArea, &pPacket->AreaInfos[itr] );

		ChangeState( pPacket->AreaInfos[itr].State, pPacket->AreaInfos[itr].iAreaIndex, pFarmArea );
	}

	GetInterface().FarmConditionRefresh();
}

void CDnLifeSkillPlantTask::OnRecvAreaInfo( SCAreaInfo * pPacket )
{
	std::map<int, SFarmArea *>::iterator itor = m_mFarmArea.find( pPacket->AreaInfo.iAreaIndex );
	if( itor == m_mFarmArea.end() )	
		return;

	SFarmArea * pFarmArea = (*itor).second;

	SetFarmAreaInfo( pFarmArea, &pPacket->AreaInfo );

	ChangeState( pPacket->AreaInfo.State, pPacket->AreaInfo.iAreaIndex, pFarmArea );
	
	GetInterface().FarmConditionRefresh();
}

void CDnLifeSkillPlantTask::OnRecvPlant( SCFarmPlant * pPacket )
{
	if( !CDnActor::s_hLocalActor ) return;
	CDnLocalPlayerActor * localActor = ((CDnLocalPlayerActor *)(CDnActor::s_hLocalActor.GetPointer()));

	if( pPacket->iRet != ERROR_NONE )
	{
		if( ERROR_FARM_CANT_START_PLANTSTATE == pPacket->iRet )
			GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7502 ) );	// Someone else is already planting a seed
		else if( ERROR_FARM_CANT_START_PLANTSTATE_MAXFIELDCOUNT == pPacket->iRet )
			GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7503 ) );	// Cannot cultivate any more plants with your current cultivation skill level.
		else if( ERROR_FARM_CANT_PLANT_OVERLAPCOUNT == pPacket->iRet )
		{
			GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7521 ) );	// Cannot grow the same plant anymore.
			localActor->CmdAction( "Stand" );
			GetInterface().CloseMovieProcessDlg(true);
			CDnLocalPlayerActor::LockInput( false );
		}
		else if( ERROR_FARM_CANT_START_PLANTSTATE_MAXPRIVATEFIELDCOUNT == pPacket->iRet )
			PrivateFarmErrorMessage();
		else if( ERROR_FARM_CANT_START_PLANTSTATE_DOWNSCALEFARM == pPacket->iRet )
			GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7543 ) );	//  The farm area is currently being closed. You can no longer plant seeds here.
		else if( ERROR_FARM_INVALUD_CHAMPIONGUILD == pPacket->iRet )
			GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7552 ) );	//  Not a member of the winning guild. Cannot plant seeds.
		else if( ERROR_FARM_CANT_START_PLANTSTATE_GUILDCHAMPION_TIMELIMIT == pPacket->iRet )
			GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7553 ) );	//  Cannot plant seeds anymore as guild war finals time is approaching.

		m_PlantInfo.m_pFarmArea->m_eState = Farm::AreaState::NONE;
		BattleAndVehicleMode( m_bLocalBattle );
		return;
	}

	if( Farm::ActionType::START == pPacket->ActionType )
	{
		Open(ePlant_Seed);
		m_PlantInfo.m_pFarmArea->m_eState = Farm::AreaState::PLANTING;
		localActor->CmdAction( "LifeSystem_Plant" );
		GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7471 ), false );	// Starting seed planting.
	}
	else if( Farm::ActionType::CANCEL == pPacket->ActionType )
	{
		if( m_bLocalBattle )
			BattleAndVehicleMode( m_bLocalBattle );
		else
			localActor->CmdAction( "Stand" );
		GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7475 ), false );	// Canceling seed planting.
	}
	else if( Farm::ActionType::COMPLETE == pPacket->ActionType )
	{
		m_PlantInfo.m_pFarmArea->m_eState = Farm::AreaState::GROWING;
		BattleAndVehicleMode( m_bLocalBattle );

		WCHAR wszString[256];
		std::wstring wszItemName = CDnItem::GetItemFullName( pPacket->iItemID );

		swprintf_s( wszString, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7473 ), wszItemName.c_str() );

		GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", wszString, false );
	}
}

void CDnLifeSkillPlantTask::OnRecvAddWater( SCFarmAddWater * pPacket )
{
	if( !CDnActor::s_hLocalActor ) return;
	CDnLocalPlayerActor * localActor = ((CDnLocalPlayerActor *)(CDnActor::s_hLocalActor.GetPointer()));

	SFarmArea * pFarmArea = GetFarmArea( pPacket->iAreaIndex );

	if( pFarmArea && Farm::AreaState::COMPLETED == pFarmArea->m_eState )
		return;

	if( ERROR_NONE != pPacket->iRet )
	{
		if( ERROR_FARM_CANT_START_ADDWATERSTATE == pPacket->iRet )
			GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7504 ) );	// Someone else is already watering the plant.

		if( Farm::AreaState::ADDWATER == m_PlantInfo.m_pFarmArea->m_eState )
			m_PlantInfo.m_pFarmArea->m_eState = Farm::AreaState::GROWING;
		BattleAndVehicleMode( m_bLocalBattle );
		return;
	}

	if( Farm::ActionType::START == pPacket->ActionType )
	{
		Open( ePlant_Water );
		m_PlantInfo.m_pFarmArea->m_eState = Farm::AreaState::ADDWATER;
		localActor->CmdAction( "LifeSystem_Watering" );
		GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7476 ), false );	// Start watering.
	}
	else if( Farm::ActionType::CANCEL == pPacket->ActionType )
	{
		if( m_bLocalBattle )
			BattleAndVehicleMode( m_bLocalBattle );
		else
			localActor->CmdAction( "Stand" );
		GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7479 ), false );	// Cancel watering.
	}
	else if( Farm::ActionType::COMPLETE == pPacket->ActionType )
	{
		m_PlantInfo.m_pFarmArea->m_eState = Farm::AreaState::GROWING;
		BattleAndVehicleMode( m_bLocalBattle );

		WCHAR wszString[256];
		DNTableFileFormat* pItemSox = GetDNTable( CDnTableDB::TITEM );
		int nWater = pItemSox->GetFieldFromLablePtr( m_PlantInfo.m_nUseWaterItemID, "_TypeParam1" )->GetInteger();
		std::wstring wszItemName = CDnItem::GetItemFullName( m_PlantInfo.m_pFarmArea->m_iItemID );

		swprintf_s( wszString, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7478 ), wszItemName.c_str(), nWater );

		GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", wszString , false);	// Water amount held by %s increased by %d
	}
}

void CDnLifeSkillPlantTask::OnRecvHarvest( SCFarmHarvest * pPacket )
{
	if( !CDnActor::s_hLocalActor ) return;
	CDnLocalPlayerActor * localActor = ((CDnLocalPlayerActor *)(CDnActor::s_hLocalActor.GetPointer()));

	if( ERROR_NONE != pPacket->iRet )
	{
		if( ERROR_FARM_CANT_START_HARVESTSTATE == pPacket->iRet )
			GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7505 ) );	// Someone else is already harvesting.

		m_PlantInfo.m_pFarmArea->m_eState = Farm::AreaState::COMPLETED;
		BattleAndVehicleMode( m_bLocalBattle );
		return;
	}

	if( Farm::ActionType::START == pPacket->ActionType )
	{
		Open( ePlant_Harvest );
		m_PlantInfo.m_pFarmArea->m_eState = Farm::AreaState::HARVESTING;
		localActor->CmdAction( "LifeSystem_harvest" );
		GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7481 ), false );	// Starting harvest.
	}
	else if( Farm::ActionType::CANCEL == pPacket->ActionType )
	{
		if( m_bLocalBattle )
			BattleAndVehicleMode( m_bLocalBattle );
		else
			localActor->CmdAction( "Stand" );
		GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7484 ), false );	// Canceling harvest.
	}
	else if( Farm::ActionType::COMPLETE == pPacket->ActionType )
	{
		m_PlantInfo.m_pFarmArea->m_eState = Farm::AreaState::HARVESTED;
		BattleAndVehicleMode( m_bLocalBattle );

		if( __wcsicmp_l( localActor->GetName(), m_PlantInfo.m_pFarmArea->m_wszCharName ) == 0 )
		{
			std::vector<CDnItem *> vItems;
			for( BYTE itr = 0; itr < Farm::Max::HARVESTDEPOT_COUNT; ++itr )
			{
				if( 0 == pPacket->iResultItemIDs[itr] )
					break;
				CDnItem * pItem;
				pItem = CDnItem::CreateItem( pPacket->iResultItemIDs[itr], 0, true );

				if( -1 == pItem->GetClassID() )
					continue;

				vItems.push_back( pItem );
			}
			GetInterface().ShowPlantAlarmDialog( vItems, textcolor::WHITE, 3.0f );

			WCHAR wszString[256];
			std::wstring wszItemName = CDnItem::GetItemFullName( m_PlantInfo.m_pFarmArea->m_iItemID );

			swprintf_s( wszString, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7483 ), wszItemName.c_str());

			GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", wszString, false );	//Harvested %s.
			GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7465 ) );	// Harvested crops can be checked in the farm storage.

		}
		else
		{
		
		}
	}
}

void CDnLifeSkillPlantTask::OnRecvAreaState( SCFarmBeginAreaState * pPacket )
{
	std::map<int, SFarmArea *>::iterator itor = m_mFarmArea.find( pPacket->iAreaIndex );
	if( itor == m_mFarmArea.end() )	
		return;

	if( Farm::AreaState::GROWING != pPacket->State )
		ChangeState( pPacket->State, pPacket->iAreaIndex, (*itor).second );

	if( Farm::AreaState::COMPLETED == pPacket->State )
		GetInterface().FarmConditionRefresh();
}

void CDnLifeSkillPlantTask::OnRecvWareHouseList( SCFarmWareHouseList * pPacket )
{
	if( ERROR_NONE != pPacket->iRet )
		return;

	GetInterface().RefreshLifeStorage( pPacket );
}

void CDnLifeSkillPlantTask::OnRecvTakeWareHouseItem( SCFarmTakeWareHouseItem * pPacket )
{
	if( ERROR_NONE != pPacket->iRet )
	{
		if( ERROR_ITEM_INVENTORY_NOTENOUGH == pPacket->iRet )
		{
			GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7470 ) );
		}
		return;
	}

	GetInterface().OnRecvTakeWareHouseItem(pPacket->biUniqueID);
}

void CDnLifeSkillPlantTask::OnRecvFieldCountInfo( SCFarmFieldCountInfo * pPacket )
{
	CDnLifeConditionDlg * pLifeCondition = GetInterface().GetLifeConditionDlg();

	if( pLifeCondition && pLifeCondition->IsShow() )
	{
		CSecondarySkill * pSecondarySkill = GetSkillTask().GetSecondarySkill( SecondarySkill::SubType::CultivationSkill );
		if( !pSecondarySkill || pSecondarySkill->GetGrade() == 0  )
			return;

		int nCount = 0;

		DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TSecondarySkillLevel );
		for( int itr = 0; itr < pSox->GetItemCount(); ++itr )
		{
			int nItemID = pSox->GetItemID(itr);
			int nSkill = pSox->GetFieldFromLablePtr(nItemID, "_SecondarySkillID")->GetInteger();
			int nLevel = pSox->GetFieldFromLablePtr(nItemID, "_SecondarySkillLevel")->GetInteger();
			int nGrade = pSox->GetFieldFromLablePtr(nItemID, "_SecondarySkillClass")->GetInteger();

			if( nSkill == pSecondarySkill->GetSkillID() &&
				nLevel == pSecondarySkill->GetLevel() &&
				nGrade == pSecondarySkill->GetGrade() )
			{
				nCount = pSox->GetFieldFromLablePtr(nItemID, "_SecondarySkillParam")->GetInteger();
				break;
			}
		}

		pLifeCondition->SetPossibleCount( pPacket->iCount, nCount );
	}
}

void CDnLifeSkillPlantTask::OnRecvWareHouseItemCount( SCFarmWareHouseItemCount * pPacket )
{
	m_nWareHouseItemCount = pPacket->iCount;

	if( GetInterface().GetMainBarDialog() ) GetInterface().GetMainBarDialog()->OnHarvestNotify( pPacket->iCount );
}

void CDnLifeSkillPlantTask::OnRecvAddWaterAnotheruser( SCFarmAddWaterAnotherUser * pPacket )
{
	WCHAR wszString[256];

	swprintf_s( wszString, _countof(wszString), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7519 ), pPacket->wszCharName );

	GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", wszString, false );
}

void CDnLifeSkillPlantTask::OnRecvChangeOwnerName( SCChangeOwnerName * pData )
{
	if( !CDnActor::s_hLocalActor) return;
	CDnLocalPlayerActor * localActor = ((CDnLocalPlayerActor *)(CDnActor::s_hLocalActor.GetPointer()));
	if( !localActor ) return;

	for( int itr = 0; itr < (int)m_vGrowingArea.size(); ++itr )
	{
		if( 0 == StrCmpW( m_vGrowingArea[itr]->m_wszCharName, pData->wszOriginName ) )
		{
			_wcscpy( m_vGrowingArea[itr]->m_wszCharName, _countof(m_vGrowingArea[itr]->m_wszCharName), pData->wszCharacterName, (int)wcslen(pData->wszCharacterName) );

			if( Farm::AreaState::GROWING == m_vGrowingArea[itr]->m_eState )
			{
				if( !m_vGrowingArea[itr]->m_pLifeGaugeDlg )
					return;

				// Set info if the owner of the growing farm crop is in the same guild.
				// Since guild task info is not updated yet, find the guild member by previous name and update to the new name.
				TGuildMember * pMember = CDnGuildTask::GetInstance().GetGuildMemberFromName(pData->wszOriginName);
				if( pMember )
					m_vGrowingArea[itr]->m_pLifeGaugeDlg->SetGuildInfo( true, localActor->GetGuildSelfView(), m_vGrowingArea[itr]->m_wszCharName );
				else
					m_vGrowingArea[itr]->m_pLifeGaugeDlg->SetGuildInfo( false, localActor->GetGuildSelfView(), m_vGrowingArea[itr]->m_wszCharName );
			}
		}
	}
}

void CDnLifeSkillPlantTask::RequestPlant(const int iAreaIndex, const Farm::ActionType::eType ActionType, const BYTE cSeedInvenIndex, const BYTE cCount, const INT64 * AttachItems, BYTE * pCount)
{
	Farm::Send::SendFarmPlant( iAreaIndex, ActionType, cSeedInvenIndex, cCount, AttachItems, pCount );
}

void CDnLifeSkillPlantTask::RequestHarvest(const int iAreaIndex, const Farm::ActionType::eType ActionType, const BYTE cNeedItemInvenIndex)
{
	Farm::Send::SendFarmHarvest( iAreaIndex, ActionType, cNeedItemInvenIndex );
}

void CDnLifeSkillPlantTask::RequestAddWater(const int iAreaIndex, const Farm::ActionType::eType ActionType, const BYTE cInvenType, const BYTE cWaterItemInvenIndex, const INT64 biWaterItemItemSerial)
{
	Farm::Send::SendFarmAddWater( iAreaIndex, ActionType, cInvenType, cWaterItemInvenIndex, biWaterItemItemSerial );
}

void CDnLifeSkillPlantTask::RequestWareHouseList()
{
	Farm::Send::SendFarmWareHouseList();
}

void CDnLifeSkillPlantTask::RequestTakeWareHouseItem(INT64 biUniqueID)
{
	Farm::Send::SendFarmTakeWareHouseItem( biUniqueID );
}


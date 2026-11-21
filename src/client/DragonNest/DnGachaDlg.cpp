#include "StdAfx.h"
#include "DnGachaDlg.h"
#include "DnGachaRouletteDlg.h"
#include "DnGachaBuyConfirm.h"
#include "DnGachaResultDlg.h"
#include "DnMainMenuDlg.h"
#include "DnGachaPreview.h"
#include "DnItemTask.h"
#include "DnTableDB.h"
#include <mmsystem.h>
#include "DnInterfaceString.h"
#include "DnInterface.h"
#include "DnLocalPlayerActor.h"
#include "DnNPCActor.h"
#include "DnPartyTask.h"
#include "DnCashShopTask.h"
#include "EtSoundEngine.h"
#include "DnInvenTabDlg.h"
#include "DnNPCTalkReturnDlg.h"


#ifdef PRE_ADD_GACHA_JAPAN


using namespace DN_INTERFACE;
using namespace STRING;


const float GACHA_ROULETTE_SPEED = 10.0f;
const int GACHA_ROULETTE_STOP_SEL = 20;				// Receive result from server and stop at the 10th item.

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


CDnGachaDlg::CDnGachaDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback ) : 
						  CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, true ),
						  m_iGachaIndex( 0 ),
						  m_pGachaPreviewDlg( NULL ),
						  m_pGachaResultDlg( NULL ),
						  m_pGachaBuyConfirm( NULL ),
						  m_pComboPartsSelect( NULL ),
						  m_pBtnOpenCashShop( NULL ),
						  m_pBtnClose( NULL ),
						  m_pStaticCoin( NULL ),
						  m_pBtnBuyGacha( NULL ),
						  m_pNameRouletteDlg( NULL ),
						  m_pStatRouletteDlg( NULL ),
						  m_pStaticItemNameArea( NULL ),
						  m_pStaticItemStatArea( NULL ),
						  m_iSelectedJobBtnID( -1 ),
						  m_iRouletteCount( 0 ),
						  m_bInitialized( false ),
						  m_bProcessRoulette( false ),
						  m_iSelectedPart( -1 ),
						  m_pResultItem( NULL ),
						  m_fStopAccel( 0.0f ),
						  m_iLeftStopSelCount( 0 ),
						  m_fRouletteSpeed( GACHA_ROULETTE_SPEED ),
						  m_fLeftRouletteDistance( 0.0f ),
						  m_fStopTimeStamp( 0.0f ),
						  m_fRouletteStopTimeGap( 0.0f ),
						  m_iResultItemStaticIndex( 0 ),
						  m_fRouletteElapsedTime( 0.0f ),
						  m_iPrevRouletteCount( 0 ),
						  m_eGachaStatus( WAITING ),
						  m_iRouletteChangeSound( -1 ),
						  m_iRouletteResultSound( -1 )
{
	SecureZeroMemory( m_apRadioBtnJob, sizeof(m_apRadioBtnJob) );
}

CDnGachaDlg::~CDnGachaDlg(void)
{
	Reset();

	SAFE_DELETE( m_pGachaPreviewDlg );
	SAFE_DELETE( m_pGachaBuyConfirm );
	SAFE_DELETE( m_pGachaResultDlg );
	SAFE_DELETE( m_pNameRouletteDlg );
	SAFE_DELETE( m_pStatRouletteDlg );
}


void CDnGachaDlg::Initialize( bool bShow )
{
	CDnCustomDlg::Initialize( CEtResourceMng::GetInstance().GetFullName( "Jp_Base.ui" ).c_str(), bShow );


}


void CDnGachaDlg::_InitializeComboBox( void )
{
	// Initialize combo box.
	// In Japan gacha, only cash items are drawn.
	m_pComboPartsSelect->AddItem( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 4798), NULL, -1 );

	//m_pComboPartsSelect->AddItem( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 2221), NULL, 1 ); // Head
	m_pComboPartsSelect->AddItem( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 2222), NULL, CDnParts::CashHelmet );	// Cash helmet index: 0
	m_pComboPartsSelect->AddItem( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 2223), NULL, CDnParts::CashBody );	// Cash upper body index: 1 ...
	m_pComboPartsSelect->AddItem( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 2224), NULL, CDnParts::CashLeg );	// Cash lower body
	m_pComboPartsSelect->AddItem( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 2225), NULL, CDnParts::CashHand );	// Cash gloves
	m_pComboPartsSelect->AddItem( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 2226), NULL, CDnParts::CashFoot ); // Cash shoes
	//m_pComboPartsSelect->AddItem( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 2227), NULL, 7 ); // Cash Necklace
	//m_pComboPartsSelect->AddItem( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 2228), NULL, 8 ); // Cash Earring
	//m_pComboPartsSelect->AddItem( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 2229), NULL, 9 ); // Cash Ring
	//m_pComboPartsSelect->AddItem( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 4798), NULL, 10 ); // Heraldry
	//m_pComboPartsSelect->AddItem( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 4798), NULL, 11 ); // 

	m_pComboPartsSelect->SetSelectedByIndex( 0 );
}


void CDnGachaDlg::InitialUpdate( void )
{
	m_pGachaPreviewDlg = new CDnGachaPreview( UI_TYPE_CHILD, this, PREVIEW_DLG );
	m_pGachaPreviewDlg->Initialize( false );

	m_pComboPartsSelect = GetControl<CEtUIComboBox>( "ID_COMBOBOX_PARTS" );

	char acBuf[ 64 ] = { 0 };
	for( int i = 0; i < GACHA_NUM_JOB_BTN; ++i )
	{
		sprintf_s( acBuf, sizeof(acBuf), "ID_TAB_JOB%d", i );
		m_apRadioBtnJob[ i ] = GetControl<CEtUIRadioButton>( acBuf );
		m_apRadioBtnJob[ i ]->SetButtonID( i );
	}

	m_pStaticItemNameArea = GetControl<CEtUIStatic>( "ID_STATIC_R_AREA1" );
	m_pStaticItemStatArea = GetControl<CEtUIStatic>( "ID_STATIC_R_AREA2" );

	m_pBtnOpenCashShop = GetControl<CEtUIButton>( "ID_BUTTON_BUYCOIN" );

	m_pStaticCoin = GetControl<CEtUIStatic>( "ID_STATIC_COIN" );
	m_pBtnBuyGacha = GetControl<CEtUIButton>( "ID_BUTTON_BUYGACHA" );

	m_pBtnClose = GetControl<CEtUIButton>( "ID_BUTTON_CLOSE" );

	m_pGachaBuyConfirm = new CDnGachaBuyConfirmDlg( UI_TYPE_MODAL, NULL, GACHA_BUY_CONFIRM_DLG, this );
	m_pGachaBuyConfirm->Initialize( false );

	m_pGachaResultDlg = new CDnGachaResultDlg( UI_TYPE_MODAL, NULL, GACHA_RESULT_DLG, this );
	m_pGachaResultDlg->Initialize( false );

	_InitializeComboBox();

	m_bInitialized = true;
}


void CDnGachaDlg::Reset( void )
{
	SAFE_DELETE( m_pResultItem );

	for_each( m_dqItemBuffer.begin(), m_dqItemBuffer.end(), 
			  DeleteData<CDnItem*>() );
	m_dqItemBuffer.clear();

	m_fStopAccel = 0.0f;
	m_fRouletteSpeed = GACHA_ROULETTE_SPEED;
	m_fLeftRouletteDistance = 0.0f;
	m_fStopTimeStamp = 0.0f;
	m_fRouletteStopTimeGap = 0.0f;
	m_fRouletteElapsedTime = 0.0f;
	m_iResultItemStaticIndex = 0;

	m_iPrevRouletteCount = 0;

	m_eGachaStatus = WAITING;

	// Set roulette dialog position to where it should appear.
	m_iRouletteCount = 0;
	if( m_pNameRouletteDlg )
	{
		SUICoord RouletteAreaCoord;
		m_pStaticItemNameArea->GetUICoord( RouletteAreaCoord );
		m_pNameRouletteDlg->SetPosition( RouletteAreaCoord.fX+GetXCoord(), RouletteAreaCoord.fY+GetYCoord() );
	}

	if( m_pStatRouletteDlg )
	{
		SUICoord RouletteAreaCoord;
		m_pStaticItemStatArea->GetUICoord( RouletteAreaCoord );
		m_pStatRouletteDlg->SetPosition( RouletteAreaCoord.fX+GetXCoord(), RouletteAreaCoord.fY+GetYCoord() );
	}

	// Disable buy button.
	//m_pBtnBuyGacha->Enable( false );

	// Reset roulette contents
	m_pNameRouletteDlg->Clear();
	m_pStatRouletteDlg->Clear();
}


void CDnGachaDlg::_GatherGachaItems( void )
{
	DNTableFileFormat*  pItemTable = GetDNTable( CDnTableDB::TITEM );
	
	// Contains the item drop table ID. Item class info is obtained by querying the item table directly.
	DNTableFileFormat*  pGachaTable = GetDNTable( CDnTableDB::TGACHA_JP );

	DNTableFileFormat*  pItemDropTable = GetDNTable( CDnTableDB::TITEMDROP );
	DNTableFileFormat*  pPartsTable = GetDNTable( CDnTableDB::TPARTS );
	char acBuffer[ 256 ] = { 0 };

	// Search through the drop group table and collect all possible droppable items.
	// TODO: Later the server should send the gacha table number, but using index 1 while in development. Handle when NPC is placed.
	//int iGachaIndex = 1;
	int iGachaIndex = m_iGachaIndex;
	vector<int> vlGachaTableIDs;
	pGachaTable->GetItemIDListFromField( "_GachaNum", iGachaIndex, vlGachaTableIDs );

	// TODO: Currently 4 character types, could become 5 when Academic is added. Just need to add to table.
	// Let's fix the assertion.
	_ASSERT( CLASSKINDMAX == vlGachaTableIDs.size() );

	// Within the same gachapon ID, data MUST be entered from top to bottom in job class ID order!!
	int iNumTableIDs = (int)vlGachaTableIDs.size();
	for( int iTableID = 0; iTableID < iNumTableIDs; ++iTableID )
	{
		int iGachaTableID = vlGachaTableIDs.at( iTableID );
		int iNeedJobClassID = iTableID+1;
		for( int iPart = 0; iPart < CDnParts::CashPartsTypeEnum_Amount; ++iPart )
		{
			// Currently only 5 parts are set to roll in gacha. Must update later if expanded.
			// Cannot let a nonexistent label keep returning weird values..
			if( CDnParts::CashFoot < iPart )
				continue;

			sprintf_s( acBuffer, "_LinkedDrop%d", iPart+1 );
			int iLinkedDropTableID = pGachaTable->GetFieldFromLablePtr( iGachaTableID, acBuffer )->GetInteger();
			if( 0 == iLinkedDropTableID || 1 == iLinkedDropTableID )		// Looking up a nonexistent table label returns 1...(??)
				continue;
			
			// TODO: Drop table recursion depth is up to 8 levels.. Handle later if needed to avoid falling into a huge loop.
			vector<int> vlToCalcDropTableIDs;
			vlToCalcDropTableIDs.push_back( iLinkedDropTableID );

			for( int i = 0; i < (int)vlToCalcDropTableIDs.size(); ++i )
			{
				int iDropTableID = vlToCalcDropTableIDs.at( i );
				// Drop table has 20 entries
				for( int iLabel = 0; iLabel <= 20; ++iLabel )
				{
					sprintf_s( acBuffer, "_IsGroup%d", iLabel );
					int iIsGroup = pItemDropTable->GetFieldFromLablePtr( iDropTableID, acBuffer )->GetInteger();
					if( iIsGroup == 1 )
					{
						sprintf_s( acBuffer, "_Item%dIndex", iLabel );
						int iReferenceDropTableID = pItemDropTable->GetFieldFromLablePtr( iDropTableID, acBuffer )->GetInteger();

						if( 0 == iReferenceDropTableID )
							continue;

						vlToCalcDropTableIDs.push_back( iReferenceDropTableID );
					}
					else
					{
						sprintf_s( acBuffer, "_Item%dIndex", iLabel );
						int iItemTableID = pItemDropTable->GetFieldFromLablePtr( iDropTableID, acBuffer )->GetInteger();

						if( 0 == iItemTableID )
							continue;

						//// An item can have multiple required classes. However, items from Japanese gacha always have only one.
						//// If required class is 0, it becomes usable by all classes.
						//int iNeedJobClassID = CDnGachaPreview::GetItemNeedJob( pItemTable, iItemTableID );

						// Get what part type it is. Must match the part type of the current loop iteration.
						int iPartIndexInTable = pPartsTable->GetFieldFromLablePtr( iItemTableID, "_Parts" )->GetInteger();
						//_ASSERT( iPartIndexInTable == iPart );

						//// Insert according to the item's class.
						//// Since required class 0 means all classes can equip it, add it to all classes without duplicates.
						//if( 0 < iNeedJobClassID )
						//{
							m_mapItemsByJob[ iNeedJobClassID ].avlItemsByPart[ iPartIndexInTable ].push_back( iItemTableID );
						//}
						//else
						//{
						//	for( int iClassID = 1; iClassID <= 4; ++iClassID )
						//	{
						//		vector<int>& vlPartsItemIDs = m_mapItemsByJob[ iClassID ].avlItemsByPart[ iPartIndexInTable ];
						//		vector<int>::iterator iter = find( vlPartsItemIDs.begin(), vlPartsItemIDs.end(), iItemTableID );
						//		if( vlPartsItemIDs.end() == iter )
						//			vlPartsItemIDs.push_back( iItemTableID );
						//	}
						//}
					}
				}
			}
		}
	}
}


void CDnGachaDlg::Show( bool bShow )
{
	CDnCustomDlg::Show( bShow );
	
	// Set shop ID in gacha preview dialog
	m_pGachaPreviewDlg->SetGachaponShopID( m_iGachaIndex );
	m_pGachaPreviewDlg->Show( bShow );

	if( bShow )
	{
		Reset();

		m_pComboPartsSelect->Enable( true );
		m_pComboPartsSelect->SetSelectedByIndex( 0 );

		m_iSelectedJobBtnID = CDnActor::s_hLocalActor->GetClassID() - 1;
		m_apRadioBtnJob[ m_iSelectedJobBtnID ]->SetChecked( true );

		for( int i = 0; i < GACHA_NUM_JOB_BTN; ++i )
			m_apRadioBtnJob[ i ]->Enable( true );

		m_pBtnOpenCashShop->Enable( true );
		m_pBtnBuyGacha->Enable( true );

		// Count and display the number of gacha coins the user owns.
		UpdateGachaCoin();

		// Collect all item IDs that can drop from the currently opened Gachapon.
		m_mapItemsByJob.clear();
		//SecureZeroMemory( m_aiDropTableIDs, sizeof(m_aiDropTableIDs) );
		_GatherGachaItems();

		// Load sound
		if( -1 == m_iRouletteChangeSound )
			m_iRouletteChangeSound = CEtSoundEngine::GetInstance().LoadSound( "UI_Gacha_roulette.wav", false, false );

		if( -1 == m_iRouletteResultSound )
			m_iRouletteResultSound = CEtSoundEngine::GetInstance().LoadSound( "UI_Gacha_get.wav", false, false );

		// Switch inventory to the cash tab.
		CDnInvenTabDlg* pInvenDlg = (CDnInvenTabDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::INVENTORY_DIALOG );
		pInvenDlg->ShowTab( ST_INVENTORY_CASH );
	}
	else
	{
		GetItemTask().ResetSendGachaReq();
		m_bProcessRoulette = false;

		// If closed directly with pMainMenu->CloseInvenDialog,
		// On entering village, loading dialogs closes even the quest notifier, so check if open and close it.
		// Since inventory dialog is a child, close it via its parent instead of Show( false ).
		CDnInvenTabDlg *pInvenDlg = (CDnInvenTabDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::INVENTORY_DIALOG );
		if( pInvenDlg && pInvenDlg->IsShow() )
		{
			// Restore inventory back to the first tab.
			pInvenDlg->ShowTab( ST_INVENTORY );
			GetInterface().GetMainMenuDialog()->CloseInvenDialog();
		}

		GetInterface().CloseBlind();
		//GetInterface().CloseNpcTalkReturnDlg();
	}

}


// Item info displayed in gachapon only shows STR, INT, AGI, and VIT.
void CDnGachaDlg::_MakeItemStatString( CDnState* pItemState, wstring& strItemStat )
{
#ifdef PRE_MOD_GACHA_SYSTEM
	if (pItemState)
		CommonUtil::GetItemStatString_Basic(strItemStat, pItemState);
#else
	WCHAR awcBuffer[ 256 ] = { 0 };

	if( pItemState->GetAttackPMin() > 0 || pItemState->GetAttackPMax() > 0 )
	{
		swprintf_s( awcBuffer, 256, L"%s : %d~%d ", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5018 ), pItemState->GetAttackPMin(), pItemState->GetAttackPMax() );
		strItemStat.append( awcBuffer );
	}
	if( pItemState->GetAttackMMin() > 0 || pItemState->GetAttackMMax() > 0 )
	{
		swprintf_s( awcBuffer, 256, L"%s : %d~%d ", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5019 ), pItemState->GetAttackMMin(), pItemState->GetAttackMMax() );
		strItemStat.append( awcBuffer );
	}

	if( pItemState->GetDefenseP() > 0 )
	{
		swprintf_s( awcBuffer, 256, L"%s : %d ", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5020 ), pItemState->GetDefenseP() );
		strItemStat.append( awcBuffer );
	}
	if( pItemState->GetDefenseM() > 0 )
	{
		swprintf_s( awcBuffer, 256, L"%s : %d ", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5062 ), pItemState->GetDefenseM() );
		strItemStat.append( awcBuffer );
	}

	if( pItemState->GetSpirit() > 0 )
	{
		swprintf_s( awcBuffer, 256, L"%s : %d ", ITEM::GET_STATIC( emTOOLTIP_STATIC::ITEM_SPIRIT ), pItemState->GetSpirit() );
		strItemStat.append( awcBuffer );
	}

	int nTemp(0);
	INT64 nTempHP = 0;

	// Increase internal stats
	nTemp = pItemState->GetStiff();
	if( nTemp > 0 )
	{
		swprintf_s( awcBuffer, 256, L"%s : %d ", ITEM::GET_STATIC( emTOOLTIP_STATIC::ITEM_STIFFNESS_P ), nTemp );
		strItemStat.append( awcBuffer );
	}

	nTemp = pItemState->GetStiffResistance();
	if( nTemp > 0 )
	{
		swprintf_s( awcBuffer, 256, L"%s : %d ", ITEM::GET_STATIC( emTOOLTIP_STATIC::ITEM_STIFFNESS_R ), nTemp );
		strItemStat.append( awcBuffer );
	}

	nTemp = pItemState->GetStun();
	if( nTemp > 0 )
	{
		swprintf_s( awcBuffer, 256, L"%s : %d ", ITEM::GET_STATIC( emTOOLTIP_STATIC::ITEM_STUN_P ), nTemp );
		strItemStat.append( awcBuffer );
	}

	nTemp = pItemState->GetStunResistance();
	if( nTemp > 0 )
	{
		swprintf_s( awcBuffer, 256, L"%s : %d ", ITEM::GET_STATIC( emTOOLTIP_STATIC::ITEM_STUN_R ), nTemp );
		strItemStat.append( awcBuffer );
	}

	nTemp = pItemState->GetCritical();
	if( nTemp > 0 )
	{
		swprintf_s( awcBuffer, 256, L"%s : %d ", ITEM::GET_STATIC( emTOOLTIP_STATIC::ITEM_CRITICAL_P ), nTemp );
		strItemStat.append( awcBuffer );
	}

	nTemp = pItemState->GetCriticalResistance();
	if( nTemp > 0 )
	{
		swprintf_s( awcBuffer, 256, L"%s : %d ", ITEM::GET_STATIC( emTOOLTIP_STATIC::ITEM_CRITICAL_R ), nTemp );
		strItemStat.append( awcBuffer );
	}

	nTemp = pItemState->GetMoveSpeed();
	if( nTemp > 0 )
	{
		swprintf_s( awcBuffer, 256, L"%s : +%d ", ITEM::GET_STATIC( emTOOLTIP_STATIC::ITEM_MOVE ), nTemp );
		strItemStat.append( awcBuffer );
	}

	nTempHP = pItemState->GetMaxHP();
	if( nTempHP > 0 )
	{
		swprintf_s( awcBuffer, 256, L"%s +%I64d ", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2254 ), nTempHP );
		strItemStat.append( awcBuffer );
	}

	nTemp = pItemState->GetMaxSP();
	if( nTemp > 0 )
	{
		swprintf_s( awcBuffer, 256, L"%s +%d ", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2255 ), nTemp );
		strItemStat.append( awcBuffer );
	}

	nTemp = pItemState->GetRecoverySP();
	if( nTemp > 0 )
	{
		swprintf_s( awcBuffer, 256, L"%s : %d ", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2256 ), nTemp );
		strItemStat.append( awcBuffer );
	}

	// Super armor, final damage
	nTemp = pItemState->GetSuperAmmor();
	if( nTemp > 0 )
	{
		swprintf_s( awcBuffer, 256, L"%s : %d ", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2279 ), nTemp );
		strItemStat.append( awcBuffer );
	}

	nTemp = pItemState->GetFinalDamage();
	if( nTemp > 0 )
	{
		swprintf_s( awcBuffer, 256, L"%s : %d ", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2280 ), nTemp );
		strItemStat.append( awcBuffer );
	}

	// Item stat display #2. Ratio-related.
	// Attack power / Defense
	if( pItemState->GetAttackPMinRatio() > 0 || pItemState->GetAttackPMaxRatio() > 0 )
	{
		swprintf_s( awcBuffer, 256, L"%s : %.1f%%~%.1f%% ", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5018 ), pItemState->GetAttackPMinRatio() * 100.0f, pItemState->GetAttackPMaxRatio() * 100.0f );
		strItemStat.append( awcBuffer );
	}
	if( pItemState->GetAttackMMinRatio() > 0 || pItemState->GetAttackMMaxRatio() > 0 )
	{
		swprintf_s( awcBuffer, 256, L"%s : %.1f%%~%.1f%% ", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5019 ), pItemState->GetAttackMMinRatio() * 100.0f, pItemState->GetAttackMMaxRatio() * 100.0f );
		strItemStat.append( awcBuffer );
	}

	if( pItemState->GetDefensePRatio() > 0 )
	{
		swprintf_s( awcBuffer, 256, L"%s : %.1f%% ", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5020 ), pItemState->GetDefensePRatio() * 100.0f );
		strItemStat.append( awcBuffer );
	}
	if( pItemState->GetDefenseMRatio() > 0 )
	{
		swprintf_s( awcBuffer, 256, L"%s : %.1f%% ", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5062 ), pItemState->GetDefenseMRatio() * 100.0f );
		strItemStat.append( awcBuffer );
	}

	// Default stat info (max random stat values)
	if( pItemState->GetStrengthRatio() > 0 )
	{
		swprintf_s( awcBuffer, 256, L"%s : %.2f%% ", ITEM::GET_STATIC( emTOOLTIP_STATIC::ITEM_STRENGTH ), pItemState->GetStrengthRatio() * 100.0f );
		strItemStat.append( awcBuffer );
	}
	if( pItemState->GetAgilityRatio() > 0 )
	{
		swprintf_s( awcBuffer, 256, L"%s : %.2f%% ", ITEM::GET_STATIC( emTOOLTIP_STATIC::ITEM_AGILITY ), pItemState->GetAgilityRatio() * 100.0f );
		strItemStat.append( awcBuffer );
	}
	if( pItemState->GetIntelligenceRatio() > 0 )
	{
		swprintf_s( awcBuffer, 256, L"%s : %.2f%% ", ITEM::GET_STATIC( emTOOLTIP_STATIC::ITEM_INTELLIGENCE ), pItemState->GetIntelligenceRatio() * 100.0f );
		strItemStat.append( awcBuffer );
	}
	if( pItemState->GetStaminaRatio() > 0 )
	{
		swprintf_s( awcBuffer, 256, L"%s : %.2f%% ", ITEM::GET_STATIC( emTOOLTIP_STATIC::ITEM_STAMINA ), pItemState->GetStaminaRatio() * 100.0f );
		strItemStat.append( awcBuffer );
	}
	if( pItemState->GetSpiritRatio() > 0 )
	{
		swprintf_s( awcBuffer, 256, L"%s : %.2f%% ", ITEM::GET_STATIC( emTOOLTIP_STATIC::ITEM_SPIRIT ), pItemState->GetSpiritRatio() * 100.0f );
		strItemStat.append( awcBuffer );
	}

	float fTemp(0);

	// Increase internal stats
	fTemp = pItemState->GetStiffRatio();
	if( fTemp > 0 )
	{
		swprintf_s( awcBuffer, 256, L"%s : %.2f%% ", ITEM::GET_STATIC( emTOOLTIP_STATIC::ITEM_STIFFNESS_P ), fTemp * 100.0f );
		strItemStat.append( awcBuffer );
	}

	fTemp = pItemState->GetStiffResistanceRatio();
	if( fTemp > 0 )
	{
		swprintf_s( awcBuffer, 256, L"%s : %.2f%% ", ITEM::GET_STATIC( emTOOLTIP_STATIC::ITEM_STIFFNESS_R ), fTemp * 100.0f );
		strItemStat.append( awcBuffer );
	}

	fTemp = pItemState->GetStunRatio();
	if( fTemp > 0 )
	{
		swprintf_s( awcBuffer, 256, L"%s : %.2f%% ", ITEM::GET_STATIC( emTOOLTIP_STATIC::ITEM_STUN_P ), fTemp * 100.0f );
		strItemStat.append( awcBuffer );
	}

	fTemp = pItemState->GetStunResistanceRatio();
	if( fTemp > 0 )
	{
		swprintf_s( awcBuffer, 256, L"%s : %.2f%% ", ITEM::GET_STATIC( emTOOLTIP_STATIC::ITEM_STUN_R ), fTemp * 100.0f );
		strItemStat.append( awcBuffer );
	}

	fTemp = pItemState->GetCriticalRatio();
	if( fTemp > 0 )
	{
		swprintf_s( awcBuffer, 256, L"%s : %.2f%% ", ITEM::GET_STATIC( emTOOLTIP_STATIC::ITEM_CRITICAL_P ), fTemp * 100.0f );
		strItemStat.append( awcBuffer );
	}

	fTemp = pItemState->GetCriticalResistanceRatio();
	if( fTemp > 0 )
	{
		swprintf_s( awcBuffer, 256, L"%s : %.2f%% ", ITEM::GET_STATIC( emTOOLTIP_STATIC::ITEM_CRITICAL_R ), fTemp * 100.0f );
		strItemStat.append( awcBuffer );
	}


	fTemp = pItemState->GetMoveSpeedRatio();
	if( fTemp > 0 )
	{
		swprintf_s( awcBuffer, 256, L"%s : +%.2f%% ", ITEM::GET_STATIC( emTOOLTIP_STATIC::ITEM_MOVE ), fTemp * 100.0f );
		strItemStat.append( awcBuffer );
	}

	fTemp = pItemState->GetMaxHPRatio();
	if( fTemp > 0 )
	{
		swprintf_s( awcBuffer, 256, L"%s +%.2f%% ", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2254 ), fTemp * 100.0f );
		strItemStat.append( awcBuffer );
	}

	fTemp = pItemState->GetMaxSPRatio();
	if( fTemp > 0 )
	{
		swprintf_s( awcBuffer, 256, L"%s +%.2f%% ", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2255 ), fTemp * 100.0f );
		strItemStat.append( awcBuffer );
	}

	fTemp = pItemState->GetRecoverySPRatio();
	if( fTemp > 0 )
	{
		swprintf_s( awcBuffer, 256, L"%s +%.2f%% ", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2256 ), fTemp * 100.0f );
		strItemStat.append( awcBuffer );
	}

	// Super armor, final damage
	fTemp = pItemState->GetSuperAmmorRatio();
	if( fTemp > 0 )
	{
		swprintf_s( awcBuffer, 256, L"%s : %.2f%% ", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2279 ), fTemp * 100.0f );
		strItemStat.append( awcBuffer );
	}

	fTemp = pItemState->GetFinalDamageRatio();
	if( fTemp > 0 )
	{
		swprintf_s( awcBuffer, 256, L"%s : %.2f%% ", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2280 ), fTemp * 100.0f );
		strItemStat.append( awcBuffer );
	}



	// Only elemental attack/resistance have no separate Ratio, so print them below.
	//
	fTemp = ( pItemState->GetElementAttack( CDnState::Light ) * 100.f );
	if( fTemp > 0 )
	{
		swprintf_s( awcBuffer, 256, L"%s : +%.2f%% ", ITEM::GET_STATIC( emTOOLTIP_STATIC::ITEM_LIGHT_A ), fTemp );
		strItemStat.append( awcBuffer );
	}

	fTemp = ( pItemState->GetElementAttack( CDnState::Dark ) * 100.f );
	if( fTemp > 0 )
	{
		swprintf_s( awcBuffer, 256, L"%s : +%.2f%% ", ITEM::GET_STATIC( emTOOLTIP_STATIC::ITEM_DARK_A ), fTemp );
		strItemStat.append( awcBuffer );
	}

	fTemp = ( pItemState->GetElementAttack( CDnState::Fire ) * 100.f );
	if( fTemp > 0 )
	{
		swprintf_s( awcBuffer, 256, L"%s : +%.2f%% ", ITEM::GET_STATIC( emTOOLTIP_STATIC::ITEM_FIRE_A ), fTemp );
		strItemStat.append( awcBuffer );
	}

	fTemp = ( pItemState->GetElementAttack( CDnState::Ice ) * 100.f );
	if( fTemp > 0 )
	{
		swprintf_s( awcBuffer, 256, L"%s : +%.2f%% ", ITEM::GET_STATIC( emTOOLTIP_STATIC::ITEM_ICE_A ), fTemp );
		strItemStat.append( awcBuffer );
	}


	fTemp = ( pItemState->GetElementDefense( CDnState::Light ) * 100.f );
	if( fTemp > 0 )
	{
		swprintf_s( awcBuffer, 256, L"%s : +%.2f%% ", ITEM::GET_STATIC( emTOOLTIP_STATIC::ITEM_LIGHT_R ), fTemp );
		strItemStat.append( awcBuffer );
	}

	fTemp = ( pItemState->GetElementDefense( CDnState::Dark ) * 100.f );
	if( fTemp > 0 )
	{
		swprintf_s( awcBuffer, 256, L"%s : +%.2f%% ", ITEM::GET_STATIC( emTOOLTIP_STATIC::ITEM_DARK_R ), fTemp );
		strItemStat.append( awcBuffer );
	}

	fTemp = ( pItemState->GetElementDefense( CDnState::Fire ) * 100.f );
	if( fTemp > 0 )
	{
		swprintf_s( awcBuffer, 256, L"%s : +%.2f%% ", ITEM::GET_STATIC( emTOOLTIP_STATIC::ITEM_FIRE_R ), fTemp );
		strItemStat.append( awcBuffer );
	}

	fTemp = ( pItemState->GetElementDefense( CDnState::Ice ) * 100.f );
	if( fTemp > 0 )
	{
		swprintf_s( awcBuffer, 256, L"%s : +%.2f%% ", ITEM::GET_STATIC( emTOOLTIP_STATIC::ITEM_ICE_R ), fTemp );
		strItemStat.append( awcBuffer );
	}
#endif // PRE_MOD_GACHA_SYSTEM
}



void CDnGachaDlg::_UpdateRouletteItemNameInfo( void )
{
	// TODO: Item roulette starting point probably needs to be random....
	DNTableFileFormat*  pItemTable = GetDNTable( CDnTableDB::TITEM );
	_ASSERT( NULL != pItemTable );

	int iJobClassID = m_iSelectedJobBtnID + 1;
	for( int i = 0; i < NUM_GACHA_ROULETTE_ITEM; ++i )
	{
		int iPartItemID = 0;
		int iNowRouletteStaticIndex = m_iRouletteCount + (NUM_GACHA_ROULETTE_ITEM-i-1);

		int iNumGachaItems = (int)m_mapItemsByJob[ iJobClassID ].avlItemsByPart[ m_iSelectedPart ].size();
		if( 0 == iNumGachaItems )
			continue;

		//TCHAR atcBuffer[ 256 ] = { 0 };
		if( m_pResultItem && (STOPPING == m_eGachaStatus || END == m_eGachaStatus) &&
			m_iResultItemStaticIndex == iNowRouletteStaticIndex )
		{
			// Set the item received as the result.
			iPartItemID = m_pResultItem->GetClassID();
			//int iNameID = pItemTable->GetFieldFromLablePtr( iPartItemID, "_NameID" )->GetInteger();
			//_stprintf_s( atcBuffer, _T("%d (Result) %s"), iNowRouletteStaticIndex, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, iNameID ) );
		}
		else
		{
			iPartItemID = m_vlMixedSelectedPartsItems.at( iNowRouletteStaticIndex%iNumGachaItems );
			//iPartItemID = m_mapItemsByJob[ iJobClassID ].avlItemsByPart[ m_iSelectedPart ].at( iNowRouletteStaticIndex%iNumGachaItems );
			//int iNameID = pItemTable->GetFieldFromLablePtr( iPartItemID, "_NameID" )->GetInteger();
			//_stprintf_s( atcBuffer, _T("%d %s"), iNowRouletteStaticIndex, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, iNameID ) );
		}

		//_ASSERT( pItemTable->IsExistItem( iPartItemID ) );
		//if( 0 == iPartItemID )
		//	continue;

		// Item name.
		int iNameID = pItemTable->GetFieldFromLablePtr( iPartItemID, "_NameID" )->GetInteger();
		//m_pNameRouletteDlg->SetStaticText( i, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, iNameID ) );

		// Item grade.
		int iItemRank = pItemTable->GetFieldFromLablePtr( iPartItemID, "_Rank" )->GetInteger();
		m_pNameRouletteDlg->SetItemText( i, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, iNameID ),
											GetEtUIXML().GetUIString( CEtUIXML::idCategory1, ITEM::RANK_2_INDEX((eItemRank)iItemRank) ),
											ITEM::RANK_2_COLOR( (eItemRank)iItemRank ) );

		// Change preview costume based on the item at the center index.
		if( END != m_eGachaStatus && NUM_GACHA_ROULETTE_ITEM/2 == i )
			m_pGachaPreviewDlg->AttachPart( iPartItemID );
	}
}



void CDnGachaDlg::_UpdateRouletteItemStatInfo( void )
{
	DNTableFileFormat*  pItemTable = GetDNTable( CDnTableDB::TITEM );
	_ASSERT( NULL != pItemTable );

	int iJobClassID = m_iSelectedJobBtnID + 1;
	for( int i = 0; i < NUM_GACHA_ROULETTE_ITEM; ++i )
	{
		int iNowRouletteStaticIndex = m_iRouletteCount + (NUM_GACHA_ROULETTE_ITEM-i-1);
		int iNumGachaItems = (int)m_mapItemsByJob[ iJobClassID ].avlItemsByPart[ m_iSelectedPart ].size();
		if( 0 == iNumGachaItems )
			continue;

		int iPartItemID = 0;
		if( m_pResultItem &&(STOPPING == m_eGachaStatus || END == m_eGachaStatus) &&
			m_iResultItemStaticIndex == iNowRouletteStaticIndex )
		{
			iPartItemID = m_pResultItem->GetClassID();
		}
		else
		{
			//iPartItemID = m_mapItemsByJob[ iJobClassID ].avlItemsByPart[ m_iSelectedPart ].at( iNowRouletteStaticIndex%iNumGachaItems );	
			iPartItemID = m_vlMixedSelectedPartsItems.at( iNowRouletteStaticIndex%iNumGachaItems );
		}
		
		_ASSERT( pItemTable->IsExistItem( iPartItemID ) );
		if( 0 == iPartItemID )
			continue;

		if( (int)m_dqItemBuffer.size() < NUM_GACHA_ROULETTE_ITEM )
		{
			SUICoord RouletteDlgCoord;
			m_pNameRouletteDlg->GetDlgCoord( RouletteDlgCoord );
			float fSelHeight = RouletteDlgCoord.fHeight / float(NUM_GACHA_ROULETTE_ITEM);	// Divide the reserved dialog height into 4 parts; 3 slots are actually visible.

			if( (STOPPING == m_eGachaStatus || END == m_eGachaStatus) &&
				m_iResultItemStaticIndex == iNowRouletteStaticIndex )
			{
				m_dqItemBuffer.push_back( m_pResultItem );
			}
			else
			{
				CDnItem* pNewGachaItem = new CDnItem;
				pNewGachaItem->InitializeForInformation( iPartItemID, 1, 1 );
				m_dqItemBuffer.push_back( pNewGachaItem );
			}
		}
		else
		if( iPartItemID != m_dqItemBuffer.at( i )->GetClassID() )
		{
			SAFE_DELETE( m_dqItemBuffer.at( i ) );

			CDnItem* pNewGachaItem = new CDnItem;
			pNewGachaItem->InitializeForInformation( iPartItemID, 1, 1 );
			m_dqItemBuffer.at( i ) = pNewGachaItem;
		}

		CDnItem* pGachaItem = m_dqItemBuffer.at( i );
	
		// Build item stat string.
		wstring strItemStat;
		_MakeItemStatString( pGachaItem, strItemStat );
		//m_pStatRouletteDlg->SetItemText( i, strItemStat.c_str() );
		m_pStatRouletteDlg->SetStatText( i, strItemStat.c_str(), ITEM::RANK_2_COLOR((eItemRank)pGachaItem->GetItemRank()) );
	}
}



void CDnGachaDlg::Process( float fDelta )
{
	CEtUIDialog::Process( fDelta );

	// Since it was created with SELF, Process must be called separately.
	if( m_pNameRouletteDlg )
		m_pNameRouletteDlg->Process( fDelta );

	if( m_pStatRouletteDlg )
		m_pStatRouletteDlg->Process( fDelta );

	// Move the roulette. Rotate item name and stats together for now.
	if( m_bProcessRoulette )
	{
		SUICoord RouletteAreaCoord;
		m_pStaticItemNameArea->GetUICoord( RouletteAreaCoord );
		float fYBoderPos = RouletteAreaCoord.fY + GetYCoord();

		SUICoord RouletteDlgCoord;
		m_pNameRouletteDlg->GetDlgCoord( RouletteDlgCoord );
		float fSelHeight = RouletteDlgCoord.fHeight / float(NUM_GACHA_ROULETTE_ITEM);	// Divide the reserved dialog height into 4 parts; 3 slots are actually visible.
	
		// Accelerated motion... however, the accelerated part uses elapsed time after stopping began, so account for that.
		float fMoveDistance = fSelHeight*m_fRouletteElapsedTime*m_fRouletteSpeed;

		if( STOPPING == m_eGachaStatus )
		{
			float fStopElapsedTime = m_fRouletteElapsedTime - m_fStopTimeStamp;
			if( m_fRouletteStopTimeGap < fStopElapsedTime )
				fStopElapsedTime = m_fRouletteStopTimeGap;

			fMoveDistance += 0.5f*m_fStopAccel*fStopElapsedTime*fStopElapsedTime*fSelHeight;
		}
		else
		if( END == m_eGachaStatus )
		{
			fMoveDistance += 0.5f*m_fStopAccel*m_fRouletteStopTimeGap*m_fRouletteStopTimeGap*fSelHeight;
		}

		m_iRouletteCount = int(fMoveDistance / fSelHeight);

		float fResultYPos = RouletteAreaCoord.fY+GetYCoord() + fMoveDistance-(fSelHeight*(m_iRouletteCount+1));
		if( m_iRouletteCount != m_iPrevRouletteCount )
		{
			// Update item string of roulette dialog.
			_UpdateRouletteItemNameInfo();

			// Play roulette tick sound
			CEtSoundEngine::GetInstance().PlaySound__( "2D", m_iRouletteChangeSound );

			// Stop distance is calculated based on slot count, so decelerate starting from the current position where the slot changes
			if( ROULETTING == m_eGachaStatus )
			{
				if( m_pResultItem )
				{
					m_eGachaStatus = STOPPING;
					m_fStopTimeStamp = m_fRouletteElapsedTime;
					int iNowRouletteStaticIndex = m_iRouletteCount + (NUM_GACHA_ROULETTE_ITEM-(NUM_GACHA_ROULETTE_ITEM/2)-1);
					m_iResultItemStaticIndex = iNowRouletteStaticIndex + GACHA_ROULETTE_STOP_SEL;

					// Calculate the uniform deceleration value to reduce over the travel distance.
					SUICoord RouletteDlgCoord;
					m_pNameRouletteDlg->GetDlgCoord( RouletteDlgCoord );
					float fSelHeight = RouletteDlgCoord.fHeight / float(NUM_GACHA_ROULETTE_ITEM);
					float fNowRouletteSelPos = fMoveDistance - (fSelHeight*m_iRouletteCount);
					float fDestRouletteSelCount = (float)GACHA_ROULETTE_STOP_SEL - (fNowRouletteSelPos/fSelHeight);

					// Simply handle speed by considering the number of roulette slots moved per unit time.
					m_fStopAccel = (0.0f - GACHA_ROULETTE_SPEED*GACHA_ROULETTE_SPEED) / (2.0f*fDestRouletteSelCount);
					m_fRouletteStopTimeGap = (0.0f - GACHA_ROULETTE_SPEED) / m_fStopAccel;
				}
			}
		}

		m_pNameRouletteDlg->SetPosition( RouletteDlgCoord.fX, fResultYPos );
		
		// Return stat items. Strings are already updated; only adjust dialog position here.
		m_pStaticItemStatArea->GetUICoord( RouletteAreaCoord );
		fYBoderPos = RouletteAreaCoord.fY + GetYCoord();
		m_pStatRouletteDlg->GetDlgCoord( RouletteDlgCoord );
		fResultYPos = RouletteAreaCoord.fY+GetYCoord() + fMoveDistance-(fSelHeight*(m_iRouletteCount+1));
		if( m_iRouletteCount != m_iPrevRouletteCount )
		{
			// Update item stat info string of roulette dialog.
			_UpdateRouletteItemStatInfo();
		}

		m_pStatRouletteDlg->SetPosition( RouletteDlgCoord.fX, fResultYPos );

		// If roulette stop time has been reached, stop accumulating time.
		switch( m_eGachaStatus )
		{
			case ROULETTING:
				m_fRouletteElapsedTime += fDelta;
				break;

			case STOPPING:
				if( m_fRouletteElapsedTime - m_fStopTimeStamp < m_fRouletteStopTimeGap )
					m_fRouletteElapsedTime += fDelta;
				else
				{
					m_fRouletteElapsedTime = m_fStopTimeStamp + m_fRouletteStopTimeGap;

					// Roulette stopped.
					GetItemTask().OnStopGachaRoulette();

					// Change costume to the resulting item.
					m_pGachaPreviewDlg->AttachPart( m_pResultItem->GetClassID() );

					// Play result sound.
					CEtSoundEngine::GetInstance().PlaySound__( "2D", m_iRouletteResultSound );

					// Enable gachapon purchase button
					m_pBtnBuyGacha->Enable( true );

					// Re-enable parts selection combo box.
					m_pComboPartsSelect->Enable( true );
	
					// Re-enable Cash Shop open button.
					m_pBtnOpenCashShop->Enable( true );

					m_pBtnClose->Enable( true );

					CDnInvenTabDlg* pInvenDlg = (CDnInvenTabDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::INVENTORY_DIALOG );
					CEtUIButton* pInvenCloseBtn = pInvenDlg->GetControl<CEtUIButton>( "ID_CLOSE_DIALOG" );
					pInvenCloseBtn->Enable( true );

					CDnNpcTalkReturnDlg* pNpcTalkReturnDlg = GetInterface().GetNpcTalkReturnDlg();
					CEtUIButton* pReturnBtn = pNpcTalkReturnDlg->GetControl<CEtUIButton>( "ID_BUTTON_RETURN" );
					pReturnBtn->Enable( true );

					// Re-enable job selection radio button.
					for( int i = 0; i < GACHA_NUM_JOB_BTN; ++i )
						m_apRadioBtnJob[ i ]->Enable( true );

					// Show the received item dialog.
					// Must pass the actual item object; one created only for info will not work.
					CDnItem* pGachaResultItem = GetItemTask().GetCashInventory().FindItemFromSerialID( m_pResultItem->GetSerialID() );
					m_pGachaResultDlg->SetItem( pGachaResultItem );
					m_pGachaResultDlg->Show( true );
					focus::ReleaseControl();

					// Execute NPC result action.
					DWORD nUID = CDnLocalPlayerActor::GetTakeNpcUID();
					DnActorHandle hNpc = CDnActor::FindActorFromUniqueID( nUID );
					if( hNpc )
					{
						hNpc->SetActionQueue( "Puchase_End" );
					}

					m_eGachaStatus = END;
				}
				break;

			case END:
				m_fRouletteElapsedTime = m_fStopTimeStamp + m_fRouletteStopTimeGap;
				break;
		}

		m_iPrevRouletteCount = m_iRouletteCount;
	}
}



void CDnGachaDlg::_RenderRouletteDialog( float fDelta, CEtUIControl* pAreaControl, CDnGachaRouletteDlg* pRouletteDlg )
{
	RECT OriginalRect;
	RECT RouletteRect;
	GetEtDevice()->GetScissorRect( &OriginalRect );

	SUICoord RouletteAreaCoord;
	pAreaControl->GetUICoord( RouletteAreaCoord );

	RouletteRect.left = int((RouletteAreaCoord.fX+GetXCoord()) / GetScreenWidthRatio() * GetEtDevice()->Width());
	RouletteRect.top = int((RouletteAreaCoord.fY+GetYCoord()) / GetScreenHeightRatio() * GetEtDevice()->Height());
	RouletteRect.right = RouletteRect.left + int(RouletteAreaCoord.fWidth / GetScreenWidthRatio() * GetEtDevice()->Width() );
	RouletteRect.bottom = RouletteRect.top + int(RouletteAreaCoord.fHeight / GetScreenHeightRatio() * GetEtDevice()->Height() );

	CEtSprite::GetInstance().Flush();
	GetEtDevice()->SetRenderState( D3DRS_SCISSORTESTENABLE, TRUE );
	GetEtDevice()->SetScissorRect( &RouletteRect );

	pRouletteDlg->Show( true );
	pRouletteDlg->Render( fDelta );
	pRouletteDlg->Show( false );

	CEtSprite::GetInstance().Flush();
	GetEtDevice()->SetScissorRect( &OriginalRect );
	GetEtDevice()->SetRenderState( D3DRS_SCISSORTESTENABLE, FALSE );
}


void CDnGachaDlg::Render( float fDelta )
{
	if( !IsAllowRender() ) return;

	// Since roulette dialog is a child, overlapping with combo looks awkward, and splitting out just the combo box as a separate child is awkward too, so handle it this way.
	int nIndex;
	CEtUIControl *pComboBox = NULL;
	int nVecCtlSize = (int)m_vecControl.size();
	for( nIndex = 0; nIndex < nVecCtlSize; nIndex++ )
	{
		if( !m_vecControl[nIndex] ) continue;
		if( m_vecControl[nIndex]->GetType() == UI_CONTROL_COMBOBOX )
		{
			pComboBox = m_vecControl[nIndex];
			m_vecControl[nIndex] = NULL;
			break;
		}
	}

	CEtUIDialog::Render( fDelta );

	if( IsShow() )
	{
		_RenderRouletteDialog( fDelta, m_pStaticItemNameArea, m_pNameRouletteDlg );
		_RenderRouletteDialog( fDelta, m_pStaticItemStatArea, m_pStatRouletteDlg );
	}

	if( pComboBox )
	{
		pComboBox->Render( fDelta );
		m_vecControl[nIndex] = pComboBox;
	}
}



void CDnGachaDlg::OnChangeResolution() 
{
	CEtUIDialog::OnChangeResolution();

	if( m_pNameRouletteDlg )
		m_pNameRouletteDlg->OnChangeResolution();

	if( m_pStatRouletteDlg )
		m_pStatRouletteDlg->OnChangeResolution();
}



void CDnGachaDlg::_OnSelectPart( void )
{
	if( -1 != m_iSelectedPart )
	{
		// Enable buy button
		//m_pBtnBuyGacha->Enable( true );

		// Copy the parts items to return and shuffle them randomly.
		m_vlMixedSelectedPartsItems = m_mapItemsByJob[ m_iSelectedJobBtnID+1 ].avlItemsByPart[ m_iSelectedPart ];
		random_shuffle( m_vlMixedSelectedPartsItems.begin(), m_vlMixedSelectedPartsItems.end() );

		// Populate roulette contents once.
		_UpdateRouletteItemNameInfo();
		_UpdateRouletteItemStatInfo();
	}
	else
	{
		m_bProcessRoulette = false;
		m_eGachaStatus = WAITING;

		// Disable buy button.
		//m_pBtnBuyGacha->Enable( false );
	}

	if( m_bProcessRoulette )
	{
		// Restore roulette dialog to original position
		if( m_pNameRouletteDlg )
		{
			SUICoord RouletteAreaCoord;
			m_pStaticItemNameArea->GetUICoord( RouletteAreaCoord );
			m_pNameRouletteDlg->SetPosition( RouletteAreaCoord.fX+GetXCoord(), RouletteAreaCoord.fY+GetYCoord() );
		}

		if( m_pStatRouletteDlg )
		{
			SUICoord RouletteAreaCoord;
			m_pStaticItemStatArea->GetUICoord( RouletteAreaCoord );
			m_pStatRouletteDlg->SetPosition( RouletteAreaCoord.fX+GetXCoord(), RouletteAreaCoord.fY+GetYCoord() );
		}

		// Set random seed to display item stats.
		srand( timeGetTime() );
	}
}



void CDnGachaDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg /* = 0 */ )
{
	CDnCustomDlg::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
	SetCmdControlName( pControl->GetControlName() );

	if( false == m_bInitialized )
		return;

	if( nCommand == EVENT_BUTTON_CLICKED ) 
	{
		if( IsCmdControl( "ID_BUTTON_CLOSE" ) )
		{
			Show( false );
		}
		else
		if( IsCmdControl( "ID_BUTTON_BUYCOIN" ) )
		{
			if( CDnCashShopTask::IsActive() )
			{
				Show( false );
				static_cast<CDnLocalPlayerActor*>(CDnActor::s_hLocalActor.GetPointer())->LockInput( false );
				CDnCashShopTask::GetInstance().RequestCashShopOpen();
			}
		}
		else
		if( IsCmdControl( "ID_BUTTON_BUYGACHA" ) )
		{
			// Show prompt message if no part is selected.
			if( -1 == m_iSelectedPart )
			{
				GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4547 ) );
			}
			else
			// Are there no coins?
			if( 0 == GetItemTask().GetCashInventory().GetItemCount( JP_GACHACOIN_ITEM_ID ) )
			{
				GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4548 ) );
			}
			else
			//// Whether the cash inventory is full.
			//if( INVENTORYMAX <= GetItemTask().GetCashInventory().GetInventoryItemList().size() )
			//{
			//	GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4549 ) );
			//}
			//else
			{
				if( m_pResultItem )
					SAFE_DELETE( m_pResultItem );

				// #25546 Taiwan requested shuffling one more time when the gacha button is pressed. ///////////////////////////////////////
				random_shuffle( m_vlMixedSelectedPartsItems.begin(), m_vlMixedSelectedPartsItems.end() );

				// Populate roulette contents once.
				_UpdateRouletteItemNameInfo();
				_UpdateRouletteItemStatInfo();
				////////////////////////////////////////////////////////////////////////////////////////////////

				// Gachapon roulette starts spinning.
				m_bProcessRoulette = true;
				m_eGachaStatus = ROULETTING;

				// Cannot select until result arrives from server.
				if( -1 != m_iSelectedPart )
				{
					pControl->Enable( false );

					for( int i = 0; i < GACHA_NUM_JOB_BTN; ++i )
						m_apRadioBtnJob[ i ]->Enable( false );
				}

				m_pComboPartsSelect->Enable( false );
				m_pBtnOpenCashShop->Enable( false );

				m_pGachaBuyConfirm->Show( true );

				// Disable close button
				m_pBtnClose->Enable( false );

				// Also disable the close button on the open inventory dialog.
				CDnInvenTabDlg* pInvenDlg = (CDnInvenTabDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::INVENTORY_DIALOG );
				CEtUIButton* pInvenCloseBtn = pInvenDlg->GetControl<CEtUIButton>( "ID_CLOSE_DIALOG" );
				pInvenCloseBtn->Enable( false );

				CDnNpcTalkReturnDlg* pNpcTalkReturnDlg = GetInterface().GetNpcTalkReturnDlg();
				CEtUIButton* pReturnBtn = pNpcTalkReturnDlg->GetControl<CEtUIButton>( "ID_BUTTON_RETURN" );
				pReturnBtn->Enable( false );

				// Execute NPC looping action.
				DWORD nUID = CDnLocalPlayerActor::GetTakeNpcUID();
				DnActorHandle hNpc = CDnActor::FindActorFromUniqueID( nUID );
				if( hNpc )
				{
					hNpc->SetActionQueue( "Puchase_Start" );
				}
			}
		}
	}
	else
	if( nCommand == EVENT_RADIOBUTTON_CHANGED )
	{
		if( strstr( pControl->GetControlName(), "ID_TAB_JOB" ) )
		{
			int iButtonID = static_cast<CEtUIRadioButton*>( pControl )->GetButtonID();
			if( iButtonID != m_iSelectedJobBtnID )
			{
				if( m_pResultItem )
					SAFE_DELETE( m_pResultItem );

				m_iSelectedJobBtnID = iButtonID;
				_UpdateJobSelection();
			}
		}
	}
	else
	if( nCommand == EVENT_COMBOBOX_SELECTION_CHANGED )
	{
		if( strstr( pControl->GetControlName(), "ID_COMBOBOX_PARTS" ) )
		{
			Reset();

			// If a part is selected, loop over that part; subtract 1 to align index.
			int iSelectedPart = -1;
			static_cast<CEtUIComboBox*>( pControl )->GetSelectedValue( iSelectedPart );
			m_pGachaPreviewDlg->SelectedPart( iSelectedPart );

			m_iSelectedPart = iSelectedPart;
			_OnSelectPart();
		}
	}

	//if( EVENT_BUTTON_CLICKED == nCommand )
	//{
	//	if( strstr( pControl->GetControlName(), "ID_BUTTON_CLOSE" ) )
	//	{
	//		int iButtonID = static_cast<CEtButto*>( pControl )->GetButtonID();
	//		m_iSelectedJobBtnID = iButtonID;

	//		_UpdateSelectionContentDlg();

	//		if( drag::IsValid() )
	//		{
	//			CDnSlotButton *pDragButton = static_cast<CDnSlotButton*>(drag::GetControl());
	//			if( pDragButton->GetSlotType() == ST_SKILL )
	//				drag::ReleaseControl();
	//		}
	//	}
	//}

}



void CDnGachaDlg::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg /* = 0 */ )
{
	SetCmdControlName( pControl->GetControlName() );

	switch( nID )
	{
		case GACHA_BUY_CONFIRM_DLG:
			if( EVENT_BUTTON_CLICKED == nCommand )
			{
				// CDnGachaBuyConfirmDlg
				if( IsCmdControl( "ID_BUTTON_OK" ) )
				{
					// Buy! Send purchase packet to server. Send only the currently selected parts.
					// The buy button is enabled only while gacha is spinning with a part selected to purchase.
					GetItemTask().RequestGacha_JP( m_iSelectedJobBtnID+1, m_iSelectedPart );
					m_pGachaBuyConfirm->Show( false );
				}
				else
				if( IsCmdControl( "ID_BUTTON_CANCEL" ) ) 
				{
					//m_pComboPartsSelect->SetSelectedByIndex( 0 );
					Reset();
					m_pComboPartsSelect->Enable( true );
					m_pGachaBuyConfirm->Show( false );
					m_pBtnBuyGacha->Enable( true );
					m_pBtnOpenCashShop->Enable( true );
					m_pBtnClose->Enable( true );

					CDnInvenTabDlg* pInvenDlg = (CDnInvenTabDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::INVENTORY_DIALOG );
					CEtUIButton* pInvenCloseBtn = pInvenDlg->GetControl<CEtUIButton>( "ID_CLOSE_DIALOG" );
					pInvenCloseBtn->Enable( true );

					CDnNpcTalkReturnDlg* pNpcTalkReturnDlg = GetInterface().GetNpcTalkReturnDlg();
					CEtUIButton* pReturnBtn = pNpcTalkReturnDlg->GetControl<CEtUIButton>( "ID_BUTTON_RETURN" );
					pReturnBtn->Enable( true );

					for( int i = 0; i < GACHA_NUM_JOB_BTN; ++i )
						m_apRadioBtnJob[ i ]->Enable( true );

					// Return to NPC default action.
					DWORD nUID = CDnLocalPlayerActor::GetTakeNpcUID();
					DnActorHandle hNpc = CDnActor::FindActorFromUniqueID( nUID );
					if( hNpc )
					{
						hNpc->SetActionQueue( "Stand" );
					}
				}
			}
			break;

		case GACHA_RESULT_DLG:
			if( EVENT_BUTTON_CLICKED == nCommand )
			{
				// CDnGachaResultDlg
				if( IsCmdControl( "ID_OK" ) )
				{
					m_pGachaResultDlg->Show( false );
				}
			}
			break;
	}
}


void CDnGachaDlg::_UpdateJobSelection( void )
{
	m_pGachaPreviewDlg->UpdateJobSelection( m_iSelectedJobBtnID+1 );

	// Collect items by job and shuffle them randomly.
	if( -1 != m_iSelectedPart )
	{
		m_vlMixedSelectedPartsItems = m_mapItemsByJob[ m_iSelectedJobBtnID+1 ].avlItemsByPart[ m_iSelectedPart ];
		random_shuffle( m_vlMixedSelectedPartsItems.begin(), m_vlMixedSelectedPartsItems.end() );

		// Populate roulette contents once.
		_UpdateRouletteItemNameInfo();
		_UpdateRouletteItemStatInfo();
	}
}


void CDnGachaDlg::UpdateGachaCoin( void )
{
	// Japan gacha coin item ID is 1107345408.
	// Other countries may change later, so this part should be split by country defines..
	int iNumOwnedGachaCoin = GetItemTask().GetCashInventory().GetItemCount( JP_GACHACOIN_ITEM_ID );

	m_pStaticCoin->SetIntToText( iNumOwnedGachaCoin );

	//// Cannot purchase without at least one gacha coin.
	//if( 0 < iNumOwnedGachaCoin )
	//	m_pBtnBuyGacha->Enable( false );
}


void CDnGachaDlg::OnChangeItem( int iItemID )
{
	//if( 0 == iItemID )
	//	return;

	//DNTableFileFormat*  pItemTable = GetDNTable( CDnTableDB::TITEM );

	//// Only the item name for now..
	//int iNameID = pItemTable->GetFieldFromLablePtr( iItemID, "_NameID" )->GetInteger();
	//m_pLBItemName->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, iNameID ), NULL, 0 );
}


void CDnGachaDlg::OnRecvGachaResult( const SCRefreshCashInven* pPacket )
{
	// Set to stop at the item's name and stats. 
	// This dialog class is responsible for freeing the memory.
	m_pResultItem = new CDnItem;
	if( pPacket->nCount == 1 )
	{
		// The number of cash inventory refreshes arriving at this timing should be 1.
		m_pResultItem->InitializeForInformation( pPacket->ItemList[0].nItemID, 
												 pPacket->ItemList[0].nRandomSeed, 
												 pPacket->ItemList[0].cOption );
		m_pResultItem->SetSerialID( pPacket->ItemList[0].nSerial );
	}

	// Update coin count
	UpdateGachaCoin();

	// Disable buy button while roulette is stopping.
	m_pBtnBuyGacha->Enable( false );

	//// Calculate the uniform deceleration value to reduce over the travel distance.
	//SUICoord RouletteDlgCoord;
	//m_pNameRouletteDlg->GetDlgCoord( RouletteDlgCoord );
	//float fSelHeight = RouletteDlgCoord.fHeight / float(NUM_GACHA_ROULETTE_ITEM); // Divide reserved dialog height into 4 parts. Actually visible is 3 slots.

	//// Simply handle speed by considering the number of roulette slots moved per unit time.
	//m_fStopAccel = (0.0f - GACHA_ROULETTE_SPEED*GACHA_ROULETTE_SPEED) / (2.0f*GACHA_ROULETTE_STOP_SEL);
	//m_fRouletteStopTimeGap = (0.0f - GACHA_ROULETTE_SPEED) / m_fStopAccel;

	//// Update coin count
	//UpdateGachaCoin();
}
#endif // PRE_ADD_GACHA_JAPAN
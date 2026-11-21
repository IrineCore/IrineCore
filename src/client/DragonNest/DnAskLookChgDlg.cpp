#include "StdAfx.h"
#include "DnAskLookChgDlg.h"
#include "DnItemSlotButton.h"
#include "DnItem.h"
#include "DnItemTask.h"
#include "DnMailDlg.h"
#include "DnMainMenuDlg.h"
#include "DnInterface.h"
#include "DnTableDB.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnAskLookChgDlg::CDnAskLookChgDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback  )
	, m_pItem( NULL )
	, m_nSlotIndex( -1 )
	, m_cSlotType( -1 )
{
}

CDnAskLookChgDlg::~CDnAskLookChgDlg(void)
{
	SAFE_DELETE( m_pItem );
	SAFE_RELEASE_SPTR( m_hCommonEffect );
}

void CDnAskLookChgDlg::Initialize( bool bShow )
{
	BaseClass::Initialize( CEtResourceMng::GetInstance().GetFullName( "AskLookChgDlg.ui" ).c_str(), bShow );
}

void CDnAskLookChgDlg::InitialUpdate()
{
	m_SmartMove.SetControl( GetControl<CEtUIButton>("ID_BUTTON_OK") );
}

void CDnAskLookChgDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_BUTTON_OK") )
		{
			CDnMailDlg* pMailDlg = (CDnMailDlg*)GetInterface().GetMainMenuDialog(CDnMainMenuDlg::MAIL_DIALOG);
			if ( pMailDlg && pMailDlg->IsShow() ) {
				pMailDlg->LockDlgs(true);
			}

			CDnPlayerActor *pPlayerActor = (CDnPlayerActor*)((CDnActor*)CDnActor::s_hLocalActor);
			if( pPlayerActor ) 
			{
				// Rotha: Vehicle hair is passed as MoveCashItemType : exception case
				switch(m_pItem->GetItemType())
				{
				case ITEMTYPE_VEHICLEPARTS:	// Item for equip slot = hair.
					{
						DNTableFileFormat* pVehiclePartsTable = GetDNTable( CDnTableDB::TVEHICLEPARTS );
						if( !pVehiclePartsTable || !pVehiclePartsTable->IsExistItem(m_pItem->GetClassID())) 
							return;

						int VehicleClassType = -1;
						VehicleClassType = pVehiclePartsTable->GetFieldFromLablePtr( m_pItem->GetClassID(), "_VehicleClassID" )->GetInteger();

						// This mount cannot equip that item.
						if( (pPlayerActor->GetVehicleClassType() != VehicleClassType) ||  VehicleClassType == -1)
						{
							CDnInterface::GetInstance().ShowCaptionDialog( CDnInterface::typeCaption3, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 9212 ), textcolor::YELLOW, 4.0f );
							return;
						}

						if(pPlayerActor->IsVehicleMode() && pPlayerActor->IsCanVehicleMode() && pPlayerActor->GetMyVehicleActor())
							GetItemTask().RequestMoveItem( MoveType_CashInvenToVehicleParts, m_nSlotIndex, m_pItem->GetSerialID(), Vehicle::Slot::Hair, m_pItem->GetOverlapCount() );
					}
					break;

				case ITEMTYPE_PETCOLOR_BODY:
				case ITEMTYPE_PETCOLOR_TATOO:
					{
						pPlayerActor->UseItemFromSlotIndex( m_nSlotIndex, m_cSlotType );
					}
					break;

				default:
					{
						bool bResult = pPlayerActor->UseItemFromSlotIndex( m_nSlotIndex, m_cSlotType );
						if( bResult ) {
							if(!m_hCommonEffect)
								m_hCommonEffect = CDnInCodeResource::GetInstance().CreatePlayerCommonEffect();	
							if( m_hCommonEffect && CDnActor::s_hLocalActor ) {
								m_hCommonEffect->SetPosition( *CDnActor::s_hLocalActor->GetPosition() );
								m_hCommonEffect->SetActionQueue( "MissionAchieve" );
							}
						}
					}
					break;
				}
			}
			else {
				if ( pMailDlg && pMailDlg->IsShow() )
					pMailDlg->LockDlgs(false);
			}
			Show( false );
		}
		else if( IsCmdControl("ID_BUTTON_CANCEL") )
		{
			Show( false );
		}
		else if( IsCmdControl("ID_BUTTON0") )
		{
			Show( false );
		}
	}
	BaseClass::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnAskLookChgDlg::Show( bool bShow ) 
{
	BaseClass::Show( bShow );

	if( bShow )
	{
		m_SmartMove.MoveCursor();
	}
	else
	{
		m_SmartMove.ReturnCursor();
	}
}

bool CDnAskLookChgDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
	{
		return false;
	}

	return BaseClass::MsgProc( hWnd, uMsg, wParam, lParam );
}

bool CDnAskLookChgDlg::SetItem( CDnItem *pItem )
{
	if( pItem ) {
		int nItemID = pItem->GetClassID();

		DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TITEM );
		if( !pSox ) return false;
		if( !pSox->IsExistItem( nItemID ) ) return false;
		if( !CDnActor::s_hLocalActor ) return false;
		MAPartsBody *pPartsBody = dynamic_cast<MAPartsBody*>(CDnActor::s_hLocalActor.GetPointer());
		if( !pPartsBody ) return false;

		eItemTypeEnum Type = (eItemTypeEnum)pSox->GetFieldFromLablePtr( nItemID, "_Type" )->GetInteger();
		// Handle as error if same color as current
		switch( Type ) {
			case ITEMTYPE_HAIRDYE:
				{
					DWORD dwNewColor = (DWORD)pSox->GetFieldFromLablePtr( nItemID, "_TypeParam1" )->GetInteger();		
					DWORD dwOldColor = pPartsBody->GetPartsColor( MAPartsBody::HairColor );

					if( dwNewColor == dwOldColor ) {
						GetInterface().MessageBox( 114005, MB_OK, 114005 ); 
						return false;
					}
				}
				break;
			case ITEMTYPE_EYEDYE:
				{
					DWORD dwNewColor = (DWORD)pSox->GetFieldFromLablePtr( nItemID, "_TypeParam1" )->GetInteger();		
					DWORD dwOldColor = pPartsBody->GetPartsColor( MAPartsBody::EyeColor );

					if( dwNewColor == dwOldColor ) {
						GetInterface().MessageBox( 114005, MB_OK, 114005 ); 
						return false;
					}
				}
				break;
			case ITEMTYPE_SKINDYE:
				{
					DWORD dwNewColor = (DWORD)pSox->GetFieldFromLablePtr( nItemID, "_TypeParam1" )->GetInteger();		
					DWORD dwOldColor = pPartsBody->GetPartsColor( MAPartsBody::SkinColor );

					if( dwNewColor == dwOldColor ) {
						GetInterface().MessageBox( 114005, MB_OK, 114005 ); 
						return false;
					}
				}
				break;
			case ITEMTYPE_FACIAL:
				{
					DnPartsHandle hParts = pPartsBody->GetParts( CDnParts::Face );
					if( !hParts ) break;

					DWORD dwNewFace = (DWORD)pSox->GetFieldFromLablePtr( nItemID, "_TypeParam1" )->GetInteger();		
					DWORD dwOldFace = hParts->GetClassID();

					if( dwNewFace == dwOldFace ) {
						GetInterface().MessageBox( 114008, MB_OK, 114008 );
						return false;
					}
				}
				break;
			case ITEMTYPE_HAIRDRESS:
				{
					DnPartsHandle hParts = pPartsBody->GetParts( CDnParts::Hair );
					if( !hParts ) break;

					DWORD dwNewHair = (DWORD)pSox->GetFieldFromLablePtr( nItemID, "_TypeParam1" )->GetInteger();		
					DWORD dwOldHair = hParts->GetClassID();

					if( dwNewHair == dwOldHair ) {
						GetInterface().MessageBox( 114010, MB_OK, 114010 ); 
						return false;
					}
				}
				break;

			case ITEMTYPE_VEHICLEHAIRCOLOR:
				{
					CDnPlayerActor *pPlayer = dynamic_cast<CDnPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());
					if( !pPlayer ) break;

					if( !pPlayer->IsCanVehicleMode())
					{
						CDnInterface::GetInstance().ShowCaptionDialog( CDnInterface::typeCaption3, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 9214 ), textcolor::YELLOW, 4.0f );
						break; // Cannot mount here.
					}
					if( !pPlayer->IsVehicleMode()) 
					{
						CDnInterface::GetInstance().ShowCaptionDialog( CDnInterface::typeCaption3, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 9209 ), textcolor::YELLOW, 4.0f );
						break;// Can only be equipped while mounted.
					}

					DWORD dwNewColor = (DWORD)pSox->GetFieldFromLablePtr( nItemID, "_TypeParam1" )->GetInteger();		
					DWORD dwOldColor = pPlayer->GetVehicleInfo().dwPartsColor1;

					if( dwNewColor == dwOldColor ) {
						GetInterface().MessageBox( 114005, MB_OK, 114005 ); 
						return false;
					}
				}
				break;

			case ITEMTYPE_VEHICLEPARTS:
				{
					CDnPlayerActor *pPlayer = dynamic_cast<CDnPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());
					if( !pPlayer ) break;
								
					if( !pPlayer->IsCanVehicleMode())
					{
						CDnInterface::GetInstance().ShowCaptionDialog( CDnInterface::typeCaption3, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 9214 ), textcolor::YELLOW, 4.0f );
						break; // Cannot mount here.
					}
					if( !pPlayer->IsVehicleMode()) 
					{
						CDnInterface::GetInstance().ShowCaptionDialog( CDnInterface::typeCaption3, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 9209 ), textcolor::YELLOW, 4.0f );
						break;// Can only be equipped while mounted.
					}

					int nOldHair = pPlayer->GetVehicleInfo().Vehicle[Vehicle::Slot::Hair].nItemID;

					if( nItemID == nOldHair ) {
						GetInterface().MessageBox( 114010, MB_OK, 114010 ); 
						return false;
					}
				}
		}
		m_nSlotIndex = pItem->GetSlotIndex();
		m_cSlotType = pItem->IsCashItem() ? ITEMPOSITION_CASHINVEN : ITEMPOSITION_INVEN;
		TItemInfo itemInfo;
		memset( &itemInfo, 0, sizeof(TItemInfo));
		itemInfo.cSlotIndex = 0;
		itemInfo.Item.nItemID = nItemID;
		itemInfo.Item.wCount = 1;
		itemInfo.Item.bSoulbound = pItem->IsSoulbBound();
		itemInfo.Item.nSerial = pItem->GetSerialID();

		SAFE_DELETE( m_pItem );
		m_pItem = GetItemTask().CreateItem( itemInfo );

		DN_CONTROL( ItemSlotButton, ID_BUTTON_ITEM )->SetItem( (MIInventoryItem*)m_pItem, CDnSlotButton::ITEM_ORIGINAL_COUNT );

		int nTitleID = 0;
		int nMsgID = 0;
		switch( Type ) {
			case ITEMTYPE_HAIRDYE:
				nTitleID = 114011;
				nMsgID = 114004;
				break;
			case ITEMTYPE_EYEDYE:
				nTitleID = 114051;
				nMsgID = 114052;
				break;
			case ITEMTYPE_SKINDYE:
				nTitleID = 114012;
				nMsgID = 114006;
				break;
			case ITEMTYPE_FACIAL:
				nTitleID = 114013;
				nMsgID = 114007;
				break;
			case ITEMTYPE_HAIRDRESS:
				nTitleID = 114014;
				nMsgID = 114009;
				break;
			case ITEMTYPE_VEHICLEHAIRCOLOR:
				nTitleID = 114011;
				nMsgID = 114004;
				break;
			case ITEMTYPE_VEHICLEPARTS:
				nTitleID = 114014;
				nMsgID = 114009;
				break;
			case ITEMTYPE_PETCOLOR_BODY:
			case ITEMTYPE_PETCOLOR_TATOO:
				nTitleID = 9255;
				nMsgID = 9256;
				break;
		}
		CONTROL( Static, ID_STATIC_TITLE )->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nTitleID) );
		CONTROL( Static, ID_STATIC_MSG )->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nMsgID) );	
	}

	return true;
}

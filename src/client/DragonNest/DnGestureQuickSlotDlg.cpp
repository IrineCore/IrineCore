#include "StdAfx.h"
#include "DnGestureQuickSlotDlg.h"
#include "DnGestureButton.h"
#include "DnActor.h"
#include "DnLocalPlayerActor.h"
#include "DnMainDlg.h"
#include "DnTableDB.h"
#include "DNGestureTask.h"
#include "DnItemTask.h"
#include "TaskManager.h"
#include "DnSkillTask.h"
#include "DnMainMenuDlg.h"
#include "DnCharVehicleDlg.h"
#ifdef PRE_ADD_SECONDARY_SKILL
#include "DnLifeSkillFishingTask.h"
#include "DnLifeSkillCookingTask.h"
#endif // PRE_ADD_SECONDARY_SKILL
#include "DnVehicleTask.h"
#include "DnPetTask.h"
#include "DnCharPetDlg.h"


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CDnLifeSkillQuickSlotDlg::CDnLifeSkillQuickSlotDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
, m_nSkillDragSoundIndex(-1)
, m_nSkillLaydownSoundIndex(-1)
, m_fVehicleCoolTime(0.0f)
, m_nGestureSkillIndex(0)
{
}

CDnLifeSkillQuickSlotDlg::~CDnLifeSkillQuickSlotDlg(void)
{
	if( m_nSkillDragSoundIndex == -1 ) 
	{
		CEtSoundEngine::GetInstance().RemoveSound( m_nSkillDragSoundIndex );
	}
	if( m_nSkillLaydownSoundIndex == -1 ) 
	{
		CEtSoundEngine::GetInstance().RemoveSound( m_nSkillLaydownSoundIndex );
	}
	std::map<CDnLifeSkillButton*, LifeSkillEx>::iterator iter = m_mapMatchedButton.begin();
	LifeSkillEx Info;
	for( ; iter != m_mapMatchedButton.end(); ++iter )
	{
		Info = iter->second;
		SAFE_DELETE( Info.pItem );
	}
}

void CDnLifeSkillQuickSlotDlg::Initialize(bool bShow)
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "GestureQuickSlotDlg.ui" ).c_str(), bShow );

	const char *szFileName = CDnTableDB::GetInstance().GetFileName( 10005 );
	if( strlen( szFileName ) > 0 )
	{
		m_nSkillDragSoundIndex = CEtSoundEngine::GetInstance().LoadSound( CEtResourceMng::GetInstance().GetFullName( szFileName ).c_str(), false, false );
	}

	szFileName = CDnTableDB::GetInstance().GetFileName( 10006 );
	if( strlen( szFileName ) > 0 )
	{
		m_nSkillLaydownSoundIndex = CEtSoundEngine::GetInstance().LoadSound( CEtResourceMng::GetInstance().GetFullName( szFileName ).c_str(), false, false );
	}
}

void CDnLifeSkillQuickSlotDlg::InitCustomControl( CEtUIControl *pControl )
{
	if( pControl->GetProperty() && pControl->GetProperty()->nCustomControlID != idGestureButton )
	{
		if( m_nGestureSkillIndex >= (int)m_vecSlotButton.size() )
			return;

		((CDnSlotButton *)pControl)->SetSlotType( ST_LIFESKILL_QUICKSLOT_EX );
		((CDnSlotButton *)pControl)->SetSlotIndex( (int)m_mapMatchedButton.size() );
		LifeSkillEx LifeSkillExInfo;
		LifeSkillExInfo.pSlotButton = (CDnQuickSlotButton *)pControl;
		LifeSkillExInfo.pItem = NULL;
		m_mapMatchedButton.insert(std::make_pair(m_vecSlotButton[m_nGestureSkillIndex], LifeSkillExInfo));
		m_nGestureSkillIndex++;
		return;
	}

	CDnLifeSkillButton *pQuickSlotButton(NULL);
	pQuickSlotButton = static_cast<CDnLifeSkillButton*>(pControl);

	SUIControlProperty *pProperty(NULL);
	pProperty = pQuickSlotButton->GetProperty();

	pQuickSlotButton->SetSlotType( ST_LIFESKILL_QUICKSLOT );
	pQuickSlotButton->SetSlotIndex( (int)m_vecSlotButton.size() );

	m_vecSlotButton.push_back( pQuickSlotButton );
}

void CDnLifeSkillQuickSlotDlg::RefreshVehicleItem() // For mounts, item check is needed; gesture buttons did not do a validity check, so perform an inventory check.
{
	if(m_mapMatchedButton.empty() || m_vecSlotButton.empty())
		return;

	if( m_mapMatchedButton.size() == m_vecSlotButton.size() )
	{
		for( int i=0; i<(int)m_vecSlotButton.size(); i++ )
		{
			if( m_mapMatchedButton[m_vecSlotButton[i]].pItem && m_mapMatchedButton[m_vecSlotButton[i]].pItem->GetItemType() == ITEMTYPE_VEHICLE )
			{
				if( CDnVehicleTask::IsActive() )
				{
					CDnVehicleTask *pVehicleTask = (CDnVehicleTask *)CTaskManager::GetInstance().GetTask( "VehicleTask" );
					if( pVehicleTask && !pVehicleTask->IsExistVehicle( m_mapMatchedButton[m_vecSlotButton[i]].pItem->GetSerialID() ) ) // Distinguish by serial ID
						ResetSlot( i,true );
				}
			}

			if( m_mapMatchedButton[m_vecSlotButton[i]].pItem && m_mapMatchedButton[m_vecSlotButton[i]].pItem->GetItemType() == ITEMTYPE_PET )
			{
				if( CDnVehicleTask::IsActive() )
				{
					CDnVehicleTask *pVehicleTask = (CDnVehicleTask *)CTaskManager::GetInstance().GetTask( "VehicleTask" );
					if( pVehicleTask && !pVehicleTask->IsExistVehicle( m_mapMatchedButton[m_vecSlotButton[i]].pItem->GetSerialID() ) ) // Distinguish by serial ID
						ResetSlot( i,true );

					TVehicleCompact* pPetCompact = pVehicleTask->GetVehicleInfoFromSerial( m_mapMatchedButton[m_vecSlotButton[i]].pItem->GetSerialID() );
					__time64_t tExpireDate = pPetCompact && m_mapMatchedButton[m_vecSlotButton[i]].pItem->GetExpireDate();
					if( pPetCompact && tExpireDate != pPetCompact->Vehicle[Pet::Slot::Body].tExpireDate )
						m_mapMatchedButton[m_vecSlotButton[i]].pItem->SetExpireDate( pPetCompact->Vehicle[Pet::Slot::Body].tExpireDate );
				}
			}
		}
	}
}

void CDnLifeSkillQuickSlotDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );

	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		switch( uMsg )
		{
		case WM_LBUTTONDOWN:
			{
				if( strstr( pControl->GetControlName(), "ID_QUICKSLOT" ) )
				{
					CDnSlotButton *pDragButton = static_cast<CDnSlotButton*>(drag::GetControl());
					CDnSlotButton *pPressedButton = static_cast<CDnSlotButton*>(pControl);

					if( pDragButton == NULL )
					{
						if( !pPressedButton->GetItemID() )
							return;

						pPressedButton->EnableSplitMode(CDnSlotButton::ITEM_ORIGINAL_COUNT);
						drag::SetControl(pPressedButton);

						// Play a sound even when selecting from the quickslot.
						switch( pPressedButton->GetItemType() )
						{
						case MIInventoryItem::Skill:
							_ASSERT(0&&"Cannot pick up Skill from Radio quickslot.");
							break;
						case MIInventoryItem::Item:
						case MIInventoryItem::Gesture:
						case MIInventoryItem::SecondarySkill:
							if( m_nSkillDragSoundIndex != -1 ) CEtSoundEngine::GetInstance().PlaySound( "2D", m_nSkillDragSoundIndex );
							break;
						}
					}
					else
					{
						if( pDragButton == pPressedButton )
						{
							pDragButton->DisableSplitMode(true);
							drag::ReleaseControl();

							// Play a sound even when moved to the same slot.
							switch( pDragButton->GetItemType() )
							{
							case MIInventoryItem::Skill:
								break;
							case MIInventoryItem::Item:
							case MIInventoryItem::Gesture:
							case MIInventoryItem::SecondarySkill:
								if( m_nSkillDragSoundIndex != -1 ) CEtSoundEngine::GetInstance().PlaySound( "2D", m_nSkillDragSoundIndex );
								break;
							}
						}
						else
						{
							ITEM_SLOT_TYPE soltType = pDragButton->GetSlotType();
							switch( soltType )
							{
							case ST_GESTURE:
							case ST_SECONDARYSKILL:
								{
									MIInventoryItem::InvenItemTypeEnum ItemType = (soltType == ST_GESTURE) ? MIInventoryItem::Gesture : MIInventoryItem::SecondarySkill;
									ResetSlot( FindLifeSkillSlotItem( pDragButton->GetItemID(), ItemType ) );
									SetSlot( pDragButton, pPressedButton );

									if( m_nSkillLaydownSoundIndex != -1 )
										CEtSoundEngine::GetInstance().PlaySound( "2D", m_nSkillLaydownSoundIndex );
								}
								break;
							case ST_INVENTORY_VEHICLE:
								{
									ResetSlot( FindVehicleSlotItem( dynamic_cast<CDnItem*>( pDragButton->GetItem() )->GetSerialID(), MIInventoryItem::Item ) );
									SetSlot( pDragButton, pPressedButton );

									if( m_nSkillLaydownSoundIndex != -1 )
										CEtSoundEngine::GetInstance().PlaySound( "2D", m_nSkillLaydownSoundIndex );
								}
								break;
							case ST_INVENTORY:
							case ST_SKILL:
							case ST_QUICKSLOT:
								break;
							case ST_LIFESKILL_QUICKSLOT:
							case ST_LIFESKILL_QUICKSLOT_EX:
								{
									// Switch quickslot
									ChangeQuickSlotButton( pDragButton, pPressedButton );

									if( m_nSkillLaydownSoundIndex != -1 )
										CEtSoundEngine::GetInstance().PlaySound( "2D", m_nSkillLaydownSoundIndex );
								}
								break;
							}
						}
					}

					return;
				}
			}
			break;
		case WM_RBUTTONUP:
		case WM_KEYDOWN:
			{
				if( !CDnActor::s_hLocalActor ) break;
				if( CDnLocalPlayerActor::IsLockInput() ) break;

				CDnSlotButton *pButton = static_cast<CDnSlotButton*>(pControl);
				if( pButton->GetSlotType() == ST_LIFESKILL_QUICKSLOT )
				{
					// When executed via hotkey, LifeSkillButton with a lower control index is always checked first. Check if a mount slot is registered to handle this.
					if( !pButton->GetItemID() )
					{
						if( m_mapMatchedButton.size() == m_vecSlotButton.size() && m_mapMatchedButton[(CDnLifeSkillButton*)pButton].pSlotButton->IsShow() )
							pButton = m_mapMatchedButton[(CDnLifeSkillButton*)pButton].pSlotButton;
					}
				}
				if( !pButton->GetItemID() ) break;

				if( pButton->GetSlotType() == ST_LIFESKILL_QUICKSLOT )
				{
					switch( pButton->GetItemType() )
					{
					case MIInventoryItem::Gesture:
						GetGestureTask().UseGesture( pButton->GetItemID() );
						break;
#if defined (PRE_ADD_SECONDARY_SKILL)
					case MIInventoryItem::SecondarySkill:
						GetSkillTask().DoLifeSkill( pButton->GetItemID() );
						break;
#endif // PRE_ADD_SECONDARY_SKILL
					}
				}
				else if( pButton->GetSlotType() == ST_LIFESKILL_QUICKSLOT_EX )
				{
					CDnItem *pItem = (CDnItem *)pButton->GetItem();
					CDnPlayerActor *pActor = static_cast<CDnPlayerActor *>(CDnActor::s_hLocalActor.GetPointer());

					if( !pItem || !pActor ) return;
					
					if( pItem->GetItemType() == ITEMTYPE_PET )
					{
						int nCashInvenSlotIndex = -1;
						CDnItem* pPetInvenItem = NULL;
						CDnPetInventory::INVENTORY_MAP_ITER iter = GetItemTask().GetPetInventory().GetInventoryItemList().begin();
						for( ; iter != GetItemTask().GetPetInventory().GetInventoryItemList().end(); ++iter )
						{
							pPetInvenItem = iter->second;
							if( !pPetInvenItem ) continue;
							if( pPetInvenItem->GetSerialID() == pItem->GetSerialID() )
							{
								nCashInvenSlotIndex = iter->first;
								break;
							}
						}

						if( nCashInvenSlotIndex > -1 )
						{
							pItem->SetSlotIndex( nCashInvenSlotIndex );
							GetPetTask().UsePetItem( pItem, true );
						}
						else if( pActor->IsSummonPet() && pItem->GetSerialID() == pActor->GetPetInfo().Vehicle[Pet::Slot::Body].nSerial )	// Unsummon the pet when using an already summoned pet
						{
							if( !GetPetTask().CheckCanMovePet( true ) ) return;
							CDnCharStatusDlg* pCharStatusDlg = (CDnCharStatusDlg *)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::CHARSTATUS_DIALOG );
							if( pCharStatusDlg )
							{
								CDnCharPetDlg* pCharPetDlg = pCharStatusDlg->GetPetDlg();
								if( pCharPetDlg ) pCharPetDlg->PetSummonedOff();
							}
						}

						return;
					}

					if( m_fVehicleCoolTime > 0.0f )
						break;

					if( pActor->IsSwapSingleSkin() )
						break;
					
					if( pActor->IsCanVehicleMode() )
					{
						if( pActor->GetVehicleInfo().Vehicle[Vehicle::Slot::Body].nSerial == pItem->GetSerialID() )
						{
							CDnCharStatusDlg *pCharStatusDlg = (CDnCharStatusDlg *)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::CHARSTATUS_DIALOG );
							CDnCharVehicleDlg *pCharVehicleDlg = pCharStatusDlg->GetVehicleDlg();
							if( pCharVehicleDlg ) pCharVehicleDlg->ForceUnRideVehicle();
						}
						else
						{
							if( pActor->IsCallingVehicle() )
								break;

							if( pActor->GetLevel() < pItem->GetLevelLimit() )
							{
								CDnInterface::GetInstance().ShowCaptionDialog( CDnInterface::typeCaption3, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5070 ), textcolor::YELLOW, 4.0f );
								break;
							}

							int nCashInvenSlotIndex = -1;
							CDnItem *pPetInvenItem(NULL);
							CDnPetInventory::INVENTORY_MAP_ITER iter = GetItemTask().GetPetInventory().GetInventoryItemList().begin();
							for( ; iter != GetItemTask().GetPetInventory().GetInventoryItemList().end(); ++iter )
							{
								pPetInvenItem = iter->second;
								if( !pPetInvenItem ) continue;
								if( pPetInvenItem->GetSerialID() == pItem->GetSerialID() )
								{
									nCashInvenSlotIndex = iter->first;
									break;
								}
							}

#ifdef PRE_ADD_SECONDARY_SKILL
							if( nCashInvenSlotIndex != -1 && !GetItemTask().IsRequestWait() 
								&& !GetLifeSkillFishingTask().IsRequestFishingReady() && !GetLifeSkillFishingTask().IsNowFishing()
								&& !GetLifeSkillCookingTask().IsRequestCooking() && !GetLifeSkillCookingTask().IsNowCooking() )
#else // PRE_ADD_SECONDARY_SKILL
							if( nCashInvenSlotIndex != -1 && !GetItemTask().IsRequestWait() )
#endif // PRE_ADD_SECONDARY_SKILL
								GetItemTask().RequestMoveItem( MoveType_VehicleInvenToVehicleBody, nCashInvenSlotIndex, pItem->GetSerialID(), Vehicle::Slot::Body, 1 );
						}
					}
					else
					{
						CDnInterface::GetInstance().ShowCaptionDialog( CDnInterface::typeCaption3, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 9214 ), textcolor::YELLOW, 4.0f );
					}
					m_fVehicleCoolTime += 1.0f;
				}
			}
			break;
		}
	}
}

void CDnLifeSkillQuickSlotDlg::Process( float fElapsedTime )
{
	CEtUIDialog::Process( fElapsedTime );

	if( m_fVehicleCoolTime > 0.0f )
		m_fVehicleCoolTime -= fElapsedTime;

	if( !drag::IsValid() || !CDnActor::s_hLocalActor )
		return;

	// Check if mouse entered the gesture slot
	bool bMouseEnter = false;
	for( int i=0; i<(int)m_vecSlotButton.size(); i++ )
	{
		if( m_vecSlotButton[i]->IsMouseEnter() )
		{
			bMouseEnter = true;
			break;
		}
	}

	if( !bMouseEnter )
		return;

	// Check if the incoming slot is an item that can be placed in the slot (potions).
	CDnSlotButton* pDragButton = (CDnSlotButton*)drag::GetControl();
	if( pDragButton->GetItemType() != MIInventoryItem::Item )
		return;

	// Normally this case is not hit.
	// Even though the item was deleted via refresh inven after selling to shop, etc.,
	// Entered when the slot button is set in drag::GetControl, etc.
	if( !pDragButton->GetItem() )
	{
		_ASSERT(0&&"GestureQuickSlotDlg::Process. drag::GetConrol has a value, but m_pItem is deleted.");
		return;
	}

	if( !pDragButton->GetItem()->IsQuickSlotItem() )
		return;

	CDnPlayerActor *pActor = static_cast<CDnPlayerActor *>(CDnActor::s_hLocalActor.GetPointer());
	if( pActor->IsBattleMode() )
		return;

	if( !pActor->IsDie() && ( pActor->IsStay() || pActor->IsMove() ) )
	{
		if( pActor->IsCanBattleMode() )
		{
			if( pActor->IsMove() ) pActor->CmdStop( "Stand" );
			pActor->CmdToggleBattle( true );
		}
	}	
}

bool CDnLifeSkillQuickSlotDlg::SetSlot( CDnSlotButton *pDragButton, CDnSlotButton *pPressedButton )
{
	ASSERT(pDragButton&&"CDnGestureQuickSlotDlg::SetSlot, pDragButton is NULL!");
	ASSERT(pPressedButton&&"CDnGestureQuickSlotDlg::SetSlot, pPressedButton is NULL");

	if( pDragButton->GetItemID() == 0 )
		return false;

	int nIndex = pPressedButton->GetSlotIndex();
	ResetSlot( nIndex, false );

	INT64 biID = pDragButton->GetItemID();
	if( pDragButton->GetItemType() == MIInventoryItem::Gesture )
	{
		m_vecSlotButton[nIndex]->SetGestureInfo( pDragButton->GetItemID() );
	}
	else if( pDragButton->GetItemType() == MIInventoryItem::SecondarySkill )
	{
		m_vecSlotButton[nIndex]->SetSecondarySkillInfo( pDragButton->GetItemID() );
	}
	else if( pDragButton->GetItemType() == MIInventoryItem::Item )
	{
		if( m_mapMatchedButton.size() == m_vecSlotButton.size() )
		{
			TItemInfo itemInfo;
			((CDnItem*)pDragButton->GetItem())->GetTItemInfo(itemInfo);
			itemInfo.Item.bEternity = ((CDnItem*)pDragButton->GetItem())->IsEternityItem();
			m_mapMatchedButton[m_vecSlotButton[nIndex]].pItem = GetItemTask().CreateItem(itemInfo);
			m_mapMatchedButton[m_vecSlotButton[nIndex]].pSlotButton->SetQuickItem( m_mapMatchedButton[m_vecSlotButton[nIndex]].pItem );
			m_mapMatchedButton[m_vecSlotButton[nIndex]].pSlotButton->Show( true );
			focus::ReleaseControl();
			biID = ((CDnItem*)pDragButton->GetItem())->GetSerialID();
		}
	}

	pDragButton->DisableSplitMode(false);
	drag::ReleaseControl();

	GetItemTask().RequestAddLifeSkillQuickSlot( pPressedButton->GetSlotIndex(), pDragButton->GetItemType(), biID );

	return true;
}

bool CDnLifeSkillQuickSlotDlg::InitSlot( int nIndex, int nLifeSkillID, MIInventoryItem *pItem, MIInventoryItem::InvenItemTypeEnum ItemType )
{
	if( nIndex >= static_cast<int>( m_vecSlotButton.size() ) )
		return false;

	// When first entering and receiving the list from the server, set it with this function.
	if( ItemType != MIInventoryItem::Item )
	{
		ASSERT(nLifeSkillID&&"CDnLifeSkillQuickSlotDlg::InitSlot");
		if( !nLifeSkillID ) false;
	}
	else
	{
		if( !pItem ) false;
	}

	if( m_vecSlotButton[nIndex]->GetItemID() )
	{
		ASSERT(0&&"Not an empty slot." );
		return false;
	}

	if( ItemType == MIInventoryItem::Gesture )
		m_vecSlotButton[nIndex]->SetGestureInfo( nLifeSkillID );
	else if( ItemType == MIInventoryItem::SecondarySkill )
		m_vecSlotButton[nIndex]->SetSecondarySkillInfo( nLifeSkillID );
	else if( ItemType == MIInventoryItem::Item )
	{
		if( m_mapMatchedButton.size() == m_vecSlotButton.size() )
		{
			if (pItem == NULL)
			{
				ASSERT(0&&"pItem NULL" );
				return false;
			}

			TItemInfo itemInfo;
			((CDnItem*)pItem)->GetTItemInfo(itemInfo);
			itemInfo.Item.bEternity = ((CDnItem*)pItem)->IsEternityItem();
			m_mapMatchedButton[m_vecSlotButton[nIndex]].pItem = GetItemTask().CreateItem(itemInfo);
			m_mapMatchedButton[m_vecSlotButton[nIndex]].pSlotButton->SetQuickItem( m_mapMatchedButton[m_vecSlotButton[nIndex]].pItem );
			m_mapMatchedButton[m_vecSlotButton[nIndex]].pSlotButton->Show( true );
			focus::ReleaseControl();
		}
	}
	return true;
}

bool CDnLifeSkillQuickSlotDlg::ResetSecondarySkillQuickSlot( int nSecondarySkillID )
{
	// Must be done this way because items can be deleted arbitrarily.
	ASSERT(nSecondarySkillID&&"CDnLifeSkillQuickSlotDlg::RefreshSecondarySkillQuickSlot");
	if( !nSecondarySkillID ) false;
	ResetSlot( FindLifeSkillSlotItem( nSecondarySkillID, MIInventoryItem::SecondarySkill ) );
	return true;
}

int CDnLifeSkillQuickSlotDlg::FindLifeSkillSlotItem( int nLifeSkillID, MIInventoryItem::InvenItemTypeEnum ItemType )
{
	ASSERT( nLifeSkillID&&"CDnGestureQuickSlotDlg::FindQuickSlotItem, ID is 0!" );

	for( int i=0; i<(int)m_vecSlotButton.size(); i++ )
	{
		if( m_mapMatchedButton.size() == m_vecSlotButton.size() && m_mapMatchedButton[m_vecSlotButton[i]].pSlotButton->IsShow() && m_mapMatchedButton[m_vecSlotButton[i]].pSlotButton->GetItemID() == nLifeSkillID )
			return i;

		if( !m_vecSlotButton[i]->GetItemID() )
			continue;

		if( m_vecSlotButton[i]->GetItemType() != ItemType )
			continue;

		if( m_vecSlotButton[i]->GetItemID() == nLifeSkillID )
			return i;
	}

	return -1;
}

int CDnLifeSkillQuickSlotDlg::FindVehicleSlotItem( INT64 nItemSerialID, MIInventoryItem::InvenItemTypeEnum ItemType )
{
	for( int i=0; i<(int)m_vecSlotButton.size(); i++ )
	{
		if( m_mapMatchedButton.size() == m_vecSlotButton.size() && m_mapMatchedButton[m_vecSlotButton[i]].pSlotButton->IsShow()
			&& dynamic_cast<CDnItem*>( m_mapMatchedButton[m_vecSlotButton[i]].pSlotButton->GetItem() )->GetSerialID() == nItemSerialID )
			return i;

		if( !m_vecSlotButton[i]->GetItemID() )
			continue;

		if( m_vecSlotButton[i]->GetItemType() != ItemType )
			continue;

		if (dynamic_cast<CDnItem*>( m_vecSlotButton[i]->GetItem() ) == NULL)
			continue;

		if( dynamic_cast<CDnItem*>( m_vecSlotButton[i]->GetItem() )->GetSerialID() == nItemSerialID )
			return i;
	}

	return -1;
}

void CDnLifeSkillQuickSlotDlg::ResetSlot( int nIndex, bool bSendPacket )
{
	if( m_vecSlotButton.empty() ) return;
	if( nIndex < 0 ) return;
	if( nIndex >= (int)m_vecSlotButton.size() ) return;

	if( bSendPacket ) GetItemTask().RequestDelLifeSkillQuickSlot( m_vecSlotButton[nIndex]->GetSlotIndex() );
	if( m_mapMatchedButton.size() == m_vecSlotButton.size() && m_mapMatchedButton[m_vecSlotButton[nIndex]].pSlotButton->IsShow() )
	{
		SAFE_DELETE( m_mapMatchedButton[m_vecSlotButton[nIndex]].pItem );
		m_mapMatchedButton[m_vecSlotButton[nIndex]].pSlotButton->ResetSlot();
		m_mapMatchedButton[m_vecSlotButton[nIndex]].pSlotButton->Show( false );
		focus::ReleaseControl();
	}
	else
	{
		m_vecSlotButton[nIndex]->ResetSlot();
	}
}

void CDnLifeSkillQuickSlotDlg::ChangeQuickSlotButton( CDnSlotButton *pDragButton, CDnSlotButton *pPressedButton )
{
	if( !pDragButton || !pPressedButton )
		return;

	int nDragLifeSkillID = pDragButton->GetItemID();
	int nPressedLifeSkillID = pPressedButton->GetItemID();
	MIInventoryItem *pDragItem = pDragButton->GetItem();
	MIInventoryItem *pPressedItem = pPressedButton->GetItem();
	MIInventoryItem::InvenItemTypeEnum DragItemType = pDragButton->GetItemType();
	MIInventoryItem::InvenItemTypeEnum PressedItemType = pPressedButton->GetItemType();

	if( !nDragLifeSkillID )
		return;

	// If PressedButton is a vehicle quick slot, the internally held DnItem object
	// Will be deleted by DragItem-SetSlot which is called first below.
	// Then while moving Pressed to DragSlotIndex, pPressedItem is already deleted, causing a crash dump.
	// To prevent this, create and pass a temporary object.
	CDnItem *pTempPressedItem = NULL;
	if( pPressedButton->GetSlotType() == ST_LIFESKILL_QUICKSLOT_EX && pPressedItem )
	{
		TItemInfo itemInfo;
		((CDnItem*)pPressedItem)->GetTItemInfo(itemInfo);
		itemInfo.Item.bEternity = ((CDnItem*)pPressedItem)->IsEternityItem();
		pTempPressedItem = GetItemTask().CreateItem(itemInfo);
		pPressedItem = pTempPressedItem;
	}

	int nSlotIndex = pPressedButton->GetSlotIndex();
	SetSlot( nSlotIndex, nDragLifeSkillID, pDragItem, DragItemType );

	if( nPressedLifeSkillID )
	{
		nSlotIndex = pDragButton->GetSlotIndex();
		SetSlot( nSlotIndex, nPressedLifeSkillID, pPressedItem, PressedItemType );
	}

	SAFE_DELETE( pTempPressedItem );

	pDragButton->DisableSplitMode(false);
	pPressedButton->DisableSplitMode(false);
	drag::ReleaseControl();
}

bool CDnLifeSkillQuickSlotDlg::SetSlot( int nIndex, int nLifeSkillID, MIInventoryItem *pItem, MIInventoryItem::InvenItemTypeEnum ItemType )
{
	ASSERT(nLifeSkillID&&"CDnRadioMsgQuickSlotDlg::SetSlot");
	if( !nLifeSkillID ) false;

	TItemInfo itemInfo;
	memset(&itemInfo,0,sizeof(itemInfo));

	if( ItemType == MIInventoryItem::Item && pItem )
	{
		((CDnItem*)pItem)->GetTItemInfo(itemInfo);
		itemInfo.Item.bEternity = ((CDnItem*)pItem)->IsEternityItem();
	}

	if( pItem && ( dynamic_cast<CDnItem*>( pItem )->GetItemType() == ITEMTYPE_VEHICLE || dynamic_cast<CDnItem*>( pItem )->GetItemType() == ITEMTYPE_PET ) )
		ResetSlot( FindVehicleSlotItem( dynamic_cast<CDnItem*>( pItem )->GetSerialID(), ItemType ) );
	else
		ResetSlot( FindLifeSkillSlotItem( nLifeSkillID, ItemType ) );

	GetItemTask().RequestDelLifeSkillQuickSlot( m_vecSlotButton[nIndex]->GetSlotIndex() );
	ResetSlot( nIndex, false );

	INT64 biID = nLifeSkillID;
	if( ItemType == MIInventoryItem::Gesture )
		m_vecSlotButton[nIndex]->SetGestureInfo( nLifeSkillID );
	else if( ItemType == MIInventoryItem::SecondarySkill )
		m_vecSlotButton[nIndex]->SetSecondarySkillInfo( nLifeSkillID );
	else if( ItemType == MIInventoryItem::Item )
	{
		if( m_mapMatchedButton.size() == m_vecSlotButton.size() )
		{
			m_mapMatchedButton[m_vecSlotButton[nIndex]].pItem = GetItemTask().CreateItem(itemInfo);
			m_mapMatchedButton[m_vecSlotButton[nIndex]].pSlotButton->SetQuickItem( m_mapMatchedButton[m_vecSlotButton[nIndex]].pItem );
			m_mapMatchedButton[m_vecSlotButton[nIndex]].pSlotButton->Show( true );
			focus::ReleaseControl();
			biID = itemInfo.Item.nSerial;
		}
	}
	GetItemTask().RequestAddLifeSkillQuickSlot( m_vecSlotButton[nIndex]->GetSlotIndex(), ItemType, biID );

	return true;
}

void CDnLifeSkillQuickSlotDlg::EnableQuickSlot(bool bEnable)
{
	std::vector<CDnLifeSkillButton*>::iterator iter = m_vecSlotButton.begin();
	for (; iter != m_vecSlotButton.end(); ++iter)
	{
		CDnLifeSkillButton* pBtn = (*iter);
		if (pBtn)
			pBtn->Enable(bEnable);
	}
}
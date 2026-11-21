#include "StdAfx.h"
#include "DnQuickSlotDlg.h"
#include "DnQuickSlotButton.h"
#include "DnActor.h"
#include "DnSkill.h"
#include "DnLocalPlayerActor.h"
#include "DnItemTask.h"
#include "DnPlayerActor.h"
#include "DnMainDlg.h"
#include "DnTableDB.h"
#include "DnSkillTask.h"
#include "SmartPtrDef.h"
#include "DnStateBlow.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CDnQuickSlotDlg::CDnQuickSlotDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
	, m_nSkillDragSoundIndex(-1)
	, m_nSkillLaydownSoundIndex(-1)
{
}

CDnQuickSlotDlg::~CDnQuickSlotDlg(void)
{
	if( m_nSkillDragSoundIndex == -1 ) 
	{
		CEtSoundEngine::GetInstance().RemoveSound( m_nSkillDragSoundIndex );
	}
	if( m_nSkillLaydownSoundIndex == -1 ) 
	{
		CEtSoundEngine::GetInstance().RemoveSound( m_nSkillLaydownSoundIndex );
	}
}

void CDnQuickSlotDlg::Initialize(bool bShow)
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "QuickSlotDlg.ui" ).c_str(), bShow );

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

void CDnQuickSlotDlg::InitCustomControl( CEtUIControl *pControl )
{
	CDnQuickSlotButton *pQuickSlotButton(NULL);
	pQuickSlotButton = static_cast<CDnQuickSlotButton*>(pControl);

	SUIControlProperty *pProperty(NULL);
	pProperty = pQuickSlotButton->GetProperty();

	int nOrder;
	nOrder = pProperty->szUIName[ strlen( pProperty->szUIName ) - 1 ] - '1';

	if( nOrder == -1 )
	{
		nOrder = 9;
	}

	pQuickSlotButton->SetButtonOrder( nOrder );
	pQuickSlotButton->SetSlotType( ST_QUICKSLOT );
	pQuickSlotButton->SetTabIndex( GetDialogID() );
	pQuickSlotButton->SetSlotIndex( (int)m_vecSlotButton.size() );

	m_vecSlotButton.push_back( pQuickSlotButton );
}

void CDnQuickSlotDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
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

					int nReqSlotIndex = ((CDnQuickSlotButton*)pPressedButton)->GetItemSlotIndex();
					int nTabIndex = nReqSlotIndex / CDnMainDlg::QUICKSLOT_BUTTON_MAX;
					if(!(nTabIndex < CDnMainDlg::QUICKSLOT_DLG_MAX && nTabIndex >= CDnMainDlg::QUICKSLOT_DLG_00) )
						return;
					
					if( pDragButton == NULL )
					{
						if( pPressedButton->IsEmptySlot() )
							return;

						pPressedButton->EnableSplitMode(0);
						drag::SetControl(pPressedButton);

						// Play a sound even when selecting from the quickslot.
						MIInventoryItem *pItem = pPressedButton->GetItem();
						if( !pItem ) return;

						switch( pPressedButton->GetItemType() )
						{
						case MIInventoryItem::Item:
							CEtSoundEngine::GetInstance().PlaySound( "2D", ((CDnItem*)pItem)->GetDragSoundIndex() );
							break;
						case MIInventoryItem::Skill:
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
							MIInventoryItem *pItem = pDragButton->GetItem();
							if( !pItem ) break;

							switch( pDragButton->GetItemType() )
							{
							case MIInventoryItem::Item:
								CEtSoundEngine::GetInstance().PlaySound( "2D", ((CDnItem*)pItem)->GetDragSoundIndex() );
								break;
							case MIInventoryItem::Skill:
								if( m_nSkillLaydownSoundIndex != -1 ) CEtSoundEngine::GetInstance().PlaySound( "2D", m_nSkillLaydownSoundIndex );
								break;
							}
						}
						else
						{
							ITEM_SLOT_TYPE soltType = pDragButton->GetSlotType();
							switch( soltType )
							{
							case ST_INVENTORY:
								{
									ResetSlot( FindQuickSlotItem( pDragButton->GetItem() ) );
									SetSlot( pDragButton, pPressedButton );
								}
								break;
							case ST_SKILL:
								{
									if( GetSkillTask().IsGuildWarSkill( pDragButton->GetItem()->GetClassID() ) )	// Prevent guild war skills from being registered.
										break;
#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
									if (GetSkillTask().IsTotalLevelSkill(pDragButton->GetItem()->GetClassID()))
										break;
#endif // PRE_ADD_TOTAL_LEVEL_SKILL
									CDnMainDlg *pMainDlg = GetInterface().GetMainBarDialog();
									if (pMainDlg)
									{
										MIInventoryItem *pItem = pDragButton->GetItem();
										if (!pItem) break;

										if (CDnActor::s_hLocalActor)
										{
										}
									}

									ResetSlot( FindQuickSlotItem( pDragButton->GetItem() ) );
									SetSlot( pDragButton, pPressedButton );



									if( m_nSkillLaydownSoundIndex != -1 )
										CEtSoundEngine::GetInstance().PlaySound( "2D", m_nSkillLaydownSoundIndex );
								}
								break;
							case ST_QUICKSLOT:
								{
									if( GetSkillTask().IsGuildWarSkill( pDragButton->GetItem()->GetClassID() ) )	// Prevent guild war skills from being registered.
										break;
#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
									if (GetSkillTask().IsTotalLevelSkill(pDragButton->GetItem()->GetClassID()))
										break;
#endif // PRE_ADD_TOTAL_LEVEL_SKILL

									CDnMainDlg *pMainDlg = GetInterface().GetMainBarDialog();
									if( pMainDlg )
									{
										MIInventoryItem *pItem = pDragButton->GetItem();
										if( !pItem ) break;

										if(CDnActor::s_hLocalActor)
										{
										}
										// Switch quickslot
										pMainDlg->ChangeQuickSlotButton( pDragButton, pPressedButton );
									}
								}
							    break;
							case ST_INVENTORY_CASH:
								{
									ResetSlot( FindQuickSlotItem( pDragButton->GetItem() ) );
									SetSlot( pDragButton, pPressedButton );
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

				CDnQuickSlotButton *pButton = static_cast<CDnQuickSlotButton*>(pControl);
				MIInventoryItem *pItem = pButton->GetItem();
				if( !pItem ) break;

				switch( pButton->GetItemType() )
				{
				case MIInventoryItem::Item:
					{
						char cType = 0;
						cType = ( CDnItem::IsCashItem( pItem->GetClassID() ) ) ? ITEMPOSITION_CASHINVEN : ITEMPOSITION_INVEN;
						CDnPlayerActor *pPlayerActor = (CDnPlayerActor*)((CDnActor*)CDnActor::s_hLocalActor);
						if( pPlayerActor->UseItemFromItemID( pItem->GetClassID(), cType ) )
						{
							// Play sound when response arrives from server.
							//CEtSoundEngine::GetInstance().PlaySound( "2D", ((CDnItem*)pItem)->GetUseSoundIndex() );
						}
					}
					break;
				case MIInventoryItem::Skill:
					{
						DnSkillHandle hSkill = CDnActor::s_hLocalActor->FindSkill(pItem->GetClassID());
						if( !hSkill ) break;
						if( hSkill->GetSkillType() != CDnSkill::Active ) break;
						CDnActor::s_hLocalActor->UseSkill( pItem->GetClassID() );
					}
					break;
				}
			}
			break;
		}
	}
}

void CDnQuickSlotDlg::Process( float fElapsedTime )
{
	CEtUIDialog::Process( fElapsedTime );

	if( !drag::IsValid() || !CDnActor::s_hLocalActor )
		return;

	// Check if mouse entered the gesture slot
	bool bMouseEnter = false;
	for( int i=0; i<(int)m_vecSlotButton.size(); i++ )
	{
		if( m_vecSlotButton[i] && m_vecSlotButton[i]->IsMouseEnter() )
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
		return;

	if( pDragButton->GetSlotType() != ST_INVENTORY_VEHICLE )
		return;

	CDnPlayerActor *pActor = static_cast<CDnPlayerActor *>(CDnActor::s_hLocalActor.GetPointer());
	if( !pActor->IsBattleMode() )
		return;

	if( !pActor->IsDie() && ( pActor->IsStay() || pActor->IsMove() ) )
	{
		if( pActor->IsMove() ) pActor->CmdStop( "Stand" );
		pActor->CmdToggleBattle(false);
	}
}

bool CDnQuickSlotDlg::SetSlot( CDnSlotButton *pDragButton, CDnSlotButton *pPressedButton )
{
	ASSERT(pDragButton&&"CDnQuickSlotDlg::SetSlot, pDragButton is NULL!");
	ASSERT(pPressedButton&&"CDnQuickSlotDlg::SetSlot, pPressedButton is NULL");

	int nReqSlotIndex = ((CDnQuickSlotButton*)pPressedButton)->GetItemSlotIndex();
	int nTabIndex = nReqSlotIndex / CDnMainDlg::QUICKSLOT_BUTTON_MAX;

	if(!(nTabIndex < CDnMainDlg::QUICKSLOT_DLG_MAX && nTabIndex >= CDnMainDlg::QUICKSLOT_DLG_00) )
	{
		drag::Command(UI_DRAG_CMD_CANCEL);
		drag::ReleaseControl();
		return false;
	}

	MIInventoryItem *pInvenItem = pDragButton->GetItem();
	if( !pInvenItem ) return false;

	if( !pInvenItem->IsQuickSlotItem() )
		return false;

	if(nTabIndex < CDnMainDlg::QUICKSLOT_DLG_MAX && nTabIndex >= CDnMainDlg::QUICKSLOT_DLG_00 )
		((CDnQuickSlotButton*)pPressedButton)->SetQuickItem( pInvenItem );

	pDragButton->DisableSplitMode(true);
	drag::ReleaseControl();

	if(nTabIndex < CDnMainDlg::QUICKSLOT_DLG_MAX && nTabIndex >= CDnMainDlg::QUICKSLOT_DLG_00 )
		GetItemTask().RequestAddQuickSlot( nReqSlotIndex , pInvenItem->GetType(), pInvenItem->GetClassID() );

	CDnItem *pItem = dynamic_cast<CDnItem *>(pInvenItem);
	if( pItem ) CEtSoundEngine::GetInstance().PlaySound( "2D", pItem->GetDragSoundIndex() );

	return true;
}

bool CDnQuickSlotDlg::InitSlot( int nIndex, MIInventoryItem *pItem )
{
	ASSERT(pItem&&"CDnQuickSlotDlg::InitSlot");
	if( !pItem ) return false;

	if( !pItem->IsQuickSlotItem() )
		return false;

	if( nIndex < 0 || nIndex >= static_cast<int>( m_vecSlotButton.size() ) )
		return false;

	if( m_vecSlotButton[nIndex] && !m_vecSlotButton[nIndex]->IsEmptySlot() )
	{
		ASSERT(0&&"Not an empty slot." );
		return false;
	}

	m_vecSlotButton[nIndex]->SetQuickItem( pItem );
	return true;
}

bool CDnQuickSlotDlg::SetSlot( int nIndex, MIInventoryItem *pItem )
{
	if( nIndex < 0 || nIndex >= static_cast<int>( m_vecSlotButton.size() ) )
		return false;

	if( m_vecSlotButton[nIndex] == NULL )
		return false;

	int nReqSlotIndex = m_vecSlotButton[nIndex]->GetItemSlotIndex();
	int nTabIndex = nReqSlotIndex / CDnMainDlg::QUICKSLOT_BUTTON_MAX;

	if(!(nTabIndex < CDnMainDlg::QUICKSLOT_DLG_MAX && nTabIndex >= CDnMainDlg::QUICKSLOT_DLG_00))
		return false;

	ASSERT(pItem&&"CDnQuickSlotDlg::SetSlot");
	if( !pItem ) false;

	if( !pItem->IsQuickSlotItem() )
		return false;

	ResetSlot( FindQuickSlotItem( pItem ) );

	GetItemTask().RequestDelQuickSlot( m_vecSlotButton[nIndex]->GetItemSlotIndex() );
	m_vecSlotButton[nIndex]->ResetSlot();
 	m_vecSlotButton[nIndex]->SetQuickItem( pItem );

	if(nTabIndex < CDnMainDlg::QUICKSLOT_DLG_MAX && nTabIndex >= CDnMainDlg::QUICKSLOT_DLG_00 )
		GetItemTask().RequestAddQuickSlot( nReqSlotIndex, pItem->GetType(), pItem->GetClassID() );

	CDnItem *pSetItem = dynamic_cast<CDnItem *>(pItem);
	if( pSetItem ) CEtSoundEngine::GetInstance().PlaySound( "2D", pSetItem->GetDragSoundIndex() );

	return true;
}

bool CDnQuickSlotDlg::SetEventSlot( MIInventoryItem *pItem )
{
	int nEmptyIndex = 0;

	for(int i=0; i<(int)m_vecSlotButton.size() ; i++)
	{
		if( m_vecSlotButton[i] == NULL ) continue;
		if(m_vecSlotButton[i]->GetItem() && pItem && (m_vecSlotButton[i]->GetItem()->GetClassID() == pItem->GetClassID()) )
			return false; // Return if an identical one exists
	}

	for(int i=0; i<(int)m_vecSlotButton.size() ; i++)
	{
		if( m_vecSlotButton[i] == NULL ) continue;
		if(m_vecSlotButton[i]->GetItem() == NULL )
		{
			nEmptyIndex = i; // Insert if there is an empty slot; if full, it cannot enter so it goes into slot 0.
			break;
		}
	}

	ASSERT( pItem && "CDnQuickSlotDlg::SetEventSlot" );
	if( !pItem ) false;
	if( !pItem->IsQuickSlotItem() )
		return false;

	if( m_vecSlotButton[nEmptyIndex] )
	{
		m_vecSlotButton[nEmptyIndex]->ResetSlot();
		m_vecSlotButton[nEmptyIndex]->SetQuickItem( pItem );
	}

	return true;
}


void CDnQuickSlotDlg::ResetAllSlot()
{
	for( int i=0; i<(int)m_vecSlotButton.size(); i++ )
	{
		if( m_vecSlotButton[i] == NULL ) continue;
		m_vecSlotButton[i]->ResetSlot();
	}
}

void CDnQuickSlotDlg::OnRefreshSlot()
{
	if( !CDnActor::s_hLocalActor )
		return;

	int nItemID(0);
	MIInventoryItem *pItem(NULL);

	for( int i=0; i<(int)m_vecSlotButton.size(); i++ )
	{
		if( m_vecSlotButton[i] == NULL ) continue;

		nItemID = m_vecSlotButton[i]->GetItemID();
		if( nItemID == 0 ) continue;

		switch( m_vecSlotButton[i]->GetItemType() )
		{
		case MIInventoryItem::Item:
			if( CDnItem::IsCashItem(nItemID) )
				pItem = GetItemTask().FindItem( nItemID, ITEM_SLOT_TYPE::ST_INVENTORY_CASH );
			else
				pItem = GetItemTask().FindItem( nItemID, ITEM_SLOT_TYPE::ST_INVENTORY );
			break;
		case MIInventoryItem::Skill:

			int nTabIndex =  m_vecSlotButton[i]->GetItemSlotIndex() / CDnMainDlg::QUICKSLOT_BUTTON_MAX;
			int nPageIndex = nTabIndex / DualSkill::Type::MAX;
			pItem = GetSkillTask().FindSkillBySkillPage( nItemID, nPageIndex );

			if(!pItem)
				pItem = CDnActor::s_hLocalActor->FindSkill( nItemID );

			break;
		}

		if( !pItem )
		{
			m_vecSlotButton[i]->ResetSlot();
			m_vecSlotButton[i]->SetItemID( nItemID );
		}
		else
		{
			m_vecSlotButton[i]->SetItem(pItem, CDnSlotButton::ITEM_ORIGINAL_COUNT);
		}
	}
}

int CDnQuickSlotDlg::FindQuickSlotItem( MIInventoryItem *pItem )
{
	ASSERT( pItem&&"CDnQuickSlotDlg::FindQuickSlotItem, pItem is NULL!" );

	for( int i=0; i<(int)m_vecSlotButton.size(); i++ )
	{
		if( m_vecSlotButton[i] == NULL ) continue;
		if( m_vecSlotButton[i]->IsEmptySlot() )
			continue;

		if( pItem->GetType() != m_vecSlotButton[i]->GetItemType() )
			continue;

		if( m_vecSlotButton[i]->GetItemID() == pItem->GetClassID() )
			return i;
	}

	return -1;
}

int CDnQuickSlotDlg::FindEmptyQuickSlotIndex()
{
	for( int i=0; i<(int)m_vecSlotButton.size(); i++ )
	{
		if( m_vecSlotButton[i] == NULL ) continue;
		if( m_vecSlotButton[i]->IsEmptySlot() )
			return m_vecSlotButton[i]->GetItemSlotIndex();
	}
	return -1;
}

int CDnQuickSlotDlg::FindSkillQuickSlotIndex( int nSkillID )
{
	for( int i=0; i<(int)m_vecSlotButton.size(); i++ )
	{
		if( m_vecSlotButton[i] == NULL || m_vecSlotButton[i]->IsEmptySlot() )
			continue;

		CDnSkill * pSkill = (CDnSkill *)(m_vecSlotButton[i]->GetItem());
		if( NULL == pSkill )
			continue;

		if( pSkill->GetClassID() == nSkillID )
			return m_vecSlotButton[i]->GetItemSlotIndex();
	}
	return -1;
}



void CDnQuickSlotDlg::ResetSlot( int nIndex )
{
	if( m_vecSlotButton.empty() ) return;
	if( nIndex < 0 ) return;
	if( nIndex >= (int)m_vecSlotButton.size() ) return;

	int nReqSlotIndex =  m_vecSlotButton[nIndex]->GetItemSlotIndex();
	int nTabIndex = nReqSlotIndex / CDnMainDlg::QUICKSLOT_BUTTON_MAX;
	if(!(nTabIndex < CDnMainDlg::QUICKSLOT_DLG_MAX && nTabIndex >= CDnMainDlg::QUICKSLOT_DLG_00))
		return;

	GetItemTask().RequestDelQuickSlot( m_vecSlotButton[nIndex]->GetItemSlotIndex() );
	m_vecSlotButton[nIndex]->ResetSlot();
}

void CDnQuickSlotDlg::EnableQuickSlot(bool bEnable)
{
	std::vector<CDnQuickSlotButton*>::iterator iter = m_vecSlotButton.begin();
	for( ; iter != m_vecSlotButton.end(); ++iter )
	{
		CDnQuickSlotButton* pBtn = (*iter);
		if( pBtn && pBtn->GetItem() != NULL )
			pBtn->Enable( bEnable );
	}
}

void CDnQuickSlotDlg::GetItemList(std::vector<int> &vecList)
{
	vecList.clear();

	for( DWORD i=0; i<m_vecSlotButton.size(); i++ )
	{
		if( m_vecSlotButton[i] == NULL ) continue;
		vecList.push_back( m_vecSlotButton[i]->GetItemID() );
	}
}
#include "StdAfx.h"
#include "DnPetExpandSkillSlotDlg.h"
#include "DnItem.h"
#include "ClientSessionManager.h"
#include "DnPlayerActor.h"
#include "DnItemTask.h"



#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnPetExpandSkillSlotDlg::CDnPetExpandSkillSlotDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, true )
, m_pItemSlot( NULL )
, m_pButtonOK( NULL )
, m_pExpandSkillSlotItem( NULL )
, m_pExpandSkillSlotItemSlot( NULL )
, m_pStatic(NULL)
{
}

CDnPetExpandSkillSlotDlg::~CDnPetExpandSkillSlotDlg()
{
	SAFE_DELETE( m_pExpandSkillSlotItemSlot );
}

void CDnPetExpandSkillSlotDlg::InitialUpdate()
{
	CEtUIDialog::InitialUpdate();

	m_pItemSlot = GetControl<CDnItemSlotButton>( "ID_ITEM" );
	m_pButtonOK = GetControl<CEtUIButton>( "ID_OK" );
}

void CDnPetExpandSkillSlotDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "CharPetSkillSlotDlg.ui" ).c_str(), bShow );
}

void CDnPetExpandSkillSlotDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		if( m_pExpandSkillSlotItem )
		{
			TItemInfo itemInfo;
			m_pExpandSkillSlotItem->GetTItemInfo( itemInfo );
			m_pExpandSkillSlotItemSlot = GetItemTask().CreateItem( itemInfo );
			m_pItemSlot->SetItem( m_pExpandSkillSlotItemSlot, CDnSlotButton::ITEM_ORIGINAL_COUNT );
		}
	}
	else
	{
		m_pButtonOK->Enable( true );
		m_pItemSlot->ResetSlot();
		SAFE_DELETE( m_pExpandSkillSlotItemSlot );
	}

	CEtUIDialog::Show( bShow );
}

void CDnPetExpandSkillSlotDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl( "ID_OK" ) ) 
		{
			if( !m_pExpandSkillSlotItem ) return;

			CSPetSkillExpand petSkillExpand;
			petSkillExpand.biItemSerial = m_pExpandSkillSlotItem->GetSerialID();
			CClientSessionManager::GetInstance().SendPacket( CS_ITEMGOODS, eItemGoods::CS_PETSKILLEXPAND, (char*)&petSkillExpand, int(sizeof(CSPetSkillExpand)) );

			m_pButtonOK->Enable( false );
			Show( false );
		}
		else if( IsCmdControl( "ID_CANCEL" ) ) 
		{
			Show( false );
			return;
		}
		else if( IsCmdControl( "ID_CLOSE" ) )
		{
			Show( false );
			return;
		}
	}
	CDnCustomDlg::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

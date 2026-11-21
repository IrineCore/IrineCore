#include "StdAfx.h"
#include "DnPetAddSkillDlg.h"
#include "DnItem.h"
#include "ClientSessionManager.h"
#include "DnPlayerActor.h"
#include "DnItemTask.h"



#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnPetAddSkillDlg::CDnPetAddSkillDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, true )
, m_pItemSlot( NULL )
, m_pButtonOK( NULL )
, m_pStatic( NULL )
, m_pSkillItem( NULL )
, m_pSkillItemSlot( NULL )
{
}

CDnPetAddSkillDlg::~CDnPetAddSkillDlg()
{
	SAFE_DELETE( m_pSkillItemSlot );
}

void CDnPetAddSkillDlg::InitialUpdate()
{
	CEtUIDialog::InitialUpdate();

	m_pItemSlot = GetControl<CDnItemSlotButton>( "ID_ITEM" );
	m_pButtonOK = GetControl<CEtUIButton>( "ID_OK" );
	m_pStatic = GetControl<CEtUIStatic>( "ID_TEXT_ASK0" );
}

void CDnPetAddSkillDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "CharPetSkillDlg.ui" ).c_str(), bShow );
}

void CDnPetAddSkillDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		if( m_pSkillItem )
		{
			std::wstring strMessage = FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 9248 ), m_pSkillItem->GetName() );
			m_pStatic->SetText( strMessage );

			TItemInfo itemInfo;
			m_pSkillItem->GetTItemInfo( itemInfo );
			m_pSkillItemSlot = GetItemTask().CreateItem( itemInfo );
			m_pItemSlot->SetItem( m_pSkillItemSlot, CDnSlotButton::ITEM_ORIGINAL_COUNT );
		}
	}
	else
	{
		m_pButtonOK->Enable( true );
		m_pItemSlot->ResetSlot();
		SAFE_DELETE( m_pSkillItemSlot );
	}

	CEtUIDialog::Show( bShow );
}

void CDnPetAddSkillDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl( "ID_OK" ) ) 
		{
			if( !m_pSkillItem ) return;

			CDnPlayerActor* pPlayerActor = dynamic_cast<CDnPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());
			if( !pPlayerActor )
				return;

			TVehicleCompact* pPetCompact = &pPlayerActor->GetPetInfo();
			if( pPetCompact == NULL || pPetCompact->Vehicle[Pet::Slot::Body].nItemID == 0 ) return;

			CSPetSkillItem petSkillItem;
			petSkillItem.biItemSerial = m_pSkillItem->GetSerialID();
			CClientSessionManager::GetInstance().SendPacket( CS_ITEMGOODS, eItemGoods::CS_PETSKILLITEM, (char*)&petSkillItem, int(sizeof(CSPetSkillItem)) );

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

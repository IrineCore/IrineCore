#include "StdAfx.h"
#include "DnSkillBuyDlg.h"
#include "DnSkill.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnSkillBuyDlg::CDnSkillBuyDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallBack )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallBack )
	, m_pSkillSlotButton(NULL)
	, m_pItemSlotButton(NULL)
	, m_pStaticName(NULL)
	, m_pStaticSP(NULL)
	, m_pStaticCoin(NULL)
	, m_pStaticItemCount(NULL)
{
}

CDnSkillBuyDlg::~CDnSkillBuyDlg(void)
{
}

void CDnSkillBuyDlg::Initialize(bool bShow)
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "SkillBuyDlg.ui" ).c_str(), bShow );
}

void CDnSkillBuyDlg::InitialUpdate()
{
	m_pSkillSlotButton = GetControl<CDnSkillSlotButton>("ID_SKILL_ITEM");
	m_pItemSlotButton = GetControl<CDnItemSlotButton>("ID_NEED_ITEM");
	m_pStaticName = GetControl<CEtUIStatic>("ID_SKILL_NAME");
	m_pStaticSP = GetControl<CEtUIStatic>("ID_STATIC_SP");
	m_pStaticCoin = GetControl<CEtUIStatic>("ID_STATIC_COIN");
	m_pStaticItemCount = GetControl<CEtUIStatic>("ID_ITEM_COUNT");

	m_SmartMove.SetControl( GetControl<CEtUIButton>("ID_OK") );
}

void CDnSkillBuyDlg::SetSkillInfo( MIInventoryItem *pSkill, MIInventoryItem *pNeedItem, const wchar_t *wszPrice )
{
	ASSERT( pSkill&&"CDnSkillLevelUpDlg::SetSkill, pSkill is NULL!" );

	m_pSkillSlotButton->SetItem(pSkill, CDnSlotButton::NO_COUNTABLE_RENDER);
	m_pStaticName->SetText( ((CDnSkill*)pSkill)->GetName() );
	m_pStaticCoin->SetText( wszPrice );

	wchar_t szTemp[64] = {0};

	if( pNeedItem )
	{
		m_pItemSlotButton->SetItem(pNeedItem, CDnSlotButton::ITEM_ORIGINAL_COUNT);

		swprintf_s( szTemp, _countof(szTemp), L"X%d", ((CDnSkill*)pSkill)->GetNeedItemDecreaseCount() );
		m_pStaticItemCount->SetText( szTemp );
	}

	// CDnSkill::GetBuySkillPoint() function removed. 2009.12.21 -Hanki
	//swprintf_s( szTemp, _countof(szTemp), L"SP %d", ((CDnSkill*)pSkill)->GetBuySkillPoint() ); 
	//m_pStaticSP->SetText( szTemp );
}

void CDnSkillBuyDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		m_SmartMove.MoveCursor();
	}
	else
	{
		m_SmartMove.ReturnCursor();

		m_pSkillSlotButton->ResetSlot();
		m_pItemSlotButton->ResetSlot();
		m_pStaticName->ClearText();
		m_pStaticSP->ClearText();
		m_pStaticCoin->ClearText();
		m_pStaticItemCount->ClearText();
	}

	CEtUIDialog::Show( bShow );
}
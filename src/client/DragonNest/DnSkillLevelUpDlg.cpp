#include "StdAfx.h"
#include "DnSkillLevelUpDlg.h"
#include "DnSkill.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CDnSkillLevelUpDlg::CDnSkillLevelUpDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallBack )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallBack )
	, m_pSkillSlotButton(NULL)
	, m_pStaticSkillName(NULL)
	, m_pStaticSkillSP(NULL)
{
}

CDnSkillLevelUpDlg::~CDnSkillLevelUpDlg(void)
{
}

void CDnSkillLevelUpDlg::Initialize(bool bShow)
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "SkillLevelUpDlg.ui" ).c_str(), bShow );
}

void CDnSkillLevelUpDlg::InitialUpdate()
{
	m_pStaticSkillName = GetControl<CEtUIStatic>("ID_SKILL_NAME");
	m_pStaticSkillSP = GetControl<CEtUIStatic>("ID_STATIC_SP");

	m_SmartMove.SetControl( GetControl<CEtUIButton>("ID_OK") );
}

void CDnSkillLevelUpDlg::InitCustomControl( CEtUIControl *pControl )
{
	m_pSkillSlotButton = static_cast<CDnSkillSlotButton*>(pControl);
}

void CDnSkillLevelUpDlg::SetSkill( MIInventoryItem *pSkill )
{
	ASSERT( pSkill&&"CDnSkillLevelUpDlg::SetSkill, pSkill is NULL!" );
	m_pSkillSlotButton->SetItem(pSkill, CDnSlotButton::NO_COUNTABLE_RENDER);

	m_pStaticSkillName->SetText( ((CDnSkill*)pSkill)->GetName() );

	// Related to issue #15792. Entire string omitted due to Chinese version word-order issue.
	wchar_t szTemp[64] = {0};
	//swprintf_s( szTemp, _countof(szTemp), L"SP %d %s", ((CDnSkill*)pSkill)->GetNextLevelSkillPoint(), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1812 ) );
	swprintf_s( szTemp, _countof(szTemp), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 731 ), ((CDnSkill*)pSkill)->GetNextLevelSkillPoint() );
	m_pStaticSkillSP->SetText( szTemp );
}

void CDnSkillLevelUpDlg::Show( bool bShow ) 
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
		m_pStaticSkillName->ClearText();
		m_pStaticSkillSP->ClearText();
	}

	CEtUIDialog::Show( bShow );
}
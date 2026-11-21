#include "stdafx.h"
#include "DnSkillGuideInitItemDlg.h"
#include "DnTableDB.h"

CDnSkillGuideInitItemDlg::CDnSkillGuideInitItemDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallBack )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallBack )
, m_pCommentStatic( NULL )
{

}

CDnSkillGuideInitItemDlg::~CDnSkillGuideInitItemDlg(void)
{

}

void CDnSkillGuideInitItemDlg::Initialize(bool bShow)
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "SkillInititemGuide.ui" ).c_str(), bShow );

	SetElementDialog(true);
}

void CDnSkillGuideInitItemDlg::InitialUpdate()
{
	m_pCommentStatic = GetControl<CEtUIStatic>( "ID_STATIC_SKILLINITGUIDE" );

	SetResetGuideComment();
}

void CDnSkillGuideInitItemDlg::SetResetGuideComment()
{
	WCHAR wszComment[512] = {0,};
	const int nResetComment_Level = (int)CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::Skill_ResetComment_Level );
	swprintf_s( wszComment, _countof(wszComment), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1820 ), nResetComment_Level );	// UISTRING : At level %d, a skill reset item is granted. Feel free to level up and try out skills.

	m_pCommentStatic->SetText( wszComment );
}

void CDnSkillGuideInitItemDlg::SetItemSkillComment()
{
	m_pCommentStatic->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1833 ) );	// UISTRING : You must unequip skill level-up gear and raise the skill to max level before equipping it to exceed the max level.
}
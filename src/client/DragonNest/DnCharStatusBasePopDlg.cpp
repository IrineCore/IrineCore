#include "StdAfx.h"
#include "DnCharStatusBasePopDlg.h"
#include "DnCharStatusBaseInfoDlg.h"
#include "GameOption.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

#ifdef PRE_ADD_CHAR_SATUS_RENEW

CDnCharStatusBasePopDlg::CDnCharStatusBasePopDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback, true )
{
	for(int n=0; n<CDnCharStatusBaseInfoDlg::ePageType::MAX; n++)
		m_pButton[n] = NULL;

	m_nSelectedSection = CDnCharStatusBaseInfoDlg::eSection::PRIMARY;
}

CDnCharStatusBasePopDlg::~CDnCharStatusBasePopDlg(void)
{
}

void CDnCharStatusBasePopDlg::Initialize(bool bShow)
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "CharStatusBaseInfoOptionDlg.ui" ).c_str(), bShow );
}

void CDnCharStatusBasePopDlg::InitialUpdate()
{
	int arrNameString[CDnCharStatusBaseInfoDlg::ePageType::MAX] = 	{ 
		1000037816 , 1360,	1000037817 , 1000037818 , 1000037819 , 1000037820 , 1000037821 ,  // Stats / Normal / Special Attack / Special Defense / Elemental Attack / Elemental Defense / Misc
	};

	std::string strUIName;
	for(int n=0; n<CDnCharStatusBaseInfoDlg::ePageType::MAX; n++)
	{
		strUIName = FormatA("ID_BT_OPTION%d", n );
		m_pButton[n] = GetControl<CEtUIButton>(strUIName.c_str());
		m_pButton[n]->SetText( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, arrNameString[n]) );
	}
}


void CDnCharStatusBasePopDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	CEtUIDialog::Show( bShow );
}

void CDnCharStatusBasePopDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	CDnCharStatusBaseInfoDlg *pParentDlg = (CDnCharStatusBaseInfoDlg*)GetParentDialog();
	if(!pParentDlg)
		return;

	SetCmdControlName( pControl->GetControlName() );
	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		for(int nPageType = 0; nPageType < CDnCharStatusBaseInfoDlg::ePageType::MAX; nPageType++ )
		{
			if( IsCmdControl( FormatA("ID_BT_OPTION%d", nPageType ).c_str() ) )
			{
				pParentDlg->EnableSatusPage( m_nSelectedSection , nPageType );

				switch(m_nSelectedSection)
				{
				case CDnCharStatusBaseInfoDlg::eSection::PRIMARY:
					CGameOption::GetInstance().m_nCharStatusPrimaryPage = nPageType;
					break;
				case CDnCharStatusBaseInfoDlg::eSection::SECONDARY:
					CGameOption::GetInstance().m_nCharStatusSecondaryPage = nPageType;
					break;
				}

				CGameOption::GetInstance().Save();
			}
		}

		Show(false);
	}
}

#endif
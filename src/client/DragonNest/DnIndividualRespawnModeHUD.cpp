#include "StdAfx.h"
#include "DnIndividualRespawnModeHUD.h"
#include "DnPVPGameResultAllDlg.h" // Retrieve the BestUser ID from the score.
#include "DnInterface.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnIndividualRespawnModeHUD::CDnIndividualRespawnModeHUD( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnPVPBaseHUD( dialogType, pParentDialog, nID, pCallback )
, m_pBestUser( NULL )
, m_pBestUserCount( NULL )
, m_pBestUserName( NULL )
{
}

CDnIndividualRespawnModeHUD::~CDnIndividualRespawnModeHUD(void)
{

}

void CDnIndividualRespawnModeHUD::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "PvpRespawnInGame_AllDlg.ui" ).c_str(), bShow );
}


void CDnIndividualRespawnModeHUD::Process( float fElapsedTime )
{
	CDnPVPBaseHUD::Process(fElapsedTime);
	ProcessBestUserName();
}


void CDnIndividualRespawnModeHUD::ProcessBestUserName()
{
	if( !GetInterface().GetPVPGameResultAllDlg() ) return;
	int nBestUserScore = GetInterface().GetPVPGameResultAllDlg()->GetBestUserScore();
	std::wstring wstBestUserName = GetInterface().GetPVPGameResultAllDlg()->GetBestUserName();

	if(m_pBestUser)
	{
		if(nBestUserScore>0) // Set after getting at least 1 kill
		{
			m_pBestUser->Show(true);
			m_pBestUserName->SetText(wstBestUserName.c_str());
			m_pBestUserName->Show(true);
			m_pBestUserCount->SetIntToText(nBestUserScore);
			m_pBestUserCount->Show(true);
		}
		else if(m_pBestUser->IsShow() || m_pBestUserName->IsShow() || m_pBestUserCount->IsShow())
		{
			m_pBestUser->Show(false);
			m_pBestUserName->Show(false);
			m_pBestUserCount->Show(false);
		}
	}

	return;
}



void CDnIndividualRespawnModeHUD::InitialUpdate()
{
	CDnPVPBaseHUD::InitialUpdate();

	m_pRoomName = GetControl<CEtUIStatic>("ID_ROOMNAME"); //  SetRoomName is in BaseHUD.
	m_pBestUser = GetControl<CEtUIStatic>("ID_BESTUSER");
	m_pBestUserName = GetControl<CEtUIStatic>("ID_NAME");
	m_pBestUserCount = GetControl<CEtUIStatic>("ID_KILLCOUNT");
	m_pBestUser->Show(false);
	m_pBestUserName->Show(false);
	m_pBestUserCount->Show(false);

}

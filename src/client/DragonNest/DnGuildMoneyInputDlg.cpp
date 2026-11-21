#include "StdAfx.h"
#include "DnGuildMoneyInputDlg.h"
#include "DnGuildTask.h"
#include "DnInterfaceString.h"
#include "DnItemTask.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnGuildMoneyInputDlg::CDnGuildMoneyInputDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnMoneyInputDlg( dialogType, pParentDialog, nID, pCallback )
, m_pStaticLimitBack(NULL)
, m_pStaticLimitBronze(NULL)
, m_pStaticLimitGold(NULL)
, m_pStaticLimitMaster(NULL)
, m_pStaticLimitSilver(NULL)
{
}

CDnGuildMoneyInputDlg::~CDnGuildMoneyInputDlg(void)
{
}

void CDnGuildMoneyInputDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "GuildStorageMoneyInputDlg.ui" ).c_str(), bShow );
}

void CDnGuildMoneyInputDlg::InitialUpdate()
{
	CDnMoneyInputDlg::InitialUpdate();

	m_pStaticLimitGold = GetControl<CEtUIStatic>("ID_GOLD");
	m_pStaticLimitSilver = GetControl<CEtUIStatic>("ID_SILVER");
	m_pStaticLimitBronze = GetControl<CEtUIStatic>("ID_BRONZE");
	m_pStaticLimitBack = GetControl<CEtUIStatic>("ID_STATIC_BACK");
	m_pStaticLimitMaster = GetControl<CEtUIStatic>("ID_STATIC_MASTER");
}

void CDnGuildMoneyInputDlg::Process( float fElapsedTime )
{
	CDnMoneyInputDlg::Process( fElapsedTime );

	if( IsShow() )
	{
		// Refresh storage withdrawal count. (Must be handled in Process so changes to rank or per-rank count are reflected immediately.)
		if( CDnGuildTask::IsActive() && GetGuildTask().GetGuildInfo() && GetGuildTask().GetGuildInfo()->IsSet() )
		{
#ifdef PRE_ADD_CHANGEGUILDROLE
			if( static_cast<eGuildRoleType>(GetGuildTask().GetMyRole()) <= GUILDROLE_TYPE_SUBMASTER )
#else
			if( GetGuildTask().IsMaster() )
#endif
			{
				m_pStaticLimitGold->Show( false );
				m_pStaticLimitSilver->Show( false );
				m_pStaticLimitBronze->Show( false );
				m_pStaticLimitBack->Show( false );
				m_pStaticLimitMaster->Show( true );
				m_pStaticLimitMaster->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, ( GetGuildTask().IsMaster() ? 3810 : 3842 ) ) );
			}
			else
			{
				m_pStaticLimitGold->Show( true );
				m_pStaticLimitSilver->Show( true );
				m_pStaticLimitBronze->Show( true );
				m_pStaticLimitBack->Show( true );
				m_pStaticLimitMaster->Show( false );

				INT64 biCoin = 0;

				int nMaxWithDraw = GetGuildTask().GetMaxWithDrawByRole( static_cast<eGuildRoleType>(GetGuildTask().GetMyRole()) );
				INT64 biMaxWithDraw = (INT64)nMaxWithDraw;
				biCoin = biMaxWithDraw - GetItemTask().GetWithdrawGuildWareCoin();
				if( biCoin <= 0 ) biCoin = 0;

				INT64 nCurGuildWareCoin = GetItemTask().GetGuildWareCoin();
				biCoin = min( biCoin, nCurGuildWareCoin );
				SetMaxMoney( biCoin );

				INT64 nGold = biCoin/10000;
				INT64 nSilver = (biCoin%10000)/100;
				INT64 nBronze = biCoin%100;
				std::wstring strString;

				m_pStaticLimitGold->SetInt64ToText( nGold );
				DN_INTERFACE::UTIL::GetValue_2_String( (int)nGold, strString );
				m_pStaticLimitGold->SetTooltipText( strString.c_str() );

				m_pStaticLimitSilver->SetInt64ToText( nSilver );
				DN_INTERFACE::UTIL::GetValue_2_String( (int)nSilver, strString );
				m_pStaticLimitSilver->SetTooltipText( strString.c_str() );

				m_pStaticLimitBronze->SetInt64ToText( nBronze );
				DN_INTERFACE::UTIL::GetValue_2_String( (int)nBronze, strString );
				m_pStaticLimitBronze->SetTooltipText( strString.c_str() );
			}
		}
	}
}
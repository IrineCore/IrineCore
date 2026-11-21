#include "StdAfx.h"

#ifdef PRE_ADD_VIP_FARM
#include "DnLifeUnionMarkTooltip.h"
#include "DnLocalPlayerActor.h"
#include "TaskManager.h"
#include "DnInterface.h"
#include "DnCommonUtil.h"
#include "DnUIString.h"
#include "DnItemTask.h"
#include "SyncTimer.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnLifeUnionMarkTooltipDlg::CDnLifeUnionMarkTooltipDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback ) : CEtUIDialog( dialogType, pParentDialog, nID, pCallback ), 
m_pContents( NULL )
{
}

CDnLifeUnionMarkTooltipDlg::~CDnLifeUnionMarkTooltipDlg(void)
{
}

void CDnLifeUnionMarkTooltipDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "SimpleTooltipDlg.ui" ).c_str(), bShow );
}

void CDnLifeUnionMarkTooltipDlg::InitialUpdate()
{
	m_pContents = GetControl<CEtUITextBox>( "ID_TEXTBOX" );
}

void CDnLifeUnionMarkTooltipDlg::SetMarkTooltip()
{
	m_pContents->ClearText();

	DNTableFileFormat* pItemSox = GetDNTable( CDnTableDB::TITEM );
	int nPer = pItemSox->GetFieldFromLablePtr( Farm::Common::VIP_GROWING_BOOST_ITEMID, "_TypeParam1" )->GetInteger();

	time_t pNowTime = CSyncTimer::GetInstance().GetCurTime();
	time_t tFarmVIPTime = CDnItemTask::GetInstance().GetVipFarmTime();
	time_t tVIPTime = 0;

#if defined(PRE_ADD_VIP)
	if (!CDnLocalPlayerActor::s_hLocalActor)
		return;

	CDnLocalPlayerActor *pLocalActor = dynamic_cast<CDnLocalPlayerActor *>(CDnLocalPlayerActor::s_hLocalActor.GetPointer());

	if( pLocalActor && pLocalActor->IsVIP() )
		tVIPTime = pLocalActor->GetVIPTime();

	if( tFarmVIPTime < tVIPTime )
		tFarmVIPTime = tVIPTime;
#endif	//#if defined(PRE_ADD_VIP)

	time_t pItemTime =  tFarmVIPTime - pNowTime;

	m_pContents->AddText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7535 ) );	// Farm VIP membership

	tm date;
	std::wstring dateStr;
	DnLocalTime_s(&date, &tFarmVIPTime);
	CommonUtil::GetDateString(CommonUtil::DATESTR_FULL, dateStr, date);

	std::wstring result = FormatW(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2115 ), dateStr.c_str()); // UISTRING : Valid until %s
	m_pContents->AddText(result.c_str(), textcolor::RED);

	m_pContents->AddText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7537 ) );	// Receive Farm VIP membership benefits for the valid period after purchase.

	m_pContents->AddColorText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7538 ), textcolor::YELLOW );	// [Membership Benefit]

	m_pContents->AddText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7539 ) );	// Can enter VIP farm area

	if( nPer != 0 )
	{
		result = FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7540 ), nPer );	// Reduces total farm cultivation time by %d % when growing plants in the VIP farm
		m_pContents->AddText( result.c_str() );	
	}
}

bool CDnLifeUnionMarkTooltipDlg::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (m_bShow == false)
		return false;

	bool bRet = CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );
	switch( uMsg )
	{
	case WM_MOUSEMOVE:
		{
			float fMouseX = 0;
			float fMouseY = 0;
			POINT MousePoint;
			MousePoint.x = short( LOWORD( lParam ) );
			MousePoint.y = short( HIWORD( lParam ) );
			PointToFloat( MousePoint, fMouseX, fMouseY );

			if (m_pContents && m_pContents->IsInside(fMouseX, fMouseY))
				Show(false);
		}
	}

	return bRet;
}

#endif // PRE_ADD_VIP_FARM
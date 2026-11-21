#include "StdAfx.h"

#ifdef PRE_ADD_VIP

#include "DnPopupVIPDlg.h"
#include "DnInterface.h"
#include "DnInterfaceString.h"
#include "DnLocalPlayerActor.h"
#include "DnTableDB.h"
#ifdef PRE_FIX_VIP_MAXPTS
#include "DnCommonUtil.h"
#endif

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnPopupVIPDlg::CDnPopupVIPDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CEtUIDialog( dialogType, pParentDialog, nID, pCallback )
{
	int i = 0;
	for (; i < CDnVIPDataMgr::eMAX; ++i)
		m_pStatic[i] = NULL;
	m_pStaticExpireDate = NULL;
	m_pStaticPts		= NULL;
}

CDnPopupVIPDlg::~CDnPopupVIPDlg(void)
{
}

void CDnPopupVIPDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "VIPInfo.ui" ).c_str(), bShow );
}

void CDnPopupVIPDlg::InitialUpdate()
{
	int i = 0;
	for (; i < CDnVIPDataMgr::eMAX; ++i)
	{
		std::string controlID = FormatA("ID_VIP_PROFIT%d", i);
		m_pStatic[i] = GetControl<CEtUIStatic>(controlID.c_str());
	}

	m_pStaticExpireDate = GetControl<CEtUIStatic>("ID_VIP_TIME");
	m_pStaticPts		= GetControl<CEtUIStatic>("ID_VIP_LE");
}

void CDnPopupVIPDlg::SetInfo(const SUICoord& iconCoord)
{
	m_ParentUICoord = iconCoord;
}

void CDnPopupVIPDlg::Show( bool bShow )
{
	if (bShow == m_bShow)
		return;

	if (bShow)
	{
		if (CDnLocalPlayerActor::s_hLocalActor)
		{
			CDnLocalPlayerActor *pLocalActor = static_cast<CDnLocalPlayerActor *>(CDnLocalPlayerActor::s_hLocalActor.GetPointer());
			if (pLocalActor == NULL)
				return;

			const CDnVIPDataMgr& dataMgr = pLocalActor->GetVIPDataMgr();
			if (dataMgr.IsVIP() == false)
				return;

			int i = 0;
			for (; i < CDnVIPDataMgr::eMAX; ++i)
			{
				std::wstring temp;
				dataMgr.GetBonusString(temp, (CDnVIPDataMgr::eBonusType)i);
				if (temp.empty() == false)
					m_pStatic[i]->SetText(temp.c_str());
				else
					m_pStatic[i]->ClearText();

				DWORD textColor = textcolor::WHITE;
				if (dataMgr.IsVIPAutoPay())
				{
					temp = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 2020023);	// UISTRING : Auto-billing service is currently active.
				}
				else
				{
					const __time64_t* pTime = dataMgr.GetVIPExpireDate();
					if (pTime != NULL)
					{
						if (dataMgr.IsVIPNearExpireDate())
							textColor = textcolor::RED;

						std::wstring dayText;
						DN_INTERFACE::STRING::GetDayText(dayText, *pTime);
						temp = FormatW(L"%s %s", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 2020014), dayText.c_str());
					}
				}

				m_pStaticExpireDate->SetTextColor(textColor);
				m_pStaticExpireDate->SetText(temp.c_str());

#ifdef PRE_FIX_VIP_MAXPTS
#define MAX_VIP_PTS 10000
				int pts = dataMgr.GetCurrentPts();
				CommonUtil::ClipNumber(pts, dataMgr.GetCurrentPts(), MAX_VIP_PTS);
				const CVIPGradeTable::SVIPGradeUnit* pUnit = CVIPGradeTable::GetInstance().GetValue(pts);
#else
				// level pts
				const CVIPGradeTable::SVIPGradeUnit* pUnit = CVIPGradeTable::GetInstance().GetValue(dataMgr.GetCurrentPts());
#endif
				if (pUnit)
				{
					std::wstring temp1, temp2;
					temp1 = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 2020022), pUnit->level);			// UISTRING : Lv%d
#ifdef PRE_FIX_VIP_MAXPTS
					temp2 = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 120315), pts);	// UISTRING : %d pts
#else
					temp2 = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 120315), dataMgr.GetCurrentPts());	// UISTRING : %d pts
#endif
					temp = FormatW(L"%s %s", temp1.c_str(), temp2.c_str());

					m_pStaticPts->SetText(temp.c_str());
				}
				else
				{
					m_pStaticPts->ClearText();
				}
			}
		}
	}

	CEtUIDialog::Show( bShow );
}

bool CDnPopupVIPDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{	
	case WM_MOUSEMOVE:
		{
			if( IsMouseInDlg() )
			{
				float fMouseX, fMouseY;
				POINT MousePoint;
				MousePoint.x = short( LOWORD( lParam ) );
				MousePoint.y = short( HIWORD( lParam ) );
				PointToFloat( MousePoint, fMouseX, fMouseY );

				if (m_ParentUICoord.IsInside(fMouseX, fMouseY) == false)
					Show( false );
			}
		}
		break;
	}

	return CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );
}

#endif // PRE_ADD_VIP
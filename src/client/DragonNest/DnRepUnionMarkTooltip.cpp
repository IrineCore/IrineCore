#include "StdAfx.h"
#include "DnRepUnionMarkTooltip.h"
#include "DnTableDB.h"
#include "TaskManager.h"
#include "DnQuestTask.h"
#include "DnInterface.h"
#include "DnCommonUtil.h"
#include "DnUIString.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnRepUnionMarkTooltipDlg::CDnRepUnionMarkTooltipDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback ) : CEtUIDialog( dialogType, pParentDialog, nID, pCallback ), 
																																						 m_pContents( NULL )
{
}

CDnRepUnionMarkTooltipDlg::~CDnRepUnionMarkTooltipDlg(void)
{
}

void CDnRepUnionMarkTooltipDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "SimpleTooltipDlg.ui" ).c_str(), bShow );
}

void CDnRepUnionMarkTooltipDlg::InitialUpdate()
{
	m_pContents = GetControl<CEtUITextBox>( "ID_TEXTBOX" );
}

void CDnRepUnionMarkTooltipDlg::SetMarkTooltip(int itemId)
{
	m_pContents->ClearText();

	DNTableFileFormat*  pBenefitTable = GetDNTable(CDnTableDB::TREPUTEBENEFIT);
	if (pBenefitTable == NULL)
		return;

	CDnQuestTask* pQuestTask = static_cast<CDnQuestTask*>(CTaskManager::GetInstance().GetTask( "QuestTask" ));	
	if( !pQuestTask ) 
		return;

	CReputationSystemRepository* pReputationRepos = pQuestTask->GetReputationRepository();
	if (pReputationRepos == NULL)
		return;

	int nameStringID = pBenefitTable->GetFieldFromLablePtr(itemId, "_NameID")->GetInteger();
	m_pContents->AddText(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nameStringID ));

	const std::vector<CDnInterface::SUnionMembershipData>& data = pReputationRepos->GetUnionMembershipData();
	std::vector<CDnInterface::SUnionMembershipData>::const_iterator iter = data.begin();
	for (; iter != data.end(); ++iter)
	{
		const CDnInterface::SUnionMembershipData& curData = *iter;
		if (curData.itemId == itemId && curData.expireDate > 0)
		{
			tm date;
			std::wstring dateStr;
			DnLocalTime_s(&date, &(curData.expireDate));
			CommonUtil::GetDateString(CommonUtil::DATESTR_FULL, dateStr, date);

			std::wstring result = FormatW(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2115 ), dateStr.c_str()); // UISTRING : Valid until %s
			m_pContents->AddText(result.c_str(), textcolor::RED);
			break;
		}
	}

	DNTableFileFormat*  pItemTable = GetDNTable(CDnTableDB::TITEM);
	if (pItemTable == NULL)
		return;

	std::wstring desc;
	int stringNum = pItemTable->GetFieldFromLablePtr(itemId, "_DescriptionID")->GetInteger();
	char *szParam = pItemTable->GetFieldFromLablePtr(itemId, "_DescriptionIDParam")->GetString();

	MakeUIStringUseVariableParam(desc, stringNum, szParam);

	m_pContents->AddColorText(desc.c_str());
}

bool CDnRepUnionMarkTooltipDlg::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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

bool CDnRepUnionMarkTooltipDlg::AdjustPosition(float orgX, float orgY)
{
	float fRight = orgX + m_pContents->GetUICoord().fWidth;
	if (fRight > GetScreenWidthRatio())
		orgX -= m_pContents->GetUICoord().fWidth;

	SetPosition(orgX, orgY);

	return true;
}
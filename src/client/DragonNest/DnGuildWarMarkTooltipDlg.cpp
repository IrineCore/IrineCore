#include "StdAfx.h"

#include "DnGuildWarMarkTooltipDlg.h"
#include "DnTableDB.h"
#include "TaskManager.h"
#include "DnQuestTask.h"
#include "DnInterface.h"
#include "DnCommonUtil.h"
#include "DnUIString.h"
#include "DnGuildWarTask.h"
#include "DnInterfaceString.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnGuildWarMarkTooltipDlg::CDnGuildWarMarkTooltipDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback ) : CEtUIDialog( dialogType, pParentDialog, nID, pCallback ), 
																																						 m_pContents( NULL )
{
}

CDnGuildWarMarkTooltipDlg::~CDnGuildWarMarkTooltipDlg(void)
{
}

void CDnGuildWarMarkTooltipDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "SimpleTooltipDlg.ui" ).c_str(), bShow );
}

void CDnGuildWarMarkTooltipDlg::InitialUpdate()
{
	m_pContents = GetControl<CEtUITextBox>( "ID_TEXTBOX" );
}

bool CDnGuildWarMarkTooltipDlg::AdjustPosition(float orgX, float orgY)
{
	float fRight = orgX + m_pContents->GetUICoord().fWidth;
	if (fRight > GetScreenWidthRatio())
		orgX -= m_pContents->GetUICoord().fWidth;

	SetPosition(orgX, orgY);

	return true;
}

bool CDnGuildWarMarkTooltipDlg::SetMarkTooltip()
{
	m_pContents->ClearText();

	if (CDnGuildWarTask::IsActive() == false)
		return false;

	std::wstring result;

	eGuildWarStepType warStep = CDnGuildWarTask::GetInstance().GetCurrentGuildWarEventTimeType();
	if (warStep != GUILDWAR_STEP_NONE)
	{
		std::wstring beginStr[GUILDWAR_STEP_END];
		std::wstring endStr[GUILDWAR_STEP_END];
		int i = GUILDWAR_STEP_PREPARATION;
		for (; i < GUILDWAR_STEP_END; ++i)
		{
			if (i == GUILDWAR_STEP_NONE) continue;
			CDnGuildWarTask::GetInstance().GetGuildWarEventTime(beginStr[i], endStr[i], (eGuildWarStepType)i);
		}
		int nStringID = 0;

		int stepNum = 0;
		if (warStep == GUILDWAR_STEP_PREPARATION)
		{
			if( !CDnGuildWarTask::GetInstance().GetGuildWarFinalProgress() )
				nStringID = 126305;
			else
				nStringID = 126282;
			result = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, nStringID),
							beginStr[GUILDWAR_STEP_PREPARATION].c_str(), endStr[GUILDWAR_STEP_PREPARATION].c_str(),
							beginStr[GUILDWAR_STEP_TRIAL].c_str(), endStr[GUILDWAR_STEP_TRIAL].c_str(),
							beginStr[GUILDWAR_STEP_REWARD].c_str(), endStr[GUILDWAR_STEP_REWARD].c_str()
							); // UISTRING : Guild War registration period...
		}
		else if (warStep == GUILDWAR_STEP_TRIAL)
		{
			if( !CDnGuildWarTask::GetInstance().GetGuildWarFinalProgress() )
				nStringID = 126306;
			else
				nStringID = 126283;
			result = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, nStringID),
							beginStr[GUILDWAR_STEP_TRIAL].c_str(), endStr[GUILDWAR_STEP_TRIAL].c_str(),
							beginStr[GUILDWAR_STEP_REWARD].c_str(), endStr[GUILDWAR_STEP_REWARD].c_str()
							); // UISTRING : Guild War preliminaries period...
		}
		else if (warStep == GUILDWAR_STEP_REWARD)
		{
			if( !CDnGuildWarTask::GetInstance().GetGuildWarFinalProgress() )
			{
				nStringID = 126307;
				result = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, nStringID),
					beginStr[GUILDWAR_STEP_REWARD].c_str(), endStr[GUILDWAR_STEP_REWARD].c_str());
			}
			else
			{
				nStringID = 126284;
				std::wstring rewardBeginStr[GUILDWAR_FINALPART_MAX];
				std::wstring rewardEndStr[GUILDWAR_FINALPART_MAX];
				int j = GUILDWAR_FINALPART_NONE;
				for (; j < GUILDWAR_FINALPART_MAX; ++j)
				{
					if (j == GUILDWAR_FINALPART_NONE) continue;
					CDnGuildWarTask::GetInstance().GetGuildWarFinalRoundTime(rewardBeginStr[j], rewardEndStr[j], (eGuildFinalPart)j);
				}
				result = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, nStringID),
					beginStr[GUILDWAR_STEP_REWARD].c_str(), endStr[GUILDWAR_STEP_REWARD].c_str(),
					rewardBeginStr[GUILDWAR_FINALPART_16].c_str(),
					rewardBeginStr[GUILDWAR_FINALPART_8].c_str(),
					rewardBeginStr[GUILDWAR_FINALPART_4].c_str(),
					rewardBeginStr[GUILDWAR_FINALPART_FINAL].c_str()
					); // UISTRING : Guild War finals period...
			}			
		}
		else
		{
			_ASSERT(0);
			return false;
		}

		std::wstring msg;
		MakeUIStringUseVariableParamByBaseMsg(msg, result, "");
		m_pContents->AddColorText(result.c_str());
	}

	return true;
}

bool CDnGuildWarMarkTooltipDlg::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
#include "StdAfx.h"
#include "DnPartyEnterDlg.h"
#include "TaskManager.h"
#include "DnPartyTask.h"
#include "DnInterface.h"


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif



void CDnPartyEnterDlg::SPartyEnterUnit::Set(const CDnVillageTask::SPartyMemberInfo& info)
{
	pMasterMark->Show(info.bMaster);
	pName->SetText(info.wszName);
	pName->SetTextColor(CDnInterface::GetInstance().GetCharNameColor(info.wszName));

	std::wstring str;
	str = FormatW(L"%s %d", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 64), info.nUserLevel);	// UISTRING : Level
	pLevel->SetText(str.c_str());
	pJob->SetText(DN_INTERFACE::STRING::GetClassString(info.nJob));

#ifdef PRE_ADD_NEWCOMEBACK
	pComeback->Show( info.bComeback );
#endif // PRE_ADD_NEWCOMEBACK
}

void CDnPartyEnterDlg::SPartyEnterUnit::Clear()
{
	pMasterMark->Show(false);
	pName->ClearText();
	pLevel->ClearText();
	pJob->ClearText();
#ifdef PRE_ADD_NEWCOMEBACK
	pComeback->Show(false);
#endif // PRE_ADD_NEWCOMEBACK
}


//////////////////////////////////////////////////////////////////////////

#ifdef PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP
CDnPartyEnterDlg::CDnPartyEnterDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback, bool bAutoCursor ) 
	:CEtUIDialog( dialogType, pParentDialog, nID, pCallback, bAutoCursor )
	, m_Type(_NORMAL_PARTY)
	, m_pCancelBtn(NULL)
	, m_pConfirmBtn(NULL)
	, m_pStaticPartyName(NULL)
	, m_pStaticTargetStage(NULL)
{
	m_PartyIdx = -1;
}
#else
CDnPartyEnterDlg::CDnPartyEnterDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  )
	: CEtUIDialog( dialogType, pParentDialog, nID, pCallback  )
	, m_Type(_NORMAL_PARTY)
	, m_pCancelBtn(NULL)
	, m_pConfirmBtn(NULL)
	, m_pStaticPartyName(NULL)
	, m_pStaticTargetStage(NULL)
{
	m_PartyIdx = -1;
}
#endif 

CDnPartyEnterDlg::~CDnPartyEnterDlg(void)
{
}

void CDnPartyEnterDlg::Initialize( bool bShow, ePartyType type )
{
	m_Type = type;
	std::string resourcePath = "PartyEnter.ui";
	if (type == _RAID_PARTY_8)
		resourcePath = "PartyRaidEnter.ui";
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName(resourcePath.c_str()).c_str(), bShow );
}

void CDnPartyEnterDlg::InitialUpdate()
{
	m_pStaticPartyName = GetControl<CEtUIStatic>("ID_STATIC_PARTYNAME");
	m_pStaticTargetStage = GetControl<CEtUIStatic>("ID_STATIC_TARGETSTAGE");

	int i = 0;
	int nPartyMax = NORMPARTYCOUNTMAX;
	if (m_Type == _RAID_PARTY_8)
		nPartyMax = RAIDPARTYCOUNTMAX;

	for (; i < nPartyMax; ++i)
	{
		SPartyEnterUnit unit;
		std::string str;

		str = FormatA("ID_STATIC2_MASTER%d", i);
		unit.pMasterMark = GetControl<CEtUIStatic>(str.c_str());

		str = FormatA("ID_STATIC_NAME%d", i);
		unit.pName = GetControl<CEtUIStatic>(str.c_str());

		str = FormatA("ID_STATIC_LV%d", i);
		unit.pLevel = GetControl<CEtUIStatic>(str.c_str());

		str = FormatA("ID_STATIC_JOB%d", i);
		unit.pJob = GetControl<CEtUIStatic>(str.c_str());

// #78053 Returnee icon is shown.
		str = FormatA("ID_STATIC_RETURNBONUSEXP%d", i);
		CEtUIStatic * pStatic = GetControl<CEtUIStatic>(str.c_str());
#ifdef PRE_ADD_NEWCOMEBACK		
		unit.pComeback = pStatic;
#else		
		if( pStatic )
			pStatic->Show( false );
#endif // PRE_ADD_NEWCOMEBACK

		m_PartyMemberInfoList.push_back(unit);
	}

	m_pConfirmBtn = GetControl<CEtUIButton>("ID_BUTTON_OK");
	m_pCancelBtn = GetControl<CEtUIButton>("ID_BUTTON_CANCEL");
}

void CDnPartyEnterDlg::ClearMemberInfo()
{
	std::vector<SPartyEnterUnit>::iterator iter = m_PartyMemberInfoList.begin();

	for (; iter != m_PartyMemberInfoList.end(); ++iter)
	{
		SPartyEnterUnit& unit = *iter;
		unit.Clear();
	}
}

void CDnPartyEnterDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	if (bShow)
	{
		ClearMemberInfo();

		bool bError = true;
		CDnVillageTask *pTask = (CDnVillageTask *)CTaskManager::GetInstance().GetTask( "VillageTask" );
		if (pTask)
		{
			std::vector<CVillageClientSession::SPartyMemberInfo>* pPartyInfo = pTask->GetPartyMemberList();
#if defined( PRE_WORLDCOMBINE_PARTY )
			bError = false;			
#endif
			if (pPartyInfo != NULL)
			{
				if (pPartyInfo->empty() == false)
				{
					int slotCounter = 0;
					std::vector<CVillageClientSession::SPartyMemberInfo>::const_iterator iter = pPartyInfo->begin();
					for (; iter != pPartyInfo->end(); ++iter, ++slotCounter)
					{
						const CVillageClientSession::SPartyMemberInfo& info = *iter;
						if ((int)m_PartyMemberInfoList.size() <= slotCounter)
							break;

						m_PartyMemberInfoList[slotCounter].Set(info);
					}

					bError = false;
				}
			}
		}

		if (bError)
		{
			GetInterface().MessageBox(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3403));	// UISTRING : Cannot retrieve party information. Please select again.
			return;
		}
	}
	else
	{
		GetPartyTask().LockPartyReqPartyMemberInfo(false);
	}

	CEtUIDialog::Show( bShow );
}

#ifdef PRE_PARTY_DB
void CDnPartyEnterDlg::SetPartyInfo(TPARTYID partyIdx, const std::wstring& partyName, const std::wstring& targetStageName)
#else
void CDnPartyEnterDlg::SetPartyInfo(int partyIdx, const std::wstring& partyName, const std::wstring& targetStageName)
#endif
{
	m_pStaticPartyName->SetText(partyName.c_str());
	m_pStaticTargetStage->SetText(targetStageName.c_str());
	m_PartyIdx = partyIdx;
}

bool CDnPartyEnterDlg::DoEnter()
{
	if (m_PartyIdx < 0)
		return false;

#ifdef PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP 

#ifdef PRE_WORLDCOMBINE_PARTY
	if( GetPassword() ==  Party::Constants::INVALID_PASSWORD && m_Type != _WORLDCOMBINE_PARTY )
#else
	if( GetPassword() ==  Party::Constants::INVALID_PASSWORD )
#endif
	{	
		std::wstring wszMasterName;
		std::vector<SPartyEnterUnit>::iterator iter = m_PartyMemberInfoList.begin();
		for (; iter != m_PartyMemberInfoList.end(); ++iter)
		{
			SPartyEnterUnit& unit = *iter;		

			if( unit.pMasterMark->IsShow() )
			{
				wszMasterName = unit.pName->GetText();
				break;
			}
		}
		if( ! wszMasterName.empty() )
			GetPartyTask().ReqAskParty( wszMasterName.c_str() );
	}
	else 
	{
		CDnPartyTask::GetInstance().ReqJoinParty( m_PartyIdx, GetPassword() );
	}
#else
	CDnPartyTask::GetInstance().ReqJoinParty( m_PartyIdx, GetPassword() );
#endif

#ifdef PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP
	Show(false);
#endif
	return true;
}


void CDnPartyEnterDlg::ProcessCommand(UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg)
{
	SetCmdControlName( pControl->GetControlName() );

	if (CDnPartyTask::IsActive() == false)
		return;

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if (IsCmdControl("ID_BUTTON_OK"))
		{
			DoEnter();
		}
		else
		if( IsCmdControl("ID_BUTTON_CANCEL") || IsCmdControl("ID_BUTTON_CLOSE"))
		{
			Show(false);
		}
	}
}

bool CDnPartyEnterDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
		return false;

	if( uMsg == WM_KEYDOWN )
	{
		if( wParam == VK_RETURN )
		{
			if (m_pConfirmBtn->IsEnable())
			{
				DoEnter();
				focus::ReleaseControl();
				return true;
			}
		}
	}

	return CEtUIDialog::MsgProc(hWnd, uMsg, wParam, lParam);
}
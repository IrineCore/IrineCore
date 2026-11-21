#include "StdAfx.h"

#include "DnPartyRaidReGroupDlg.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnPartyRaidReGroupDlg::CDnPartyRaidReGroupDlg(UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback)
	: CDnCustomDlg(dialogType, pParentDialog, nID, pCallback)
	, m_nSwapSrcIdx(-1)
	, m_pOkBtn(NULL)
	, m_pCancelBtn(NULL)
	, m_bChanged(false)
{
	ClearGroupIdexes();
}

CDnPartyRaidReGroupDlg::~CDnPartyRaidReGroupDlg(void)
{
}

void CDnPartyRaidReGroupDlg::Initialize(bool bShow)
{
	CEtUIDialog::Initialize(CEtResourceMng::GetInstance().GetFullName("PartyRaidOptionDlg.ui").c_str(), bShow);
}

void CDnPartyRaidReGroupDlg::InitialUpdate()
{
	int i = 0;
	std::string ctrlName;
	for (; i < MAX_8RAID_GROUP; ++i)
	{
		SRaidReGroupInfo info;
		ctrlName = FormatA("ID_RAID%d", i + 1);
		info.pTitle = GetControl<CEtUIStatic>(ctrlName.c_str());

		int j = 0;
		for (; j < MAX_8RAID_GROUP_MEMBER; ++j)
		{
			SRaidReGroupCtrlUnit unit;

			ctrlName = FormatA("ID_RAID%d_BAR%d", i + 1, j);
			unit.pBase = GetControl<CEtUIStatic>(ctrlName.c_str());

			ctrlName = FormatA("ID_RAID%d_LV%d", i + 1, j);
			unit.pLevel = GetControl<CEtUIStatic>(ctrlName.c_str());

			ctrlName = FormatA("ID_RAID%d_NAME%d", i + 1, j);
			unit.pName = GetControl<CEtUIStatic>(ctrlName.c_str());

			ctrlName = FormatA("ID_RAID%d_SELECT%d", i + 1, j);
			unit.pSelect = GetControl<CEtUIStatic>(ctrlName.c_str());

			ctrlName = FormatA("ID_RAID%d_CLASS%d", i + 1, j);
			unit.pJobIcon		= GetControl<CDnJobIconStatic>(ctrlName.c_str());

			info.partyCtrlList.push_back(unit);
		}

		m_PartyReGroupList.push_back(info);
	}

	m_pOkBtn = GetControl<CEtUIButton>("ID_BUTTON_CREATE");
	m_pCancelBtn = GetControl<CEtUIButton>("ID_BUTTON_CLOSE");
}

void CDnPartyRaidReGroupDlg::AddToList(const CDnPartyTask::PartyStruct& content, int index)
{
	if (index < 0 || index >= RAIDPARTYCOUNTMAX)
		return;

	m_GroupSessionIDs[index] = content.nSessionID;

}

void CDnPartyRaidReGroupDlg::Show(bool bShow)
{
	if (bShow == m_bShow)
		return;

	if (bShow)
	{
		RefreshList();
		m_bChanged = false;
	}

	CEtUIDialog::Show(bShow);
}

void CDnPartyRaidReGroupDlg::ClearGroupIdexes()
{
	memset(m_GroupSessionIDs, 0, sizeof(m_GroupSessionIDs));
}

void CDnPartyRaidReGroupDlg::SortGroupSessionIDs()
{
	UINT tempIDs[RAIDPARTYCOUNTMAX];
	memcpy_s(tempIDs, sizeof(tempIDs), m_GroupSessionIDs, sizeof(m_GroupSessionIDs));
	ClearGroupIdexes();

	int i = 0;
	for (; i < RAIDPARTYCOUNTMAX; ++i)
	{
		if (tempIDs[i] > 0)
		{
			int slotIdx = GetGroupEmptySlotIndex(i);
			if (slotIdx >= 0)
				m_GroupSessionIDs[slotIdx] = tempIDs[i];
		}
	}
}

void CDnPartyRaidReGroupDlg::RefreshList(bool bUIOnly)
{
	std::vector<SRaidReGroupInfo>::iterator iter = m_PartyReGroupList.begin();
	for (; iter != m_PartyReGroupList.end(); ++iter)
	{
		SRaidReGroupInfo& info = *iter;
		info.InitInfo();
	}

	int i = 0;
	if (bUIOnly == false)
	{
		CDnPartyTask::PartyStruct *pStruct(NULL);
		int partyCount = CDnPartyTask::GetInstance().GetPartyCount();
		if (partyCount <= 0)
			return;

		ClearGroupIdexes();

		for (i = 0; i < partyCount; ++i)
		{
			pStruct = CDnPartyTask::GetInstance().GetPartyData(i);
			if( !pStruct ) continue;

			AddToList(*pStruct, pStruct->cMemberIndex);
		}
	}

	SortGroupSessionIDs();

	for (i = 0; i < RAIDPARTYCOUNTMAX; ++i)
	{
		if (m_GroupSessionIDs[i] != 0)
		{
			CDnPartyTask::PartyStruct* pStruct = CDnPartyTask::GetInstance().GetPartyDataFromSessionID(m_GroupSessionIDs[i]);
			if (pStruct)
			{
				int groupIdx = GetGroupIndex(i);
				if (groupIdx > (int)m_PartyReGroupList.size() || groupIdx < 0 || pStruct->bGMTrace)
					continue;

				std::vector<SRaidReGroupCtrlUnit>& ctrlList = m_PartyReGroupList[groupIdx].partyCtrlList;
				int memberIdx = (i % MAX_8RAID_GROUP_MEMBER);
				if ((int)ctrlList.size() <= memberIdx)
					break;

				int nJob = pStruct->cClassID;
				if (!pStruct->nVecJobHistoryList.empty())
					nJob = pStruct->nVecJobHistoryList[pStruct->nVecJobHistoryList.size() - 1];
				ctrlList[memberIdx].SetInfo(pStruct->nSessionID, pStruct->wszCharacterName, pStruct->cLevel, nJob);
			}
		}
	}
}

void CDnPartyRaidReGroupDlg::ProcessCommand(UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg)
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_BUTTON_CREATE" ) )
		{
			CDnPartyTask::GetInstance().ReqPartySwapMemberIndex(m_PartyReGroupList);
			Show(false);
			return;
		}

		if( IsCmdControl("ID_BUTTON_CLOSE" ) )
		{
			Show(false);
			return;
		}
	}
	CEtUIDialog::ProcessCommand(nCommand, bTriggeredByUser, pControl, uMsg);
}

void CDnPartyRaidReGroupDlg::ResetSwapIdx()
{
	m_nSwapSrcIdx = -1;

	std::vector<SRaidReGroupInfo>::iterator iter = m_PartyReGroupList.begin();
	for (; iter != m_PartyReGroupList.end(); ++iter)
	{
		SRaidReGroupInfo& info = *iter;
		std::vector<SRaidReGroupCtrlUnit>::iterator unitIter = info.partyCtrlList.begin();
		for (; unitIter != info.partyCtrlList.end(); ++unitIter)
		{
			SRaidReGroupCtrlUnit& unit = *unitIter;
			unit.pSelect->Show(false);
		}
	}
}

void CDnPartyRaidReGroupDlg::SetSelect(int nIdx)
{
	int i = 0, j = 0;
	std::vector<SRaidReGroupInfo>::iterator iter = m_PartyReGroupList.begin();
	for (; iter != m_PartyReGroupList.end(); ++iter, ++i)
	{
		SRaidReGroupInfo& info = *iter;
		std::vector<SRaidReGroupCtrlUnit>::iterator unitIter = info.partyCtrlList.begin();
		for (j = 0; unitIter != info.partyCtrlList.end(); ++unitIter, ++j)
		{
			SRaidReGroupCtrlUnit& unit = *unitIter;
			if ((i * MAX_8RAID_GROUP_MEMBER + j) == nIdx)
			{
				if (unit.IsEmpty() == false)
					unit.pSelect->Show(true);
				return;
			}
		}
	}
}

int CDnPartyRaidReGroupDlg::GetGroupIndex(int index) const
{
	return (index / MAX_8RAID_GROUP_MEMBER);
}

bool CDnPartyRaidReGroupDlg::IsOtherGroupIdxWithSwapSrcIdx(int index) const
{
	return (GetGroupIndex(index) != GetGroupIndex(m_nSwapSrcIdx));
}

int CDnPartyRaidReGroupDlg::FindInsideItem(float fMouseX, float fMouseY) const
{
	int i = 0, j = 0;
	std::vector<SRaidReGroupInfo>::const_iterator iter = m_PartyReGroupList.begin();
	for (; iter != m_PartyReGroupList.end(); ++iter, ++i)
	{
		const SRaidReGroupInfo& info = *iter;
		std::vector<SRaidReGroupCtrlUnit>::const_iterator unitIter = info.partyCtrlList.begin();
		for (j = 0; unitIter != info.partyCtrlList.end(); ++unitIter, ++j)
		{
			const SRaidReGroupCtrlUnit& unit = *unitIter;
			if (unit.pBase->IsInside(fMouseX, fMouseY))
			{
				return (i * MAX_8RAID_GROUP_MEMBER + j);
			}
		}
	}

	return -1;
}

int CDnPartyRaidReGroupDlg::GetGroupEmptySlotIndex(int destIdx) const
{
	int groupIdx = GetGroupIndex(destIdx);
	if (groupIdx >= MAX_8RAID_GROUP)
		return -1;

	int i = 0;
	int groupFirstIdx = groupIdx * MAX_8RAID_GROUP_MEMBER;
	for (i = groupFirstIdx; i < groupFirstIdx + MAX_8RAID_GROUP_MEMBER; ++i)
	{
		if (m_GroupSessionIDs[i] == 0)
			return i;
	}
	return -1;
}

void CDnPartyRaidReGroupDlg::Process(float fElapsedTime)
{
	if (m_bShow == false)
		return;

	m_pOkBtn->Enable(m_bChanged);

	CEtUIDialog::Process(fElapsedTime);
}

bool CDnPartyRaidReGroupDlg::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if( !IsShow() )
	{
		return false;
	}

	switch( uMsg )
	{
	case WM_LBUTTONDOWN:
		{
			POINT MousePoint;
			float fMouseX, fMouseY;

			MousePoint.x = short( LOWORD( lParam ) );
			MousePoint.y = short( HIWORD( lParam ) );
			PointToFloat( MousePoint, fMouseX, fMouseY );

			int nFindSelect = FindInsideItem( fMouseX, fMouseY );
			if( nFindSelect > -1 )
			{
				if (m_nSwapSrcIdx >= 0)
				{
					int srcGroupIdx = GetGroupIndex(m_nSwapSrcIdx);
					if (m_nSwapSrcIdx >= RAIDPARTYCOUNTMAX)
						return false;

					UINT& swapSrc = m_GroupSessionIDs[m_nSwapSrcIdx];

					int destGroupIdx = GetGroupIndex(nFindSelect);
					if (destGroupIdx < 0 || nFindSelect >= RAIDPARTYCOUNTMAX)
						return false;

					UINT& swapDest = m_GroupSessionIDs[nFindSelect];
					if (swapDest != 0)
					{
						::swap(swapSrc, swapDest);
						m_bChanged = true;
						ResetSwapIdx();
					}
					else
					{
						if (srcGroupIdx != destGroupIdx)
						{
							int emptySlotIdx = GetGroupEmptySlotIndex(nFindSelect);
							if (emptySlotIdx >= 0)
							{
								m_GroupSessionIDs[emptySlotIdx] = swapSrc;
								swapSrc = 0;
								ResetSwapIdx();
								m_bChanged = true;
							}
						}
					}

					RefreshList(true);
				}
				else
				{
					ResetSwapIdx();
					if (m_GroupSessionIDs[nFindSelect] != 0)
					{
						m_nSwapSrcIdx = nFindSelect;
						SetSelect(m_nSwapSrcIdx);
					}
				}
			}
			else
			{
				ResetSwapIdx();
			}
		}
		break;
	}

	return CEtUIDialog::MsgProc(hWnd, uMsg, wParam, lParam);
}
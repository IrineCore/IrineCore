#include "StdAfx.h"

#ifdef PRE_ADD_NEWCOMEBACK
#include "DnPartyTask.h"
#include "DnPlayerActor.h"
#include "DnTableDB.h"
#endif // PRE_ADD_NEWCOMEBACK

#include "DnPartyNormalMemberDlg.h"
#include "DnPartyTask.h"
#include "DnSimpleTooltipDlg.h"
#include "DnInterface.h"
#include "DNIsolate.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnPartyNormalMemberDlg::CDnPartyNormalMemberDlg(UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback)
	: CDnPartyMemberDlg(dialogType, pParentDialog, nID, pCallback)
{
}

CDnPartyNormalMemberDlg::~CDnPartyNormalMemberDlg(void)
{
}

void CDnPartyNormalMemberDlg::Initialize(bool bShow)
{
	CEtUIDialog::Initialize(CEtResourceMng::GetInstance().GetFullName("PartyInfoNormalListDlg.ui").c_str(), bShow);
}

void CDnPartyNormalMemberDlg::InitialUpdate()
{
	CDnPartyMemberDlg::InitialUpdate();

	char szUserLevel[32]={0};
	char szMaster[32]={0};
	char szClass[32]={0};
	char szName[32]={0};
	char szBase[32]={0};
	char szState[32] = {0};
	char szFreeCoinIcon[32] = {0};
	char szFreeCoinCount[32] = {0};
	char szCashCoinIcon[32] = {0};
	char szCashCoinCount[32] = {0};
	char szPartyInvite[32] = {0};
	char szPartyInviteText[32] = {0};

	int i = 0;
	for (; i < NORMPARTYCOUNTMAX; i++)
	{
		SPartyMemberNormalUIData data;
		sprintf_s( szUserLevel, 32, "ID_LEVEL_%02d", i );
		sprintf_s( szMaster, 32, "ID_MASTER_%02d", i );
		sprintf_s( szClass, 32, "ID_CLASS_%02d", i );
		sprintf_s( szName, 32, "ID_NAME_%02d", i );

		sprintf_s( szBase, 32, "ID_BASE_%02d", i );

		sprintf_s( szState, 32, "ID_STATE_%02d", i );
		sprintf_s( szFreeCoinIcon, 32, "ID_HEART_BLUE%02d", i );
		sprintf_s( szFreeCoinCount, 32, "ID_COIN_FREE%02d", i );
		sprintf_s( szCashCoinIcon, 32, "ID_HEART_RED%02d", i );
		sprintf_s( szCashCoinCount, 32, "ID_COIN_CASH%02d", i );

		data.pUserLevel		= GetControl<CEtUIStatic>(szUserLevel);
		data.pMaster		= GetControl<CEtUIStatic>(szMaster);
		data.pClass			= GetControl<CEtUIStatic>(szClass);
		data.pName			= GetControl<CEtUIStatic>(szName);
		data.pBase			= GetControl<CEtUIStatic>(szBase);
		data.pState			= GetControl<CEtUIStatic>(szState);
		data.pFreeCoinIcon	= GetControl<CEtUIStatic>(szFreeCoinIcon);
		data.pFreeCoinCount = GetControl<CEtUIStatic>(szFreeCoinCount);
		data.pCashCoinIcon	= GetControl<CEtUIStatic>(szCashCoinIcon);
		data.pCashCoinCount = GetControl<CEtUIStatic>(szCashCoinCount);

// #78053 Returnee icon is shown.
		sprintf_s( szUserLevel, 32, "ID_STATIC_RETURNBONUSEXP%d", i );
		CEtUIStatic * pStatic = GetControl<CEtUIStatic>(szUserLevel);
#ifdef PRE_ADD_NEWCOMEBACK		
		data.pComebackUser = pStatic;
#else
		if( pStatic )
			pStatic->Show( false );
#endif // PRE_ADD_NEWCOMEBACK 

		if (i > 0)
		{
			sprintf_s(szPartyInvite, 32, "ID_STATIC_WAIT%d", i - 1);
			sprintf_s(szPartyInviteText, 32, "ID_TEXT_WAIT%d", i - 1);
			data.pStaticInvitation		= GetControl<CEtUIStatic>(szPartyInvite);
			data.pTextInviting			= GetControl<CEtUIStatic>(szPartyInviteText);
		}

		m_PartyMemberDataList.push_back(data);
	}
}

void CDnPartyNormalMemberDlg::Show(bool bShow)
{
	if (m_bShow == bShow)
		return;

	if (bShow == false)
	{
		ResetCurSelectIndex();
	}

	else
	{
#ifdef PRE_ADD_NEWCOMEBACK
		int size = CDnPartyTask::GetInstance().GetPartyCount();
		for( int i=0; i<size; ++i )
		{
			CDnPartyTask::PartyStruct * pStruct = CDnPartyTask::GetInstance().GetPartyData( i );
			if( pStruct->hActor )
			{
				CDnActor * pActor = static_cast< CDnActor * >( pStruct->hActor.GetPointer() );				
				CDnPlayerActor * pPlayer = dynamic_cast< CDnPlayerActor * >( pActor );				
				if( pPlayer )
				{
					DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TAPPELLATION );
					int nItemID = pSox->GetItemID( pPlayer->GetAppellationIndex() );				
					if( pSox->GetFieldFromLablePtr( nItemID, "_Type" )->GetInteger() == AppellationType::Type::eCode::ComeBack )
					{
						m_PartyMemberDataList[ i ].SetComeback( true );
					}
				}
			}
		}
#endif // PRE_ADD_NEWCOMEBACK

	}

	CDnPartyMemberDlg::Show(bShow);
}

void CDnPartyNormalMemberDlg::Process(float fElapsedTime)
{
	CDnPartyMemberDlg::Process(fElapsedTime);
}

bool CDnPartyNormalMemberDlg::ShowInvitationCtrl(bool bShow)
{
	if (bShow)
	{
		std::vector<SPartyMemberNormalUIData>::iterator iter = m_PartyMemberDataList.begin();
		for (; iter < m_PartyMemberDataList.end(); ++iter)
		{
			SPartyMemberNormalUIData& memberUnit = *iter;
			if (memberUnit.GetUserLevel() <= 0)
			{
				memberUnit.ShowInvitationCtrl(true);
				return true;
			}
		}
	}
	else
	{
		std::vector<SPartyMemberNormalUIData>::iterator iter = m_PartyMemberDataList.begin();
		for (; iter < m_PartyMemberDataList.end(); ++iter)
		{
			SPartyMemberNormalUIData& memberUnit = *iter;
			if (memberUnit.IsShowInvitationCtrl())
				memberUnit.ShowInvitationCtrl(false);
		}
		return true;
	}

	return false;
}

void CDnPartyNormalMemberDlg::RefreshList()
{
	std::vector<SPartyMemberNormalUIData>::iterator iter = m_PartyMemberDataList.begin();
	for (; iter != m_PartyMemberDataList.end(); ++iter)
	{
		SPartyMemberNormalUIData& data = *iter;
		data.InitInfoList();
	}

	if (CDnPartyTask::IsActive() == false)
		return;

	CDnPartyTask::PartyStruct *pStruct(NULL);
	int partyCount = (int)CDnPartyTask::GetInstance().GetPartyCount();

	int i = 0;
	for (iter = m_PartyMemberDataList.begin(); iter != m_PartyMemberDataList.end(); ++iter, ++i)
	{
		if (partyCount <= i)
			break;

		pStruct = CDnPartyTask::GetInstance().GetPartyData(i);
		if( !pStruct ) continue;
		if( pStruct->bGMTrace ) continue;

		SPartyMemberNormalUIData& data = *iter;

		data.SetUserLevel( pStruct->cLevel );
		data.SetMaster( pStruct->bPartyMaster );
		int nJob = pStruct->cClassID;
		if( !pStruct->nVecJobHistoryList.empty() ) nJob = pStruct->nVecJobHistoryList[ pStruct->nVecJobHistoryList.size() - 1 ];
		data.SetJobClass( nJob );
		data.SetName( pStruct->wszCharacterName, GetIsolateTask().IsBlackList(pStruct->wszCharacterName));
		data.SetSessionID( pStruct->nSessionID );
		data.SetFatigueValue( pStruct->wFatigue );
		data.Enable((pStruct->wFatigue > 0));
		data.SetState(pStruct->bAbsence);
		data.SetCoin((int)pStruct->wStdRebirthCoin, (int)pStruct->wCashRebirthCoin);
#ifdef PRE_ADD_NEWCOMEBACK
		data.SetComeback( pStruct->bCheckComebackAppellation );
#endif // PRE_ADD_NEWCOMEBACK

	}
}

void CDnPartyNormalMemberDlg::SetPartyInfoLeader(int nLeaderIndex)
{
	std::vector<SPartyMemberNormalUIData>::iterator iter = m_PartyMemberDataList.begin();
	for (; iter != m_PartyMemberDataList.end(); ++iter)
	{
		SPartyMemberNormalUIData& data = *iter;
		data.SetMaster(false);
	}

	if (CDnPartyTask::IsActive() == false)
		return;

	if (CDnPartyTask::GetInstance().GetPartyRole() == CDnPartyTask::SINGLE)
		return;

	CDnPartyTask::PartyStruct* pStruct(NULL);
	DWORD dwPartyCount = CDnPartyTask::GetInstance().GetPartyCount();

	for (DWORD i = 0; i < dwPartyCount; i++)
	{
		if (m_PartyMemberDataList.size() <= i)
			break;

		pStruct = CDnPartyTask::GetInstance().GetPartyData(i);
		if (!pStruct)
			continue;

		m_PartyMemberDataList[i].SetMaster(pStruct->bPartyMaster);
	}
}

int	CDnPartyNormalMemberDlg::FindInsideItem(float fX, float fY)
{
	int i = 0;
	std::vector<SPartyMemberNormalUIData>::iterator iter = m_PartyMemberDataList.begin();
	for (; iter != m_PartyMemberDataList.end(); ++iter, ++i)
	{
		SPartyMemberNormalUIData& data = *iter;
		if (data.IsInsideItem(fX, fY))
		{
			if (data.GetUserLevel() > 0)
				return i;
			else
				return -2;
		}
	}

	return -1;
}

bool CDnPartyNormalMemberDlg::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if( !IsShow() )
		return false;

	switch( uMsg )
	{
	case WM_MOUSEMOVE:
		{
			POINT MousePoint;
			float fMouseX, fMouseY;

			MousePoint.x = short(LOWORD(lParam));
			MousePoint.y = short(HIWORD(lParam));
			PointToFloat(MousePoint, fMouseX, fMouseY);

			int nFindSelect = FindInsideItem(fMouseX, fMouseY);
			if (nFindSelect > -1)
			{
				const SPartyMemberNormalUIData& member = m_PartyMemberDataList[nFindSelect];
				std::wstring tooltipStr;
				CDnSimpleTooltipDlg* tooltipDlg = GetInterface().GetSimpleTooltipDialog();

				if (tooltipDlg)
				{
					//	todo : display simultaneously
					if (member.GetFatigueValue() <= 0)
					{
						//CDnPartyTask::PartyStruct* foundMemberData = GetPartyTask().GetPartyData(nFindSelect); // Fine if RefreshList() is called on every network update, otherwise consider fetching directly from task by kalliste
						tooltipStr = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3540);	// UISTRING : That member has used up all FTG and cannot enter the stage.
					}
					else if (GetIsolateTask().IsBlackList(member.GetName().c_str()))
					{
						tooltipStr = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3541);	// UISTRING : Blocked character.
					}

					if (tooltipStr.empty() == false)
					{
						tooltipDlg->ShowTooltipDlg(member.GetControl(), true, tooltipStr, 0xffffffff, true);
					}
				}
			}
			else
			{
				CDnSimpleTooltipDlg* tooltipDlg = GetInterface().GetSimpleTooltipDialog();
				if (tooltipDlg && tooltipDlg->IsShow())
					tooltipDlg->Show(false);
			}
		}
		break;

	case WM_RBUTTONUP:
		{
			POINT MousePoint;
			float fMouseX, fMouseY;

			MousePoint.x = short( LOWORD( lParam ) );
			MousePoint.y = short( HIWORD( lParam ) );
			PointToFloat(MousePoint, fMouseX, fMouseY);

			int nFindSelect = FindInsideItem(fMouseX, fMouseY);
			if( nFindSelect > -1 && nFindSelect < (int)m_PartyMemberDataList.size())
			{
				SetCurSelectIndex(nFindSelect);
				UpdateSelectBar();

				float dlgPosX, dlgPosY;
				GetPosition(dlgPosX, dlgPosY);
				dlgPosX += fMouseX;
				dlgPosY += fMouseY;
				ShowPartyContextMenu(true, dlgPosX, dlgPosY, m_PartyMemberDataList[nFindSelect]);
			}
		}
	}

	return CDnPartyMemberDlg::MsgProc( hWnd, uMsg, wParam, lParam );
}

CEtUIControl* CDnPartyNormalMemberDlg::GetCurSelectCtrl()
{
	if (GetCurSelectIndex() < 0 || GetCurSelectIndex() >= (int)m_PartyMemberDataList.size())
		return NULL;
	return m_PartyMemberDataList[GetCurSelectIndex()].GetControl();
}

int CDnPartyNormalMemberDlg::GetCurSelectUserSessionID() const
{
	if (GetCurSelectIndex() < 0 || GetCurSelectIndex() >= (int)m_PartyMemberDataList.size())
		return 0;

	return m_PartyMemberDataList[GetCurSelectIndex()].nSessionID;
}


#ifdef PRE_ADD_NEWCOMEBACK
void CDnPartyNormalMemberDlg::SetComebackAppellation( UINT sessionID, bool bComeback )
{
	int size = (int)m_PartyMemberDataList.size();
	for( int i=0; i<size; ++i )
	{
		if( m_PartyMemberDataList[ i ].nSessionID == sessionID )
		{
			m_PartyMemberDataList[ i ].SetComeback( bComeback );
			break;
		}
	}
}
#endif // PRE_ADD_NEWCOMEBACK
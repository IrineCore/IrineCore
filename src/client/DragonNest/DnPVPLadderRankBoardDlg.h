#pragma once

#include "DnCustomDlg.h"
#include "DnBlindDlg.h"

class CDnPVPLadderRankBoardDlg : public CDnCustomDlg, public CDnBlindCallBack
{
public:
	CDnPVPLadderRankBoardDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL  );
	virtual ~CDnPVPLadderRankBoardDlg();

	enum ePVPLadderRankCommonValue
	{
		TopPlayerNumber = 3, // The top 3 players (1st, 2nd, 3rd) are drawn directly on static controls instead of the ladder item dialog.
	};

	struct sPVPLadderRankInfoUI
	{
		CEtUIStatic *pStatic_Name;
		CDnJobIconStatic *pStatic_JobIcon;
		CEtUIStatic *pStatic_Rank;
		CEtUIStatic *pStatic_WinCount;
		CEtUIStatic *pStatic_LadderCount;
		CEtUIStatic *pStatic_WinPercent;
		
		sPVPLadderRankInfoUI()
		{
			pStatic_Name = NULL;
			pStatic_JobIcon = NULL;
			pStatic_Rank = NULL;
			pStatic_WinCount = NULL;
			pStatic_LadderCount = NULL;
			pStatic_WinPercent = NULL;
		}
	
	};

protected:
	
	bool m_bRankInfoReqeust;
	sPVPLadderRankInfoUI m_sMyPVPLadderRankInfo;
	sPVPLadderRankInfoUI m_sTopPlayerPVPLadderRankInfo[TopPlayerNumber];
	CEtUIStatic *m_pStatic_RankTitle;
	CEtUIListBoxEx *m_pListBoxEX_UserList;

public:
	virtual void InitialUpdate();
	virtual void Initialize( bool bShow );
	virtual void Show( bool bShow );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

	void InitializeRankInfo(sPVPLadderRankInfoUI &pRankInfo);
	void InitializeRankInfoByCount(int nUserNumber,sPVPLadderRankInfoUI &pRankInfo);

	void SetUILadderInfo(TPvPLadderRanking *pInfo,sPVPLadderRankInfoUI &RankInfo);
	void SetPVPLadderRankInfo(SCPvPLadderRankBoard* pData);
	void SetPVPLadderRankMatchType(int nMatchType);

	void SetRankInfoRequest( bool bRequest ) { m_bRankInfoReqeust = bRequest; }
	bool IsRankInfoRequest() { return m_bRankInfoReqeust; }

public:

};

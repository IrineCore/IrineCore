#pragma once
#include "EtUIDialog.h"
#include "DnQuestPathFinder.h"

struct Journal;
struct TQuest;
class CDnQuestSummaryInfoDlg;

class CDnNotifierControls
{
public:
	CDnNotifierControls();

public:
	CEtUIStatic *m_pIcon;
	CEtUIStatic *m_pTitle;
	CEtUIStatic *m_Board;
	CEtUIStatic *m_OverBoard;
	CEtUITextBox *m_pTextBox;
	CEtUIButton *m_pButtonClose;
	CEtUIDialog *m_pDialog;

	SUICoord m_IconCoord;
	SUICoord m_TitleCoord;
	SUICoord m_TextBoxCoord;
	SUICoord m_BoardCoord;
	SUICoord m_SelectCoord;
	SUICoord m_CloseCoord;

	float m_PosY;
	float m_fPosY;
	float m_fSmoothY;
	float m_fDistance;
	int m_nNotifierIndex;

	bool m_bShow;
	bool m_bMouseOver;
	bool m_bClicked;

	void Show( bool bShow );
	bool IsShow() { return m_bShow; }
	void SetPosY( float fY );
	void SetNotifier( int nNotifier );
	void SetDialog( CEtUIDialog *pDialog ) { m_pDialog = pDialog; }
	void Refresh( const wchar_t *szTitle, const wchar_t *szDest, std::vector< boost::tuple<std::wstring, int, int> > &vecGetList, std::wstring szProgress );
	void MoveY( float fY ) { m_fPosY = fY; }
	void SetDistance( float fDistance ) { m_fDistance = fDistance; }
	void Process( float fElapsedTime );
	void MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	bool IsMouseOver() { return m_bMouseOver; }
	void SetClicked( bool bClick ) { m_bClicked = bClick; }
	bool IsClicked() { return m_bClicked; }

	static EtVector2 CalcArrowDestination( int nQuestIndex, int nJurnalStep );
	static int GetNotifyArrowState( int nQuestID, int nJurnalStep );
	static int CheckNotifyArrowStateByMap( int nArrowState, int nDestinationMapIndex );
};

class CDnQuestSummaryInfoListDlg : public CEtUIDialog
{
public:
	enum eQuestTraceStyle
	{
		Style_Main = 0,
		Style_Sub = 1,
		Style_Track = 2,
		Style_Max = 3,
	};

	CDnQuestSummaryInfoListDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_SELF, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnQuestSummaryInfoListDlg();

	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );

	void Clear();
	void Refresh( int nQuestIconIndex, int nNotifierIndex, const wchar_t *szTitle, const wchar_t *szDest, std::vector< boost::tuple<std::wstring, int, int> > &vecGetList, std::deque<PathInfo> &path );
	void UpdateDestinationMode();
	void ChangeArrowStateByPathCheck();
	void DrawArrow();

	void SetParentSummaryDlg( CDnQuestSummaryInfoDlg *pDlg ) { m_pParentSummaryDlg = pDlg; }
	void SetQuestInfo( TQuest *pQuest ) { m_Quest = pQuest; }
	void ClearQuestInfo() { m_Quest = NULL; }
	void SetJournal( Journal *pJournal ) { m_pJournal = pJournal; }
	Journal *GetJournal() { return m_pJournal; }
	void SetQuestID( int nQuestID ) { m_nQuestID = nQuestID; }
	int GetQuestID() const { return m_nQuestID; }
	void SetTraceQuest( bool bTrace ) { m_bNowTraceQuest = bTrace; }
	bool IsMouseOver() const { return m_bMouseOver; }
	int GetArrowState() const { return m_nArrowState; }
	size_t GetPathSize() const { return m_pathResult.size(); }

	CEtUIStatic *GetBoard() { return m_Board; }
	CEtUIStatic *GetOverBoard() { return m_OverBoard; }
	CEtUIButton *GetCloseButton() { return m_pButtonClose; }

protected:
	CEtUIStatic *m_pIcon[2];
	CEtUIStatic *m_pTitle[2];
	CEtUIStatic *m_pDistance[Style_Max];
	CEtUIStatic *m_pArrowBase;
	CEtUIStatic *m_pArrow[Style_Max];
	CEtUIStatic *m_pArrowArrival[Style_Max];
	CEtUIStatic *m_pArrowWrong[Style_Max];
	CEtUIStatic *m_pArrowBattle[Style_Max];
	CEtUIStatic *m_pArrowFind[Style_Max];
	CEtUIStatic *m_pArrowArrivalDistance[Style_Max];
	CEtUIStatic *m_Board;
	CEtUIStatic *m_OverBoard;
	CEtUITextBox *m_pTextBox;
	CEtUIButton *m_pButtonClose;

	CDnQuestSummaryInfoDlg *m_pParentSummaryDlg;
	int m_nQuestIconIndex;
	int m_nNotifierIndex;
	bool m_bTraceQuest;
	TQuest *m_Quest;
	Journal *m_pJournal;
	bool m_bMouseOver;
	int m_nArrowState;
	int m_nQuestID;
	bool m_bNowTraceQuest;
	std::deque<PathInfo> m_pathResult;
};

class CDnQuestSummaryInfoDlg : public CEtUIDialog
{
public:
	CDnQuestSummaryInfoDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnQuestSummaryInfoDlg(void);
	virtual void OnChangeResolution();

public:
	bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) override;
	void Process( float fElapsedTime ) override;
	void InitialUpdate() override;
	void Initialize( bool bShow ) override;
	void Render( float fElapsedTime ) override;

public:
	void RefreshNotifier();
	void RefreshMainNotifier();
	void RefreshSubNotifier();
	void RefreshMissionNotifier();
	void ClearAll();

	bool IsCanProgressQuest( int nQuestIndex );
	void SetPriorityMode( bool bTrue ) { m_bPriorityMode = bTrue; }
	bool IsPriorityMode() const { return m_bPriorityMode; }
	void DeletePriorityQuest( int nQuestID ) { m_nDeletePriorityQuestID = nQuestID; }
	void RefreshPriortyNotifier();

protected:
	CDnNotifierControls m_NotifierControls[DNNotifier::RegisterCount::TotalMission];
	typedef CEtUIDialog BaseClass;

	std::vector<int> m_renderIndices;
	std::map<int, float> m_fLastPos;
	SUICoord m_SelectCoord;
	SUICoord m_BlurCoord;

	int m_nClickIndex;
	int m_nOverIndex;
	int m_lastOverIndex;
	float m_fSelectAlpha;
	float m_fOverIndexSmooth;
	float m_fControlDistance;
	float m_lastHeight;
	bool m_bPriorityMode;
	int m_nDeletePriorityQuestID;
	CEtUIListBoxEx *m_pListBoxEx;
};

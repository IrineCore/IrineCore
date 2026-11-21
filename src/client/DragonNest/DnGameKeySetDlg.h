#pragma once

#include "EtUIDialog.h"
#include "DnGameControlTask.h"

class CDnGameKeySetListDlg;

class CDnGameKeySetDlg : public CEtUIDialog, public CEtUICallback
{
public:
	enum 
	{
		eControlType_Count = 6,
		eCategorySection_Count = CDnGameControlTask::eCategoryType_Max,
	};

	enum eNotice_Type
	{
		eNotice_Enable,
		eNotice_Disable,
		eNotice_Success,
		eNotice_Fail,
	};

	struct SNoticeData
	{
		bool bShow;
		float fShowTime;
		DWORD dwTextColor;
		std::wstring wszText;

		SNoticeData()
		{
			Reset();
		}

		void Reset()
		{
			bShow = false;
			fShowTime = -1.f;
			dwTextColor = textcolor::WHITE;
			wszText = std::wstring();
		}
	};

public:
	CDnGameKeySetDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnGameKeySetDlg();

	void SetNotice( const eNotice_Type eType );
	void ProcessNotice( float fElapsedTime );

	void RefreshListData();
	void ResetSelectListItem( const bool bNoticeDisable = true );

protected:
	void InitializeControlTypeTab();
	bool ChangeControlTypeTab( int nControlTabID );
	void SetCategoryTypeList( const int eCategoryIndex, std::vector<CDnGameControlTask::SKeyData *> & vKeyData );
	void SelectListCommand( CEtUIListBoxEx * pListBox );

protected:
	int m_nControlTabID;
	int m_nControlTabIndex;
	bool m_bApplyQuestion;

	CEtUIRadioButton * m_pButton_ControlType[eControlType_Count];
	CEtUIListBoxEx * m_pList_CategorySection[eCategorySection_Count];

	CEtUIButton * m_pButton_Default;
	CEtUIButton * m_pButton_Apply;
	CEtUIButton * m_pButton_Cancel;

	CEtUIStatic * m_pStatic_Notice;

	CDnGameKeySetListDlg * m_pSelectListItem;

	SNoticeData m_sNoticeData;

protected:
	CEtUICheckBox *m_pCheckSmartMove;
#ifdef PRE_ADD_AIM_ASSIST_BUTTON
	CEtUICheckBox *m_pCheckSmartMoveEx;
#endif

	CEtUIButton *m_pButtonDown;
	CEtUIButton *m_pButtonUp;
	CEtUISlider *m_pSliderMouse;
	CEtUIStatic *m_pStaticMouse;

	CEtUIButton *m_pButtonDownCamera;
	CEtUIButton *m_pButtonUpCamera;
	CEtUISlider *m_pSliderCamera;
	CEtUIStatic *m_pStaticCamera;

	CEtUIButton *m_pButtonDownCursor;
	CEtUIButton *m_pButtonUpCursor;
	CEtUISlider *m_pSliderCursor;
	CEtUIStatic *m_pStaticCursor;


	void _InitialUpdate_MouseControl();
	void _Export_MouseControl();
	void _Import_MouseControl();
	void _Reset_MouseControl();

public :
	virtual void Initialize(bool bShow);
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void Process( float fElapsedTime );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0 );
};
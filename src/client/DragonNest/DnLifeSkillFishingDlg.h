#pragma once
#include "DnCustomDlg.h"


#define FISHING_POWER_TIME	100.0f

struct stFishingInfo
{
	float m_fStrengthTime;		// HP gauge completion time
	float m_fPowerSpeedTime;	// Power gauge speed
	float m_fPowerTime;			// Power gauge completion time
	float m_fSpeedUpStartTime;	// Start time of the speed-up interval during power adjustment
	float m_fSpeedUpEndTime;	// End time of the speed-up interval during power adjustment
	float m_fSpeedUpFactor;		// Speed-up factor during power adjustment

	stFishingInfo() : m_fStrengthTime( 0.0f ), m_fPowerSpeedTime( 0.0f ), m_fPowerTime( 0.0f ), 
					m_fSpeedUpStartTime( 0.0f ), m_fSpeedUpEndTime( 0.0f ), m_fSpeedUpFactor( 0.0f ) {}
};

class CDnLifeSkillFishingDlg : public CDnCustomDlg, public CEtUICallback
{
public:
	CDnLifeSkillFishingDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnLifeSkillFishingDlg();

protected:
	CEtUIStatic* m_pStaticState;
	CEtUIStatic* m_pControlBar;
	CEtUIProgressBar* m_pProgressStrength;
	CDnFishingProgressBar* m_pProgressPower;
	CEtUIButton* m_pButtonControlGuide;
	CEtUIButton* m_pButtonAutoFishing;
	CEtUIButton* m_pButtonCancel;

	// Guid UI
	CEtUIStatic* m_pGuideBoard;
	CEtUIStatic* m_pGuideMouse;
	CEtUIStatic* m_pGuideMouseLeftButton;
	CEtUIStatic* m_pGuideMouseRightButton;
	CEtUIStatic* m_pGuideLeftChatBalloon;
	CEtUIStatic* m_pGuideLeftChatBalloonTail;
	CEtUIStatic* m_pGuideRightChatBalloon;
	CEtUIStatic* m_pGuideRightChatBalloonTail;
	CEtUIButton* m_pButtonCloseGuide;

	SUICoord m_ProgressStrengthUiCoord;

	bool m_bAutoFishing;			// Auto-fishing flag
	bool m_bShowGuide;				// Guide window flag
	bool m_bSpeedUp;				// Speed boost flag when left mouse button is pressed
	bool m_bDoFishingEnd;			// Flag when starting fishing
	bool m_bMouseLButtonDown;		// Left mouse button pressed flag
	bool m_bRecieveResult;

	stFishingInfo m_FishingInfo;
	float m_fElapsedStrengthTime;	// HP consumption time
	float m_fElapsedStrengthTemp;	// Temporary HP drain time without speed boost applied
	float m_fElapsedPowerTime;		// Power adjustment time

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void Process( float fElapsedTime );
	virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0 );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

public:
	void SetShowGuide( bool bShow );
	void SetFishingInfo( stFishingInfo& finshinginfo );
	bool IsAutoFishing() { return m_bAutoFishing; }
	void FishingTimeSync( int nReduceTick, int nFishingGauge );
	void RecieveResult( bool bRecieveResult );

private:
	void ShowGuide( bool bShow );
	bool IsValidFishingState();
	void ResetFishingInfo();
	void ToggleLeftChatBalloon( bool bShow );
	void ToggleRightChatBalloon( bool bShow );
};

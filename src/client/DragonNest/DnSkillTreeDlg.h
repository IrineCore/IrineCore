#pragma once
#include "DnCustomDlg.h"
#include "DnSkill.h"
#include "DnSmartMoveCursor.h"

//#undef PRE_ADD_SKILL_LEVELUP_RESERVATION

#ifdef PRE_ADD_TOTAL_LEVEL_SKILL
const int NUM_JOB_BTN = 6;
const int GUILD_BTN = 4;
const int TOTAL_LEVEL_SKILL = 5;
#else
const int NUM_JOB_BTN = 5;
#endif // PRE_ADD_TOTAL_LEVEL_SKILL

const int NUM_JOB_HIGHLIGHT = 3;


class CDnSkillTreeContentDlg;
class CDnLifeSkillContent;
class CSecondarySkill;
class CEtUIRadioButton;
class CDnSkillMovieDlg;
class CDnSkillGuideInitItemDlg;
class CDnSkillTreeButtonDlg;
class CDnSkillTreePreSetDlg;
class CDnSkillTreeSetMessageDlg;

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
class CDnTotalLevelSkillActivateListDlg;
#endif // PRE_ADD_TOTAL_LEVEL_SKILL

class CDnSkillTreeDlg : public CDnCustomDlg, public CEtUICallback
{
private:
	CDnSkillTreeContentDlg* m_pSkillTreeContentDlg;
	CDnLifeSkillContent* m_pLifeSkillContent;
	CEtUIListBoxEx* m_pContentListBox;
	CEtUIRadioButton* m_apDualSkillRadioButton[DualSkill::Type::MAX];
	CEtUIScrollBar* m_pScrollBar;
	CEtUIRadioButton* m_apJobRadioButton[ NUM_JOB_BTN ];
	CEtUIStatic * m_pJobHighLightStatic[ NUM_JOB_HIGHLIGHT ];
#if defined( PRE_ADD_SKILL_RESET_COMMENT )
	CDnSkillGuideInitItemDlg * m_pSkillGuideInitItemDlg;
	bool m_bResetComment;
#endif	// #if defined( PRE_ADD_SKILL_RESET_COMMENT )
#if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )
	CDnSkillTreeButtonDlg * m_pSkillTreeButtonDlg;
#endif	// #if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )
#if defined( PRE_ADD_PRESET_SKILLTREE )
	CDnSkillTreePreSetDlg * m_pSkillTreePreSetDlg;
	CDnSkillTreeSetMessageDlg * m_pSkillTreeSetMessageDlg;
	CEtUIComboBox * m_pPresetComboBox;
	bool m_bPresetSelect;
	bool m_bPresetDelEnable;
	bool m_bPresetModify;
#endif	// #if defined( PRE_ADD_PRESET_SKILLTREE )

	// Related to used skill points UI.
	CEtUIStatic* m_pWholeUsedSP;
	CEtUIStatic* m_pLeaveSP;
	DNVector(CEtUIStatic*) m_vlpUsedSPJobName;
	DNVector(CEtUIStatic*) m_vlpUsedSPByJob;	// Total skill points spent per job. Currently 4 are defined in the UI.
	DNVector(CEtUIStatic*) m_vlpJobLine;
	DNVector(CEtUIStatic*) m_vlpHighLight;

	float m_fOriContentDlgXPos;
	float m_fOriContentDlgYPos;
	float m_fContentWidth;
	float m_fContentHeight;

	int m_iSelectedJobBtnID;
	int m_iSelectedJobID;

	int m_iSkillTreeScrollPos;		// #28673 Life skill shares the list box, so save the position separately since resetting the list box loses it.

	CDnSmartMoveCursorEx m_SmartMoveEx;
	CDnSkillMovieDlg*	m_pSkillMovieDlg;
	CEtUIStatic*	m_pStaticTitle;	// Top description section
	bool	m_bGuildSkillMode;		// If in guild skill mode
	bool	m_bSetGuildSkill;		// Whether guild skill can be modified

	bool m_bOpendSkillPage[DualSkill::Type::MAX];
	int m_iSelectedSkillPage;
	float m_fSkillPageDelay;

protected:
	void _SetContentOffset( void );

public:
	CDnSkillTreeDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnSkillTreeDlg(void);

	// from CEtUIDialog
	void Initialize( bool bShow );
	CEtUIListBoxEx* GetSkillListBox() { return m_pContentListBox; }
	void SetContentPanelDlg( CDnSkillTreeContentDlg* pSkillTreeContentDlg );
#ifdef PRE_FIX_SKILL_TOOLTIP_REFRESH
	CDnSkillTreeContentDlg* GetContentPanelDlg() { return m_pSkillTreeContentDlg; }
#endif
	void LifeSkillAdd( CSecondarySkill* pAddSecondarySkill );
	void LifeSkillDelete( int nSkillID );
	void LifeSkillUpdateExp( int nSkillID, SecondarySkill::Grade::eType eGrade, int nLevel, int nExp );
	void InitialUpdate();
	//CEtUIControl* CreateControl( SUIControlProperty *pProperty );
	bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	void Render( float fElapsedTime );
	void Process( float fElapsedTime );

	void GetContentDialog( /*IN OUT*/ std::vector<CEtUIDialog*>& vlpContentDialogs );
	void GetContentDialogCoord( /*IN OUT*/ SUICoord& Coord );

	void Show( bool bShow );
	void ShowGuildSkillPointInfo();
	// Move scroll directly with mouse wheel.
	void OnMouseWheel( int iScrollAmount );
	void UpdateView( void );

	void _UpdateSelectionContentDlg( void );
	int GetSelectedJobID( void ) { return m_iSelectedJobID; };

	void SetUnlockByMoneyMode( void );
	bool IsUnlockByMoneyMode();

	void ShowSkillMovieDlg(bool bShow, const std::wstring& skillName, const std::string& fileName);

	void ResetCheckedButton();
	void SetGuildSkillMode( bool bGuildSkillMode, bool bSetGuildSkill );
	void UpdateGuildWarSkillPointInfo();
	void UpdateGuildWarSkillTreeContent();

	void SetSkillPageDelay(float fDelay);
	void ProcessSkillPageDelay(float fElapsedTime);
	void SelectSkillPage(int nSkillPage);
	void EnableSkillPage(int nSkillPage ,bool bEnable = true);
	bool IsEnableSkillPage(int nSkillPage);

	void _Show_Highlight_Job( const int nNeedJobClassID );
	void _Hide_Highlight_Job();
	void _Show_Highlight_SP( std::vector<int> & nNeedSPValues );
	void _Hide_Highlight_SP();

	virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0 );

#if defined( PRE_ADD_SKILL_RESET_COMMENT )
	bool IsResetCommentShow();
#endif	// #if defined( PRE_ADD_SKILL_RESET_COMMENT )

#if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )
	void SetSkillButtonDlg( CDnSkillTreeButtonDlg * pSkillTreeButtonDlg );

	void ButtonStateSkillReservation();
	void ResetSkillReservation();

	void ResetButtonCommand();
	void ApplyButtonCommand();
#endif	//	#if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )

#if defined( PRE_ADD_PRESET_SKILLTREE )
	void SetSkillTreePreSetDlg( CDnSkillTreePreSetDlg * pSkillTreePreSetDlg );
	void SetSkillTreeSetMessageDlg( CDnSkillTreeSetMessageDlg * pSkillTreeSetMessageDlg );
	void RefreshSkillTreePresetComboBox();
	void PresetSkillProcess( const BYTE cIndex );
	void ButtonStateSkillTreePreset();
	void ShowPresetSkillTreeSaveMessage();
	void ShowPresetSkillTreeDelMessage();
	void PresetSkillTreeSave( std::wstring wszName );
	void PresetSkillTreeDel();
	void SetPresetModify( const bool bPresetModify ) { m_bPresetModify = bPresetModify; }
#endif	// #if defined( PRE_ADD_PRESET_SKILLTREE )

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
protected:
	CEtUIStatic* m_pTotalLevelInfo;
	CDnTotalLevelSkillActivateListDlg* m_pTotalLevelSkillActivateDlg;
	CEtUIListBoxEx* m_pTotalLevelSkillContentListBox;

	void UpdateTotalLevel();

public:
	CDnTotalLevelSkillActivateListDlg* GetTotalLevelSkillActivateListDlg() { return m_pTotalLevelSkillActivateDlg; }

	void UpdateTotalLevelSkillTreeContent();
	void AddTotalLevelSkill(int nSlotIndex, DnSkillHandle hSkill);
	void RemoveTotalLevelSkill(int nSlotIndex);

	void ActivateTotalLevelSkillSlot(int nSlotIndex, bool bActivate);
	void ActivateTotalLevelSkillCashSlot(int nSlotIndex, bool bActivate, __time64_t tExpireDate);

#endif // PRE_ADD_TOTAL_LEVEL_SKILL
};

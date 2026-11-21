#pragma once
#include "DnCustomDlg.h"
#include "DnSkill.h"
#include "DnSkillTask.h"

//#define DRAW_AUTO_TREE_LINE

//#undef PRE_ADD_SKILL_LEVELUP_RESERVATION

class CDnSkillTreeDlg;
class CDnSkillAcquireDlg;
class CDnSkillLevelUpDlg;
class CDnSkillUnlockDlg;

class CDnSkillTreeContentDlg : public CDnCustomDlg, public CEtUICallback
{
private:
	enum
	{
		SKILL_ACQUIRE_DIALOG,
		SKILL_LEVELUP_DIALOG,
		SKILL_UNLOCK_DIALOG,
	};

	struct S_SKILL_SLOT_UI
	{
		CEtUIStatic* pSlotBase;
		CEtUIStatic* pPassiveSlotBase;
		CDnQuickSlotButton* pSkillQuickSlot;
		CEtUIStatic* pSkillLevel;
		CDnSkillAcquireButton* pSkillLockButton;
		CDnSkillAcquireButton* pExclusiveSkillLockButton;
		CEtUIButton* pSkillUpButton;
#if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )
		CEtUIButton* pSkillDownButton;
#endif	// #if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )
		CEtUIStatic * pHighlightStatic;

		S_SKILL_SLOT_UI( void ) : pSlotBase( NULL ), pPassiveSlotBase( NULL ), pSkillQuickSlot( NULL ), 
								  pSkillLevel( NULL ), pSkillLockButton( NULL ), pSkillUpButton( NULL ),
								  pExclusiveSkillLockButton( NULL ), pHighlightStatic( NULL )
		{}

		void Reset( void )
		{
			pSkillQuickSlot->ResetSlot();
			pSkillLevel->ClearText();

			pSlotBase->Show( false );
			pPassiveSlotBase->Show( false );
			pSkillQuickSlot->Show( false );
			pSkillLevel->Show( false );
			pSkillLockButton->Show( false );
			pExclusiveSkillLockButton->Show( false );
			pSkillUpButton->Show( false );
#if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )
			pSkillDownButton->Show( false );
#endif	// #if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )
			pHighlightStatic->Show( false );
		}
	};

	vector<S_SKILL_SLOT_UI> m_vlSkillSlot;
	
	// Dialog wrapping this dialog
	CDnSkillTreeDlg* m_pSkillTreeDlg;

	// Skill unlock confirmation dialog
	CDnSkillUnlockDlg* m_pSkillUnlockDlg;
	
	// Skill level-up confirmation dialog
	CDnSkillLevelUpDlg* m_pSkillLevelUpDlg;

	// Skill acquisition confirmation dialog
	CDnSkillAcquireDlg* m_pSkillAcquireDlg;

	int m_iSelectedSlotIndex;

	// Textures related to automatic arrow rendering..
#ifdef DRAW_AUTO_TREE_LINE
	EtTextureHandle m_hLineDownTexture;
	EtTextureHandle m_hLineLeftTexture;
	EtTextureHandle m_hLineRightTexture;
	EtTextureHandle m_hArrowDownTexture;
	EtTextureHandle m_hArrowLeftTexture;
	EtTextureHandle m_hArrowRightTexture;
#endif
	//

	// Name linked to UI per job. Based on the name in the job table.
	// If a name in the Job table changes, the UI tree line static must also be updated accordingly.
	map<int, string> m_mapJobUIStatic;
	//vector<CEtUIStatic*> m_vlpTreeLineStatics;

	int m_iDragSoundIndex;

	bool m_bUnlockByMoneyMode;

	// Tree texture control
	EtTextureHandle m_hTreeTexture;
	CEtUITextureControl* m_pTreeTextureCtrl;
	bool m_bGuildWarSkillMode;	
#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
	bool m_bTotalLevelSkillMode;
#endif // PRE_ADD_TOTAL_LEVEL_SKILL

protected:
#ifdef DRAW_AUTO_TREE_LINE
	void _DrawTreeLeftLine( const SUICoord FromRight, const SUICoord ToLeft, bool bRenderArrow = true );
	void _DrawTreeRightLine( const SUICoord FromLeft, const SUICoord ToRight, bool bRenderArrow = true );
	void _DrawTreeDownLine( const SUICoord FromUp, const SUICoord ToDown, bool bRenderArrow = true );
#endif

public:
	CDnSkillTreeContentDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback );
	virtual ~CDnSkillTreeContentDlg(void);

	void Initialize( bool bShow );
	void InitialUpdate( void );
	void InitCustomControl( CEtUIControl *pControl );
	bool OnMouseMove( float fX, float fY );
	void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg /* = 0 */ );
	void SetUnlockByMoneyMode( bool bUnlockByMoneyMode );
	bool IsUnlockByMoneyMode();

	void Render( float fElapsedTime );

#ifdef DRAW_AUTO_TREE_LINE
	void _DrawAutoTreeLine( void );
#endif
	void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg /* = 0 */ );
	bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

	void SetSkillTreeDlg( CDnSkillTreeDlg* pSkillTreeDlg ) { m_pSkillTreeDlg = pSkillTreeDlg; };
	void UpdateGuildWarSkillTree( bool bSetGuildSkill );

	void UpdateSkillTree( void );
	void OnSkillLevelUp( bool bSuccess );

	void CloseChildDialog( void );

	void UnLockSkill( CDnSkill * pSkill );

	void ShowSlotInformation( const float fX, const float fY );
	void Show_SlotTooltipDlg( const float fX, const float fY, CDnQuickSlotButton * pControl );
	void Show_Highlight( CDnQuickSlotButton * pControl );
	void Show_Highlight_ParentSkill( CDnSkill * pSkill );
	void Show_Highlight_SP_Point( CDnSkill * pSkill );

	bool IsParentSkill_Complete( const int nParentSkillID, const int nParentSkillLevel );
	bool IsSP_Complete( DnSkillHandle hParentSkill );

	void HideSlotInformation();
	void Hide_SlotTooltipDlg();
	void Hide_Highlight();

#ifdef PRE_FIX_SKILL_TOOLTIP_REFRESH
	void RefreshToolTip(CDnSkill* pSkill);
#endif

	void Update_SkillSlotUI( CDnSkillTask::S_SKILLTREE_SLOT_INFO & SlotInfo, const int nSlotIndex );

	virtual bool FindControl( std::vector< CEtUIControl* > &vecControl, int nTypeCount, UI_CONTROL_TYPE *pType, bool bCheckCoveredControl, std::vector<SUICoord> &vecDlgCoord );

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
public:
	void UpdateTotalLevelSkillTree( bool bSetTotalLevelSkill );
#endif // PRE_ADD_TOTAL_LEVEL_SKILL

};

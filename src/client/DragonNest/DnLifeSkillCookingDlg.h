#pragma once
#include "DnCustomDlg.h"

#ifdef PRE_ADD_COOKING_SYSTEM

#define MAX_COOKING_LIST	4
#define MAX_REQUIRED_NUM	5

class CDnLifeSkillCookingRecipeDlg;
class CSecondarySkillRecipe;
class CDnItem;

class CDnLifeSkillCookingDlg : public CDnCustomDlg, public CEtUICallback
{
public:
	CDnLifeSkillCookingDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnLifeSkillCookingDlg();

	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0 );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

public:
	void ClearSelectedRecipe();
	void UpdateCookingList();
	void ResetSelectedRecipeInfo();

private:
	CSecondarySkillRecipe* GetCurrentCookRecipe();
	void SetSelectedRecipeInfo();
	void UpdatePageUI();

private:
	enum { MSGBOX_DELETE_RECIPE, MSGBOX_EXTRACT_RECIPE };

	CDnLifeSkillCookingRecipeDlg* m_pDnLifeSkillCookingRecipeDlg;

	struct stUICookingListInfo
	{
		CDnItemSlotButton*	m_pCookingSlotBtn;		// Cooking icon
		CEtUIStatic*		m_pStaticTitle;			// Dish title
		CEtUIStatic*		m_pStaticLevel;			// Level restriction
		CEtUIStatic*		m_pStaticPercent;		// Proficiency rate
		CEtUIStatic*		m_pStaticSelect;		// Selection indicator
		CEtUIStatic*		m_pStaticBackground;	// Background image
		CEtUIStatic*		m_pStaticLevelText;		// Level limit text
		CEtUIProgressBar*	m_pProgressAchievement;	// Proficiency progress bar

		stUICookingListInfo() : m_pCookingSlotBtn( NULL ), m_pStaticTitle( NULL ), m_pStaticLevel( NULL ), m_pStaticPercent( NULL ), 
			m_pStaticSelect( NULL ), m_pStaticBackground( NULL ), m_pStaticLevelText( NULL ), m_pProgressAchievement( NULL )
		{
		}
	};

	stUICookingListInfo m_stUICookingListInfo[ MAX_COOKING_LIST ];

	CEtUIButton* m_pButtonAddRecipe;		// Add recipe
	CEtUIButton* m_pButtonRemoveRecipe;		// Delete recipe
	CEtUIButton* m_pButtonCook;				// Cook
	CEtUIButton* m_pButtonCookAll;			// Cook all
	CEtUIButton* m_pButtonExtract;			// Extract dish
	CEtUIButton* m_pButtonCancel;			// Cancel cooking (close window)
	CEtUIButton* m_pButtonPrevious;			// Previous cooking list
	CEtUIButton* m_pButtonNext;				// Next cooking list
	CEtUIStatic* m_pStaticPage;				// Cooking page

	// Display selected recipe info
	CDnItemSlotButton*	m_pCookingSlotBtn;		// Cooking icon
	CEtUIStatic*		m_pStaticTitle;			// Dish title
	CEtUIStatic*		m_pStaticLevel;			// Level restriction
	CEtUIStatic*		m_pStaticCompletion;	// Cooking perfection

	CDnItemSlotButton*	m_pRequiredToolSlotBtn[ MAX_REQUIRED_NUM ];			// Required tool icon
	CDnItemSlotButton*	m_pRequiredIngredientSlotBtn[ MAX_REQUIRED_NUM ];	// Required material icon

	int m_nCurrentPage;				// Current page
	int m_nMaximumPage;				// Max page
	int m_nCurrentPageListCount;	// Number of items on current page
	int m_nSelectRecipe;			// Selected item
};

#define MAX_RECIPE_LIST	16

class CDnLifeSkillCookingRecipeDlg : public CDnCustomDlg, public CEtUICallback
{
public:
	CDnLifeSkillCookingRecipeDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnLifeSkillCookingRecipeDlg();

	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0 );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

private:
	void UpdaetRecipeList();
	void SetRecipePageList();
	void UpdatePageUI();

private:
	std::vector<CDnItem*>	m_vecRecipeIDList;					// Recipe item list

	CDnItemSlotButton*	m_pRecipeSlotBtn[ MAX_RECIPE_LIST ];	// Recipe icon
	CEtUIStatic*		m_pRecipeSelect[ MAX_RECIPE_LIST ];		// Display recipe selection
	CEtUIButton*		m_pButtonPrevious;						// Previous recipe list
	CEtUIButton*		m_pButtonNext;							// Next recipe list
	CEtUIStatic*		m_pStaticPage;							// Cooking page
	CEtUIButton*		m_pButtonAddRecipe;						// Add recipe

	int m_nCurrentPage;				// Current page
	int m_nMaximumPage;				// Max page
	int m_nCurrentPageListCount;	// Number of items on current page
	int m_nSelectRecipe;			// Selected item
};

#endif // PRE_ADD_COOKING_SYSTEM
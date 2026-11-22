#pragma once

#if defined( PRE_ADD_DWC )

#include "EtUIDialog.h"

class CDnDWCCharCreateDlg;

class CDnDWCCharCreateButtonDlg : public CEtUIDialog, public CEtUICallback
{
public:
	CDnDWCCharCreateButtonDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnDWCCharCreateButtonDlg();

protected:
	CDnDWCCharCreateDlg *m_pParentBaseDlg;

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );

	void SetParentBaseDlg( CDnDWCCharCreateDlg *pDlg ) { m_pParentBaseDlg = pDlg; }
};

class CDnDWCCharCreateSelectDlg : public CEtUIDialog
{
public:
	enum {
		MAX_DWC_CLASS_RADIO = 10,
	};

	struct SDWCClassPreset {
		int nRowID;
		int nClassID;
		int nJobCode;
		int nJobCode2;
		int nJobNameUIStringID;
		int nLevel;
		int nExp;
		int nCreateMapIndex;
		int nGold;
		int nSkillPoint;
		std::string strClassImage;
		SDWCClassPreset() : nRowID( 0 ), nClassID( 0 ), nJobCode( 0 ), nJobCode2( 0 ), nJobNameUIStringID( 0 ), nLevel( 60 ), nExp( 0 ), nCreateMapIndex( 0 ), nGold( 0 ), nSkillPoint( 0 ) {}
	};

public:
	CDnDWCCharCreateSelectDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnDWCCharCreateSelectDlg();

protected:
	std::vector<CEtUIRadioButton*> m_vecRadioClass;
	CEtUIComboBox *m_pComboChar;
	std::vector<SDWCClassPreset> m_vecPresets;
	int m_nSelectedIndex;
	bool m_bSyncingSelection;
	CDnDWCCharCreateDlg *m_pParentBaseDlg;

	void LoadPresets();

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );

	void SetParentBaseDlg( CDnDWCCharCreateDlg *pDlg ) { m_pParentBaseDlg = pDlg; }
	const SDWCClassPreset *GetSelectedPreset() const;
	int GetSelectedClassID() const;
	int GetSelectedJobCode() const;
};

class CDnDWCCharCreateDlg : public CEtUIDialog
{
public:
	CDnDWCCharCreateDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnDWCCharCreateDlg();

protected:
	CDnDWCCharCreateButtonDlg *m_pButtonDlg;
	CDnDWCCharCreateSelectDlg *m_pSelectDlg;
	CEtUITextureControl *m_pTextureClass;
	CEtUIStatic *m_pStaticName;
	EtTextureHandle m_hClassTexture;

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

	void OnSelectPreset( const CDnDWCCharCreateSelectDlg::SDWCClassPreset *pPreset );
	CDnDWCCharCreateSelectDlg *GetSelectDlg() const { return m_pSelectDlg; }
};

#endif // PRE_ADD_DWC

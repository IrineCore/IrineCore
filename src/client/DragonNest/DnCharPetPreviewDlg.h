#pragma once
#include "EtUIDialog.h"
#include "DnRenderAvatar.h"



class CDnCharPetPreviewDlg : public CEtUIDialog
{
public:
	CDnCharPetPreviewDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnCharPetPreviewDlg();

	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void Process( float fElapsedTime );
	virtual void Render( float fElapsedTime );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

	void RefreshPetEquip();
	void SetAvatarAction( const char* szActionName );

protected:
	CEtUIButton* m_pButtonRotateLeft;
	CEtUIButton* m_pButtonRotateRight;
	CEtUIButton* m_pButtonRotateFront;
	CEtUIButton* m_pButtonRotateBack;
	CEtUIButton* m_pButtonAvatarViewArea;

	CDnRenderAvatarNew m_RenderAvatar;

	float m_fAvatarViewMouseX;
	float m_fAvatarViewMouseY;
};

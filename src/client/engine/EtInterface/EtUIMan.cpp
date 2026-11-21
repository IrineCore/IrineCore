#include "StdAfx.h"
#include "EtUIMan.h"
#include "EtUIControl.h"
#include "EtUIDialog.h"
#include "EtUIXML.h"
#include "EtPrimitiveDraw.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

extern bool g_bSkipScene;

namespace EtInterface
{
	bool g_bEtUIInitTool = false;
	bool g_bEtUIRender = true;
	bool g_bEtUIMsgProc = true;
	bool g_bEtUIProcess = true;

	CEtFontMng *g_pFontMng = NULL;
	CEtUIDialog *g_pFocusDialog = NULL;
	CEtUINameLinkMgr* g_pNameLinkMgr = NULL;

	void Initialize()
	{
		// Not sure why, but in release mode once IME is unloaded, reloading the DLL fails to re-enable IME, so
		// Increment once at the very beginning to prevent RefCount from ever reaching 0.
		CEtUIIME::Initialize();
		g_pFontMng = new CEtFontMng();
		g_pNameLinkMgr = new CEtUINameLinkMgr;
	}

	void Finalize()
	{
		DeleteAllDialog();
		SAFE_DELETE( g_pFontMng );
		SAFE_DELETE(g_pNameLinkMgr);
		CEtUIIME::Uninitialize();
	}

	void Process( float fElapsedTime )
	{
		if( !g_bEtUIProcess ) return;
		CEtUIDialogBase::ProcessAll( fElapsedTime );
	}

	void Render( float fElapsedTime )
	{
		if( g_bSkipScene ) return;

		bool bRender = false;
		if( g_bEtUIRender ) bRender = true;
		if( !g_bEtUIRender && CEtUIDialog::s_pFadeDlg && CEtUIDialog::s_pFadeDlg->IsShow() ) bRender = true;
		if( !bRender ) return;

		CEtDevice *pEtDevice = GetEtDevice();
		if( !pEtDevice ) return;	

		IDirect3DDevice9 *pd3dDevice = (IDirect3DDevice9*)pEtDevice->GetDevicePtr();
		if( !pd3dDevice ) return;

		pEtDevice->EnableAlphaBlend( true );
		CEtSprite::GetInstance().Begin( D3DXSPRITE_DONOTSAVESTATE );
		GetEtDevice()->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
		GetEtDevice()->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );

		if( g_bEtUIRender )
		{
			CEtUIDialogBase::RenderAll( fElapsedTime );
			drag::Render( fElapsedTime );
		}
		else
		{
			if( CEtUIDialog::s_pFadeDlg )
				CEtUIDialog::s_pFadeDlg->Render( fElapsedTime );
		}

		GetEtDevice()->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP );
		GetEtDevice()->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP );
		CEtSprite::GetInstance().End();
		pEtDevice->EnableAlphaBlend( false );
	}

	bool StaticMsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
	{
		if( !g_bEtUIMsgProc )
			return false;

		if( !g_bEtUIRender )
			return false;

		g_pFocusDialog = NULL;

		if( focus::IsValid() )
		{
			if( ( focus::GetType() == UI_CONTROL_IMEEDITBOX ) || ( focus::GetType() == UI_CONTROL_LINE_IMEEDITBOX ) )
			{
				if( CEtUIIME::StaticMsgProc( uMsg, wParam, lParam ) )
					return true;
			}
		}

		// Note : Process windows registered as modal with priority.
		//
		if( CEtUIDialogBase::MsgProcModal( hWnd, uMsg, wParam, lParam ) )
			return true;

		if( CEtUIDialogBase::StaticMsgProc( hWnd, uMsg, wParam, lParam ) )
			return true;

		// Note : Release drag on mouse clicks that are not handled by the UI.
		//
		if( uMsg == WM_RBUTTONDOWN )
		{
			drag::Command(UI_DRAG_CMD_CANCEL);
			drag::ReleaseControl();
		}
		else if( uMsg == WM_LBUTTONDOWN )
		{
			if( drag::IsValid() )
			{
				// Note : Discard the item into the trash.
				//		Actual item discard handling is done in the inventory dialog.
				//
				trash::SetControl( drag::GetControl() );
			}
		}

		return false;
	}

	void DeleteAllDialog()
	{
		// Note : If it crashes here, a dialog was created and not deleted.
		//
		CSmartPtrBase< CEtUIDialog >::DeleteAllObject();
	}

	const CEtUINameLinkMgr& GetNameLinkMgr()
	{
		_ASSERT(g_pNameLinkMgr);
		return *g_pNameLinkMgr;
	}

	namespace xml
	{
		CEtUIXML *s_pEtUIXML(NULL);

		CEtUIXML *GetXML() {return s_pEtUIXML; }
		void SetXML( CEtUIXML *pXML )
		{
			ASSERT(pXML&&"EtInterface::xml::SetEtUIXML");
			s_pEtUIXML = pXML;
		}
		bool IsValid() {return s_pEtUIXML!=NULL; }
		const wchar_t* GetString( int nCategoryID, int nIndex )
		{
			return s_pEtUIXML->GetUIString( (CEtUIXML::emCategoryId)nCategoryID, nIndex );
		}
	}

	namespace drag
	{
		CEtUIControl*	s_pDragControl(NULL);
		bool			bRender = true;

		void SetControl( CEtUIControl *pControl )		
		{ 
			ASSERT(pControl&&"EtInterface::drag::SetControl");
			s_pDragControl = pControl; 
			bRender = true;
			focus::ReleaseControl();
		}

		CEtUIControl *GetControl()						{ return s_pDragControl; }
		void Command(UIDragCmdParam param)				{ if (s_pDragControl) s_pDragControl->OnCmdDrag(param); }
		void ReleaseControl()							{ s_pDragControl = NULL; }
		bool IsValid()									{ return s_pDragControl!=NULL; }
		void SetRender(bool enable)						{ bRender = enable; }

		void Render( float fElapsedTime )
		{
			if( !s_pDragControl || bRender == false ) return;
			s_pDragControl->RenderDrag( fElapsedTime );
		}
	}

	namespace trash
	{
		CEtUIControl *s_pTrashControl(NULL);

		void SetControl( CEtUIControl *pControl )		
		{ 
			ASSERT(pControl&&"EtInterface::trash::SetControl");
			s_pTrashControl = pControl;
			focus::ReleaseControl();
		}

		CEtUIControl *GetControl()						{ return s_pTrashControl; }
		void ReleaseControl()							{ s_pTrashControl = NULL; }
		bool IsValid()									{ return s_pTrashControl!=NULL; }
	}

	namespace focus
	{
		CEtUIControl *s_pFocusControl(NULL);
		std::vector< CEtUIControl * > s_vecPrevFocusControl;

		std::vector<CEtUIControl*>& GetPrevControl()	{ return s_vecPrevFocusControl; }

		void PushControl(CEtUIDialog *pDialog)
		{
			// If a dialog other than the one with the currently set s_pFocusControl arrives,
			if( !IsSameParent(pDialog) )
			{
				// The comparison was !=, which makes no logical sense.
				// Searching if not registered to register it, shouldn't it be ==?
				// However, focus analysis is still incomplete and many things behave strangely, so
				// Decided to just leave it as is.
				if( std::find( s_vecPrevFocusControl.begin(), s_vecPrevFocusControl.end(), s_pFocusControl ) != s_vecPrevFocusControl.end() )
				{
					s_vecPrevFocusControl.push_back( s_pFocusControl );
					UnsetFocus();
					ReleaseControl();
				}
			}
		}

		void PopControl(CEtUIDialog *pDialog)
		{
			// Inferring the creator's intent,
			// It seems this pop is meant to run when the last opened window is immediately Show( false ) after Push without another window opening,
			//
			// Do not request focus on a specific control when a message box or most other windows open (RequestFocus)
			// At this point s_pFocusControl is NULL,
			// Fails the check below.
			// Because s_pFocusControl is NULL.. Strange code.
			if( IsSameParent(pDialog) )
			{
				ReleaseControl();

				if( !s_vecPrevFocusControl.empty() )
				{
					CEtUIControl *pFocusControl;
					pFocusControl = s_vecPrevFocusControl[ s_vecPrevFocusControl.size() - 1 ];
					//pFocusControl->GetParent()->RequestFocus( pFocusControl );
					// Seems it was safe because Push was never called even once.
					if ( pFocusControl ) pFocusControl->GetParent()->RequestFocus( pFocusControl );
					s_vecPrevFocusControl.pop_back();
				}
			}
			else
			{
				// Instead of the newly opened window being set to Show(false),
				// If a window opened in the middle of the order becomes Show(false), search the focus list and remove all of it.
				// (If dialogs didn't request randomly, removing just one would probably suffice.)
				for( int i = 0; i < ( int )s_vecPrevFocusControl.size(); i++ )
				{
					if( s_vecPrevFocusControl[ i ] && s_vecPrevFocusControl[ i ]->GetParent() == pDialog )
					{
						s_vecPrevFocusControl.erase( s_vecPrevFocusControl.begin() + i );
						break;
					}
				}
			}
		}

		CEtUIControl* GetControl()						
		{ 
			return s_pFocusControl; 
		}

		void SetControl(CEtUIControl* pControl)			
		{ 
			ASSERT(pControl&&"EtInterface::focus::SetControl");
			if( s_pFocusControl ) ReleaseControl();
			s_pFocusControl=pControl; 
		}

		void ReleaseControl()
		{
			if( s_pFocusControl )
			{
				s_pFocusControl->Focus(false);
				OnReleaseControl();
			}

			s_pFocusControl=NULL;
		}

		void OnReleaseControl()
		{ 
			// Perform additional handling here if needed when released from focused state.
			if( s_pFocusControl )
			{
				if( s_pFocusControl->GetParent() && s_pFocusControl->GetParent()->IsElementDialog() == false )
				{
					if( s_pFocusControl->IsPressed() || s_pFocusControl->IsRightPressed() || s_pFocusControl->IsHaveScrollBarControl() )
					{
						s_pFocusControl->SetPressed( false );
						s_pFocusControl->SetRightPressed( false );
						ReleaseCapture();
					}
				}
			}
		}

		bool IsValid()
		{ 
			return s_pFocusControl!=NULL; 
		}

		bool IsEnable()									
		{
			if( s_pFocusControl )
				return s_pFocusControl->IsEnable();

			return false;
		}

		bool IsSameParent( CEtUIDialog *pDlg)
		{ 
			ASSERT(pDlg&&"EtInterface::focus::IsSameParent");

			if( s_pFocusControl == NULL )
				return false;

			return (s_pFocusControl->GetParent() == pDlg);
		}

		bool IsSameControl( CEtUIControl* pControl )
		{
			ASSERT(pControl&&"EtInterface::focus::IsSameControl, pControl is NULL!");
			return s_pFocusControl==pControl;
		}

		void SetFocus()									
		{
			if( s_pFocusControl )
			{
				s_pFocusControl->Focus(true);
			}
		}

		void SetFocus(CEtUIControl* pControl)
		{
			ASSERT(pControl&&"EtInterface::focus::SetFocus");
			if (IsSameControl(pControl) == false)
			{
				ReleaseControl();
				s_pFocusControl = pControl;
				s_pFocusControl->Focus(true);
			}
		}

		void UnsetFocus()								
		{ 
			if( s_pFocusControl )
			{
				s_pFocusControl->Focus(false);
			}
		}
		
		CEtUIDialog* GetParent()						
		{
			if( s_pFocusControl )
			{
				return s_pFocusControl->GetParent();
			}

			return NULL;
		}

		UI_CONTROL_TYPE GetType()						
		{ 
			if( s_pFocusControl )
			{
				return s_pFocusControl->GetType();
			}

			return UI_CONTROL_SIZE;
		}

		bool MsgProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
		{ 
			if( s_pFocusControl )
			{
				return s_pFocusControl->MsgProc(uMsg, wParam, lParam);
			}

			return false;
		}

		bool HandleKeyboard( UINT uMsg, WPARAM wParam, LPARAM lParam )
		{
			if( s_pFocusControl )
				return s_pFocusControl->HandleKeyboard(uMsg, wParam, lParam);

			return false;
		}

		bool HandleMouse( UINT uMsg, float fX, float fY, WPARAM wParam, LPARAM lParam )
		{
			if( s_pFocusControl )
				return s_pFocusControl->HandleMouse(uMsg, fX, fY, wParam, lParam);

			return false;
		}
	}
}
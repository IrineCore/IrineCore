#pragma once

class CEtUIDialog;
class CEtUIControl;
class CEtUIXML;
#include "EtUIType.h"
#include "EtUINameLinkMgr.h"

namespace EtInterface
{
	extern void Initialize();
	extern void Finalize();
	extern void Process( float fElapsedTime );
	extern void Render( float fElapsedTime );
	extern bool StaticMsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

	extern void DeleteAllDialog();

	extern bool g_bEtUIInitTool;
	extern bool g_bEtUIRender;
	extern bool g_bEtUIMsgProc;
	extern bool g_bEtUIProcess;
	extern CEtUIDialog *g_pFocusDialog;
	extern const CEtUINameLinkMgr& GetNameLinkMgr();

	// Note : Sets and holds CEtUIXML for using XML data in the tool.
	//
	namespace xml
	{
		extern CEtUIXML *s_pEtUIXML;

		extern CEtUIXML *GetXML();
		extern void SetXML( CEtUIXML *pXML );
		extern bool IsValid();
		extern const wchar_t* GetString( int nCategoryID, int nIndex );
	}

	namespace drag
	{
		extern CEtUIControl *s_pDragControl;

		extern void SetControl( CEtUIControl *pControl );
		extern CEtUIControl *GetControl();
		extern void Command(UIDragCmdParam param);	//	todo : Method added as needed on ReleaseControl(). Consider renaming to OnReleaseControl().
		extern void ReleaseControl();
		extern bool IsValid();
		extern void SetRender(bool enable);
		extern void Render( float fElapsedTime );
	}

	namespace trash
	{
		extern CEtUIControl *s_pTrashControl;

		extern void SetControl( CEtUIControl *pControl );
		extern CEtUIControl *GetControl();
		extern void ReleaseControl();
		extern bool IsValid();
	}

	namespace focus
	{
		extern std::vector< CEtUIControl * > s_vecPrevFocusControl;
		extern CEtUIControl *s_pFocusControl;

		extern std::vector<CEtUIControl*>& GetPrevControl();

		extern void PushControl(CEtUIDialog *pDialog);
		extern void PopControl(CEtUIDialog *pDialog);

		extern CEtUIControl* GetControl();
		extern void SetControl(CEtUIControl* pControl);
		extern void ReleaseControl();
		extern void OnReleaseControl();

		extern bool IsValid();
		extern bool IsEnable();
		extern bool IsSameParent( CEtUIDialog *pDlg);
		extern bool IsSameControl( CEtUIControl* pControl );

		extern void SetFocus();
		extern void SetFocus(CEtUIControl* pControl);
		extern void UnsetFocus();
		
		extern CEtUIDialog* GetParent();
		extern UI_CONTROL_TYPE GetType();

		extern bool MsgProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
		extern bool HandleKeyboard( UINT uMsg, WPARAM wParam, LPARAM lParam );
		extern bool HandleMouse( UINT uMsg, float fX, float fY, WPARAM wParam, LPARAM lParam );
	}
}

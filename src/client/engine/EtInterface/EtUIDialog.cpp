#include "StdAfx.h"
#include "EtUIDialog.h"
#include "EtUIControl.h"
#include "EtUIStatic.h"
#include "EtUIButton.h"
#include "EtUICheckBox.h"
#include "EtUIRadioButton.h"
#include "EtUIScrollBar.h"
#include "EtUISlider.h"
#include "EtUIListBox.h"
#include "EtUIComboBox.h"
#include "EtUIEditBox.h"
#include "EtUIIMEEditBox.h"
#include "EtUIProgressBar.h"
#include "EtUITemplate.h"
#include "EtUIMan.h"
#include "EtUIControlCreator.h"
#include "EtUISound.h"
#include "DebugSet.h"
#include "EtResourceMng.h"
#include "EtType.h"
#include "EtLoader.h"
#include "EtFontMng.h"
#include "EtSprite.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CEtUIDialog *CEtUIDialog::s_pTooltipDlg = NULL;
int CEtUIDialog::s_nTooltipStringIndex = 0;
std::wstring CEtUIDialog::s_strTooltipString = L"";

CEtUIDialog *CEtUIDialog::s_pFadeDlg = NULL;

float CEtUIDialog::s_fScreenMouseX = 0.0f;
float CEtUIDialog::s_fScreenMouseY = 0.0f;

float CEtUIDialog::s_fDialogScale = 1.0f;
float CEtUIDialog::s_fDialogScaleValueVert = 0.0f;
float CEtUIDialog::s_fDialogScaleValueHori = 0.0f;
float CEtUIDialog::s_fMinDialogScale = 1.0f;
float CEtUIDialog::s_fMaxDialogScale = 1.0f;
int CEtUIDialog::s_nUISize = 0;
void (_stdcall *CEtUIDialog::s_pAutoCursorPtr)( bool bShow ) = NULL;

int CEtUIDialog::s_nDialogTextureSize = 0;

// Setting 0.5f makes the offset too large, causing left and right menus to overlap. (Changed to 0.18 for #15687.)
#define UI_DIALOG_SCALE_VALUE	0.18f

//#if defined(_DEBUG) || defined(_RDEBUG)
bool CEtUIDialog::s_bUITest = false;
std::map<std::string, int> CEtUIDialog::s_mapUITest;
//#endif

CEtUIControl *CEtUIDialog::s_pMouseEnterControl = NULL;

#define CHECK_DIALOG_CRASH
#ifdef CHECK_DIALOG_CRASH
char g_szCurMsgProcDialog[256] = {0,};
#endif

#pragma warning(disable:4482)
CEtUIDialog::CEtUIDialog( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback, bool bAutoCursor )
	: CEtUIDialogBase(dialogType)
{
	m_nDialogID = nID;
	m_bShow = true;
	m_bAcceptInputMsgWhenHide = false;
	m_hWnd = NULL;
	m_pDefaultControl = NULL;
	m_pCallback = pCallback;
	m_fScreenWidthRatio = 1.0f;
	m_fScreenHeightRatio = 1.0f;
	m_fScreenWidthBorder = 0.0f;
	m_fScreenHeightBorder = 0.0f;
	m_fScreenWidth = 0.0f;
	m_fScreenHeight = 0.0f;
	m_bMouseInDialog = false;
	m_FadeMode = FadeModeEnum::None;
	m_bFadeOut = false;
	m_bEndInitialize = false;
	m_fMouseX = 0.0f;
	m_fMouseY = 0.0f;
	m_HotKeyState = 0;
	m_nHotKey = 0;
	m_bPassMessageToChild = false;
	m_bShowModal = true;
	m_bAutoCursor = bAutoCursor;
	m_bCursor = false;
	m_bElementDialog = false;
	m_bElementDialogShowState = false;

	if( dialogType >= UI_TYPE_CHILD && dialogType <= UI_TYPE_CHILD_MODAL )
	{
		ASSERT( pParentDialog&&"CEtUIDialog::CEtUIDialog" );
	}

	m_pParentDialog = pParentDialog;
	if( m_pParentDialog )
	{
		// Note : Register as a child window if it has a parent.
		//
		m_pParentDialog->AddChildDialog( this );

		if( dialogType == UI_TYPE_CHILD_MODAL )
		{
			m_pParentDialog->AddChildModalDialog( this );
		}

		// Thought this was obviously right and fixed it, then children stopped showing up.
		// Upon checking, only child list is rendered, and child modal only does modal handling in MsgProc and is no longer used.
		// Why was it done like this..
		//if( dialogType == UI_TYPE_CHILD )
		//{
		//	m_pParentDialog->AddChildDialog( this );
		//}
		//else if( dialogType == UI_TYPE_CHILD_MODAL )
		//{
		//	m_pParentDialog->AddChildModalDialog( this );
		//}
	}
	m_fLastShowDelta = 10.f;
	m_bLoadedTexture = false;
}

CEtUIDialog::~CEtUIDialog(void)
{
	if( m_bAutoCursor && m_bCursor && s_pAutoCursorPtr ) {
		s_pAutoCursorPtr( false );
		m_bCursor = false;
	}
	PopFocusControl();
	DeleteAllControl();
	SAFE_DELETE_PVEC( m_vecUITemplate );
	if( m_hTexture && m_hTexture->GetRefCount() == 1 )
		s_nDialogTextureSize -= m_hTexture->GetFileSize();
	SAFE_RELEASE_SPTR( m_hTexture );
	if( m_hDlgTexture && m_hDlgTexture->GetRefCount() == 1 )
		s_nDialogTextureSize -= m_hDlgTexture->GetFileSize();
	SAFE_RELEASE_SPTR( m_hDlgTexture );
	m_bLoadedTexture = false;

	SAFE_DELETE_PVEC( m_TempControlList );

	// Tried deleting in the destructor since it registers in the constructor, which crashed due to the tooltip's static dialog.
	// Only the destruction order matters, so delete sequentially starting from the lowest child.
	if( m_pParentDialog )
	{
		// If it has a parent, remove it from the child window list.
		m_pParentDialog->DelChildDialog( this );

		if( m_emDialogType == UI_TYPE_CHILD_MODAL )
		{
			m_pParentDialog->DelChildModalDialog( this );
		}
	}
}

void CEtUIDialog::OnEndInitialize()
{
	m_bEndInitialize = true;
}

void CEtUIDialog::Initialize( bool bShow )
{
	// Note : The call order of the functions below is important.
	//
	UpdateScreen();

	InitialUpdate();

	if( !IsRootDialog() )
	{
		m_pParentDialog->ShowChildDialog( this, bShow );
	}
	else
	{
		Show( bShow );
	}

	OnEndInitialize();
	ThreadDelay();
}

void CEtUIDialog::Initialize( const char *pFileName, bool bShow )
{
	// Note : The call order of the functions below is important.
	//
	UpdateScreen();

	m_strDialogFileName.clear();

	if( pFileName )
	{
		m_strDialogFileName = pFileName;

		CResMngStream Stream( pFileName );

		if( Stream.IsValid() ) 
		{
			if( !Load( Stream ) )
			{
				CDebugSet::ToLogFile( "CEtUIDialog::Initialize, %s load failed!", pFileName );
			}
		}
		else
		{
			CDebugSet::ToLogFile( "CEtUIDialog::Initialize, %s File Not Found!", pFileName );
		}
	}

	InitialUpdate();

	if( !IsRootDialog() )
	{
		m_pParentDialog->ShowChildDialog( this, bShow );
	}
	else
	{
		Show( bShow );
	}

	OnEndInitialize();
}

bool CEtUIDialog::Load( CStream &Stream )
{
	SUIFileHeader Header;
	char szReserved[ UI_HEADER_RESERVED ]={0};

	Stream.ReadBuffer( &Header, sizeof( SUIFileHeader ) );
	Stream.ReadBuffer( szReserved, UI_HEADER_RESERVED );

	switch( Header.nVersion )
	{
	case UI_FILE_VERSION_01:
		Load_01( Stream, Header );
		break;
	case UI_FILE_VERSION:
		{
			m_DlgInfo.Load(Stream);

			for( int i = 0; i < Header.nTemplateCount; i++ )
			{
				CEtUITemplate *pTemplate = new CEtUITemplate();
				pTemplate->Load( Stream );
				pTemplate->OnLoaded();
				m_vecUITemplate.push_back( pTemplate );
			}

			for( int i = 0; i < Header.nControlCount; i++ )
			{
				SUIControlProperty Property;
				Property.Load(Stream);

				CEtUIControl *pControl = CreateControl( &Property );
				if( Property.bDefaultControl )
				{
					m_pDefaultControl = pControl;
				}
			}
		}
		break;
	default:
		ASSERT(0&&"CEtUIDialog::Load, default");
		break;
	}

	// UI textures are size-sensitive and must look identical on cards without non-power-of-two support, so do this.
	//CEtTexture::SetPow2( true );
	/*
	if( m_hTexture && m_hTexture->GetRefCount() == 1 )
		s_nDialogTextureSize -= m_hTexture->GetFileSize();
	SAFE_RELEASE_SPTR( m_hTexture );
	if( EtInterface::g_bEtUIInitTool ) CEtResource::FlushWaitDelete( RT_TEXTURE );
	m_hTexture = LoadResource( m_DlgInfo.szUITexturename, RT_TEXTURE, true );
	if( m_hTexture && m_hTexture->GetRefCount() == 1 )
		s_nDialogTextureSize += m_hTexture->GetFileSize();
	if( m_hDlgTexture && m_hDlgTexture->GetRefCount() == 1 )
		s_nDialogTextureSize -= m_hDlgTexture->GetFileSize();
	SAFE_RELEASE_SPTR( m_hDlgTexture );
	if( EtInterface::g_bEtUIInitTool ) CEtResource::FlushWaitDelete( RT_TEXTURE );
	m_hDlgTexture = LoadResource( m_DlgInfo.szDlgTextureName, RT_TEXTURE, true );
	if( m_hDlgTexture && m_hDlgTexture->GetRefCount() == 1 )
		s_nDialogTextureSize += m_hDlgTexture->GetFileSize();
	*/
	
	LoadDialogTexture();
	//CEtTexture::SetPow2( false );

	m_renderDlgColor.dwColor[UI_STATE_NORMAL] = m_DlgInfo.dwDlgColor;
	m_renderDlgColor.dwCurrentColor = m_DlgInfo.dwDlgColor;
	m_BaseDlgCoord = m_DlgInfo.DlgCoord;

	UpdateRects();
	UpdateAllControl();

	for( int i = 0; i < Header.nTemplateCount; i++ )
		SAFE_RELEASE_SPTR( m_vecUITemplate[i]->m_hTemplateTexture );

	return true;
}

bool CEtUIDialog::Save( CStream &Stream )
{
	SUIFileHeader Header;
	char szReserved[ UI_HEADER_RESERVED ]={0};
	
	Header.nTemplateCount = ( int ) m_vecUITemplate.size();
	Header.nControlCount = ( int )m_vecControl.size();

	Stream.WriteBuffer( &Header, sizeof( SUIFileHeader ) );
	Stream.WriteBuffer( szReserved, UI_HEADER_RESERVED );

	m_DlgInfo.Save(Stream);

	for( int i = 0; i < Header.nTemplateCount; i++ )
	{
		m_vecUITemplate[i]->Save( Stream );
	}

	for( int i = 0; i < Header.nControlCount; i++ )
	{
		SUIControlProperty Property;
		m_vecControl[i]->GetProperty( Property );
		Property.Save(Stream);
	}

	return true;
}

bool CEtUIDialog::Load_01( CStream &Stream, SUIFileHeader &fileheader )
{
	m_DlgInfo.Load(Stream);

	ASSERT(fileheader.nFontCount&&"CEtUIDialog::Load, Font Count is 0!");

	for( int i = 0; i < fileheader.nFontCount; i++ )
	{
		SFontInfo FontInfo;
		FontInfo.Load(Stream);
	}

	for( int i = 0; i < fileheader.nTemplateCount; i++ )
	{
		CEtUITemplate *pTemplate = new CEtUITemplate();
		pTemplate->Load( Stream );
		m_vecUITemplate.push_back( pTemplate );
	}

	for( int i = 0; i < fileheader.nControlCount; i++ )
	{
		SUIControlProperty Property;
		Property.Load(Stream);

		CEtUIControl *pControl = CreateControl( &Property );
		if( Property.bDefaultControl )
		{
			m_pDefaultControl = pControl;
		}
	}

	return true;
}

void CEtUIDialog::Show( bool bShow )
{ 
	if( bShow == m_bShow )
		return;

	if( bShow )
	{
		if( !m_bLoadedTexture ) {
			LoadDialogTexture();
		}
		SetFadeIn();
		PushFocusControl();
	}
	else
	{
		SetFadeOut();
		PopFocusControl();
		
		// If Ctrl is held while the window is closing (e.g. cancel button on enhance/craft request window),
		// Because ReleaseCapture is not called after SetCapture, or m_bPressed stays true, etc.
		// Tooltip and mouseover rendering will not work properly.
		// So iterate through controls and restore any pressed controls to their original state.
		//
		// SetPressed was not applied to element dialogs added as items in an expandable list box.
		// However, since it changed to SELF type, no separate handling is needed.
		if( !IsElementDialog() )
		{
			bool bTooltipControl = false;

			for( int i = 0; i < ( int )m_vecControl.size(); i++ )
			{
				if( m_vecControl[ i ]->IsPressed() || m_vecControl[ i ]->IsRightPressed() || m_vecControl[ i ]->IsHaveScrollBarControl() )
				{
					m_vecControl[ i ]->SetPressed( false );
					m_vecControl[ i ]->SetRightPressed( false );
					ReleaseCapture();
				}

				// Not only m_bPressed but also m_bMouseEnter became necessary.
				// When WM_MOUSEMOVE arrives in CEtUIDialog::MsgProc, CEtUIDialog::OnMouseMove is called,
				// Looking at this function, it calls ReleaseMouseEnterControl when the mouse pointer is outside its dialog area.
				// Normally in most cases this function call clears the control's m_bMouseEnter, but
				// Only one case was problematic.
				// Directly, it was a message box asking 'Would you like to play the tutorial?',
				// (This is the only message box shown while all other windows are hidden.)
				// Pressing No on this message box fades out and returns to town.
				// At this time, since no dialog is visible, MsgProc is never called, and as a result
				// ReleaseMouseEnterControl is never called, so nothing changes the MouseEnter value of the No button.
				// And then as functions like DnInterface::Finalize are called, forcibly assigning NULL to CEtUIDialog::s_pMouseEnterControl,
				// m_bMouseEnter of the No button does not change unless the mouse actually enters and leaves the No button.
				//
				// Come to think of it, having MouseEnter=true on a control in a Show(false) dialog seemed weird in itself,
				// Release it here.
				if( m_vecControl[ i ]->IsMouseEnter() )
				{
					m_vecControl[ i ]->MouseEnter(false);
				}

				if( s_pTooltipDlg && s_pTooltipDlg->GetTooltipCtrl() && m_vecControl[ i ] == s_pTooltipDlg->GetTooltipCtrl() )
					bTooltipControl = true;
			}

			if( bTooltipControl && s_pTooltipDlg != this ) s_pTooltipDlg->Show( false );
		}
	}

	if( m_emDialogType == UI_TYPE_MODAL && m_bShowModal )
	{
		ShowModal( this, bShow );
	}

	if( m_bEndInitialize && m_DlgInfo.bSound )
	{
		if( bShow )
		{
			GetUISound().Play( CEtUISound::UI_WINDOW_OPEN );
		}
		else
		{
			GetUISound().Play( CEtUISound::UI_WINDOW_CLOSE );
		}
	}

	m_bShow = bShow;
	m_fMouseX = 0.0f;
	m_fMouseY = 0.0f;
	m_HotKeyState = 0;
	m_nHotKey = 0;

	if( m_bAutoCursor && s_pAutoCursorPtr ) {
		if( m_bShow && m_bCursor == false ) {
			m_bCursor = true;
			s_pAutoCursorPtr( true );
		}
		else if( m_bShow == false && m_bCursor ) {
			m_bCursor = false;
			s_pAutoCursorPtr( false );
		}
	}
}

void CEtUIDialog::PointToFloat( POINT pt, float &fX, float &fY )
{
	fX = pt.x / GetScreenWidth() - GetXCoord();
	fY = pt.y / GetScreenHeight() - GetYCoord();
}

CEtUIControl *CEtUIDialog::GetControl( const char *pszControlName )
{
	for( int i = 0; i < ( int )m_vecControl.size(); i++ )
	{
		if( strcmp( pszControlName, m_vecControl[ i ]->GetControlName() ) == 0 )
		{
			return m_vecControl[ i ];
		}
	}

	CDebugSet::ToLogFile( "CEtUIDialog::GetControl, %s control not found!", pszControlName );
	// Note : Calls assert() instead of ASSERT() since this is where UI is created.
	//
	//assert(0&&"CEtUIDialog::GetControl, call Kyungdae!");
	return NULL;
}

CEtUIControl *CEtUIDialog::CreateControl( SUIControlProperty *pProperty )
{
	CEtUIControl *pControl = CreateControl( pProperty->UIType );
	pControl->Initialize( pProperty );

	return pControl;
}

CEtUIControl *CEtUIDialog::CreateControl( UI_CONTROL_TYPE Type )
{
	static CEtUIControlCreator controlCreator;
	CEtUIControl *pControl(NULL);

	pControl = controlCreator.CreateControl( Type, this );
	if( pControl )
	{
		m_vecControl.push_back( pControl );
	}

	return pControl;
}

void CEtUIDialog::DeleteControl( CEtUIControl *pControl )
{
	for( int i = 0; i < ( int )m_vecControl.size(); i++ )
	{
		if( m_vecControl[ i ] == pControl )
		{
			if( s_pMouseEnterControl == pControl )
				s_pMouseEnterControl = NULL;
			if( focus::IsSameControl(pControl) )
				focus::ReleaseControl();

			delete m_vecControl[ i ];
			m_vecControl.erase( m_vecControl.begin() + i );
			break;
		}
	}
}

void CEtUIDialog::DeleteAllControl()
{
	if( focus::IsSameParent(this) )
	{
		focus::ReleaseControl();
	}

	// Stability code.
	// For expandable list boxes, Show( false ) immediately deletes the dialog and child controls,
	// If the mouse was hovering over it at this time, the static pointer variable below becomes a deleted control.
	// So check before deleting.
	for( int i = 0; i < ( int )m_vecControl.size(); i++ )
	{
		if( s_pMouseEnterControl == m_vecControl[ i ] )
		{
			s_pMouseEnterControl->MouseEnter( false );
			s_pMouseEnterControl = NULL;

			ShowTooltipDlg( NULL, false );	
		}
	}

	SAFE_DELETE_PVEC( m_vecControl );
}

bool CEtUIDialog::FindControl( std::vector< CEtUIControl* > &vecControl, int nTypeCount, UI_CONTROL_TYPE *pType, bool bCheckCoveredControl, std::vector<SUICoord> &vecDlgCoord )
{
	std::list< CEtUIDialog* >::reverse_iterator iter;
	if( !m_listChildModalDialog.empty() )
	{
		iter = m_listChildModalDialog.rbegin();
		if( iter != m_listChildModalDialog.rend() )
		{
			if( (*iter) && (*iter)->IsShow() )
			{
				(*iter)->FindControl( vecControl, nTypeCount, pType, bCheckCoveredControl, vecDlgCoord );
				return true;
			}
		}
	}

	CEtUIDialog *pDialog = NULL;
	iter = m_listChildDialog.rbegin();
	for( ; iter != m_listChildDialog.rend(); ++iter )
	{
		// Skip LastRender check for now.
		pDialog = (*iter);
		if( !pDialog ) continue;
		if( (pDialog->GetDialogType() != UI_TYPE_SELF && pDialog->IsElementDialog() == false && pDialog->IsShow()) ||
			(pDialog->IsElementDialog() && pDialog->IsShowElementDialog()) )
		{
			if( pDialog->FindControl( vecControl, nTypeCount, pType, bCheckCoveredControl, vecDlgCoord ) )
				return true;
		}
	}

	std::vector<int> vecType;
	for( int i = 0; i < nTypeCount; ++i )
	{
		int nValue = pType[i];
		vecType.push_back( nValue );
	}

	CEtUIControl *pControl(NULL);
	int nVecCtlSize = ( int )m_vecControl.size();
	for( int i = 0; i < nVecCtlSize; i++ )
	{
		pControl = m_vecControl[i];

		bool bMatched = false;
		for( int j = 0; j < (int)vecType.size(); ++j )
		{
			if( pControl->GetType() == vecType[j] )
			{
				bMatched = true;
				break;
			}
		}
		if( bMatched || (int)vecType.size() == 0 )
		{
			if( pControl->IsShow() )
			{
				bool bPushControl = true;
				if( bCheckCoveredControl )
				{
					// Treat as normal control and check if center point is occluded by another window.
					// For controls like listboxes, checking each list item individually is too expensive, so do a simple check instead.
					SUICoord ControlCoord;
					pControl->GetUICoord( ControlCoord );
					ControlCoord.fX = ControlCoord.fX + ControlCoord.fWidth/2.0f;
					ControlCoord.fY = ControlCoord.fY + ControlCoord.fHeight/2.0f;
					ControlCoord = DlgCoordToScreenCoord( ControlCoord );

					for( int k = 0; k < (int)vecDlgCoord.size(); ++k )
					{
						if( ControlCoord.fX >= vecDlgCoord[k].fX && ControlCoord.fY >= vecDlgCoord[k].fY &&
							ControlCoord.fX <= vecDlgCoord[k].fX+vecDlgCoord[k].fWidth && ControlCoord.fY <= vecDlgCoord[k].fY+vecDlgCoord[k].fHeight )
						{
							bPushControl = false;
							break;
						}
					}
				}

				if( bPushControl )
					vecControl.push_back( pControl );
			}
		}
	}

	if( bCheckCoveredControl && IsElementDialog() == false )
	{
		SUICoord DlgCoord;
		GetDlgCoord( DlgCoord );
		DlgCoord.fX = 0.0f;
		DlgCoord.fY = 0.0f;
		DlgCoord = DlgCoordToScreenCoord( DlgCoord );
		vecDlgCoord.push_back( DlgCoord );
	}

	return false;
}

void CEtUIDialog::MoveToHead( CEtUIControl *pControl )
{
	int nSize = ( int )m_vecControl.size();
	for( int i = 0; i < nSize; i++ ) {
		if( m_vecControl[ i ] == pControl ) {
			m_vecControl.erase( m_vecControl.begin() + i );
			m_vecControl.insert( m_vecControl.begin(), pControl );
			break;
		}
	}
}

void CEtUIDialog::MoveToTail( CEtUIControl *pControl )
{
	int nSize = ( int )m_vecControl.size();
	for( int i = 0; i < nSize; i++ ) {
		if( m_vecControl[ i ] == pControl ) {
			m_vecControl.erase( m_vecControl.begin() + i );
			m_vecControl.push_back( pControl );
			break;
		}
	}
}

bool CEtUIDialog::IsUsableFocusControl()
{
	if( focus::IsSameParent(this) && focus::IsEnable() )
	{
		return true;
	}
	else
	{
		return false;
	}
}

int CEtUIDialog::FindControlIndex( CEtUIControl *pControl )
{
	for( int i = 0; i < ( int )m_vecControl.size(); i++ )
	{
		if( pControl == m_vecControl[ i ] )
		{
			return i;
		}
	}

	return -1;
}

CEtUIControl *CEtUIDialog::GetNextControl( CEtUIControl *pControl )
{
	if( m_vecControl.empty() )
	{
		return NULL;
	}

	int nIndex;

	nIndex = FindControlIndex( pControl );
	if( nIndex == -1 )
	{
		nIndex = ( int )m_vecControl.size() - 1;
	}
	if( nIndex == ( int )m_vecControl.size() - 1 )
	{
		return m_vecControl[ 0 ];
	}
	else
	{
		return m_vecControl[ nIndex + 1 ];
	}
}

CEtUIControl *CEtUIDialog::GetPrevControl( CEtUIControl *pControl )
{
	if( m_vecControl.empty() )
	{
		return NULL;
	}

	int nIndex;

	nIndex = FindControlIndex( pControl );
	if( nIndex == -1 )
	{
		nIndex = 0;
	}
	if( nIndex == 0 )
	{
		return m_vecControl[ m_vecControl.size() - 1 ];
	}
	else
	{
		return m_vecControl[ nIndex - 1 ];
	}
}

bool CEtUIDialog::OnCycleFocus( bool bForward )
{
	if( !focus::IsSameParent(this) )
	{
		//pControl = NULL;
		return false;
	}

	CEtUIControl *pControl = focus::GetControl();
	int nLoopCount(0);

	while( true )
	{
		if( bForward )
		{
			pControl = GetNextControl( pControl );
		}
		else
		{
			pControl = GetPrevControl( pControl );
		}

		if( pControl == NULL )
		{
			break;
		}

		if( focus::IsSameControl(pControl) )
		{
			return true;
		}

		if( pControl->CanHaveFocus() )
		{
			RequestFocus( pControl );
			return true;
		}

		nLoopCount++;

		if( nLoopCount > ( int )m_vecControl.size() )
		{
			break;
		}
	}

	return false;
}

void CEtUIDialog::RequestFocus( CEtUIControl* pControl )
{
	if( !pControl )
		return;

	if( focus::IsSameControl(pControl) )
	{
		return;
	}

	if( !pControl->CanHaveFocus() )
	{
		return;
	}

	focus::SetFocus(pControl);
}

void CEtUIDialog::PushFocusControl()
{
	focus::PushControl(this);

	// Why on earth was this called...
	// Because of this, even though focus was set to the listbox on server list show(true),
	// Skipped one item so the Connect button gets focus (list box is 3, Connect button is 4).
	// Decided not to call this for now; will investigate if anything weird shows up.
	//OnCycleFocus( true );
}

void CEtUIDialog::PopFocusControl()
{
	focus::PopControl(this);
}

void CEtUIDialog::ShowAllControl( bool bShow )
{
	for( int i = 0; i < ( int )m_vecControl.size(); i++ )
	{
		m_vecControl[ i ]->Show( bShow );
	}
}

void CEtUIDialog::UpdateAllControl()
{
	for( int i = 0; i < ( int )m_vecControl.size(); i++ )
	{
		m_vecControl[ i ]->UpdateRects();
	}
}

void CEtUIDialog::EnableAllControl( bool bEnable )
{
	for( int i = 0; i < ( int )m_vecControl.size(); i++ )
	{
		m_vecControl[ i ]->Enable( bEnable );
	}
}

void CEtUIDialog::ClearRadioButtonGroup( int nButtonGroup )
{
	for( int i = 0; i < ( int )m_vecControl.size(); i++ )
	{
		if( m_vecControl[ i ] && m_vecControl[ i ]->GetType() == UI_CONTROL_RADIOBUTTON )
		{
			CEtUIRadioButton *pRadioButton = ( CEtUIRadioButton * )m_vecControl[ i ];
			if( pRadioButton->GetButtonGroup() == nButtonGroup )
			{
				pRadioButton->SetChecked( false, false );
			}
		}
	}
}

void CEtUIDialog::ReloadDlgTexture()
{
	// UI textures are size-sensitive and must look identical on cards without non-power-of-two support, so do this.
	//CEtTexture::SetPow2( true );
	if( m_hDlgTexture )
		s_nDialogTextureSize -= m_hDlgTexture->GetFileSize();
	SAFE_RELEASE_SPTR( m_hDlgTexture );
	m_hDlgTexture = LoadResource( m_DlgInfo.szDlgTextureName, RT_TEXTURE, true );
	if( m_hDlgTexture )
		s_nDialogTextureSize += m_hDlgTexture->GetFileSize();
	//CEtTexture::SetPow2( false );
}

void CEtUIDialog::DeleteDlgTexture()
{
	ZeroMemory( m_DlgInfo.szDlgTextureName, _countof(m_DlgInfo.szDlgTextureName) );
	SAFE_RELEASE_SPTR( m_hDlgTexture );
}

void CEtUIDialog::UpdateRects()
{
	if( ( !m_hDlgTexture ) )
	{
		return;
	}

	if( (m_DlgInfo.nFrameLeft > 0) || (m_DlgInfo.nFrameRight > 0) || (m_DlgInfo.nFrameTop > 0) || (m_DlgInfo.nFrameBottom > 0) )
	{
		UpdateFrameRectsEx();
	}
}

void CEtUIDialog::UpdateFrameRectsEx()
{
	// Note : The dialog splits the texture into 9 parts when rendering so it does not distort on resize.
	//		Left/right and top/bottom values are set in the tool. The center stretches, so keep that in mind during design.
	//
	float fFrameLeft = m_DlgInfo.nFrameLeft / ( float )m_hDlgTexture->OriginalWidth();
	float fFrameTop = m_DlgInfo.nFrameTop / ( float )m_hDlgTexture->OriginalHeight();
	float fFrameRight = m_DlgInfo.nFrameRight / ( float )m_hDlgTexture->OriginalWidth();
	float fFrameBottom = m_DlgInfo.nFrameBottom / ( float )m_hDlgTexture->OriginalHeight();
	float fModWidth = 1.0f - (fFrameLeft+fFrameRight);
	float fModHeight = 1.0f - (fFrameTop+fFrameBottom);

	if( fModWidth <= 0.0f || fModHeight <= 0.0f )
	{
		//ASSERT(0&&"Sum of Frame Left+Right or Top+Bottom exceeds original texture size.");
	}

	m_vecDlgFrameElement.resize( 9 );
	m_vecDlgFrameElement[ 0 ].UVCoord.SetCoord( 0.0f,					0.0f,					fFrameLeft,		fFrameTop );
	m_vecDlgFrameElement[ 1 ].UVCoord.SetCoord( fFrameLeft,				0.0f,					fModWidth,		fFrameTop );
	m_vecDlgFrameElement[ 2 ].UVCoord.SetCoord( fFrameLeft+fModWidth,	0.0f,					fFrameRight,	fFrameTop );
	m_vecDlgFrameElement[ 3 ].UVCoord.SetCoord( 0.0f,					fFrameTop,				fFrameLeft,		fModHeight );
	m_vecDlgFrameElement[ 4 ].UVCoord.SetCoord( fFrameLeft,				fFrameTop,				fModWidth,		fModHeight );
	m_vecDlgFrameElement[ 5 ].UVCoord.SetCoord( fFrameLeft+fModWidth,	fFrameTop,				fFrameRight,	fModHeight );
	m_vecDlgFrameElement[ 6 ].UVCoord.SetCoord( 0.0f,					fFrameTop+fModHeight,	fFrameLeft,		fFrameBottom );
	m_vecDlgFrameElement[ 7 ].UVCoord.SetCoord( fFrameLeft,				fFrameTop+fModHeight,	fModWidth,		fFrameBottom );
	m_vecDlgFrameElement[ 8 ].UVCoord.SetCoord( fFrameLeft+fModWidth,	fFrameTop+fModHeight,	fFrameRight,	fFrameBottom );

	fFrameLeft = m_DlgInfo.nFrameLeft / (float)DEFAULT_UI_SCREEN_WIDTH;
	fFrameTop = m_DlgInfo.nFrameTop / (float)DEFAULT_UI_SCREEN_HEIGHT;
	fFrameRight = m_DlgInfo.nFrameRight / (float)DEFAULT_UI_SCREEN_WIDTH;
	fFrameBottom = m_DlgInfo.nFrameBottom / (float)DEFAULT_UI_SCREEN_HEIGHT;
	fModWidth = m_DlgInfo.DlgCoord.fWidth - (fFrameLeft+fFrameRight);
	fModHeight = m_DlgInfo.DlgCoord.fHeight - (fFrameTop+fFrameBottom);

	m_vecDlgFrameCoord.resize( 9 );
	m_vecDlgFrameCoord[ 0 ].SetCoord( 0.0f,						0.0f,					fFrameLeft,		fFrameTop );
	m_vecDlgFrameCoord[ 1 ].SetCoord( fFrameLeft,				0.0f,					fModWidth,		fFrameTop );
	m_vecDlgFrameCoord[ 2 ].SetCoord( fFrameLeft + fModWidth,	0.0f,					fFrameRight,	fFrameTop );
	m_vecDlgFrameCoord[ 3 ].SetCoord( 0.0f,						fFrameTop,				fFrameLeft,		fModHeight );
	m_vecDlgFrameCoord[ 4 ].SetCoord( fFrameLeft,				fFrameTop,				fModWidth,		fModHeight );
	m_vecDlgFrameCoord[ 5 ].SetCoord( fFrameLeft + fModWidth,	fFrameTop,				fFrameRight,	fModHeight );
	m_vecDlgFrameCoord[ 6 ].SetCoord( 0.0f,						fFrameTop + fModHeight,	fFrameLeft,		fFrameBottom );
	m_vecDlgFrameCoord[ 7 ].SetCoord( fFrameLeft,				fFrameTop + fModHeight,	fModWidth,		fFrameBottom );
	m_vecDlgFrameCoord[ 8 ].SetCoord( fFrameLeft + fModWidth,	fFrameTop + fModHeight,	fFrameRight,	fFrameBottom );
}

//void CEtUIDialog::GetScreenSize( float &fWidth, float &fHeight )
//{
//	fHeight = ( float )GetEtDevice()->Height();
//	fWidth = fHeight * 4.0f / 3.0f;
//}

float CEtUIDialog::GetXCoord()
{
	float fBase(0.0f);

	switch( m_DlgInfo.AllignHori )
	{
		case AT_HORI_NONE:			fBase = GetScreenWidthBorderSize();																									break;
		case AT_HORI_LEFT:			fBase = s_fDialogScaleValueHori * UI_DIALOG_SCALE_VALUE;																			break;
		case AT_HORI_CENTER:		fBase = ( ( 1.0f + GetScreenWidthBorderSize() * 2.0f ) - m_DlgInfo.DlgCoord.fWidth ) * 0.5f;										break;
		case AT_HORI_RIGHT:			fBase = ( 1.0f + GetScreenWidthBorderSize() * 2.0f ) - m_DlgInfo.DlgCoord.fWidth - s_fDialogScaleValueHori * UI_DIALOG_SCALE_VALUE;	break;
		case AT_HORI_LEFT_FIXED:	fBase = 0.0f;																														break;
		case AT_HORI_RIGHT_FIXED:	fBase = ( 1.0f + GetScreenWidthBorderSize() * 2.0f ) - m_DlgInfo.DlgCoord.fWidth;													break;
	}

	return m_DlgInfo.DlgCoord.fX + fBase;
}

float CEtUIDialog::GetYCoord()
{
	float fBase(0.0f);

	switch( m_DlgInfo.AllignVert )
	{
		case AT_VERT_NONE:		fBase = GetScreenHeightBorderSize();	break;
		case AT_VERT_TOP:		fBase = 0.0f;							break;
		case AT_VERT_CENTER:	fBase = ( ( 1.0f + GetScreenHeightBorderSize() * 2.0f ) - m_DlgInfo.DlgCoord.fHeight ) * 0.5f;	break;
		case AT_VERT_BOTTOM:	fBase = ( 1.0f + GetScreenHeightBorderSize() * 2.0f ) - m_DlgInfo.DlgCoord.fHeight;				break;
	}

	return m_DlgInfo.DlgCoord.fY + fBase;
}

void CEtUIDialog::MoveDialog( float fX, float fY )
{
	m_DlgInfo.DlgCoord.fX += fX;
	m_DlgInfo.DlgCoord.fY += fY;
	UpdateRects();
}

void CEtUIDialog::GetPosition( float &fX, float &fY )
{ 
	fX = GetXCoord();
	fY = GetYCoord(); 
}

void CEtUIDialog::SetPosition( float fX, float fY )
{
	m_DlgInfo.DlgCoord.fX = fX;
	m_DlgInfo.DlgCoord.fY = fY;

	UpdateRects();
}

CEtUIControl *CEtUIDialog::GetControlAtPoint( float fX, float fY )
{
	CEtUIControl *pControl(NULL);

	for( int i=(int)m_vecControl.size()-1; i>=0; i-- )
	{
		pControl = m_vecControl[i];

		if (pControl && ( pControl->IsInside( fX, fY ) ) && ( pControl->IsShow() ))//( pControl->IsEnable() ) ) // commented by kalliste 090824
		{
			return pControl;
		}
	}

	return NULL;
}

void CEtUIDialog::SetTemplate( int nIndex, CEtUITemplate &Template )
{
	( *m_vecUITemplate[ nIndex ] ) = Template;

	int i;

	for( i = 0; i < ( int )m_vecControl.size(); i++ )
	{
		if( m_vecControl[ i ]->GetTemplateIndex() == nIndex )
		{
			SUIControlProperty *pProperty = m_vecControl[ i ]->GetProperty();
			m_vecControl[ i ]->Initialize( pProperty );
		}
	}
}

int CEtUIDialog::AddTemplate( CEtUITemplate &Template )
{
	CEtUITemplate *pTemplate;
	pTemplate = new CEtUITemplate();
	(*pTemplate) = Template;
	m_vecUITemplate.push_back( pTemplate );

	return ( int )m_vecUITemplate.size() - 1;
}

void CEtUIDialog::DeleteTemplate( int nIndex )
{
	ASSERT( nIndex >= 0 && nIndex < (int)m_vecUITemplate.size() );

	delete m_vecUITemplate[ nIndex ];
	m_vecUITemplate.erase( m_vecUITemplate.begin() + nIndex );
}

void CEtUIDialog::DeleteAllTemplate()
{
	SAFE_DELETE_PVEC( m_vecUITemplate );
}

float CEtUIDialog::GetFontHeight( int &nFontSetIndex, int nFontHeight )
{
	SFontInfo FontInfo;
	CEtFontMng::GetInstance().GetFontInfo( nFontSetIndex, nFontHeight, FontInfo );

	return FontInfo.nFontHeight / GetScreenHeight();
}

//ID3DXFont *CEtUIDialog::GetFont( int &nFontSetIndex, int nFontHeight )
//{
//	SFontInfo FontInfo;
//	CEtFontMng::GetInstance().GetFontInfo( nFontSetIndex, nFontHeight, FontInfo );
//
//	return FontInfo.pFont;
//}

const wchar_t* CEtUIDialog::GetUIString( int nCategoryID, int nIndex )
{
	if( xml::IsValid() )
	{
		return xml::GetString( nCategoryID, nIndex );
	}

	return NULL;
}

bool CEtUIDialog::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	m_hWnd = hWnd;

	// Note : Return if the dialog is not visible or there is no need to send the message to child windows
	//
	if( !m_bShow && !m_bPassMessageToChild && !m_bAcceptInputMsgWhenHide )
	{
		return false;
	}
	else
	{
#ifdef CHECK_DIALOG_CRASH
		// Wonder if it comes in already broken.
		strcpy_s( g_szCurMsgProcDialog, 256, m_strDialogFileName.c_str() );
#endif

		std::list< CEtUIDialog* >::reverse_iterator iter;

		// Note : Process the modal if one exists and is currently visible.
		//
		if( !m_listChildModalDialog.empty() )
		{
			iter = m_listChildModalDialog.rbegin();
			if( iter != m_listChildModalDialog.rend() )
			{
				if( *iter != NULL )
				{
					if( (*iter)->IsShow() )
					{
						(*iter)->MsgProc( hWnd, uMsg, wParam, lParam );

						// The party creation name input dialog is CHILD_MODAL,
						// Only in this input box did the issue occur where Chinese characters selected from the candidate window were not entered.
						// Original code returned true immediately after calling MsgProc, but
						// Suspect that Msg with GCS_RESULTSTR is not passed to the default procedure here,
						// Did not reach external IME, so the Chinese character selected from the candidate list should come from the external IME (via wm_char message)
						// Seems characters weren't drawn because this didn't arrive.
						//
						// So after checking whether the candidate window appeared using s_bRESULTSTR_NotSendComp below,
						// If triggered, do not return the message; let it pass through to the default procedure.
						// _CH cannot be used because this is at the EtInterface layer.
						if( CEtUIIME::s_bRESULTSTR_NotSendComp == false )
							return true;
						else
						{
							if( (uMsg == WM_IME_COMPOSITION) && ((lParam & GCS_COMPSTR) || (lParam & GCS_RESULTSTR)) ) {
							}
							else return true;
						}
					}
				}
				else
				{
					_ASSERT(0&&"NULL found in Modal-iterator inside CEtUIDialog::MsgProc!");
				}
			}
		}

		iter = m_listChildDialog.rbegin();
		for( ; iter != m_listChildDialog.rend(); ++iter )
		{
			if( *iter == NULL )
			{
				_ASSERT(0&&"NULL found in iterator inside CEtUIDialog::MsgProc!");
				continue;
			}

			// The reason for passing the handle directly is,
			// When calling Show(true) on a Child dialog with IME and setting Focus to the IME at the same time,
			// CEtUIIMEEditBox::Focus gets called with m_hWnd equal to NULL.
			// At this time, since the parent handle is NULL, the IME-Enable feature fails to work properly,
			// Had to close and reopen the window to give focus to IME for input.
			// So pass the window handle directly like this.
			//
			// Even for Child_Modal, it is in the Child list, so no separate handling is needed for Child_Modal.
			(*iter)->SetHWnd( hWnd );
			if( (*iter)->MsgProc( hWnd, uMsg, wParam, lParam ) )
			{
				return true;
			}
		}
	}

	if( IsUsableFocusControl() )
	{
		if( focus::MsgProc( uMsg, wParam, lParam ) )
		{
			return true;
		}
	}

	// If a modal opens while holding a normal button (e.g. DN help window), the button stays in Pressed state.
	// No suitable place to handle this, so process it here for now.
	// Calling OnReleaseControl instead of ReleaseControl seems better to just perform procedures like canceling Pressed.
	if( m_emDialogType == UI_TYPE_MODAL && focus::IsEnable() && focus::GetParent() != this && IsChildDialog( focus::GetParent() ) == false )
		focus::OnReleaseControl();

	switch( uMsg )
	{
	case WM_SIZE:
	case WM_MOVE:
		{
			POINT pt = { -1, -1 };
			float fMouseX, fMouseY;
			PointToFloat( pt, fMouseX, fMouseY );
			OnMouseMove( fMouseX, fMouseY );
		}
		break;
	case WM_ACTIVATEAPP:
		if( IsUsableFocusControl() )
		{
			if( wParam )
			{
				focus::SetFocus();
			}
			else
			{
				//s_pFocusControl->Focus( false );
			}
		}
		break;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
		{
			if( IsUsableFocusControl() )
			{
				if( focus::HandleKeyboard( uMsg, wParam, lParam ) )
				{
					return true;
				}
			}

			if( uMsg == WM_KEYDOWN )
			{
				bool bIsFocusEditBox = false;
				if( focus::IsValid() )
				{
					if( focus::GetType() == UI_CONTROL_EDITBOX ||
						focus::GetType() == UI_CONTROL_IMEEDITBOX ||
						focus::GetType() == UI_CONTROL_LINE_EDITBOX ||
						focus::GetType() == UI_CONTROL_LINE_IMEEDITBOX )
						bIsFocusEditBox = true;
				}

				if( !bIsFocusEditBox )
				{
					for( int i = 0; i < ( int )m_vecControl.size(); i++ )
					{
						if( !m_vecControl[i]->IsShow() || !m_vecControl[i]->IsEnable() )
							continue;

						int nHotKey = m_vecControl[i]->GetHotKey();
						SHORT keyState = GetKeyState( nHotKey );

						// Note : Check for repeated key presses and prevent processing them.
						//		However, pressing two different keys consecutively is not checked. Probably needs fixing...
						//
						if( keyState&0x80 )
						{

							// Seems there was an ancient bug, but no matter how much I look, I cannot find anything wrong.
							// Reproduction steps are as follows.
							// Open a window with an edit box and type a key (e.g., open party creation window and press 'p' in party name input)
							// Pressing 'p' twice, closing with Esc, and pressing 'p' again triggers the character status shortcut and opens the status window.
							// However, after pressing 3 times, closing with Esc, and pressing p again, it does not open immediately and requires pressing one more time.
							// Strangely, each time the count of pressing 'p' increases, the state alternates (0,2,4,6.. work normally; 1,3,5 require pressing once more to open).
							// No matter how much I check, there is nothing alternatingly handled in Editbox, so
							// In the hotkey handling below, the part using GetKeyState to prevent key holding looks most suspicious,
							// The m_HotKeyState != keyState comparison in if( (m_nHotKey != nHotKey) || (m_HotKeyState != keyState) ).
							// Yet it is also awkward to replace it with something else at this point..
							//
							// Initially, since m_HotKeyState is reset on Show-Hide by default, I considered finding parents and children to reset them all together, but
							// This method did not work because hotkeys sometimes open completely unrelated windows.
							// So.. decided to forcibly release it where truly needed.
							if( (m_nHotKey != nHotKey) || (m_HotKeyState != keyState) )
							{
								m_vecControl[i]->OnHotkey();
								m_HotKeyState = keyState;
								m_nHotKey = nHotKey;
								return true;
							}
						}
					}
				}
			}

			if( uMsg == WM_KEYDOWN )
			{
				if( !focus::IsValid() )
					break;

				switch( wParam )
				{
				case VK_RIGHT:
				case VK_DOWN:	return OnCycleFocus( true );
				case VK_LEFT:
				case VK_UP:		return OnCycleFocus( false );
				case VK_TAB:	return OnCycleFocus( ( ( GetKeyState( VK_SHIFT ) & 0x8000 ) == 0 ) );
				}
			}
		}
		break;
	}

	POINT MousePoint;
	float fMouseX, fMouseY;
	MousePoint.x = short( LOWORD( lParam ) );
	MousePoint.y = short( HIWORD( lParam ) );
	PointToFloat( MousePoint, fMouseX, fMouseY );

	switch( uMsg )
	{
	case WM_MOUSEMOVE:
		{
			s_fScreenMouseX = MousePoint.x / GetScreenWidth();
			s_fScreenMouseY = MousePoint.y / GetScreenHeight();
			OnMouseMove( fMouseX, fMouseY );
		}
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_LBUTTONDBLCLK:
	case WM_MBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
	case WM_XBUTTONDBLCLK:
	case WM_MOUSEWHEEL:
		{
			if( (uMsg != WM_MOUSEMOVE) && (uMsg != WM_MOUSEWHEEL) )
			{
				g_pFocusDialog = this;
			}

			if( IsUsableFocusControl() )	// Prioritize GetControlAtPoint over Focus Control.... // (to place checkboxes on quest TreeCtrl) by realgaia 091215 
			{
				if( focus::HandleMouse( uMsg, fMouseX, fMouseY, wParam, lParam ) )
				{
					return true;
				}
			}

			CEtUIControl* pControl = GetControlAtPoint( fMouseX, fMouseY );
			if (pControl && pControl->IsShow()) //&& pControl->IsEnable() ) // commented by kalliste 090824
			{
				if( pControl->HandleMouse( uMsg, fMouseX, fMouseY, wParam, lParam ) )
				{
					return true;
				}
				else {
					//if( IsUsableFocusControl() ) // Process Focus Control after GetControlAtPoint... by realgaia 091215 		
					//{
					//	if( focus::HandleMouse( uMsg, fMouseX, fMouseY, wParam, lParam ) )
					//	{
					//		return true;
					//	}
					//}
				}
			}
			else
			{
				if( uMsg == WM_LBUTTONDOWN )
				{
					if( focus::IsSameParent(this) )
					{
						focus::ReleaseControl();
					}
				}
			}

			if( IsMouseInDlg() && !GetCapture() )
			{
				// Note : Message forwarding to the rear window when an event occurs in the front window
				//		do not do. Window order is important.
				//
				return true;
			}
#define MOUSEOVER_CONTROL_OUTSIDE_DLG
#ifdef MOUSEOVER_CONTROL_OUTSIDE_DLG
			else
			{
				// If there is a control being processed during mouse move,
				// (Decided to exclude texture controls, as power-of-2 padding causes many to exceed dialog bounds.
				//  And to do it properly, texture controls should be excluded when getting pControl via GetControlAtPoint,
				//  Doing all that seems overkill, so for now let's just handle it as below.)
				if( uMsg == WM_MOUSEMOVE && pControl && pControl->GetProperty()->UIType != UI_CONTROL_TEXTURECONTROL )
					return true;
			}
#endif
		}
		break;
	}

	return false;
}

bool CEtUIDialog::OnMouseMove( float fX, float fY )
{
	m_fMouseX = fX;
	m_fMouseY = fY;

	SUICoord dlgCoord(m_DlgInfo.DlgCoord);
	dlgCoord.SetPosition( GetXCoord(), GetYCoord() );

	if( dlgCoord.IsInside( s_fScreenMouseX, s_fScreenMouseY ) )
	{
		m_bMouseInDialog = true;

		// Note : Get the control located at the mouse pointer.
		//
		CEtUIControl *pControl = GetControlAtPoint( m_fMouseX, m_fMouseY );
		if( pControl == NULL )
		{
			ReleaseMouseEnterControl();
			return true;
		}
		else if( pControl == s_pMouseEnterControl )
		{
			return true;
		}

		SetMouseEnterControl( pControl );
		return true;
	}
	else
	{
#ifdef MOUSEOVER_CONTROL_OUTSIDE_DLG
		m_bMouseInDialog = false;
		CEtUIControl *pControl = GetControlAtPoint( m_fMouseX, m_fMouseY );
		if( pControl == NULL )
		{
			ReleaseMouseEnterControl();
			return false;
		}
		else if( pControl == s_pMouseEnterControl )
		{
			return true;
		}

		SetMouseEnterControl( pControl );
		return true;
#else
		m_bMouseInDialog = false;
		ReleaseMouseEnterControl();
		return false;
#endif
	}

	return false;
}

void CEtUIDialog::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	if( ( nCommand == EVENT_EDITBOX_STRING ) && m_pDefaultControl )
	{
		if( m_pDefaultControl->IsShow() && m_pDefaultControl->IsEnable() )
			m_pDefaultControl->OnHotkey();
	}

	if( nCommand == EVENT_EDITBOX_ESCAPE ) {
		for( int i = 0; i < ( int )m_vecControl.size(); i++ ) {
			if( !m_vecControl[i]->IsShow() || !m_vecControl[i]->IsEnable() )
				continue;
			if( m_vecControl[i]->GetHotKey() == VK_ESCAPE ) {
				m_vecControl[i]->OnHotkey();
				break;
			}
		}
	}

	if( pControl->IsShow() && pControl->IsEnable() )
	{
		if( m_pCallback )
		{
			// Crashes here on break during loading.. m_pCallBack is already deleted.
			m_pCallback->OnUICallbackProc( m_nDialogID, nCommand, pControl, uMsg );
		}
	}
}

void CEtUIDialog::Process( float fElapsedTime )
{
	CEtUIControl *pControl(NULL);
	int nVecCtlSize = ( int )m_vecControl.size();
	for( int i = 0; i < nVecCtlSize; i++ )
	{
		pControl = m_vecControl[i];
		if (pControl)
			pControl->Process( fElapsedTime );
	}

	if( m_FadeMode != FadeModeEnum::None )
	{
		if( !IsShow() )	
			FadeOut( fElapsedTime );
		else			
			FadeIn( fElapsedTime );
	}

	std::list<CEtUIDialog*>::iterator iter = m_listChildDialog.begin();
	for( ; iter != m_listChildDialog.end(); ++iter )
	{
		if( *iter == NULL )
		{
			_ASSERT(0&&"NULL found in iterator inside CEtUIDialog::Process!");
			continue;
		}

		(*iter)->Process( fElapsedTime );
	}	

	ProcessMemoryOptimize( fElapsedTime );
	RefreshChildRenderPriority();
}

#ifndef _FINAL_BUILD
extern bool g_bPauseMode;
#endif

void CEtUIDialog::ProcessMemoryOptimize( float fElapsedTime )
{
#ifndef _FINAL_BUILD
	if( g_bPauseMode ) return;
#endif

	bool bDraging = false;
	if( drag::IsValid() && drag::GetControl()->GetParent() == this ) {
		bDraging = true;
	}
	if( ( IsShow() ) || bDraging || IsAllowRender() ) {
		if( m_fLastShowDelta < 10.f ) m_fLastShowDelta = 10.f;
		m_fLastShowDelta += fElapsedTime;
		if( m_fLastShowDelta > 30.f ) m_fLastShowDelta = 30.f;
	}
	else m_fLastShowDelta -= fElapsedTime;

	if( m_fLastShowDelta <= 0.f ) {
		FreeDialogTexture();
	}
}

void CEtUIDialog::FreeDialogTexture()
{
	if( !m_bLoadedTexture ) return;
	if( m_hDlgTexture && m_hDlgTexture->GetRefCount() == 1 )
		s_nDialogTextureSize -= m_hDlgTexture->GetFileSize();
	SAFE_RELEASE_SPTR( m_hDlgTexture );
	if( m_hTexture && m_hTexture->GetRefCount() == 1 )
		s_nDialogTextureSize -= m_hTexture->GetFileSize();
	SAFE_RELEASE_SPTR( m_hTexture );
	CEtResource::FlushWaitDelete( RT_TEXTURE );
	m_bLoadedTexture = false;

	std::list<CEtUIDialog*>::iterator iter = m_listChildDialog.begin();
	for( ; iter != m_listChildDialog.end(); ++iter )
	{
		(*iter)->FreeDialogTexture();
	}

	CEtUIControl *pControl(NULL);
	int nVecCtlSize = ( int )m_vecControl.size();
	for( int i = 0; i < nVecCtlSize; i++ )
	{
		pControl = m_vecControl[i];
		SAFE_RELEASE_SPTR( pControl->GetTemplate().m_hTemplateTexture );
	}
}

void CEtUIDialog::LoadDialogTexture()
{
	if( m_bLoadedTexture ) return;
	// No longer needed now that template textures are used. No need to rewrite all code, just comment out this one line.
	// Clean up the source code after it stabilizes.
	//m_hTexture = LoadResource( m_DlgInfo.szUITexturename, RT_TEXTURE, true );
	if( m_hTexture && m_hTexture->GetRefCount() == 1 )
		s_nDialogTextureSize += m_hTexture->GetFileSize();
	m_hDlgTexture = LoadResource( m_DlgInfo.szDlgTextureName, RT_TEXTURE, true );
	if( m_hDlgTexture && m_hDlgTexture->GetRefCount() == 1 )
		s_nDialogTextureSize += m_hDlgTexture->GetFileSize();
	m_bLoadedTexture = true;

	std::list<CEtUIDialog*>::iterator iter = m_listChildDialog.begin();
	for( ; iter != m_listChildDialog.end(); ++iter )
	{
		(*iter)->LoadDialogTexture();
	}

	CEtUIControl *pControl(NULL);
	int nVecCtlSize = ( int )m_vecControl.size();
	for( int i = 0; i < nVecCtlSize; i++ )
	{
		pControl = m_vecControl[i];
		pControl->GetTemplate().OnLoaded();
	}
}

bool CEtUIDialog::IsAllowRender()
{
	if( !IsShow() ) {
		switch( m_FadeMode ) {
			case FadeModeEnum::None: return false;
			case FadeModeEnum::CancelRender:
				if( ( m_renderDlgColor.dwCurrentColor >> 24 ) == 0 ) return false;
				break;
			case FadeModeEnum::AllowRender: return true;
		}
	}
	return true;
}

void CEtUIDialog::RefreshChildRenderPriority()
{
	for( DWORD i=0; i<m_vecChildRenderPriorityInfo.size(); i++ )
	{
		CEtUIDialog* pDlg = m_vecChildRenderPriorityInfo[i].first;
		bool bTail = m_vecChildRenderPriorityInfo[i].second;

		if( !pDlg )
			continue;

		std::list<CEtUIDialog*>::iterator iter = std::find( m_listChildDialog.begin(), m_listChildDialog.end(), pDlg );
		if ( iter != m_listChildDialog.end() )
		{
			CEtUIDialog* pDlg = *iter;

			if (bTail)
				m_listChildDialog.push_back(pDlg);
			else
				m_listChildDialog.push_front(pDlg);
			m_listChildDialog.erase(iter);
		}
	}

	m_vecChildRenderPriorityInfo.clear();
}

void CEtUIDialog::SetChildRenderPriority(CEtUIDialog* pDlg, bool bTail)
{
	if( !pDlg )
		return;

	m_vecChildRenderPriorityInfo.push_back( std::make_pair( pDlg, bTail ) );
}

void CEtUIDialog::Render( float fElapsedTime )	
{
	if( !IsAllowRender() ) return;

	SUICoord DlgCoord(m_DlgInfo.DlgCoord), DlgUVCoord(0.0f, 0.0f, 1.0f, 1.0f);
	DlgCoord.fX = 0.0f;
	DlgCoord.fY = 0.0f;

	if( m_hDlgTexture )
	{
		if( (m_DlgInfo.nFrameLeft > 0) || (m_DlgInfo.nFrameRight > 0) || (m_DlgInfo.nFrameTop > 0) || (m_DlgInfo.nFrameBottom > 0) )
		{
			DrawDlgFrame();
		}
		else
		{
			DrawSprite( m_hDlgTexture, DlgUVCoord, m_renderDlgColor.dwCurrentColor, DlgCoord );
		}
	}
	else if( (m_renderDlgColor.dwCurrentColor & 0xFF000000) != 0 )
	{
		DrawRect( DlgCoord, m_renderDlgColor.dwCurrentColor );
	}

	// Note : Use this when debugging
	//
	//  DrawRect( DlgCoord, EtInterface::debug::BLUE );

	// Note : Render its own controls.
	//
	CEtUIControl *pControl(NULL);
	int nVecCtlSize = ( int )m_vecControl.size();
	for( int i = 0; i < nVecCtlSize; i++ )
	{
		pControl = m_vecControl[i];
		if( !pControl ) continue;
		pControl->Render( fElapsedTime );
	}

	// Render child windows.
	//
	bool bRenderMostFocusDlg = false;
	std::list<CEtUIDialog*>::iterator iter = m_listChildDialog.begin();
	for( ; iter != m_listChildDialog.end(); ++iter )
	{
		if( *iter == NULL )
		{
			//_ASSERT(0&&"NULL iterator found inside CEtUIDialog::Render!");
			continue;
		}
		(*iter)->Render( fElapsedTime );
	}
}

void CEtUIDialog::DrawDlgFrame()
{
	int nFrameSize = ( int )m_vecDlgFrameElement.size();

	for( int i = 0; i < nFrameSize; i++ )
	{
		DrawSprite( m_hDlgTexture, m_vecDlgFrameElement[ i ].UVCoord, m_renderDlgColor.dwCurrentColor, m_vecDlgFrameCoord[ i ] );
	}
}

void CEtUIDialog::CalcTextRect( LPCWSTR szText, SUIElement *pElement, SUICoord &CalcCoord, int nCount, DWORD dwFontFormat )
{
	ASSERT( pElement&&"CEtUIDialog::CalcTextRect, pElement is NULL!" );
	ASSERT( pElement->nFontIndex>=0&&"CEtUIDialog::CalcTextRect" );

	if( dwFontFormat == 0xffffffff )
	{
		dwFontFormat = pElement->dwFontFormat;
	}
	dwFontFormat |= DT_WORDBREAK;

	CEtFontMng::GetInstance().CalcTextRect( pElement->nFontIndex, pElement->nFontHeight, szText, dwFontFormat, CalcCoord, nCount );
	CalcCoord.fWidth *= GetScreenWidthRatio();
	CalcCoord.fHeight *= GetScreenHeightRatio();
}

void CEtUIDialog::CalcTextRect( LPCWSTR szText, int fontIndex, int fontHeight, SUICoord &CalcCoord, DWORD dwFontFormat, bool bUseCache )
{
	//ASSERT( pElement&&"CEtUIDialog::CalcTextRect, pElement is NULL!" );
	ASSERT( fontHeight>=0&&"CEtUIDialog::CalcTextRect" );

	dwFontFormat |= DT_WORDBREAK;

	CEtFontMng::GetInstance().CalcTextRect( fontIndex, fontHeight, szText, dwFontFormat, CalcCoord, -1, bUseCache );
	CalcCoord.fWidth *= GetScreenWidthRatio();
	CalcCoord.fHeight *= GetScreenHeightRatio();
}

void CEtUIDialog::DrawDlgText( LPCWSTR szText, SUIElement *pElement, DWORD dwFontColor, const SUICoord &Coord, int nCount, DWORD dwFontFormat, bool bClip, float ZValue, DWORD dwBgColor, int nBorderFlag )
{
	ASSERT( pElement&&"CEtUIDialog::DrawDlgText, pElement is NULL!" );
	ASSERT( pElement->nFontIndex>=0&&"CEtUIDialog::DrawDlgText" );

	if( !szText  || szText[0] == '\0' ) 
		return;

	SUICoord ScreenCoord;
	//blondy
	if( m_DlgInfo.bLockScalingByResolution )
	{
		ScreenCoord.fX = Coord.fX + GetXCoord() ;
		ScreenCoord.fY = Coord.fY + GetYCoord() ;
		ScreenCoord.fWidth = Coord.fWidth ;
		ScreenCoord.fHeight = Coord.fHeight ;

	}else
	{
		ScreenCoord.fX = ( Coord.fX + GetXCoord() ) / GetScreenWidthRatio();
		ScreenCoord.fY = ( Coord.fY + GetYCoord() ) / GetScreenHeightRatio();
		ScreenCoord.fWidth = Coord.fWidth / GetScreenWidthRatio();
		ScreenCoord.fHeight = Coord.fHeight / GetScreenHeightRatio();
	}

	if( dwFontFormat == 0xffffffff )
	{
		dwFontFormat = pElement->dwFontFormat;
	}

	if( !bClip )
	{
		dwFontFormat |= DT_WORDBREAK;
	}

	//if( pElement->bShadowFont )
	//{
	//	SUICoord ShadowCoord(ScreenCoord);
	//	ShadowCoord.fX += 1.0f / GetEtDevice()->Width();
	//	ShadowCoord.fY += 1.0f / GetEtDevice()->Height();
	//	CEtFontMng::GetInstance().DrawTextW( pElement->nFontIndex, pElement->nFontHeight, szText, pElement->ShadowFontColor.dwCurrentColor, dwFontFormat, ShadowCoord, nCount );
	//}
	D3DXCOLOR textureColor( dwFontColor );
	D3DXCOLOR shadowColor( pElement->ShadowFontColor.dwCurrentColor );

	if( textureColor.a != 1.0f )
		shadowColor.a = shadowColor.a * textureColor.a;

	SFontDrawEffectInfo Info;
	Info.nDrawType = pElement->nDrawType;
	Info.nWeight = pElement->nWeight;
	Info.fAlphaWeight = pElement->fAlphaWeight;
	Info.fGlobalBlurAlphaWeight = pElement->fGlobalBlurAlphaWeight;
	Info.dwFontColor = dwFontColor;
	Info.dwEffectColor = shadowColor; //pElement->ShadowFontColor.dwColor[UI_STATE_NORMAL];
	CEtFontMng::GetInstance().DrawTextW( pElement->nFontIndex, pElement->nFontHeight, szText, dwFontFormat, ScreenCoord, nCount, Info, true, ZValue , dwBgColor, nBorderFlag );
}

void CEtUIDialog::DrawSprite( EtTextureHandle hTexture, SUICoord &UVCoord, DWORD dwUIColor, SUICoord &Coord, float fRotate, float ZValue )
{
	if( UVCoord.fWidth == 0.0f )
		return;

	if( ( ( dwUIColor & 0xff000000 ) == 0 ) || ( !hTexture ) )
		return;

	if( !hTexture || !hTexture->IsReady() )
		return;

	SUICoord ScreenCoord;
	ScreenCoord.SetPosition( ( Coord.fX + GetXCoord() ) / GetScreenWidthRatio(), ( Coord.fY + GetYCoord() ) / GetScreenHeightRatio() );
	ScreenCoord.SetSize( Coord.fWidth / GetScreenWidthRatio(), Coord.fHeight / GetScreenHeightRatio() );

	CEtSprite::GetInstance().DrawSprite( ( EtTexture * )hTexture->GetTexturePtr(), hTexture->Width(), hTexture->Height(), UVCoord, dwUIColor, ScreenCoord, fRotate, ZValue );
}

void CEtUIDialog::DrawSprite( SUICoord &UVCoord, DWORD dwUIColor, SUICoord &Coord, float fRotate, float ZValue )
{
	DrawSprite( m_hTexture, UVCoord, dwUIColor, Coord, fRotate, ZValue );
}

void CEtUIDialog::DrawRect( SUICoord &Coord, DWORD dwColor )
{
	if( ( Coord.fWidth <= 0.0f ) && ( Coord.fHeight <= 0.0f ) )
	{
		return;
	}

	SUICoord ScreenCoord = DlgCoordToScreenCoord( Coord );

	CEtSprite::GetInstance().DrawRect( ScreenCoord, dwColor );
}

SUICoord CEtUIDialog::DlgCoordToScreenCoord( const SUICoord &Coord )
{
	SUICoord ResultCoord;
	ResultCoord.fX = ( Coord.fX + GetXCoord() ) / GetScreenWidthRatio();
	ResultCoord.fY = ( Coord.fY + GetYCoord() ) / GetScreenHeightRatio();
	ResultCoord.fWidth = Coord.fWidth / GetScreenWidthRatio();
	ResultCoord.fHeight = Coord.fHeight / GetScreenHeightRatio();
	return ResultCoord;
}

SUICoord CEtUIDialog::ScreenCoordToDlgCoord( SUICoord &Coord )
{
	SUICoord ResultCoord;
	ResultCoord.fX = ( Coord.fX * GetScreenWidthRatio() ) - GetXCoord();
	ResultCoord.fY = ( Coord.fY * GetScreenHeightRatio() ) - GetYCoord();
	ResultCoord.fWidth = Coord.fWidth * GetScreenWidthRatio();
	ResultCoord.fHeight = Coord.fHeight * GetScreenHeightRatio();
	return ResultCoord;
}

void CEtUIDialog::UpdateDlgCoord( float fX, float fY, float fWidth, float fHeight )
{
	m_DlgInfo.DlgCoord.fX += fX;
	m_DlgInfo.DlgCoord.fY += fY;
	m_DlgInfo.DlgCoord.fWidth += fWidth;
	m_DlgInfo.DlgCoord.fHeight += fHeight;
	
	UpdateRects();
}

bool CEtUIDialog::IsCmdControl( const char *szCmdCtlName )
{
	ASSERT(szCmdCtlName);
	return (m_strCmdControlName == szCmdCtlName);
}

CEtUITemplate *CEtUIDialog::GetTemplate( int nIndex ) 
{
	if( nIndex >= (int)m_vecUITemplate.size() )
	{
		CDebugSet::ToLogFile("CEtUIDialog::GetTemplate, nIndex >= (int)m_vecUITemplate.size()");
		nIndex = (int)(m_vecUITemplate.size()-1);
	}

	return m_vecUITemplate[ nIndex ];
}

void CEtUIDialog::FadeIn( float fElapsedTime )
{
	m_renderDlgColor.BlendEx( UI_STATE_NORMAL, fElapsedTime, 0.2f );
}

void CEtUIDialog::FadeOut( float fElapsedTime )
{
	m_renderDlgColor.BlendEx( UI_STATE_HIDDEN, fElapsedTime, 0.2f );
}

void CEtUIDialog::GetMouseMovePoints( float &fMouseX, float &fMouseY ) 
{ 
	fMouseX = m_fMouseX;
	fMouseY = m_fMouseY;
}

void CEtUIDialog::ProcessChangeResolution()
{
	for each( CEtUIDialog *pDialog in s_plistDialogBottomMsg ) pDialog->OnChangeResolution();	
	for each( CEtUIDialog *pDialog in s_plistDialogBottom ) pDialog->OnChangeResolution();	
	for each( CEtUIDialog *pDialog in s_plistDialogTopMsg ) pDialog->OnChangeResolution();	
	for each( CEtUIDialog *pDialog in s_plistDialogTop ) pDialog->OnChangeResolution();	
	for each( CEtUIDialog *pDialog in s_plistDialogFocus ) pDialog->OnChangeResolution();	
	for each( CEtUIDialog *pDialog in s_plistDialogModal ) pDialog->OnChangeResolution();	
	for each( CEtUIDialog *pDialog in s_plistDialogMostTop ) pDialog->OnChangeResolution();	

	// Seems right to do it even for Self (probably right to do while making bubble dialog created with Self..)
	for each( CEtUIDialog *pDialog in s_plistDialogSelf ) pDialog->OnChangeResolution();
}

void CEtUIDialog::SetForceDialogScale( float fScale )
{
	s_fDialogScale = fScale;
	if( CEtFontMng::IsActive() ) {
		CEtFontMng::GetInstance().OnLostDevice();
		CEtFontMng::GetInstance().OnResetDevice();
	}
	ProcessChangeResolution();
}

void CEtUIDialog::OnChangeResolution()
{
	UpdateScreen();
	UpdateRects();

	CEtUIControl *pControl(NULL);
	for( int i = 0; i < ( int )m_vecControl.size(); i++ )
	{
		pControl = m_vecControl[i];
		if( !pControl ) continue;
		pControl->OnChangeResolution();
	}
	
	for each( CEtUIDialog *pDialog in m_listChildDialog )
	{
		pDialog->OnChangeResolution();
	}

	for each( CEtUIDialog *pDialog in m_listChildModalDialog )
	{
		pDialog->OnChangeResolution();
	}
}

void CEtUIDialog::SetDlgInfo( SUIDialogInfo &DlgInfo )
{ 
	m_DlgInfo = DlgInfo; 
	m_renderDlgColor.dwColor[UI_STATE_NORMAL] = m_DlgInfo.dwDlgColor;
	m_renderDlgColor.dwCurrentColor = m_DlgInfo.dwDlgColor;
	UpdateRects();
}

void CEtUIDialog::GetDlgInfo( SUIDialogInfo &DlgInfo )
{ 
	DlgInfo = m_DlgInfo; 
}

void CEtUIDialog::ShowTooltipDlg( CEtUIControl *pControl, bool bShow, int nStringIndex, DWORD color, bool bPermanent )
{
	if( bShow )
	{
		s_nTooltipStringIndex = nStringIndex;
	}

	if( s_pTooltipDlg )
	{
		s_pTooltipDlg->SetTooltipControl( pControl );
		s_pTooltipDlg->SetTooltipInfo(color, bPermanent);
		s_pTooltipDlg->Show( bShow );
	}
}

void CEtUIDialog::ShowTooltipDlg( CEtUIControl *pControl, bool bShow, const std::wstring &strTooltip, DWORD color, bool bPermanent )
{
	if( bShow )
	{
		s_strTooltipString = strTooltip;
	}

	if( s_pTooltipDlg )
	{
		s_pTooltipDlg->SetTooltipControl( pControl );
		s_pTooltipDlg->SetTooltipInfo(color, bPermanent);
		s_pTooltipDlg->Show( bShow );
	}
}

bool CEtUIDialog::IsTooltipControl(CEtUIControl* pCtrl)
{
	CEtUIControl* ctrl = s_pTooltipDlg->GetTooltipCtrl();
	if (s_pTooltipDlg && ctrl)
		return (pCtrl == ctrl);

	return false;
}

void CEtUIDialog::GetScreenMouseMovePoints( float &fMouseX, float &fMouseY )
{
	fMouseX = s_fScreenMouseX;
	fMouseY = s_fScreenMouseY;
}

void CEtUIDialog::UpdateScreen()
{
	// Considered handling it this way, but there are two major concerns.
	// One is font handling, and the other is held as a sub-element like a scrollbar.
	// It seems it can only be used properly after handling these two.
	// from blondy Probably need to handle those things case by case
	
	if( m_DlgInfo.bLockScalingByResolution )
	{
		m_fScreenHeight = DEFAULT_UI_SCREEN_HEIGHT;
		m_fScreenWidth = DEFAULT_UI_SCREEN_WIDTH;
	}
	else
	{
		m_fScreenHeight = DEFAULT_UI_SCREEN_HEIGHT * s_fDialogScale;
		m_fScreenWidth = DEFAULT_UI_SCREEN_WIDTH * s_fDialogScale;
	}

	m_fScreenHeight = DEFAULT_UI_SCREEN_HEIGHT * s_fDialogScale;
	m_fScreenHeightRatio = GetEtDevice()->Height() / m_fScreenHeight;
	m_fScreenHeightBorder = (m_fScreenHeightRatio - 1.0f) * 0.5f;

	m_fScreenWidth = DEFAULT_UI_SCREEN_WIDTH * s_fDialogScale;
	m_fScreenWidthRatio = GetEtDevice()->Width() / m_fScreenWidth;
	m_fScreenWidthBorder = ( m_fScreenWidthRatio - 1.0f ) * 0.5f;

	//if( (fWidth/fHeight) >= DEFAULT_UI_SCREEN_RATIO )
	//{
	//	// Note: If aspect ratio is wider than default 4:3, apply widescreen mode.
	//	//
	//	m_fScreenWidth = fHeight * DEFAULT_UI_SCREEN_RATIO * s_fDialogScale;
	//}
	//else
	//{
	//	m_fScreenWidth = DEFAULT_UI_SCREEN_WIDTH * s_fDialogScale;
	//}
}

void CEtUIDialog::CalcDialogScaleByResolution( int nWidth, int nHeight )
{
	float fHR = float(nHeight) / DEFAULT_UI_SCREEN_HEIGHT;
	float fWR = float(nWidth) / DEFAULT_UI_SCREEN_WIDTH;

	s_fDialogScaleValueVert = ( fHR - 1.0f ) * UI_DIALOG_SCALE_VALUE;
	if( s_fDialogScaleValueVert < 0.0f )
	{
		s_fDialogScaleValueVert = 0.0f;
	}
	s_fDialogScaleValueHori = ( fWR - 1.0f ) * UI_DIALOG_SCALE_VALUE;
	if( s_fDialogScaleValueHori < 0.0f )
	{
		s_fDialogScaleValueHori = 0.0f;
	}

	// Need to reprocess UISize when resolution changes.
	SetDialogSize( s_nUISize, nWidth, nHeight );
}

void CEtUIDialog::AddChildDialog( CEtUIDialog *pDialog )
{
	ASSERT(pDialog&&"CEtUIDialog::AddChildDialog");
	m_listChildDialog.push_back( pDialog );
}

void CEtUIDialog::AddChildModalDialog( CEtUIDialog *pDialog )
{
	ASSERT(pDialog&&"CEtUIDialog::AddChildModalDialog");
	m_listChildModalDialog.push_back( pDialog );
}

void CEtUIDialog::DelChildDialog( CEtUIDialog *pDialog )
{
	ASSERT(pDialog&&"CEtUIDialog::DelChildDialog");

	std::list<CEtUIDialog*>::iterator iter = m_listChildDialog.begin();
	for( ; iter != m_listChildDialog.end(); )
	{
		if( (*iter) == pDialog )
		{
			iter = m_listChildDialog.erase( iter );
			continue;
		}
		++iter;
	}
}

void CEtUIDialog::DelChildModalDialog( CEtUIDialog *pDialog )
{
	ASSERT(pDialog&&"CEtUIDialog::DelChildModalDialog");

	std::list<CEtUIDialog*>::iterator iter = m_listChildModalDialog.begin();
	for( ; iter != m_listChildModalDialog.end(); )
	{
		if( (*iter) == pDialog )
		{
			iter = m_listChildModalDialog.erase( iter );
			continue;
		}
		++iter;
	}
}

void CEtUIDialog::ShowChildDialog( CEtUIDialog *pDialog, bool bShow )
{
	if( !pDialog ) 
		return;

	ASSERT( (pDialog!=this)&&"CEtUIDialog::ShowChildDialog" );

	if( pDialog->GetDialogType() == UI_TYPE_CHILD_MODAL )
	{
		ShowChildDialog( m_listChildModalDialog, pDialog, bShow );
	}
	else
	{
		ShowChildDialog( m_listChildDialog, pDialog, bShow );
	}
}

void CEtUIDialog::ShowChildDialog( std::list<CEtUIDialog*> &listDialog, CEtUIDialog *pDialog, bool bShow )
{
	if( !pDialog )				return;
	if( listDialog.empty() )	return;

	CEtUIDialog *pChildDialog(NULL);

	std::list<CEtUIDialog*>::iterator iter = listDialog.begin();
	for( ; iter != listDialog.end(); ++iter )
	{
		if( (*iter) == pDialog )
		{
			pChildDialog = (*iter);
			listDialog.erase( iter );
			break; // Note : Assumes child window IDs are unique.
		}
	}

	// Note : Reorder child windows in the list when Show is called.
	//		In the end, the render and message processing order is reversed.
	//
	if( pChildDialog )
	{
		// When content dialogs overlap like a skill tree, input messages do not reach the smaller dialog 
		// Check when calling CEtUIDialog::ShowChildDialog() and set it to come last during focusing.
		vector<CEtUIDialog*> vlpContentDialogs;
		pChildDialog->GetContentDialog( vlpContentDialogs );
		if( false == vlpContentDialogs.empty() )
		{
			for( int i = 0; i < (int)vlpContentDialogs.size(); ++i )
			{
				CEtUIDialog* pContentDialog = vlpContentDialogs.at( i );
				list<CEtUIDialog*>::iterator iterContentDlg = find( listDialog.begin(), listDialog.end(), pContentDialog );
				_ASSERT( listDialog.end() != iterContentDlg );
				if( listDialog.end() != iterContentDlg )
					listDialog.erase( iterContentDlg );
			}
		}

		// Since message delivery is in reverse order, move to the back when focused,
		// When hidden, place right behind the main dialog moved to the front.
		if( bShow )
		{
			listDialog.push_back( pChildDialog );
			if( false == vlpContentDialogs.empty() )
			{
				for( int i = 0; i < (int)vlpContentDialogs.size(); ++i )
				{
					CEtUIDialog* pContentDialog = vlpContentDialogs.at( i );
					listDialog.push_back( pContentDialog );
				}
			}
		}
		else
		{
			if( false == vlpContentDialogs.empty() )
			{
				for( int i = 0; i < (int)vlpContentDialogs.size(); ++i )
				{
					CEtUIDialog* pContentDialog = vlpContentDialogs.at( i );
					listDialog.push_front( pContentDialog );
				}
			}
			listDialog.push_front( pChildDialog );
		}

		pChildDialog->Show( bShow );
	}
	else
	{
		CDebugSet::ToLogFile( "CEtUIDialog::ShowChildDialog, pChildDialog is NULL!" );
	}
}

bool CEtUIDialog::IsChildDialog( const CEtUIDialog *pDialog )
{
	if( !pDialog )
		return false;

	std::list<CEtUIDialog*>::iterator iter = m_listChildDialog.begin();
	for( ; iter != m_listChildDialog.end(); ++iter )
	{
		if( (*iter) == pDialog )
		{
			return true;
		}

		if( (*iter)->IsChildDialog( pDialog ) )
		{
			return true;
		}
	}

	iter = m_listChildModalDialog.begin();
	for( ; iter != m_listChildModalDialog.end(); ++iter )
	{
		if( (*iter) == pDialog )
		{
			return true;
		}

		if( (*iter)->IsChildDialog( pDialog ) )
		{
			return true;
		}
	}

	return false;
}

void CEtUIDialog::UpDialogSize()
{
	CalcDialogScaleValue();

	if( s_fDialogScaleValueVert <= 0.0f )			return;
	if( s_fDialogScale == s_fMaxDialogScale )	return;

	s_fDialogScale += s_fDialogScaleValueVert;
	if( s_fDialogScale > s_fMaxDialogScale )
	{
		s_fDialogScale = s_fMaxDialogScale;
	}

	if( CEtFontMng::IsActive() ) {
		CEtFontMng::GetInstance().OnLostDevice();
		CEtFontMng::GetInstance().OnResetDevice();
	}
	ProcessChangeResolution();
}

void CEtUIDialog::DownDialogSize()
{
	CalcDialogScaleValue();

	if( s_fDialogScaleValueVert <= 0.0f )			return;
	if( s_fDialogScale == s_fMinDialogScale )	return;

	s_fDialogScale -= s_fDialogScaleValueVert;
	if( s_fDialogScale < s_fMinDialogScale )
	{
		s_fDialogScale = s_fMinDialogScale;
	}

	if( CEtFontMng::IsActive() ) {
		CEtFontMng::GetInstance().OnLostDevice();
		CEtFontMng::GetInstance().OnResetDevice();
	}
	ProcessChangeResolution();
}

void CEtUIDialog::CalcDialogScaleValue( int nWidth, int nHeight )
{
	// Keeps trying to support resolutions smaller than the default resolution.
	// So like this, when a resolution smaller than the default resolution is used, disable separate UISize adjustment
	// Use the calculated default scale as is.
	if( nWidth < DEFAULT_UI_SCREEN_WIDTH || nHeight < DEFAULT_UI_SCREEN_HEIGHT )
	{
		s_fMaxDialogScale = s_fDialogScale;
		s_fMinDialogScale = s_fDialogScale;
		return;
	}

	if( nWidth == 0 && nHeight == 0 )
	{
		nWidth = GetEtDevice()->Width();
		nHeight = GetEtDevice()->Height();
	}

	float fHR = float(nHeight) / DEFAULT_UI_SCREEN_HEIGHT;
	float fWR = float(nWidth) / DEFAULT_UI_SCREEN_WIDTH;

	s_fMaxDialogScale = min( fHR, fWR );
	s_fMinDialogScale = 2.0f - s_fMaxDialogScale;
	s_fDialogScaleValueVert = (s_fMaxDialogScale - 1.0f) / 2.0f;

	// Do not let it become too small.
	s_fMinDialogScale = (s_fMinDialogScale + 3.0f) / 4.0f;
}

// Requires resolution as an argument due to the window maximize button.
void CEtUIDialog::SetDialogSize( int nStep, int nWidth, int nHeight )
{
	if( nStep < 1 || nStep > 4 )
		return;

	if( nWidth == 0 && nHeight == 0 )
	{
		nWidth = GetEtDevice()->Width();
		nHeight = GetEtDevice()->Height();
	}

	// Dialog size calculation is scattered everywhere, so each case has to be handled individually like this.
	// Should clean this all up sometime later..
	static int s_nWidthInFunc = -1;
	static int s_nHeightInFunc = -1;
	if( s_nWidthInFunc == nWidth && s_nHeightInFunc == nHeight && s_nUISize == nStep )
		return;
	s_nWidthInFunc = nWidth;
	s_nHeightInFunc = nHeight;

	// Recalculate reference value.
	float fHR = float(nHeight) / DEFAULT_UI_SCREEN_HEIGHT;
	float fWR = float(nWidth) / DEFAULT_UI_SCREEN_WIDTH;

	s_fDialogScaleValueVert = ( fHR - 1.0f ) * UI_DIALOG_SCALE_VALUE;
	if( s_fDialogScaleValueVert < 0.0f ) s_fDialogScaleValueVert = 0.0f;
	s_fDialogScaleValueHori = ( fWR - 1.0f ) * UI_DIALOG_SCALE_VALUE;
	if( s_fDialogScaleValueHori < 0.0f ) s_fDialogScaleValueHori = 0.0f;

	if( fHR < fWR )
	{
		s_fDialogScale = fHR;
		s_fDialogScale -= s_fDialogScaleValueVert;
	}
	else
	{
		s_fDialogScale = fWR;
		s_fDialogScale -= s_fDialogScaleValueHori;
	}

	CalcDialogScaleValue( nWidth, nHeight );

	if( nStep == 1 )
	{
		s_fDialogScale += s_fDialogScaleValueVert;
		if( s_fDialogScale > s_fMaxDialogScale )
		{
			s_fDialogScale = s_fMaxDialogScale;
		}
	}
	else if( nStep == 2 )
	{
	}
	else if( nStep == 3 )
	{
		s_fDialogScale -= s_fDialogScaleValueVert;
		if( s_fDialogScale < s_fMinDialogScale )
		{
			s_fDialogScale = s_fMinDialogScale;
		}
	}
	else if( nStep == 4 )
	{
		s_fDialogScale -= s_fDialogScaleValueVert;
		s_fDialogScale -= s_fDialogScaleValueVert;
		if( s_fDialogScale < s_fMinDialogScale )
		{
			s_fDialogScale = s_fMinDialogScale;
		}
	}

	// Reset the font manager before calling ProcessChangeResolution since some controls compute regions from font size.
	if( CEtFontMng::IsActive() ) {
		CEtFontMng::GetInstance().OnLostDevice();
		CEtFontMng::GetInstance().OnResetDevice();
	}
	ProcessChangeResolution();

	s_nUISize = nStep;
}

void CEtUIDialog::ReleaseMouseEnterControl()
{
	if( s_pMouseEnterControl )
	{
		s_pMouseEnterControl->MouseEnter( false );
		s_pMouseEnterControl = NULL;

		ShowTooltipDlg( NULL, false );
	}
}

void CEtUIDialog::SetMouseEnterControl( CEtUIControl *pControl )
{
	ASSERT( pControl&&"CEtUIDialog::SetMouseEnterControl" );

	ReleaseMouseEnterControl();

	// Note : Register a new control and show its tooltip.
	//
	s_pMouseEnterControl = pControl;
	s_pMouseEnterControl->MouseEnter( true );

	if( drag::IsValid() )
	{
		// Note : Do not show tooltip if an item is being dragged.
		//
		return;
	}

	SUIControlProperty sProperty;
	s_pMouseEnterControl->GetProperty( sProperty );

	if( pControl->IsShow() )
	{
		if( sProperty.nTooltipStringIndex > 0 )
		{
			ShowTooltipDlg( pControl, true, sProperty.nTooltipStringIndex );
		}
		else
		{
			std::wstring strTooltipText = s_pMouseEnterControl->GetTooltipText();
			if( !strTooltipText.empty() )
			{
				ShowTooltipDlg( pControl, true, strTooltipText );
			}
		}
	}
}

#ifdef PRE_FIX_COMBOBOX_ELLIPSIS
bool CEtUIDialog::MakeStringWithEllipsis(SUIElement* pElement, float lineWidth, std::wstring& text, const std::wstring& symbol)
#else
void CEtUIDialog::MakeStringWithEllipsis(SUIElement* pElement, float lineWidth, std::wstring& text, const std::wstring& symbol)
#endif
{
#ifdef PRE_FIX_COMBOBOX_ELLIPSIS
	if( !pElement ) return false;
#else
	if( !pElement ) return;
#endif

	int fontIndex		= pElement->nFontIndex;
	int fontHeight		= pElement->nFontHeight;
	DWORD fontFormat	= pElement->dwFontFormat;

	SUICoord sSymbolCoord;
	//CalcTextRect( symbol.c_str(), pElement, sSymbolCoord );
	CalcTextRect(symbol.c_str(), fontIndex, fontHeight, sSymbolCoord, fontFormat, true);

	SUICoord sTextCoord;
	//CalcTextRect( text.c_str(), pElement, sTextCoord );
	CalcTextRect(text.c_str(), fontIndex, fontHeight, sTextCoord, fontFormat, false);
	const float& width = lineWidth;
	const float& height = GetFontHeight( fontIndex, fontHeight );

	if( sTextCoord.fWidth > width )
	{
		float fRemainWidth = width - sSymbolCoord.fWidth;
		int nStrLen = (int)text.length();
		float fTextCoordHeight = sTextCoord.fHeight;
		int i(0), nStartPos(0);

		for( ; (nStartPos+i)<nStrLen; i++)
		{
			SUICoord sSubTextCoord;
			//CalcTextRect( text.substr(nStartPos,i).c_str(), pElement, sSubTextCoord );
			CalcTextRect( text.substr(nStartPos,i).c_str(), fontIndex, fontHeight, sSubTextCoord, fontFormat, false);

			if( sTextCoord.fHeight < (height - fTextCoordHeight) )
			{
				// If text continues onto the next line...
				if( sSubTextCoord.fWidth > width )
				{
					nStartPos += i-1;
					i = 0;
					fTextCoordHeight += sSubTextCoord.fHeight;
				}
			}
			else
			{
				// Check the last line.
				if( sSubTextCoord.fWidth > fRemainWidth )
				{
					i--;

					text = text.substr(0,nStartPos+i);
					text += symbol;
#ifdef PRE_FIX_COMBOBOX_ELLIPSIS
					return true;
#else
					return;
#endif
				}
			}
		}
	}

#ifdef PRE_FIX_COMBOBOX_ELLIPSIS
	return false;
#endif
}
#pragma warning(default:4482)
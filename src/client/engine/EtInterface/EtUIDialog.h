#pragma once
#include "EtUIDialogInfo.h"
#include "EtUIControlProperty.h"
#include "EtUIDialogBase.h"
#include "LostDeviceProcess.h"
#include "EtUITemplate.h"

#define UI_FILE_STRING	"Eternity Engine UI File 0.3"
#define UI_HEADER_RESERVED	( 1024 - sizeof( SUIFileHeader ) )
#define UI_FILE_VERSION_01		0x0101
#define UI_FILE_VERSION			0x0102

#define PRE_FIX_COMBOBOX_ELLIPSIS

struct SUIFileHeader
{
	char szHeaderString[ 256 ];
	int nVersion;
	int nTemplateCount;
	int nControlCount;
	int nFontCount;

	SUIFileHeader()
	{
		ZeroMemory( this, sizeof(SUIFileHeader) );
		strcpy_s( szHeaderString, sizeof(szHeaderString), UI_FILE_STRING );
		nVersion = UI_FILE_VERSION;
	}
};

class CEtUIControl;
class CEtUITemplate;
struct SUIElement;
class CEtUIDialog;

class CEtUICallback
{
public:
	CEtUICallback() {}
	~CEtUICallback() {}

public:
	virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0 ) {}
};

class CEtUIDialog : public CEtUIDialogBase
{
public:
	CEtUIDialog( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL, bool bAutoCursor = false );
	virtual ~CEtUIDialog(void);

	enum FadeModeEnum {
		None,
		AllowRender,
		CancelRender,
	};

protected:
	bool m_bShow;
	bool m_bAcceptInputMsgWhenHide;		// Flag allowing messages to be received regardless of show state. Used to handle messages in skill tree where linked dialogs are hidden and rendered separately.
	HWND m_hWnd;
	std::vector< CEtUIControl * > m_vecControl;
	std::vector< CEtUITemplate * > m_vecUITemplate;
	std::vector< SUIElement > m_vecDlgFrameElement;
	std::vector< SUICoord > m_vecDlgFrameCoord;
	EtTextureHandle m_hTexture;
	EtTextureHandle m_hDlgTexture;
	CEtUIControl *m_pDefaultControl;
	std::vector< CEtUIControl* > m_TempControlList;

	int m_nDialogID;
	CEtUICallback *m_pCallback;

	SUIDialogInfo m_DlgInfo;

	float m_fScreenWidth;
	float m_fScreenWidthRatio;
	float m_fScreenWidthBorder;

	float m_fScreenHeight;
	float m_fScreenHeightRatio;
	float m_fScreenHeightBorder;

	std::string m_strCmdControlName;	// Note : Control name to be used inside ProcessCommand
	
	SUIColorEx m_renderDlgColor;
	bool m_bFadeOut;
	FadeModeEnum m_FadeMode;
	bool m_bMouseInDialog;
	bool m_bEndInitialize;

	float m_fMouseX;
	float m_fMouseY;

	SHORT m_HotKeyState;
	int m_nHotKey;

	CEtUIDialog *m_pParentDialog;						// Note : Own parent window.
	std::list<CEtUIDialog*> m_listChildDialog;			// Note : Child windows.
	std::list<CEtUIDialog*> m_listChildModalDialog;		// Note : Child windows that were created as modal.

	std::string m_strDialogFileName;
	SUICoord m_BaseDlgCoord;

	bool m_bPassMessageToChild;
	bool m_bShowModal;
	bool m_bAutoCursor;
	bool m_bCursor;

	// Mark listbox items or element items to distinguish them from regular dialogs.
	bool m_bElementDialog;
	bool m_bElementDialogShowState;

	std::map< std::string, CEtUIDialog*> m_cachedControls;
	std::vector< std::pair<CEtUIDialog*, bool> > m_vecChildRenderPriorityInfo;

public:
	// Forced scaling on the market dialog caused an edit box caret misalignment issue.
	// Please avoid using this elsewhere unless unavoidable.
	//
	// The drawback of the existing forced-scale method is,
	// Not integrated with FontMng, so mouse dragging and rendering must be handled separately.
	// If setting scale for visible dialogs (market, inventory, tooltip), it is better to modify s_fDialogScale.
	// Therefore, changed it to forcibly modify s_fDialogScale.
	static void SetForceDialogScale( float fScale );

public:
	static float s_fScreenMouseX;
	static float s_fScreenMouseY;

protected:
	// Note : Static variables related to tooltips below. Currently only one tooltip can be displayed at a time.
	//		However, this structure doesn't seem very flexible. Later, instead of static, per dialog
	//		It would be good to have these variables and functions.
	//
	static int s_nTooltipStringIndex;
	static std::wstring s_strTooltipString;
	static CEtUIDialog *s_pTooltipDlg;

	static float s_fDialogScale;
	static float s_fDialogScaleValueVert;
	static float s_fDialogScaleValueHori;
	static float s_fMinDialogScale;
	static float s_fMaxDialogScale;
	static int s_nUISize;
	static void (_stdcall *s_pAutoCursorPtr)( bool bShow );

	static int s_nDialogTextureSize;

	float m_fLastShowDelta;
	bool m_bLoadedTexture;
	

public:
	static CEtUIControl *s_pMouseEnterControl;

	static void ReleaseMouseEnterControl();
	static void SetMouseEnterControl( CEtUIControl *pControl );
	static void SetAutoCursorCallback( void (_stdcall *pFunc)( bool bShow ) ) { s_pAutoCursorPtr = pFunc; }

	// FadeForBlow apparently needs to be rendered while UI is hidden. Moving it up to the upper layer.
	static CEtUIDialog *s_pFadeDlg;

public:
	void AddChildDialog( CEtUIDialog *pDialog );
	void AddChildModalDialog( CEtUIDialog *pDialog );
	void SetChildRenderPriority(CEtUIDialog* pDlg, bool bTail);
	void RefreshChildRenderPriority();

protected:
	// Perhaps because child dialogs were never deleted in the middle before EtUIListBoxEx was introduced,
	// There was no function to delete child dialogs at all.
	// Then this function was added when trying to handle EtUIListBoxEx items as child dialogs.
	// (Not actually used since it is not handled as a child dialog.)
	//
	// Currently used in EtUIDialog's destructor to tell its parent to remove it from the list when destroyed.
	void DelChildDialog( CEtUIDialog *pDialog );
	void DelChildModalDialog( CEtUIDialog *pDialog );

public:
	bool IsRootDialog() { return (m_pParentDialog==NULL)?true:false; }
	bool IsChildDialog( const CEtUIDialog *pDialog );
	CEtUIDialog* GetParentDialog() { return m_pParentDialog; }

public:
	// Note : When showing a child window, call the function below instead of calling that window's Show().
	//
	void ShowChildDialog( CEtUIDialog *pDialog, bool bShow );

protected:
	void ShowChildDialog( std::list<CEtUIDialog*> &listDialog, CEtUIDialog *pDialog, bool bShow );
	bool IsAllowRender();

public:
	static void SetTooltipDlg( CEtUIDialog *pTooltipDlg ) { s_pTooltipDlg = pTooltipDlg; }
	static void ShowTooltipDlg( CEtUIControl *pControl, bool bShow, int nStringIndex = 0, DWORD color = 0xffffffff, bool bPermanent = false );
	static void ShowTooltipDlg( CEtUIControl *pControl, bool bShow, const std::wstring &strTooltip, DWORD color = 0xffffffff, bool bPermanent = false );
	static int GetTooltipStringIndex()		{ return s_nTooltipStringIndex; }
	static bool IsTooltipControl(CEtUIControl* pCtrl);
	static bool IsShowTooltipDlg() { return s_pTooltipDlg->IsShow(); }

	virtual void SetTooltipControl( CEtUIControl *pControl ) {}
	virtual void SetTooltipInfo(DWORD color = 0xffffffff, bool bPermanent = true)	{ _ASSERT(0); }
	virtual void ClearTooltipInfo()													{ _ASSERT(0); }
	virtual CEtUIControl* GetTooltipCtrl() const									{ _ASSERT(0); return NULL; }

	static float GetDialogScale() { return s_fDialogScale; }	
	static void UpDialogSize();
	static void DownDialogSize();
	static void CalcDialogScaleValue( int nWidth = 0, int nHeight = 0 );

	// Calculate and apply scale value based on resolution.
	static void CalcDialogScaleByResolution( int nWidth, int nHeight );

	static void SetDialogSize( int nStep, int nWidth = 0,  int nHeight = 0 );

	static int GetTextureMemorySize() { return s_nDialogTextureSize; }
	void ProcessMemoryOptimize( float fElapsedTime );
	void FreeDialogTexture();
	void LoadDialogTexture();

public:
	void SetFadeMode( FadeModeEnum Mode )	{ m_FadeMode = Mode; }

	bool IsMouseInDlg() { return m_bMouseInDialog; }
	void GetMouseMovePoints( float &fMouseX, float &fMouseY );
	void GetScreenMouseMovePoints( float &fMouseX, float &fMouseY );

	void SetRenderColor( DWORD dwColor )
	{ 
		m_renderDlgColor.dwColor[UI_STATE_NORMAL] = dwColor;
		m_renderDlgColor.dwCurrentColor = dwColor;
	}
	void SetSoundEnable( bool bEnable )			{ m_DlgInfo.bSound = bEnable; }
	// Note : Enter the child's message procedure even if the parent window is not visible.
	//		For example, used so clicking the chat window gives it focus when the chat window is hidden.
	//
	void SetPassMessageToChild( bool bPass )	{ m_bPassMessageToChild = bPass; }
	void SetShowModal( bool bShowModal )		{ m_bShowModal = bShowModal; }

	void SetElementDialog( bool bElement )		{ m_bElementDialog = bElement; }
	bool IsElementDialog()						{ return m_bElementDialog; }
	// In the case of ElementDialog, since it is usually created as UI_TYPE_SELF and directly handles MsgProc and Render,
	// Often IsShow cannot check if the dialog is currently shown. In that case, we have to set and retrieve it directly.
	virtual void SetElementDialogShowState( bool bShow )	{ m_bElementDialogShowState = bShow; }
	bool IsShowElementDialog()					{ return m_bElementDialogShowState; }

	// Function unavoidably created due to hotkey bug.
	void SetHotKeyState( SHORT HotKeyState )	{ m_HotKeyState = HotKeyState; }

	bool IsCmdControl( const char *szCmdCtlName );
protected:
	void UpdateScreen();
	void UpdateFrameRectsEx();

	void SetCmdControlName( const char *szCmdCtlName )		{ m_strCmdControlName = szCmdCtlName; }
	const std::string& GetCmdControlName() const			{ return m_strCmdControlName; }

	void FadeIn( float fElapsedTime );
	void FadeOut( float fElapsedTime );

	void SetFadeIn()		{ m_bFadeOut = false; }
	void SetFadeOut()		{ m_bFadeOut = true; }

	virtual bool OnMouseMove( float fX, float fY );

public:
	void CreateTestUIControl();

	bool Load_01( CStream &Stream, SUIFileHeader &fileheader );
	bool Load( CStream &Stream );
	bool Save( CStream &Stream );
	const char *GetDialogFileName() { return m_strDialogFileName.c_str(); }
	const char *GetDialogUITextureFileName() { return m_DlgInfo.szUITexturename; }

	void SetDialogID( int nID ) { m_nDialogID = nID; }
	int GetDialogID() { return m_nDialogID; }

	virtual void Show( bool bShow );
	virtual bool IsShow() const { return m_bShow; }
	void SetAcceptInputMsgWhenHide( bool bAcceptMsg ) { m_bAcceptInputMsgWhenHide = bAcceptMsg; };

	HWND GetHWnd() { return m_hWnd; }
	void SetHWnd( HWND hWnd ) { m_hWnd = hWnd; }

	void SetCallback( CEtUICallback *pCallback )	{ m_pCallback = pCallback; }
	CEtUICallback *GetCallBack()					{ return m_pCallback; }

	void SetDlgInfo( SUIDialogInfo &DlgInfo );
	void GetDlgInfo( SUIDialogInfo &DlgInfo );
	void ReloadDlgTexture();
	void DeleteDlgTexture();

	// Note : Default base resolution of the game is 1024x768. 
	//		Border is the size of the current screen size minus the default screen size.
	//		Ratio is the ratio between the default size and the current screen size.
	//
	float GetScreenWidth() { return m_fScreenWidth; }
	float GetScreenWidthBorderSize() { return m_fScreenWidthBorder; }
	float GetScreenWidthRatio() { return m_fScreenWidthRatio; }

	float GetScreenHeight() { return m_fScreenHeight; }
	float GetScreenHeightBorderSize() { return m_fScreenHeightBorder; }
	float GetScreenHeightRatio() { return m_fScreenHeightRatio; }

	//void GetScreenSize( float &fWidth, float &fHeight );
	float GetXCoord();
	float GetYCoord();
	virtual void MoveDialog( float fX, float fY );

	void SetAllignType( UIAllignHoriType Hori, UIAllignVertType Vert ) { m_DlgInfo.AllignHori = Hori; m_DlgInfo.AllignVert = Vert; }
	void GetAllignType( UIAllignHoriType &Hori, UIAllignVertType &Vert ) { Hori = m_DlgInfo.AllignHori; Vert = m_DlgInfo.AllignVert; }
	virtual void SetDlgCoord( const SUICoord &DlgCoord ) { m_DlgInfo.DlgCoord = DlgCoord; UpdateRects(); }
	void GetDlgCoord( SUICoord &Coord ) { Coord = m_DlgInfo.DlgCoord; }
	const SUICoord& GetDlgCoord() const { return m_DlgInfo.DlgCoord; }

	SUICoord DlgCoordToScreenCoord( const SUICoord &Coord );
	SUICoord ScreenCoordToDlgCoord( SUICoord &Coord );
	float Width() { return m_DlgInfo.DlgCoord.fWidth; }
	float Height() { return m_DlgInfo.DlgCoord.fHeight; }
	void GetPosition( float &fX, float &fY );
	void SetPosition( float fX, float fY );
	CEtUIControl *GetControlAtPoint( float fX, float fY );

	int GetTemplateCount() { return ( int )m_vecUITemplate.size(); }
	void SetTemplate( int nIndex, CEtUITemplate &Template );
	int AddTemplate( CEtUITemplate &Template );
	void DeleteTemplate( int nIndex );
	void DeleteAllTemplate();
	CEtUITemplate *GetTemplate( int nIndex );
	float GetFontHeight( int &nFontSetIndex, int nFontHeight );

	//ID3DXFont *GetFont( int &nFontSetIndex, int nFontHeight );
	const wchar_t* GetUIString( int nCategoryID, int nIndex );

	void PointToFloat( POINT pt, float &fX, float &fY );

	void SetUITextureName( const char *pszTextureName ) { strcpy_s( m_DlgInfo.szUITexturename, _MAX_PATH, pszTextureName ); }
	EtTextureHandle GetUITexture() { return m_hTexture; }
	void SetUITexture( EtTextureHandle hTexture ) { SAFE_RELEASE_SPTR( m_hTexture ); m_hTexture = hTexture; }

	// Note : Return a list of controls of the same class type.
	//		ex) std::vector<CButton*> vector
	//
	template < typename T > void GetGontrolList( std::vector< T* > &vecControl );
	std::vector<CEtUIControl*>& GetControlList() { return m_vecControl; }

	int GetControlCount() { return ( int )m_vecControl.size(); }
	CEtUIControl *GetControl( const char *pszControlName );
	template < typename T > 
	T* _GC( const char *pszControlName );
	template < typename T > 
	T* GetControl( const char *pszControlName, bool *pExistControl = NULL );

//#if defined(_DEBUG) || defined(_RDEBUG)
	static bool s_bUITest;
	static std::map<std::string, int> s_mapUITest;
//#endif

	CEtUIControl *GetControl( int nIndex ) { return m_vecControl[ nIndex ]; }
	void SetControl( int nIndex, CEtUIControl *pControl ) { m_vecControl[ nIndex ] = pControl; }
	CEtUIControl *CreateControl( UI_CONTROL_TYPE Type );
	void DeleteControl( CEtUIControl *pControl );
	void DeleteAllControl();

	virtual bool FindControl( std::vector< CEtUIControl* > &vecControl, int nTypeCount, UI_CONTROL_TYPE *pType, bool bCheckCoveredControl, std::vector<SUICoord> &vecDlgCoord );

	void MoveToHead( CEtUIControl *pControl );
	void MoveToTail( CEtUIControl *pControl );

	//void DeleteAllFont();
	bool IsUsableFocusControl();
	int FindControlIndex( CEtUIControl *pControl );
	CEtUIControl *GetNextControl( CEtUIControl *pControl );
	CEtUIControl *GetPrevControl( CEtUIControl *pControl );

	bool OnCycleFocus( bool bForward );
	void RequestFocus( CEtUIControl* pControl );
	void PushFocusControl();
	void PopFocusControl();
#ifdef PRE_FIX_COMBOBOX_ELLIPSIS
	bool MakeStringWithEllipsis(SUIElement* pElement, float lineWidth, std::wstring& text, const std::wstring& symbol);
#else
	void MakeStringWithEllipsis(SUIElement* pElement, float lineWidth, std::wstring& text, const std::wstring& symbol);
#endif

	// Note : The functions below are used in UITool.
	//
	void ShowAllControl( bool bShow );
	void EnableAllControl( bool bEnable );
	void UpdateAllControl();

	void ClearRadioButtonGroup( int nButtonGroup );

	void DrawDlgFrame();
	void CalcTextRect( LPCWSTR strText, SUIElement *pElement, SUICoord &Coord, int nCount = -1, DWORD dwFontFormat = 0xffffffff );
	void CalcTextRect( LPCWSTR szText, int fontIndex, int fontHeight, SUICoord &CalcCoord, DWORD dwFontFormat, bool bUseCache );
	void DrawDlgText( LPCWSTR szText, SUIElement *pElement, DWORD dwFontColor, const SUICoord &Coord, int nCount = -1, DWORD dwFontFormat = 0xffffffff, bool bClip = false, float ZValue = 0.f, DWORD dwBgColor = 0, int nBorderFlag = 0 );
	void DrawSprite( EtTextureHandle hTexture, SUICoord &UVCoord, DWORD dwUIColor, SUICoord &Coord, float fRotate = 0.0f, float ZValue = 0.f );
	void DrawSprite( SUICoord &UVCoord, DWORD dwUIColor, SUICoord &Coord, float fRotate = 0.0f, float ZValue = 0.f );
	void DrawRect( SUICoord &Coord, DWORD dwColor );

public:
	virtual void Initialize( const char *pFileName, bool bShow );
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate() {}
	virtual CEtUIControl *CreateControl( SUIControlProperty *pProperty );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void Render( float fElapsedTime );
	virtual void Process( float fElapsedTime );
	virtual void UpdateDlgCoord( float fX, float fY, float fWidth, float fHeight );
	virtual void UpdateRects();
	virtual void OnCloseCompletely() {}
	// Used in skill tree. Returns pointer to sibling dialog.
	// When content dialogs overlap like a skill tree, input messages do not reach the smaller dialog 
	// Check when calling CEtUIDialog::ShowChildDialog() and set it to come last during focusing.
	virtual void GetContentDialog( /*IN OUT*/ std::vector<CEtUIDialog*>& vlpContentDialogs ) {};

	virtual void OnChangeResolution();
	static void ProcessChangeResolution();

	virtual void OnEndInitialize();

	virtual void OnHandleMsg(int protocol, char* pData) {}
};

template < typename T > 
void CEtUIDialog::GetGontrolList( std::vector< T* > &vecControl )
{
	for( int i=0; i<(int)m_vecControl.size(); i++ )
	{
		if( m_vecControl[i] )
		{
			if( typeid(*m_vecControl[i]) == typeid( T ) )
			{
				vecControl.push_back( (T*)m_vecControl[i] );
			}
		}
	}
}

#define CONTROL( cls, name )	_GC<CEtUI##cls>(#name)
#define DN_CONTROL( cls, name )	_GC<CDn##cls>(#name)
// Optimized version
template < typename T > 
T* CEtUIDialog::_GC( const char *pszControlName )
{
	std::map< std::string, CEtUIDialog*>::iterator it = m_cachedControls.find(pszControlName);
	if( it != m_cachedControls.end() ) {
		return (T*)it->second;
	}
	bool bExistControl = false;
	T *pResult = GetControl<T>( pszControlName, &bExistControl);
	if( bExistControl ) {	// If it does not exist, keep showing a warning message so it can be noticed..
		m_cachedControls[ pszControlName ] = (CEtUIDialog*)pResult;
	}
	else {
		static bool bFirst = true;
		if( bFirst ) {
			ASSERT( false );
			bFirst = false;
		}
	}
	return pResult;
}

template < typename T > 
T* CEtUIDialog::GetControl( const char *pszControlName, bool *pExistControl )
{
	for( int i = 0; i < ( int )m_vecControl.size(); i++ )
	{
		if( strcmp( pszControlName, m_vecControl[ i ]->GetControlName() ) == 0 )
		{
#if defined(_DEBUG) || defined(_RDEBUG)			// Perform type checking only in debug builds to guard against mistakes.
			T* pResult = dynamic_cast<T*>(m_vecControl[ i ]);
			if( !pResult ) {
				ASSERT( 0 && "UI Control cast failed!!");
				T* pTempControl = new T( 0 );
				m_TempControlList.push_back( pTempControl );
				if( pExistControl ) *pExistControl = false;
				return pTempControl;
			}
#else
			T* pResult = static_cast<T*>(m_vecControl[ i ]);
#endif
			if( pExistControl ) *pExistControl = true;
			return pResult;
		}
	}

//#if defined(_DEBUG) || defined(_RDEBUG)
	//assert( 0 && "UI Control not found! See EtUIDialog.h line 374 for details");
	char szFileName[256];
	_GetFullFileName(szFileName, _countof(szFileName), m_strDialogFileName.c_str());
	OutputDebug(" [UI Error] In file \"%s\", %s Control could not be found!\n", szFileName, pszControlName);
//#endif

	// When outputting to a file, store entries in a string map and only output if absent to skip duplicates.
	if( s_bUITest )
	{
		std::string szKey = szFileName;
		szKey += "_";
		szKey += pszControlName;

		std::map<std::string, int>::iterator it = s_mapUITest.find( szKey );
		if( it == s_mapUITest.end() )
		{
			FILE *fp = NULL;
			fopen_s( &fp, "uitest.txt", "at" );
			fprintf_s( fp, "[UI Error] In file \"%s\", %s Control could not be found!\n", szFileName, pszControlName );
			fclose( fp );
			s_mapUITest.insert( make_pair( szKey, 1 ) );
		}
	}

	// Won't crash even without the control. Just won't work properly..
	for( DWORD i=0; i<m_TempControlList.size(); i++ ) {
		if( strcmp( m_TempControlList[i]->GetControlName(), pszControlName ) == NULL ) return static_cast<T*>(m_TempControlList[i]);
	}
	T* pTempControl = new T( 0 );
	pTempControl->SetParent( this );
	if( pszControlName ) _strcpy( pTempControl->GetProperty()->szUIName, _countof(pTempControl->GetProperty()->szUIName), pszControlName, (int)strlen(pszControlName) );
	m_TempControlList.push_back( pTempControl );
	if( pExistControl ) *pExistControl = false;
	return pTempControl;
}

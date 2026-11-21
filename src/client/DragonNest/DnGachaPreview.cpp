#include "StdAfx.h"
#include "DnGachaPreview.h"
#include "DnPlayerActor.h"
#include "DNTableFile.h"
#include "DnItem.h"
#include "DnTableDB.h"
#include "DnCommonUtil.h"
#include "DnGachaDlg.h"
#include "DnInterfaceString.h"
#include "DnInterface.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


#ifdef PRE_ADD_GACHA_JAPAN

using namespace DN_INTERFACE;
using namespace STRING;

// Try setting the gacha part spin duration to 0.2 seconds for now.
const float CHANGE_PART_GAP = 0.1f;

// Currently up to 15 items can be grouped into a set in the gacha set table.
const int NUM_GACHA_SETITEM = 15;

CDnGachaPreview::CDnGachaPreview( UI_DIALOG_TYPE dialogType /* = UI_TYPE_FOCUS */, 
								  CEtUIDialog *pParentDialog /* = NULL */, int nID /* = -1 */, CEtUICallback *pCallback /* = NULL */ ) :
								  CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, true ),
								  m_pBtnRotateLeft( NULL ),
								  m_pBtnRotateRight( NULL ),
								  m_pBtnRotateFront( NULL ),
								  m_pBtnRotateBack( NULL ),
								  m_pBtnAvatarViewArea( NULL ),
								  m_pTextBoxSetName( NULL ),
								  m_fAvatarViewMouseX( 0.0f ),
								  m_fAvatarViewMouseY( 0.0f ),
								  m_iNowSelectedClassID( 0 ),
								  m_iSelectedPart( -1 ),
								  m_iRouletteCostumePartIndex( 0 ),
								  m_fPartChangeTime( 0.0f ),
								  m_fElapsedTime( 0.0f ),
								  m_bSelectedMyJobClassCostume( false ),
								  m_iGachaShopID( 0 )
{

}

CDnGachaPreview::~CDnGachaPreview(void)
{
}


void
CDnGachaPreview::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "Jp_GachaPreview.ui" ).c_str(), bShow );

}


void
CDnGachaPreview::InitialUpdate( void )
{
	m_pBtnRotateLeft = GetControl<CEtUIButton>("ID_ROTATE_LEFT");
	m_pBtnRotateRight = GetControl<CEtUIButton>("ID_ROTATE_RIGHT");

	m_pBtnRotateFront = GetControl<CEtUIButton>("ID_BUTTON_FRONT");
	m_pBtnRotateBack = GetControl<CEtUIButton>("ID_BUTTON_BACK");

	m_pBtnAvatarViewArea = GetControl<CEtUIButton>("ID_BUTTON_PREVIEW");

	m_pTextBoxSetName = GetControl<CEtUITextBox>("ID_TEXTBOX_SETITEM_NAME");

	SUICoord CharViewCoord;
	if (m_pBtnAvatarViewArea->GetProperty() != NULL)
		CharViewCoord = m_pBtnAvatarViewArea->GetProperty()->UICoord;

	const int nTextureSize = 512;
	SCameraInfo CameraInfo;
	CameraInfo.Target = CT_RENDERTARGET_NO_GENERATE_BACKBUFFER;
	CameraInfo.fNear = 10.f;
	CameraInfo.Type = CT_ORTHOGONAL;
	m_RenderAvatar.Initialize( nTextureSize, nTextureSize, CameraInfo, 160, 120, FMT_A8R8G8B8, true, true );
	m_RenderAvatar.CalcUVCoord( CharViewCoord.fWidth, CharViewCoord.fHeight );
}


void 
CDnGachaPreview::Show( bool bShow )
{
	if( IsShow() == bShow )
		return;

	CDnCustomDlg::Show( bShow );

	if( bShow )
	{
		m_iSelectedPart = -1;
		m_fPartChangeTime = 0.0f;
		m_fElapsedTime = 0.0f;

		// Gachapon shop ID received from server
		_ASSERT( 0 < m_iGachaShopID );
		InitializeCostumeSetInfo( m_iGachaShopID );

		//	Default : Local Actor
		CDnPlayerActor *pDnPlayerActor = static_cast<CDnPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());
		if( pDnPlayerActor == NULL ) 
			return;

		m_iNowSelectedClassID = pDnPlayerActor->GetClassID();

		// Show the designated product first instead of what the player is currently wearing.
		S_COSTUME_SET_INFO& CostumeSetInfo = m_mapCostumeSetInfoByJob[ m_iNowSelectedClassID ];
		CostumeSetInfo.iNowSelectedCostumeSet = 0;
		AttachCostume( pDnPlayerActor->GetClassID(), false );
	}
	else 
	{
		m_RenderAvatar.ResetActor();
	}	
}

int 
CDnGachaPreview::GetItemNeedJob( DNTableFileFormat*  pItemTable, int iItemTableID )
{
	DNTableCell Field;
	if( pItemTable->GetFieldFromLable( iItemTableID, "_NeedJobClass", Field ) == false )
		return 0;

	int iNeedJobClassID = 0;
	string strNeedJobClassID = pItemTable->GetFieldFromLablePtr( iItemTableID, "_NeedJobClass" )->GetString();
	_ASSERT( string::npos == strNeedJobClassID.find_first_of( ';' ) );

	iNeedJobClassID = atoi( strNeedJobClassID.c_str() );

	return iNeedJobClassID;
}


// Depending on which gacha NPC was clicked, viewable in that gacha by the ID sent from the server
// Fetch and hold appearance costume info from the table for later use.
void
CDnGachaPreview::InitializeCostumeSetInfo( int iGachaTableID )
{
	m_mapCostumeSetInfoByJob.clear();

	// #ifdef PRE_ADD_GACHA_JAPAN This class is only used when PRE_ADD_GACHA_JAPAN is defined.
	// Removed the define; if a compile error occurs here saying table is not registered, this UI class is not for Japanese version yet compiled 
	// Means it is included.
	DNTableFileFormat*  pItemTable = GetDNTable( CDnTableDB::TITEM );
	DNTableFileFormat*  pPartsTable = GetDNTable( CDnTableDB::TPARTS );
	DNTableFileFormat*  pGachaSetItemTable = GetDNTable( CDnTableDB::TGACHASETITEM_JP );

	// Collect items whose gacha number matches the shop numbers sent by the server.
	vector<int> vlSetItems; 
	pGachaSetItemTable->GetItemIDListFromField( "_GachaNum", iGachaTableID, vlSetItems );
	_ASSERT( false == vlSetItems.empty() );

	int iNumSetInfos = (int)vlSetItems.size();
	for( int iSetInfo = 0; iSetInfo < iNumSetInfos; ++iSetInfo )
	{
		int iSetItemTableItemID = vlSetItems.at( iSetInfo );

		S_COSTUME_SET_INFO* pCostumeSetInfo = NULL;
		S_COSTUME_INFO CostumeInfo;

		// Currently up to 15 items can be grouped into a set in the gacha set table.
		char acBuffer[ 128 ];
		for( int iItemInfoIndex = 1; iItemInfoIndex <= NUM_GACHA_SETITEM; ++iItemInfoIndex )
		{
			sprintf_s( acBuffer, "_SetItem%02d", iItemInfoIndex );
			int iItemTableID = pGachaSetItemTable->GetFieldFromLablePtr( iSetItemTableItemID, acBuffer )->GetInteger();

			if( 0 == iItemTableID )
				continue;

			// Get which class it belongs to and keep recording in that class's struct.
			// An item can have multiple required classes. However, in Japanese gacha there is always only one.
			// If required class is 0, it becomes usable by all classes.
			int iNeedJobClassID = GetItemNeedJob( pItemTable, iItemTableID );
			if( NULL == pCostumeSetInfo )
			{
				// If required class is 0, all classes can equip it.
				int k = iItemInfoIndex;
				int iLoopCount = 0;
				while( (k < NUM_GACHA_SETITEM) &&
					   (0 == iNeedJobClassID) )
				{
					// Check next item info and match job class.
					sprintf_s( acBuffer, "_SetItem%02d", k+1 );
					int iNextSetElementItemTableID = pGachaSetItemTable->GetFieldFromLablePtr( iSetItemTableItemID, acBuffer )->GetInteger();
					if( 0 == iNextSetElementItemTableID )
					{
						// Prevent infinite loop when job info cannot be matched due to invalid gacha set table data.
						++iLoopCount;
						if( 20 < iLoopCount )
							break;

						continue;
					}

					// If the item does not exist, show a message box.
					if( false == pGachaSetItemTable->IsExistItem( iNextSetElementItemTableID ) )
					{
						TCHAR atcBuffer[ 256 ] = { 0 };
						_stprintf_s( atcBuffer, _T("GachaSetItemTable Error - Not Exist Item: %d"), iNextSetElementItemTableID );
						//GetInterface().MessageBox( atcBuffer );
						GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", atcBuffer, false );
						iNeedJobClassID = -1;
						break;
					}

					iNeedJobClassID = GetItemNeedJob( pItemTable, iNextSetElementItemTableID );
				}

				// Ultimately, not all other set items can be released for all classes.
				_ASSERT( 0 < iNeedJobClassID );
				if( 0 < iNeedJobClassID )
				{
					pCostumeSetInfo = &(m_mapCostumeSetInfoByJob[ iNeedJobClassID ]);
					pCostumeSetInfo->iClassID = iNeedJobClassID;
				}
			}

			// Get what part type it is.
			int iType = pPartsTable->GetFieldFromLablePtr( iItemTableID, "_Parts" )->GetInteger();
			CostumeInfo.aiCashParts[ iType ] = iItemTableID;
		}

		if( pCostumeSetInfo )
		{
			// Name of this costume
			int iUIStringID = pGachaSetItemTable->GetFieldFromLablePtr( iSetItemTableItemID, "_SetName" )->GetInteger();
			CostumeInfo.strSetName = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, iUIStringID );

			// Grade of this costume set
			CostumeInfo.iGradeNameStringID = pGachaSetItemTable->GetFieldFromLablePtr( iSetItemTableItemID, "_Grade" )->GetInteger();

			pCostumeSetInfo->vlCostumes.push_back( CostumeInfo );
		}
	}
}



void
CDnGachaPreview::_ChangeCostumeParts( const S_COSTUME_INFO& CostumeInfo )
{
	for( int iPart = 0; iPart < CDnParts::PartsTypeEnum_Amount; ++iPart )
	{
		if( 0 < CostumeInfo.aiCashParts[ iPart ] )
		{
			AttachPart( CostumeInfo.aiCashParts[ iPart ] );
		}
	}

	// Change costume set name
	//m_pTextBoxSetName->SetText( CostumeInfo.strSetName.c_str() );
	m_pTextBoxSetName->ClearText();
	m_pTextBoxSetName->SetText( CostumeInfo.strSetName.c_str(), ITEM::RANK_2_COLOR((eItemRank)CostumeInfo.iGradeNameStringID) );
}


void
CDnGachaPreview::AttachCostume( int iClassID, bool bAllowAttachLocalActorCostume /*= true*/ )
{
	CDnPlayerActor* pLocalActor = (CDnActor::s_hLocalActor.GetPointer()->GetClassID() == iClassID) ? static_cast<CDnPlayerActor*>(CDnActor::s_hLocalActor.GetPointer()) : NULL;

	m_RenderAvatar.ResetActor( m_RenderAvatar.GetActorID() != iClassID );

	int i = 0;
	if( pLocalActor && 
		bAllowAttachLocalActorCostume )
	{
		m_RenderAvatar.SetActor( CDnActor::s_hLocalActor, false );
		m_bSelectedMyJobClassCostume = true;
	}
	else
	{
		m_RenderAvatar.SetActor( iClassID, false );
		
		// Replace the part defined as the currently selected costume set.
		map<int, S_COSTUME_SET_INFO>::iterator iter = m_mapCostumeSetInfoByJob.find( iClassID );
		if( m_mapCostumeSetInfoByJob.end() != iter )
		{
			const S_COSTUME_SET_INFO& CostumeSetInfo = iter->second;
			_ASSERT( false == CostumeSetInfo.vlCostumes.empty() );
			if( false == CostumeSetInfo.vlCostumes.empty() )
			{
				_ChangeCostumeParts( CostumeSetInfo.vlCostumes.at(CostumeSetInfo.iNowSelectedCostumeSet) );
			}
		}

		// If it is the player character's own class, apply own character's appearance.
		if( pLocalActor && iClassID == pLocalActor->GetClassID() )
		{
			// Replace head part.
			AttachPart( pLocalActor->GetParts(CDnParts::Face)->GetClassID() );
			AttachPart( pLocalActor->GetParts(CDnParts::Hair)->GetClassID() );

			CDnPlayerActor *pPlayerActor = static_cast<CDnPlayerActor*>(m_RenderAvatar.GetActor().GetPointer());
			if( pPlayerActor )
			{
				pPlayerActor->SetPartsColor( MAPartsBody::HairColor, pLocalActor->GetPartsColor( MAPartsBody::HairColor ) );
				pPlayerActor->SetPartsColor( MAPartsBody::SkinColor, pLocalActor->GetPartsColor( MAPartsBody::SkinColor ) );
				pPlayerActor->SetPartsColor( MAPartsBody::EyeColor, pLocalActor->GetPartsColor( MAPartsBody::EyeColor ) );
			}
		}

		//// Equip weapon in hand.
		//for( int iWeapon = 0; iWeapon < 2; ++iWeapon )
		//	AttachPart( DefaultCharPartsInfo.aiWeapons[ iWeapon ] );

		m_bSelectedMyJobClassCostume = false;

//#ifdef PRE_ADD_CASH_COSTUME_AURA
//		CDnPlayerActor *pPlayerActor = static_cast<CDnPlayerActor*>(m_RenderAvatar.GetActor().GetPointer());
//		if(pPlayerActor)
//			pPlayerActor->ComputeRTTModeCostumeAura();
//#endif
	}
}

void
CDnGachaPreview::AttachPart( int iItemID )
{
	m_RenderAvatar.AttachItem( iItemID );
}



void
CDnGachaPreview::UpdateJobSelection( int iClassID )
{
	// If self, attach own appearance as is; if another class, display dummy avatar.
	//DNVector( int ) vlJobHistory;
	//CDnActor::s_hLocalActor->GetJobHistory( vlJobHistory );
	//if( vlJobHistory.front() == iRootJobID )
	//{
	//	AttachPartsFromLocalActor();
	//}
	//else
	//{
	//	CreateAvatar( iRootJobID );
	//}
	m_pTextBoxSetName->ClearText();

	m_iNowSelectedClassID = iClassID;
	AttachCostume( iClassID, false );
}



// Show next costume set.
void
CDnGachaPreview::NextCostume( void )
{
	S_COSTUME_SET_INFO& CostumeSetInfo = m_mapCostumeSetInfoByJob[ m_iNowSelectedClassID ];
	
	// Changed so own costume is not visible.
	++CostumeSetInfo.iNowSelectedCostumeSet;
	if( (int)CostumeSetInfo.vlCostumes.size() <= CostumeSetInfo.iNowSelectedCostumeSet )
		CostumeSetInfo.iNowSelectedCostumeSet = 0;

	//_ASSERT( false == CostumeSetInfo.vlCostumes.empty() );

	//bool bMyClass = (CDnActor::s_hLocalActor->GetClassID() == m_iNowSelectedClassID);
	//bool bAllowAttachLocalActorCostume = false;

	//CostumeSetInfo.iNowSelectedCostumeSet += 1;
	//if( m_bSelectedMyJobClassCostume )
	//	CostumeSetInfo.iNowSelectedCostumeSet = 0;

	//if( (int)CostumeSetInfo.vlCostumes.size() <= CostumeSetInfo.iNowSelectedCostumeSet )
	//{
	//	if( bMyClass )
	//	{
	//		// When browsing costumes of own class, after cycling all the way around, set to own costume.
	//		//m_mapCostumeSetInfoByJob[ m_iNowSelectedClassID ].iNowSelectedCostumeSet = -1;
	//		bAllowAttachLocalActorCostume = true;
	//	}
	//	else
	//	{
	//		m_mapCostumeSetInfoByJob[ m_iNowSelectedClassID ].iNowSelectedCostumeSet = 0;
	//	}
	//}

	AttachCostume( m_iNowSelectedClassID, false );
}


// Show previous costume set.
void
CDnGachaPreview::PrevCostume( void )
{
	S_COSTUME_SET_INFO& CostumeSetInfo = m_mapCostumeSetInfoByJob[ m_iNowSelectedClassID ];

	// Changed so own costume is not visible.
	--CostumeSetInfo.iNowSelectedCostumeSet;
	if( CostumeSetInfo.iNowSelectedCostumeSet < 0 )
		CostumeSetInfo.iNowSelectedCostumeSet = (int)CostumeSetInfo.vlCostumes.size() - 1;

	//_ASSERT( false == CostumeSetInfo.vlCostumes.empty() );

	//bool bMyClass = (CDnActor::s_hLocalActor->GetClassID() == m_iNowSelectedClassID);
	//bool bAllowAttachLocalActorCostume = false;

	//CostumeSetInfo.iNowSelectedCostumeSet -= 1;
	//if( m_bSelectedMyJobClassCostume )
	//	CostumeSetInfo.iNowSelectedCostumeSet = (int)CostumeSetInfo.vlCostumes.size()-1;

	//if( CostumeSetInfo.iNowSelectedCostumeSet < 0 )
	//{
	//	if( bMyClass )
	//	{
	//		//m_mapCostumeSetInfoByJob[ m_iNowSelectedClassID ].iNowSelectedCostumeSet = (int)CostumeSetInfo.vlCostumes.size();
	//		bAllowAttachLocalActorCostume = true;
	//	}
	//	else
	//	{
	//		CostumeSetInfo.iNowSelectedCostumeSet = (int)CostumeSetInfo.vlCostumes.size()-1;
	//	}
	//}

	AttachCostume( m_iNowSelectedClassID, false );
}


void
CDnGachaPreview::SelectedPart( int iSelectedPart )
{
	// Selected part side keeps changing.
	//int iPrevSelectedPart = m_iSelectedPart;
	m_iSelectedPart = iSelectedPart;
	m_fPartChangeTime = 0.0f;
	m_fElapsedTime = 0.0f;

	//// If combo box is moved back to the position before selecting parts, restore original appearance.
	//if( m_hAvatar )
	//{
	//	if( m_iSelectedPart )
	//	{
	//		S_COSTUME_SET_INFO& CostumeSetInfo = m_mapCostumeSetInfoByJob[ m_iNowSelectedClassID ];
	//		CostumeSetInfo.
	//		AttachPart( CostumeSetInfo.aiCashParts[ iPrevSelectedPart ] );
	//	}
	//}

	//// -1 sets it to the original avatar.
	//if( -1 == m_iSelectedPart )
	//	AttachCostume( m_iNowSelectedClassID );
}



int
CDnGachaPreview::GetNumGachPartItem( int iClassID )
{
	int iResult = 0;

	map<int, S_COSTUME_SET_INFO>::iterator iter = m_mapCostumeSetInfoByJob.find( iClassID );
	if( m_mapCostumeSetInfoByJob.end() != iter )
	{
		iResult = (int)iter->second.vlCostumes.size();
	}

	return iResult;
}



int
CDnGachaPreview::GetGachaSelectedPartItemID( int iClassID, int iIndex )
{
	int iResult = 0;

	map<int, S_COSTUME_SET_INFO>::iterator iter = m_mapCostumeSetInfoByJob.find( iClassID );
	if( m_mapCostumeSetInfoByJob.end() != iter )
	{
		const S_COSTUME_SET_INFO& CostumeSetInfo = iter->second;
		if( iIndex < (int)CostumeSetInfo.vlCostumes.size() )
			iResult = CostumeSetInfo.vlCostumes.at( iIndex ).aiCashParts[ m_iSelectedPart ];
	}

	return iResult;
}



// UI system-related functions ///////////////////////////////////////////////////////////////////////////////////////////////
void
CDnGachaPreview::Process( float fDelta )
{
	CDnCustomDlg::Process( fDelta );

	if( IsShow() )
	{
		m_RenderAvatar.Process( fDelta );

		static float fRotScale = 200.f;
		if( m_pBtnRotateLeft->IsPressed() )
		{
			m_RenderAvatar.AddRotateYaw( fDelta * fRotScale );
		}
		else if( m_pBtnRotateRight->IsPressed() )
		{
			m_RenderAvatar.AddRotateYaw( -fDelta * fRotScale );
		}

		if( m_pBtnAvatarViewArea->IsPressed() )
		{
			float fX = m_pBtnAvatarViewArea->GetMouseCoord().fX;
			float fY = m_pBtnAvatarViewArea->GetMouseCoord().fY;
			if( m_fAvatarViewMouseX != 0.f && m_fAvatarViewMouseY != 0.f )
			{
				static float fMoveScale = 500.f;
				float fAddAngle = fMoveScale * sqrtf( (m_fAvatarViewMouseX-fX)*(m_fAvatarViewMouseX-fX)+(m_fAvatarViewMouseY-fY)*(m_fAvatarViewMouseY-fY) ) *  (((m_fAvatarViewMouseX-fX)>0.f)? 1.f : -1.f) ;
				m_RenderAvatar.AddRotateYaw( fAddAngle );
			}
			m_fAvatarViewMouseX = fX;
			m_fAvatarViewMouseY = fY;

			focus::SetFocus( m_pBtnAvatarViewArea );
		}
		else if( m_pBtnAvatarViewArea->IsRightPressed() )
		{
			float fX = m_pBtnAvatarViewArea->GetMouseCoord().fX;
			float fY = m_pBtnAvatarViewArea->GetMouseCoord().fY;

			if( m_fAvatarViewMouseX != 0.f && m_fAvatarViewMouseY != 0.f ) {
				float fDistX = m_fAvatarViewMouseX - fX;
				float fDistY = m_fAvatarViewMouseY - fY;
				m_RenderAvatar.Panning( fDistX * GetScreenWidth(), fDistY * GetScreenHeight() );
			}

			m_fAvatarViewMouseX = fX;
			m_fAvatarViewMouseY = fY;

			focus::SetFocus( m_pBtnAvatarViewArea );
		}
		else
		{
			m_fAvatarViewMouseX = 0.f;
			m_fAvatarViewMouseY = 0.f;
		}

		if( m_RenderAvatar.IsFrontView() )
		{
			m_pBtnRotateFront->Show( false );
			m_pBtnRotateBack->Show( true );
		}
		else 
		{
			m_pBtnRotateFront->Show( true );
			m_pBtnRotateBack->Show( false );
		}

		//// Continuously change the changeable parts of the selected part.
		//if( -1 != m_iSelectedPart )
		//{
		//	if( CHANGE_PART_GAP < m_fElapsedTime - m_fPartChangeTime )
		//	{
		//		const S_COSTUME_SET_INFO& CostumeSetInfo = m_mapCostumeSetInfoByJob[ m_iNowSelectedClassID ];
		//		_ASSERT( false == CostumeSetInfo.vlCostumes.empty() );

		//		if( false == CostumeSetInfo.vlCostumes.empty() )
		//		{
		//			m_iRouletteCostumePartIndex = (m_iRouletteCostumePartIndex + 1) % (int)CostumeSetInfo.vlCostumes.size();
		//			const S_COSTUME_INFO& RouletteCostume = CostumeSetInfo.vlCostumes.at( m_iRouletteCostumePartIndex );

		//			S_COSTUME_INFO ChangedPartCostumeInfo = CostumeSetInfo.vlCostumes.at( CostumeSetInfo.iNowSelectedCostumeSet );
		//			ChangedPartCostumeInfo.aiCashParts[ m_iSelectedPart ] = RouletteCostume.aiCashParts[ m_iSelectedPart ];

		//			//_ChangeCostumeParts( ChangedPartCostumeInfo );
		//			AttachPart( ChangedPartCostumeInfo.aiCashParts[ m_iSelectedPart ] );

		//			static_cast<CDnGachaDlg*>(m_pParentDialog)->OnChangeItem( ChangedPartCostumeInfo.aiCashParts[ m_iSelectedPart ] );
		//		}

		//		m_fPartChangeTime = m_fElapsedTime;
		//	}

		//	m_fElapsedTime += fDelta;
		//}
	}

	if( false == m_pBtnRotateLeft->IsMouseEnter() )
		m_pBtnRotateLeft->SetPressed( false );

	if( false == m_pBtnRotateRight->IsMouseEnter() )
		m_pBtnRotateRight->SetPressed( false );
}


void
CDnGachaPreview::Render( float fDelta )
{
	CDnCustomDlg::Render( fDelta );

	if( IsShow() )
	{
		SUICoord CharViewCoord;
		if (m_pBtnAvatarViewArea->GetProperty() != NULL)
			CharViewCoord = m_pBtnAvatarViewArea->GetProperty()->UICoord;

		DrawSprite( m_RenderAvatar.GetRTT(), m_RenderAvatar.GetUVCoord(), 0xFFFFFFFF, CharViewCoord );
	}
}


void
CDnGachaPreview::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg /* = 0 */ )
{
	CDnCustomDlg::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED ) 
	{
		if( IsCmdControl("ID_BUTTON_PREVIEW"))	
		{
			focus::ReleaseControl();
		}

		if( IsCmdControl("ID_BUTTON_FRONT") )
		{
			m_RenderAvatar.SetFrontView();
		}
		else
		if( IsCmdControl("ID_BUTTON_BACK") )
		{
			m_RenderAvatar.SetRearView();
		}
		else
		if( IsCmdControl("ID_BUTTON_ZOOMIN") )
		{
			m_RenderAvatar.ZoomIn();
		}
		else
		if( IsCmdControl("ID_BUTTON_ZOOMOUT") )
		{
			m_RenderAvatar.ZoomOut();
		}
		else
		if( IsCmdControl( "ID_BUTTON_LEFT" ) )
		{
			PrevCostume();
		}
		else
		if( IsCmdControl( "ID_BUTTON_RIGHT" ) )
		{
			NextCostume();
		}
	}

}


bool
CDnGachaPreview::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
	{
		return false;
	}

	switch( uMsg )
	{
		case WM_MOUSEWHEEL:
			{
				if( IsMouseInDlg() )
				{
					UINT uLines;
					SystemParametersInfo( SPI_GETWHEELSCROLLLINES, 0, &uLines, 0 );
					int nScrollAmount = int( ( short )HIWORD( wParam ) ) / WHEEL_DELTA * uLines;
					if( nScrollAmount < 0 ) 
					{
						m_RenderAvatar.ZoomOut();
					}
					else if( nScrollAmount > 0 )
					{
						m_RenderAvatar.ZoomIn();
					}
				}
			}
			break;
	}

	return CDnCustomDlg::MsgProc( hWnd, uMsg, wParam, lParam );
}
#endif // PRE_ADD_GACHA_JAPAN
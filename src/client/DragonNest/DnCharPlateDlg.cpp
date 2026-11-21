#include "Stdafx.h"
#include "DnCharPlateDlg.h"
#include "DnCharPlateInfoDlg.h"
#include "DnTooltipPlateDlg.h"
#include "DnGlyph.h"
#include "DnItemTask.h"
#include "DnInterface.h"
#include "DnMainMenuDlg.h"
#include "DnFadeInOutDlg.h"
#include "DnInvenTabDlg.h"
#include "DnSkillTask.h"
#include "TaskManager.h"
#include "DnLocalPlayerActor.h"
#include "DnTableDB.h"
#include "DnCashShopTask.h"
#include "DnCommonUtil.h"
#include "DnMoneyInputDlg.h"
#ifdef PRE_ADD_INSTANT_CASH_BUY
#include "DnInstantCashShopBuyDlg.h"
#include "DnCashShopTask.h"
#endif // PRE_ADD_INSTANT_CASH_BUY
#ifdef PRE_ADD_SHORTCUT_HELP_DIALOG
#include "DnSystemDlg.h"
#endif



CDnCharPlateDlg::CDnCharPlateDlg(UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
, m_pCharPlateInfoDlg( NULL )
, m_pTooltipPlateDlg( NULL )
, m_pRemoveItem( NULL )
, m_pTempSlotButton( NULL )
, m_nTempSlotIndex( 0 )
{
	m_VecGlyphSlotButton.reserve( 16 );

	ClearSelectGlyphAttach();

	m_pLiftItem = NULL;
	m_pLiftSlot = NULL;
	m_bWithDrawOpen = false;

	m_pWithDrawGlyphText = NULL;
	m_pWithDarwGlyphBackground = NULL;

	memset( m_pSkillSlotButton, NULL, sizeof(CDnSkillSlotButton *)*SPECIAL_GLYPH_COUNT );
	memset( m_pCashPeriod, NULL, sizeof(CEtUIStatic *)*CASHGLYPHSLOTMAX );
}

CDnCharPlateDlg::~CDnCharPlateDlg()
{
	SAFE_DELETE( m_pCharPlateInfoDlg );
	SAFE_DELETE( m_pTooltipPlateDlg );
}

void CDnCharPlateDlg::Initialize(bool bShow)
{
	CDnCustomDlg::Initialize( CEtResourceMng::GetInstance().GetFullName( "CharPlate.ui" ).c_str(), bShow );
}

void CDnCharPlateDlg::InitialUpdate()
{
	m_pCharPlateInfoDlg = new CDnCharPlateInfoDlg;
	m_pCharPlateInfoDlg->Initialize(false);

	m_pTooltipPlateDlg = new CDnTooltipPlateDlg( UI_TYPE_CHILD, this );
	m_pTooltipPlateDlg->Initialize( false );

	char szType[32]={0,};
	for( int itr = 0; itr < SPECIAL_GLYPH_COUNT; ++itr )
	{
		if( itr > 0 )
			sprintf_s( szType, "ID_EMBLEM_SKILL%d", itr );
		else
			_strcpy( szType, _countof(szType), "ID_EMBLEM_SKILL", (int)strlen("ID_EMBLEM_SKILL"));

		m_pSkillSlotButton[itr] = static_cast<CDnSkillSlotButton*>( GetControl<CEtUIControl>(szType) );
		m_pSkillSlotButton[itr]->SetSlotType( ST_SKILL );
		m_pSkillSlotButton[itr]->SetSlotState( SKILLSLOT_DISABLE );
	}

	for( int itr = 0; itr < CASHGLYPHSLOTMAX; ++itr )
	{
		sprintf_s( szType, "ID_STATIC_TIME%d", itr );

		m_pCashPeriod[itr] = GetControl<CEtUIStatic>(szType);
		m_pCashPeriod[itr]->Show( false );
	}

	char szControlName[32] = {0,};
	for( int itr = 0; itr < GLYPHMAX; ++itr )
	{
		sprintf_s( szControlName, _countof(szControlName), "ID_STATIC_COVER%d", itr );

		m_VecGlyphCover.push_back( GetControl<CEtUIStatic>(szControlName) );
	}

#if defined(PRE_ADD_DRAGON_FELLOWSHIP_GLYPH)
	for( int itr = GLYPH_DRAGON1; itr < GLYPHMAX; ++itr )
	{
		sprintf_s( szControlName, _countof(szControlName), "ID_STATIC_DSLOT%d", itr );
		m_vecDragonGlyphGB.push_back( GetControl<CEtUIStatic>(szControlName) );
	}
#endif	// #if defined(PRE_ADD_DRAGON_FELLOWSHIP_GLYPH)

	m_pWithDrawGlyphText = GetControl<CEtUIStatic>( "ID_TEXT_REMOVE" );
	m_pWithDarwGlyphBackground = GetControl<CEtUIStatic>( "ID_STATIC_BOARD" );

	m_pWithDrawGlyphText->Show( false );
	m_pWithDarwGlyphBackground->Show( false );

	RefreshCover( true );

#if defined(PRE_ADD_DRAGON_FELLOWSHIP_GLYPH)
	InitDragonGlyphSlot();
#endif	// #if defined(PRE_ADD_DRAGON_FELLOWSHIP_GLYPH)
}

void CDnCharPlateDlg::InitCustomControl( CEtUIControl *pControl )
{
	CDnItemSlotButton *pItemSlotButton(NULL);
	pItemSlotButton = static_cast<CDnItemSlotButton*>(pControl);

	if( strstr( pControl->GetControlName(), "ID_ITEM_PLATE" ) ) 
	{
		pItemSlotButton->SetSlotIndex((int)m_VecGlyphSlotButton.size());
		pItemSlotButton->SetSlotType(ST_CHARSTATUS);
		m_VecGlyphSlotButton.push_back( pItemSlotButton );
	}
}

void CDnCharPlateDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	if( !bShow )
	{
		m_pCharPlateInfoDlg->Show( false );
		m_pTooltipPlateDlg->Show( false );

		if( GetWithDrawOpen() )
			GetInterface().CloseBlind();

		SetWithDrawOpen( false );
	}
#ifdef PRE_ADD_INSTANT_CASH_BUY
	else
	{
		int nShowType = GetCashShopTask().GetInstantCashBuyEnableType( INSTANT_BUY_PLATE );
		if( nShowType == INSTANT_BUY_HIDE )
		{
			GetControl<CEtUIButton>( "UI_BUTTON_BUYSHORTCUT" )->Show( false );
			GetControl<CEtUIStatic>( "ID_STATIC_CASHBASE" )->Show( false );
		}
		else if( nShowType == INSTANT_BUY_SHOW )
		{
			GetControl<CEtUIButton>( "UI_BUTTON_BUYSHORTCUT" )->Show( true );
			GetControl<CEtUIButton>( "UI_BUTTON_BUYSHORTCUT" )->Enable( true );
			GetControl<CEtUIStatic>( "ID_STATIC_CASHBASE" )->Show( true );
		}
		else if( nShowType == INSTANT_BUY_DISABLE )
		{
			GetControl<CEtUIButton>( "UI_BUTTON_BUYSHORTCUT" )->Show( true );
			GetControl<CEtUIButton>( "UI_BUTTON_BUYSHORTCUT" )->Enable( false );
			GetControl<CEtUIStatic>( "ID_STATIC_CASHBASE" )->Show( true );
		}
	}
#endif // PRE_ADD_INSTANT_CASH_BUY

	CEtUIDialog::Show( bShow );
}

void CDnCharPlateDlg::Process( float fElapsedTime )
{
	CEtUIDialog::Process( fElapsedTime );

	if( !IsShow() )
		return;

	//While processing, search for empty slots and show that slot's info in a tooltip on mouseover.
	float fMouseX, fMouseY;
	SUICoord uiCoord;
	GetScreenMouseMovePoints( fMouseX, fMouseY );
	fMouseX -= GetXCoord();
	fMouseY -= GetYCoord();

	CDnSlotButton *pDragButton = ( CDnSlotButton * )drag::GetControl();

	if( pDragButton == NULL )
	{
#if defined( PRE_FIX_52464 )
		for( int itr = 0; itr < (int)m_VecGlyphSlotButton.size() - 3; ++itr )
#else
		for( int itr = 0; itr < (int)m_VecGlyphSlotButton.size(); ++itr )
#endif	// #if defined( PRE_FIX_52464 )
		{
			if ( m_VecGlyphSlotButton[itr]->IsEmptySlot() )
			{
				m_VecGlyphSlotButton[itr]->GetUICoord( uiCoord );

				if( uiCoord.IsInside( fMouseX, fMouseY ) )
				{
					m_pTooltipPlateDlg->SetGlyphSlot( GLYPH_SLOTINDEX_2_EQUIPINDEX( itr ) );
					//m_pTooltipPlateDlg->SetGlyphSlot( itr );
					GetScreenMouseMovePoints( fMouseX, fMouseY );
					m_pTooltipPlateDlg->SetPosition( fMouseX, fMouseY );
					SUICoord DlgCoord;
					m_pTooltipPlateDlg->GetDlgCoord(DlgCoord);
					static float fXRGap(4.0f/DEFAULT_UI_SCREEN_WIDTH);
					if( (DlgCoord.Right()+fXRGap) > GetScreenWidthRatio() )
						DlgCoord.fX -= (DlgCoord.Right()+fXRGap - GetScreenWidthRatio());
					m_pTooltipPlateDlg->SetDlgCoord(DlgCoord);
					m_pTooltipPlateDlg->Show( true );

					break;
				}
				else
					m_pTooltipPlateDlg->Show( false );
			}
		}
	}
}

void CDnCharPlateDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl( "ID_BUTTON_INFO" ) )
		{
			if( m_pCharPlateInfoDlg->IsShow() )
				m_pCharPlateInfoDlg->Show( false );
			else
			{
				m_pCharPlateInfoDlg->SetText();
				m_pCharPlateInfoDlg->Show( true );
			}
		}

#ifdef PRE_ADD_SHORTCUT_HELP_DIALOG
		else if( IsCmdControl("ID_BT_SMALLHELP_PLATE") )
		{
			CDnMainMenuDlg* pMainMenuDlg = GetInterface().GetMainMenuDialog();
			CDnSystemDlg* pSystemDlg = pMainMenuDlg->GetSystemDlg();
			if(pSystemDlg)
				pSystemDlg->ShowChoiceHelpDlg(HELP_SHORTCUT_CHAR_SET_PLATE);
		}
		else if( IsCmdControl("ID_BT_SMALLHELP_REMOVE") )
		{
			CDnMainMenuDlg* pMainMenuDlg = GetInterface().GetMainMenuDialog();
			CDnSystemDlg* pSystemDlg = pMainMenuDlg->GetSystemDlg();
			if(pSystemDlg)
				pSystemDlg->ShowChoiceHelpDlg(HELP_SHORTCUT_DESTROY_PLATE);
		}
#endif

#ifdef PRE_ADD_INSTANT_CASH_BUY
		else if( IsCmdControl( "UI_BUTTON_BUYSHORTCUT" ) )
		{
			GetInterface().ShowInstantCashShopBuyDlg( true, INSTANT_BUY_PLATE, this );
			return;
		}
		else if( IsCmdControl( "ID_CLOSE_DIALOG" ) )
		{
			CDnCharStatusDlg* pCharStatusDlg = dynamic_cast<CDnCharStatusDlg*>( GetParentDialog() );
			if( pCharStatusDlg && pCharStatusDlg->GetCallBack() )
				pCharStatusDlg->GetCallBack()->OnUICallbackProc( pCharStatusDlg->GetDialogID(), EVENT_BUTTON_CLICKED, GetControl( "ID_CLOSE_DIALOG" ) );

			return;
		}
#endif // PRE_ADD_INSTANT_CASH_BUY

		if( GetInterface().IsOpenAcceptRequestDialog() )
			return;

		if( strstr( pControl->GetControlName() , "ID_EMBLEM_SKILL") && uMsg & WM_LBUTTONUP)
		{
			CDnSkillSlotButton * pSkillControl = static_cast<CDnSkillSlotButton *>(pControl);
			if( !pSkillControl->GetItem() )	return;

			CDnSlotButton *pDragButton = ( CDnSlotButton * )drag::GetControl();

			if( pDragButton == NULL )
			{
				CDnQuickSlotButton * pPressedButton = static_cast<CDnQuickSlotButton*>(pControl);

				pPressedButton->EnableSplitMode(0);
				drag::SetControl(pPressedButton);

				// Pre-switch combat mode. Unlike potions, skills are only picked up when trying to register them to quick slots.
				CDnPlayerActor *pActor = static_cast<CDnPlayerActor *>(CDnActor::s_hLocalActor.GetPointer());
				if( !pActor->IsBattleMode() )
				{
					if( !pActor->IsDie() && ( pActor->IsStay() || pActor->IsMove() ) )
					{
						if( pActor->IsCanBattleMode() )
						{
							if( pActor->IsMove() ) pActor->CmdStop( "Stand" );
							pActor->CmdToggleBattle( true );
						}
					}
				}
			}
			else
			{
				pDragButton->DisableSplitMode(false);
				drag::ReleaseControl();
			}
		}

		if( strstr( pControl->GetControlName(), "ID_ITEM_PLATE" ) ) {
			CDnSlotButton *pDragButton;
			CDnItemSlotButton *pPressedButton;

			if( GetItemTask().IsRequestWait() ) return;

			if( GetInterface().GetFadeDlg() && (GetInterface().GetFadeDlg()->GetFadeMode() != CDnFadeInOutDlg::modeEnd) )
				return;

			// Character window shouldn't open during trade, but block unequipping/swapping just in case it does.
			CDnInvenTabDlg *pInvenDlg = (CDnInvenTabDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::INVENTORY_DIALOG );
			if( pInvenDlg && pInvenDlg->IsShowPrivateMarketDialog() ) return;

			pDragButton = ( CDnSlotButton * )drag::GetControl();
			pPressedButton = ( CDnItemSlotButton * )pControl;
			if( pDragButton == NULL )
			{
				if( !pPressedButton->GetItem() ) 
					return;

				CDnItem *pItem = (CDnItem*)pPressedButton->GetItem();
				if( !GetItemTask().CheckRequestMoveItem( true ) ) return;

				if( uMsg == WM_RBUTTONUP )
				{
					if( GetWithDrawOpen() )
					{
						m_pLiftItem = (CDnGlyph *)pItem;

						GlyphLift( (CDnGlyph *)pItem );
						return;
					}

					int nEquipIndexP = GLYPH_SLOTINDEX_2_EQUIPINDEX(pPressedButton->GetSlotIndex());
					if( nEquipIndexP == -1 ) return;

					m_pRemoveItem = pItem;

					if( (GLYPH_CASH1 <= nEquipIndexP && GLYPH_CASH3 >= nEquipIndexP) && !GetItemTask().IsCashGlyphOpen(nEquipIndexP - GLYPH_CASH1) )	// When the duration of a time-limited cash heraldry slot expires
					{
						GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 9033 ) );
					}
					else
					{
						std::wstring wszString = wstring( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 9018 ) );
						std::wstring::size_type nPosInStr = wszString.find( L"\\n" );

						WCHAR pString[256];
						std::wstring wszFirst = wszString.substr(0, nPosInStr);
						std::wstring wszSecond = wszString.substr(nPosInStr+2);

						swprintf_s( pString, _countof(pString), L"%s\n%s", wszFirst.c_str(), wszSecond.c_str() );

						GetInterface().MessageBox( pString, MB_YESNO, CDnCharStatusDlg::GLYPH_DETACH_DIALOG, this, false, true );
					}
					return;
				}
				else if( uMsg == WM_LBUTTONDOWN )
				{
					if( GetWithDrawOpen() )
					{
						m_pLiftItem = (CDnGlyph *)pItem;
						GlyphLift( (CDnGlyph *)pItem );
						return;
					}
				}

				// todo by kalliste : refactoring with DnInvenSlotDlg.cpp
				if (uMsg & VK_SHIFT)
				{
					if (GetInterface().SetNameLinkChat(*pItem))
						return;
				}

				//Heraldry cannot be dragged with left-click
				GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 9019 ) );
				/*pPressedButton->EnableSplitMode(1);
				drag::SetControl(pPressedButton);

				CEtSoundEngine::GetInstance().PlaySound( "2D", pItem->GetDragSoundIndex() );*/
			}
			else
			{
				pDragButton->DisableSplitMode(true);
				drag::ReleaseControl();

				int nEquipIndexP = GLYPH_SLOTINDEX_2_EQUIPINDEX(pPressedButton->GetSlotIndex());
				if( nEquipIndexP == -1 ) return;

				if( pDragButton->GetItem() ) 
				{
					if( !GetItemTask().CheckRequestMoveItem( false ) ) return;
					if( pDragButton == pPressedButton )
					{
						// Move in place
						CDnItem *pItem = dynamic_cast<CDnItem *>(pDragButton->GetItem());
						if( pItem ) CEtSoundEngine::GetInstance().PlaySound( "2D", pItem->GetDragSoundIndex() );
						return;
					}

					CDnLocalPlayerActor *pActor = (CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer();
					DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TGLYPHSLOT );

					int nID = pSox->GetItemID( nEquipIndexP );
					int nLevel = pSox->GetFieldFromLablePtr( nID, "_LevelLimit" )->GetInteger();

					if( nLevel > pActor->GetLevel() )
					{
						GetInterface().MessageBox( 9015, MB_OK );
						return;
					}

					switch( pDragButton->GetItem()->GetType() ) 
					{
					case MIInventoryItem::Item:
						{
							std::wstring wszLastErrorMsg;

							if( ((CDnItem*)pDragButton->GetItem())->IsSoulbBound() == false )
							{
								GetInterface().MessageBox( 1752, MB_OK );
								return;
							}

							if( GetItemTask().IsEquipItem( nEquipIndexP, ((CDnItem*)pDragButton->GetItem()), wszLastErrorMsg ) == false ) 
							{
								GetInterface().MessageBox( wszLastErrorMsg.c_str(), MB_OK );
								return;
							}
						
							CDnCharStatusDlg::eRetWearable ret = GetSelectWearableGlyphEquipType( pDragButton->GetItem(), (eGlyph &)nEquipIndexP, &wszLastErrorMsg );

							if(ret != CDnCharStatusDlg::eWEAR_ENABLE)
							{
								if (ret == CDnCharStatusDlg::eWEAR_UNABLE)
									GetInterface().MessageBox( wszLastErrorMsg.c_str(), MB_OK );
								else if (ret == CDnCharStatusDlg::eWEAR_NEED_UNSEAL)
									GetInterface().OpenItemUnsealDialog( pDragButton->GetItem(), pDragButton );
								return;
							}
						}
						break;
					default:
						return;
					}

					if( (GLYPH_CASH1 <= nEquipIndexP && GLYPH_CASH3 >= nEquipIndexP) && GetItemTask().IsCashGlyphOpen(nEquipIndexP - GLYPH_CASH1) )
					{
						tm date;
						__time64_t tTime = GetItemTask().GetCashGlyphPeriod(nEquipIndexP - GLYPH_CASH1);
						DnLocalTime_s( &date, &tTime );

						std::wstring wszTimeString;
						CommonUtil::GetDateString( CommonUtil::DATESTR_FULL, wszTimeString, date );

						WCHAR wszOutputString[512];
						swprintf_s( wszOutputString, _countof(wszOutputString), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 9039 ), wszTimeString.c_str() );

						std::wstring wszString = std::wstring( wszOutputString );
						std::wstring::size_type nPosFirst = wszString.find( L"\\n" );
						std::wstring::size_type nPostSecond = wszString.find( L"\\n", nPosFirst+2 );

						std::wstring wszFirst = wszString.substr( 0, nPosFirst );
						std::wstring wszSecond = wszString.substr( nPosFirst+2, nPostSecond - nPosFirst - 2 );
						std::wstring wszThird = wszString.substr( nPostSecond+2 );

						swprintf_s( wszOutputString, _countof(wszOutputString), L"%s\n%s\n%s", wszFirst.c_str(), wszSecond.c_str(), wszThird.c_str() );

						GetInterface().BigMessageBox( wszOutputString, MB_YESNO, CDnCharStatusDlg::GLYPH_ATTACH_DIALOG, this );
					}
					else
					{
						std::wstring wszString = wstring( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 9038 ) );
						std::wstring::size_type nPosInStr = wszString.find( L"\\n" );

						WCHAR pString[256];
						std::wstring wszFirst = wszString.substr(0, nPosInStr);
						std::wstring wszSecond = wszString.substr(nPosInStr+2);

						swprintf_s( pString, _countof(pString), L"%s\n%s", wszFirst.c_str(), wszSecond.c_str() );

						GetInterface().MessageBox( pString, MB_YESNO, CDnCharStatusDlg::GLYPH_ATTACH_DIALOG, this );
					}

					m_pTempSlotButton = pDragButton;		
					m_nTempSlotIndex = pDragButton->GetSlotIndex();
				}
			}
			return;
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

CDnCharStatusDlg::eRetWearable CDnCharPlateDlg::ValidWearableGlyphItem( const MIInventoryItem *pItem, std::wstring *strErrorMsg )
{
	if( strErrorMsg ) strErrorMsg->clear();

	if( !pItem )
	{
		if( strErrorMsg ) *strErrorMsg = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5059 );
		return CDnCharStatusDlg::eWEAR_UNABLE;
	}

	if( pItem->GetType() != MIInventoryItem::Item )
	{
		if( strErrorMsg ) *strErrorMsg = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5060 );
		return CDnCharStatusDlg::eWEAR_UNABLE;
	}

	const CDnItem *pEquipItem = dynamic_cast<const CDnItem*>(pItem);

	if( !pEquipItem )
	{
		if( strErrorMsg ) *strErrorMsg = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5060 );
		return CDnCharStatusDlg::eWEAR_UNABLE;
	}

	if (pEquipItem->IsSoulbBound() == false)
		return CDnCharStatusDlg::eWEAR_NEED_UNSEAL;

	switch( pEquipItem->GetItemType() )
	{
	case ITEMTYPE_GLYPH:
		{
			const CDnGlyph *pGlyph = dynamic_cast<const CDnGlyph*>(pEquipItem);
			if( !pGlyph ) 
			{
				if( strErrorMsg ) *strErrorMsg = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5061 );
				return CDnCharStatusDlg::eWEAR_UNABLE;
			}

			const CDnItem * pGlyphItem;

			for( int i=GLYPH_ENCHANT1; i<GLYPHMAX; i++ ) 
			{
				int nSlotIndex = GLYPH_EQUIPINDEX_2_SLOTINDEX(i);
				pGlyphItem = (CDnItem *)(m_VecGlyphSlotButton[nSlotIndex]->GetItem());

				if( pGlyphItem && pGlyphItem->GetTypeParam(1) == pEquipItem->GetTypeParam(1) )
				{
					if( strErrorMsg ) *strErrorMsg = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 9017 );
					return CDnCharStatusDlg::eWEAR_UNABLE;
				}
			}
		}
		break;
	case ITEMTYPE_PARTS:
	case ITEMTYPE_WEAPON:
		if( strErrorMsg ) *strErrorMsg = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5056 );
		return CDnCharStatusDlg::eWEAR_UNABLE;
	}

	return CDnCharStatusDlg::eWEAR_ENABLE;
}

CDnCharStatusDlg::eRetWearable CDnCharPlateDlg::GetSelectWearableGlyphEquipType( const MIInventoryItem *pItem, const eGlyph equipType, std::wstring *strErrorMsg )
{
	if( CDnCharStatusDlg::eWEAR_ENABLE == ValidWearableGlyphItem( pItem, strErrorMsg ) )
	{
		SetSelectGlyphAttach( equipType );

		const CDnItem * pEquipItem = dynamic_cast<const CDnItem *>( pItem );
		const CDnGlyph * pGlyph = dynamic_cast<const CDnGlyph *>( pItem );

		if( NULL == pEquipItem || NULL == pGlyph )
		{
			if( strErrorMsg ) *strErrorMsg = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5061 );	// UISTRING : Undefined error.
			return CDnCharStatusDlg::eWEAR_UNABLE;
		}

		if( GLYPH_ENCHANT1 > equipType || equipType >= GLYPHMAX )
		{
			if( strErrorMsg ) *strErrorMsg = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 9016 );	// UISTRING : Cannot equip or unequip crests.
			return CDnCharStatusDlg::eWEAR_UNABLE;
		}

		CDnLocalPlayerActor *pActor = (CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer();
		DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TGLYPHSLOT );

		int nSlotIndex = GLYPH_EQUIPINDEX_2_SLOTINDEX( equipType );
		int nID = pSox->GetItemID( equipType );
		int nLevel = pSox->GetFieldFromLablePtr( nID, "_LevelLimit" )->GetInteger();
		
		if( m_VecGlyphSlotButton[nSlotIndex]->IsEmptySlot() && nLevel <= pActor->GetLevel() )
		{
			std::wstring wszTemp;
			if( GetItemTask().IsEquipItem( equipType, pEquipItem, wszTemp ) == false )
			{
				if( strErrorMsg ) *strErrorMsg = wszTemp;
				return CDnCharStatusDlg::eWEAR_UNABLE;
			}

			if( GLYPH_CASH1 <= equipType && GLYPH_CASH3 >= equipType )
			{
				if( GetItemTask().IsCashGlyphOpen( equipType - GLYPH_CASH1 ) )
					return CDnCharStatusDlg::eWEAR_ENABLE;
			}
			else
				return CDnCharStatusDlg::eWEAR_ENABLE;
		}
	}

	if( strErrorMsg && strErrorMsg->empty() ) 
		*strErrorMsg = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 9015 );
	return CDnCharStatusDlg::eWEAR_UNABLE;
}

CDnCharStatusDlg::eRetWearable CDnCharPlateDlg::GetWearableGlyphEquipType( const MIInventoryItem *pItem, eGlyph &equipType, std::wstring *strErrorMsg )
{
	if( CDnCharStatusDlg::eWEAR_ENABLE == ValidWearableGlyphItem( pItem, strErrorMsg ) )
	{
		const CDnItem * pEquipItem = dynamic_cast<const CDnItem *>( pItem );
		const CDnGlyph * pGlyph = dynamic_cast<const CDnGlyph *>( pItem );

		if( NULL == pEquipItem || NULL == pGlyph )
		{
			if( strErrorMsg ) *strErrorMsg = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5061 );
			return CDnCharStatusDlg::eWEAR_UNABLE;
		}

		equipType = (eGlyph)-1;

		CDnLocalPlayerActor *pActor = (CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer();
		DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TGLYPHSLOT );

		switch( pGlyph->GetGlyphType() ) 
		{
		case CDnGlyph::Enchant:
			{
				for( int i=GLYPH_ENCHANT1; i<=GLYPH_ENCHANT8; i++ ) {
					int nSlotIndex = GLYPH_EQUIPINDEX_2_SLOTINDEX(i);
					int nID = pSox->GetItemID( i );
					int nLevel = pSox->GetFieldFromLablePtr( nID, "_LevelLimit" )->GetInteger();

					if( m_VecGlyphSlotButton[nSlotIndex]->IsEmptySlot() && nLevel <= pActor->GetLevel() )
					{
						equipType = (eGlyph)i;
						break;
					}
				}
			}
			break;
		case CDnGlyph::PassiveSkill:
			{
				for( int i=GLYPH_SKILL1; i<=GLYPH_SKILL4; i++ ) {
					int nSlotIndex = GLYPH_EQUIPINDEX_2_SLOTINDEX(i);
					int nID = pSox->GetItemID( i );
					int nLevel = pSox->GetFieldFromLablePtr( nID, "_LevelLimit" )->GetInteger();

					if( m_VecGlyphSlotButton[nSlotIndex]->IsEmptySlot() && nLevel <= pActor->GetLevel() ) {
						equipType = (eGlyph)i;
						break;
					}
				}
			}
			break;
		case CDnGlyph::ActiveSkill:
			{
				if( m_VecGlyphSlotButton[GLYPH_EQUIPINDEX_2_SLOTINDEX(GLYPH_SPECIALSKILL)]->IsEmptySlot() )
					equipType = GLYPH_SPECIALSKILL;
			}
			break;

#if defined(PRE_ADD_DRAGON_FELLOWSHIP_GLYPH)
		case CDnGlyph::DragonFellowship:
			{
				for( int itr = GLYPH_DRAGON1; itr <= GLYPH_DRAGON4; ++itr )
				{
					int nSlotIndex = GLYPH_EQUIPINDEX_2_SLOTINDEX(itr);
					int nID = pSox->GetItemID( itr );
					int nLevel = pSox->GetFieldFromLablePtr( nID, "_LevelLimit" )->GetInteger();

					if( m_VecGlyphSlotButton[nSlotIndex]->IsEmptySlot() && nLevel <= pActor->GetLevel() ) {
						equipType = (eGlyph)itr;
						break;
					}
				}
			}
			break;
#endif	// #if defined(PRE_ADD_DRAGON_FELLOWSHIP_GLYPH)
		}

		if( equipType == -1 )
		{
#if defined(PRE_ADD_DRAGON_FELLOWSHIP_GLYPH)
			for( int i=GLYPH_CASH1; i<=GLYPH_CASH3; i++ )
#else
			for( int i=GLYPH_CASH1; i<GLYPHMAX; i++ )
#endif	// #if defined(PRE_ADD_DRAGON_FELLOWSHIP_GLYPH)
			{
				int nSlotIndex = GLYPH_EQUIPINDEX_2_SLOTINDEX(i);
				if( m_VecGlyphSlotButton[nSlotIndex]->IsEmptySlot() && GetItemTask().IsCashGlyphOpen(i - GLYPH_CASH1) ) 
				{
					equipType = (eGlyph)i;
					break;
				}
			}
		}

		if( -1 != equipType  )
		{
			std::wstring wszTemp;
			if( GetItemTask().IsEquipItem( equipType, pEquipItem, wszTemp ) == false )
			{
				if( strErrorMsg ) *strErrorMsg = wszTemp;
				return CDnCharStatusDlg::eWEAR_UNABLE;
			}

			return CDnCharStatusDlg::eWEAR_ENABLE;
		}
	}

	if( strErrorMsg && strErrorMsg->empty() ) *strErrorMsg = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 9015 );
	return CDnCharStatusDlg::eWEAR_UNABLE;
}

int CDnCharPlateDlg::GLYPH_EQUIPINDEX_2_SLOTINDEX( int nEquipIndex )
{
	// 0, 1, 2, 4, 3, 5, 6, 7, 8, 9
	// 6, 7, 5, 8, 12, 9, 11, 10
	switch( nEquipIndex ) {
		case GLYPH_ENCHANT1: return 5;
		case GLYPH_ENCHANT2: return 6;
		case GLYPH_ENCHANT3: return 7;
		case GLYPH_ENCHANT4: return 8;
		case GLYPH_ENCHANT5: return 9;
		case GLYPH_ENCHANT6: return 10;
		case GLYPH_ENCHANT7: return 11;
		case GLYPH_ENCHANT8: return 12;
		case GLYPH_SKILL1:	 return 1;
		case GLYPH_SKILL2:	 return 2;
		case GLYPH_SKILL3:	 return 4;
		case GLYPH_SKILL4:	 return 3;
		case GLYPH_SPECIALSKILL: return 0;
		case GLYPH_CASH1: return 13;
		case GLYPH_CASH2: return 14;
		case GLYPH_CASH3: return 15;
#if defined(PRE_ADD_DRAGON_FELLOWSHIP_GLYPH)
		case GLYPH_DRAGON1:	return 16;
		case GLYPH_DRAGON2:	return 17;
		case GLYPH_DRAGON3:	return 18;
		case GLYPH_DRAGON4:	return 19;
#endif	// #if defined(PRE_ADD_DRAGON_FELLOWSHIP_GLYPH)
	}
	return 0;
}

int CDnCharPlateDlg::GLYPH_SLOTINDEX_2_EQUIPINDEX( int nSlotIndex )
{
	switch( nSlotIndex ) {
		case 5: return GLYPH_ENCHANT1;
		case 6: return GLYPH_ENCHANT2;
		case 7: return GLYPH_ENCHANT3;
		case 8: return GLYPH_ENCHANT4;
		case 9: return GLYPH_ENCHANT5;
		case 10: return GLYPH_ENCHANT6;
		case 11: return GLYPH_ENCHANT7;
		case 12: return GLYPH_ENCHANT8;
		case 1: return GLYPH_SKILL1;
		case 2: return GLYPH_SKILL2;
		case 4: return GLYPH_SKILL3;
		case 3: return GLYPH_SKILL4;
		case 0: return GLYPH_SPECIALSKILL;
		case 13: return GLYPH_CASH1;
		case 14: return GLYPH_CASH2;
		case 15: return GLYPH_CASH3;
#if defined(PRE_ADD_DRAGON_FELLOWSHIP_GLYPH)
		case 16: return GLYPH_DRAGON1;
		case 17: return GLYPH_DRAGON2;
		case 18: return GLYPH_DRAGON3;
		case 19: return GLYPH_DRAGON4;
#endif	// #if defined(PRE_ADD_DRAGON_FELLOWSHIP_GLYPH)
	}
	return 0;
}


void CDnCharPlateDlg::Render( float fElapsedTime )
{
	CEtUIDialog::Render( fElapsedTime );

	if( !IsShow() )
		return;

	if( !CDnActor::s_hLocalActor )
		return;

	if( drag::IsValid() )
	{
		CDnSlotButton *pDragButton = (CDnSlotButton*)drag::GetControl();
		switch( pDragButton->GetSlotType() ) {
			case ST_INVENTORY:
			case ST_INVENTORY_CASH:
			case ST_CHARSTATUS:
				break;
			default: return;
		}

		CDnItem *pItem = dynamic_cast<CDnItem *>(pDragButton->GetItem());
		if( !pItem ) return;

		switch( pItem->GetItemType() ) {
			case ITEMTYPE_GLYPH:
				{
					eGlyph equipType;
					if( GetWearableGlyphEquipType( pDragButton->GetItem(), equipType ) == CDnCharStatusDlg::eWEAR_ENABLE )
					{
						if( equipType >= GLYPH_ENCHANT1 && equipType <= GLYPH_ENCHANT8 ) {
							for( int i=GLYPH_ENCHANT1; i<=GLYPH_ENCHANT8; i++ ) {
								int nSlotIndex = GLYPH_EQUIPINDEX_2_SLOTINDEX(i);

								if( !m_VecGlyphCover[nSlotIndex]->IsShow() )
									m_VecGlyphSlotButton[nSlotIndex]->SetWearable( true );
							}
						}
						else if( equipType >= GLYPH_SKILL1 && equipType <= GLYPH_SKILL4 ) {
							for( int i=GLYPH_SKILL1; i<=GLYPH_SKILL4; i++ ) {
								int nSlotIndex = GLYPH_EQUIPINDEX_2_SLOTINDEX(i);

								if( !m_VecGlyphCover[nSlotIndex]->IsShow() )
									m_VecGlyphSlotButton[nSlotIndex]->SetWearable( true );
							}
						}
						else if( equipType == GLYPH_SPECIALSKILL ) {
							int nSlotIndex = GLYPH_EQUIPINDEX_2_SLOTINDEX(equipType);

							if( !m_VecGlyphCover[nSlotIndex]->IsShow() )
								m_VecGlyphSlotButton[nSlotIndex]->SetWearable( true );
						}
#if defined(PRE_ADD_DRAGON_FELLOWSHIP_GLYPH)
						else if( equipType >= GLYPH_DRAGON1 && equipType <= GLYPH_DRAGON4 )
						{
							for( int itr = GLYPH_DRAGON1; itr <= GLYPH_DRAGON4; ++itr )
							{
								int nSlotIndex = GLYPH_EQUIPINDEX_2_SLOTINDEX( itr );

								if( !m_VecGlyphCover[nSlotIndex]->IsShow() )
									m_VecGlyphSlotButton[nSlotIndex]->SetWearable( true );
							}
						}
#endif	// #if defined(PRE_ADD_DRAGON_FELLOWSHIP_GLYPH)

						for( int itr = GLYPH_CASH1; itr <= GLYPH_CASH3; ++itr )
						{
							int nSlotIndex = GLYPH_EQUIPINDEX_2_SLOTINDEX( itr );

							if( GetItemTask().IsCashGlyphOpen( itr - GLYPH_CASH1 )
								&& NULL == m_VecGlyphSlotButton[nSlotIndex]->GetItem()
#if defined(PRE_ADD_DRAGON_FELLOWSHIP_GLYPH)
								&& !( GLYPH_DRAGON1 <= equipType && GLYPH_DRAGON4 >= equipType )
#endif	// #if defined(PRE_ADD_DRAGON_FELLOWSHIP_GLYPH)
								)
								m_VecGlyphSlotButton[nSlotIndex]->SetWearable( true );
						}
					}
				}
				break;
		}
	}
	else
	{
		for( int i=0; i<(int)m_VecGlyphSlotButton.size(); i++ )
			m_VecGlyphSlotButton[i]->SetWearable( false );
	}
}

void CDnCharPlateDlg::SetSlotMagneticMode( bool bMagnetic )
{
	for( DWORD i=0; i<m_VecGlyphSlotButton.size(); i++ ) {
		m_VecGlyphSlotButton[i]->SetMagnetic( bMagnetic );
	}
}

void CDnCharPlateDlg::SetGlyphEquipItem( int nEquipIndex, MIInventoryItem *pItem )
{
	int nSlotIndex = GLYPH_EQUIPINDEX_2_SLOTINDEX(nEquipIndex);
	if( nSlotIndex == -1 ) return;

	m_VecGlyphSlotButton[nSlotIndex]->SetItem(pItem, CDnSlotButton::ITEM_ORIGINAL_COUNT);
	m_VecGlyphSlotButton[nSlotIndex]->OnRefreshTooltip();

	m_pCharPlateInfoDlg->SetText();
}

void CDnCharPlateDlg::RefreshGlyphEquip()
{
	if( !CDnItemTask::IsActive() ) return;

	for( DWORD i=0; i<GLYPHMAX; i++ )
	{
		SetGlyphEquipItem( i, GetItemTask().GetGlyphItem(i) );
	}

	//Skill slot refresh
	for( int itr = 0; itr < SPECIAL_GLYPH_COUNT; ++itr )
		m_pSkillSlotButton[itr]->ResetSlot();

	CDnSkillTask* pSkillTask = static_cast<CDnSkillTask*>( CTaskManager::GetInstancePtr()->GetTask( "SkillTask" ) );

	if( NULL == pSkillTask )
		return;

	int nIndex = 0;

	CDnItem * pItem = GetItemTask().GetGlyphItem( GLYPH_SPECIALSKILL );
	if( pItem )
	{
		DnSkillHandle hSkill = pSkillTask->FindSkill( pItem->GetSkillID() );

		if( hSkill )
		{
			((CDnQuickSlotButton*)m_pSkillSlotButton[nIndex])->SetQuickItem( hSkill );
			++nIndex;
		}
	}

#if defined(PRE_ADD_DRAGON_FELLOWSHIP_GLYPH)
	for( int itr = GLYPH_CASH1; itr <= GLYPH_CASH3; ++itr )
#else
	for( int itr = GLYPH_CASH1; itr < GLYPHMAX; ++itr )
#endif
	{
		CDnItem * pItem = GetItemTask().GetGlyphItem( itr );

		if( pItem )
		{
			DnSkillHandle hSkill = pSkillTask->FindSkill( pItem->GetSkillID() );

			if( hSkill )
			{
				if( !GetItemTask().IsCashGlyphOpen( itr - GLYPH_CASH1 ) )
					hSkill->SetGlyphActiveSkillDisable( true );
				else
					hSkill->SetGlyphActiveSkillDisable( false );

				((CDnQuickSlotButton*)m_pSkillSlotButton[nIndex])->SetQuickItem( hSkill );
				++nIndex;
			}
		}
	}
}

void CDnCharPlateDlg::ResetGlyphEquipSlot( int nEquipIndex )
{
	int nSlotIndex = GLYPH_EQUIPINDEX_2_SLOTINDEX(nEquipIndex);
	if( nSlotIndex == -1 ) return;

	m_VecGlyphSlotButton[nSlotIndex]->ResetSlot();
	m_VecGlyphSlotButton[nSlotIndex]->OnRefreshTooltip();

	m_pCharPlateInfoDlg->SetText();
}

void CDnCharPlateDlg::EquipGlyphItem( int nSlotIndex, MIInventoryItem *pItem, CDnSlotButton *pSlotButton )
{
	CDnItem *pItemPtr = dynamic_cast<CDnItem *>(pItem);

	if( NULL == pItemPtr )
		return;

	std::wstring strErrorMsg;

	if( pItemPtr->IsCashItem() ) {
		eGlyph equipType;

		if( GetWearableGlyphEquipType( pItem, equipType, &strErrorMsg ) != CDnCharStatusDlg::eWEAR_ENABLE ) {
			GetInterface().MessageBox( strErrorMsg.c_str(), MB_OK );
			return;
		}

		GetItemTask().RequestMoveItem( MoveType_CashInvenToCashGlyph, nSlotIndex, pItemPtr->GetSerialID(), equipType, pItem->GetOverlapCount() );
	}
	else {
		eGlyph equipType;
		CDnCharStatusDlg::eRetWearable ret;

		if( m_bSelectAttachGlyph )
		{
			ret = GetSelectWearableGlyphEquipType( pItem, (const eGlyph)m_nSelectAttachGlyphSlotIndex, &strErrorMsg );
			equipType = (eGlyph)m_nSelectAttachGlyphSlotIndex;
		}
		else
			ret = GetWearableGlyphEquipType( pItem, equipType, &strErrorMsg );

		if(ret != CDnCharStatusDlg::eWEAR_ENABLE)
		{
			if (ret == CDnCharStatusDlg::eWEAR_UNABLE)
				GetInterface().MessageBox( strErrorMsg.c_str(), MB_OK );
			else if (ret == CDnCharStatusDlg::eWEAR_NEED_UNSEAL)
				GetInterface().OpenItemUnsealDialog(pItem, pSlotButton);
		}
		else
			GetItemTask().RequestMoveItem( MoveType_InvenToGlyph, nSlotIndex, pItemPtr->GetSerialID(), equipType, pItem->GetOverlapCount() );

		ClearSelectGlyphAttach();
	}
}

void CDnCharPlateDlg::OnUICallbackProc(int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{
#ifdef PRE_ADD_INSTANT_CASH_BUY
	if( pControl == NULL && nID == INSTANT_CASHSHOP_BUY_DIALOG )
	{
		RefreshCover();

		if( nCommand == 1 )
			Show( false );

		return;
	}
#endif // PRE_ADD_INSTANT_CASH_BUY

	if (pControl)
		SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED ) 
	{
		if (IsCmdControl("ID_YES"))
		{
			if( GetInterface().IsOpenAcceptRequestDialog() )
				return;

			switch( nID )
			{
			case CDnCharStatusDlg::GLYPH_DETACH_DIALOG :
				{
					GetItemTask().RequestRemoveItem( ITEMPOSITION_GLYPH, m_pRemoveItem->GetSlotIndex(), 1, m_pRemoveItem->GetSerialID() );
					break;
				}

			case CDnCharStatusDlg::GLYPH_ATTACH_DIALOG :
				{
					EquipGlyphItem( m_nTempSlotIndex, m_pTempSlotButton->GetItem(), m_pTempSlotButton );
					break;
				}
			case CDnCharStatusDlg::GLYPH_LIFT_DIALOG :
				{
					if( NULL == m_pLiftItem && NULL == m_pLiftSlot )
						return;

					if( m_pLiftItem )
					{
						GetItemTask().RequestMoveItem( MoveType_GlyphToInven, 
							m_pLiftItem->GetSlotIndex(), 
							m_pLiftItem->GetSerialID(),	
							GetItemTask().GetCharInventory().FindFirstEmptyIndex(), 
							m_pLiftItem->GetOverlapCount() );

						m_pLiftItem = NULL;
					}
					else if( m_pLiftSlot )
					{
						CDnItem *pItem = (CDnItem*)m_pLiftSlot->GetItem();
						m_pLiftSlot->EnableSplitMode( 1 );
						drag::SetControl( m_pLiftSlot );

						CEtSoundEngine::GetInstance().PlaySound( "2D", pItem->GetDragSoundIndex() );

						m_pLiftSlot = NULL;
					}
					break;
				}
			}
		}
		else if( IsCmdControl("ID_NO") )
		{
			switch( nID )
			{
			case CDnCharStatusDlg::GLYPH_LIFT_DIALOG :		
				{
					m_pLiftItem = NULL;
					m_pLiftSlot = NULL;
				}
				break;
			}
		}
	}
}

void CDnCharPlateDlg::RefreshCover( bool bInit )
{
	if( !CDnActor::s_hLocalActor ) return;
	CDnLocalPlayerActor *pActor = (CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer();
	if( !pActor ) return;

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TGLYPHSLOT );

	int nSize = (int)m_VecGlyphSlotButton.size();
	int nCashMaxCount = GetItemTask().GetCashGlyphCount();
	int nCashCount = 0;

	for( int itr = 0 ; itr < nSize ; ++itr )
	{
		int nSlotIndex = GLYPH_EQUIPINDEX_2_SLOTINDEX( itr );
		int nID = pSox->GetItemID( itr );
		int nLevel = pSox->GetFieldFromLablePtr( nID, "_LevelLimit" )->GetInteger();

		if( nLevel <= pActor->GetLevel() )
		{
			if( pSox->GetFieldFromLablePtr( nID, "_isCash" )->GetInteger() )
			{
				if( GetItemTask().IsCashGlyphOpen( nCashCount) )	// Slot is enabled
				{
					if( m_VecGlyphCover[nSlotIndex]->IsShow() && GetCashShopTask().IsOpenCashShop() )	// Heraldry slot newly obtained when the Cash Shop is open.
					{
						int nStringID = pSox->GetFieldFromLablePtr( nID, "_OpenDescriptionID" )->GetInteger();
						GetInterface().AddChatMessage( eChatType::CHATTYPE_SYSTEM, NULL, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nStringID ), false );
					}

					if( 0 != GetItemTask().GetCashGlyphPeriod( nCashCount ) )
					{
						std::wstring wszString = GetPeriodTooltip( nCashCount );
						m_pCashPeriod[nCashCount]->Show( true );
						m_pCashPeriod[nCashCount]->SetTooltipText( wszString.c_str() );
					}
					else
					{
						m_pCashPeriod[nCashCount]->Show( false );
						m_pCashPeriod[nCashCount]->ClearTooltipText();
					}

					m_VecGlyphSlotButton[nSlotIndex]->SetIconColor( itemcolor::NORMAL );
					m_VecGlyphCover[nSlotIndex]->Show( false );
				}
				else
				{
					CDnItem * pGlyphItem = (CDnItem *)(m_VecGlyphSlotButton[nSlotIndex]->GetItem());

					if( pGlyphItem )
					{
						m_VecGlyphSlotButton[nSlotIndex]->SetIconColor( itemcolor::EXPIRE );
						m_VecGlyphCover[nSlotIndex]->Show( false );
					}
					else
						m_VecGlyphCover[nSlotIndex]->Show( true );

					m_pCashPeriod[nCashCount]->Show( false );
					m_pCashPeriod[nCashCount]->ClearTooltipText();
				}

				++nCashCount;
				/*if( nCashCount < nCashMaxCount )
				{
if( m_VecGlyphCover[nSlotIndex]->IsShow() && GetCashShopTask().IsOpenCashShop() ) // Newly acquired crest slot while cash shop is open.
					{
						int nStringID = pSox->GetFieldFromLablePtr( nID, "_OpenDescriptionID" )->GetInteger();
						GetInterface().AddChatMessage( eChatType::CHATTYPE_SYSTEM, NULL, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nStringID ), false );
					}

					m_VecGlyphCover[nSlotIndex]->Show( false );
					++nCashCount;
				}
				else
					m_VecGlyphCover[nSlotIndex]->Show( true );*/
			}
			else
			{
				if( m_VecGlyphCover[nSlotIndex]->IsShow() )
				{
					if( !bInit )
					{
						int nStringID = pSox->GetFieldFromLablePtr( nID, "_OpenDescriptionID" )->GetInteger();
						GetInterface().AddChatMessage( eChatType::CHATTYPE_SYSTEM, NULL, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nStringID ), false );
					}

					m_VecGlyphCover[nSlotIndex]->Show( false );
				}
			}			
		}
		else
			m_VecGlyphCover[nSlotIndex]->Show( true );
	}
}

MIInventoryItem *CDnCharPlateDlg::GetEquipGlyph( int nTypeParam )
{
	for( int itr = GLYPH_ENCHANT1; itr < GLYPHMAX; ++itr )
	{
		int nSlotIndex = GLYPH_EQUIPINDEX_2_SLOTINDEX(itr);
		CDnItem * pGlyphItem = (CDnItem *)(m_VecGlyphSlotButton[nSlotIndex]->GetItem());

		if( pGlyphItem && pGlyphItem->GetTypeParam(1) == nTypeParam )
			return m_VecGlyphSlotButton[nSlotIndex]->GetItem();
	}

	return NULL;
}

std::wstring CDnCharPlateDlg::GetPeriodTooltip( int nCashIndex )
{
	tm date;
	__time64_t tTime = GetItemTask().GetCashGlyphPeriod( nCashIndex );
	DnLocalTime_s( &date, &tTime );

	std::wstring wszTimeString;
	CommonUtil::GetDateString( CommonUtil::DATESTR_FULL, wszTimeString, date );

	WCHAR wszOutputString[512];
	swprintf_s( wszOutputString, _countof(wszOutputString), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 9035 ), wszTimeString.c_str() );

	std::wstring wszString = std::wstring( wszOutputString );
	std::wstring::size_type nPosFirst = wszString.find( L"\\n" );
	std::wstring::size_type nPostSecond = wszString.find( L"\\n", nPosFirst+2 );

	std::wstring wszFirst = wszString.substr( 0, nPosFirst );
	std::wstring wszSecond = wszString.substr( nPosFirst+2, nPostSecond - nPosFirst - 2 );
	std::wstring wszThird = wszString.substr( nPostSecond+2 );

	swprintf_s( wszOutputString, _countof(wszOutputString), L"%s\n%s\n%s", wszFirst.c_str(), wszSecond.c_str(), wszThird.c_str() );

	return std::wstring( wszOutputString );
}

void CDnCharPlateDlg::SetSelectGlyphAttach( int nSelectGlyphAttachSlotIndex )
{
	m_bSelectAttachGlyph = true;
	m_nSelectAttachGlyphSlotIndex = nSelectGlyphAttachSlotIndex;
}

void CDnCharPlateDlg::ClearSelectGlyphAttach()
{
	m_bSelectAttachGlyph = false;
	m_nSelectAttachGlyphSlotIndex = -1;
}

void CDnCharPlateDlg::GlyphLift( CDnGlyph * pItem )
{
	if( !pItem )
		return;

	if( !GetInterface().IsEmptySlot( CDnMainMenuDlg::INVENTORY_DIALOG, ITEM_SLOT_TYPE::ST_INVENTORY ) )
	{
		GetInterface().MessageBox( 1925, MB_OK );
		return;
	}

	int nCharge = 0, nGold = 0, nSilver = 0, nCooper = 0;
	WCHAR wszString[512], wszGold[128], wszSilver[128], wszCooper[128];
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TGLYPHCHARGE );

	for( int itr = 0; itr < pSox->GetItemCount(); ++itr )
	{
		if( pSox->GetFieldFromLablePtr(pSox->GetItemID(itr), "_Glyph_type")->GetInteger() == pItem->GetGlyphType() &&
			pSox->GetFieldFromLablePtr(pSox->GetItemID(itr), "_Rank")->GetInteger() == pItem->GetItemRank() )
		{
			nCharge = pSox->GetFieldFromLablePtr(pSox->GetItemID(itr), "_Charge")->GetInteger();
		}
	}

	nCharge *= pItem->GetLevelLimit();

	CDnMoneyControl::GetStrMoneyG( nCharge, wszGold );
	CDnMoneyControl::GetStrMoneyS( nCharge, wszSilver );
	CDnMoneyControl::GetStrMoneyC( nCharge, wszCooper );

	nGold = (int)_wtoi64( wszGold );
	nSilver = (int)_wtoi64( wszSilver );
	nCooper = (int)_wtoi64( wszCooper );

	DN_ASSERT( 0 != nCharge, "Glyph Charge 0!!");

	swprintf_s( wszString, _countof( wszString ), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 9037 ), nGold, nSilver, nCooper, pItem->GetName() );

	GetInterface().MessageBox( wszString, MB_YESNO, CDnCharStatusDlg::GLYPH_LIFT_DIALOG, this, false, true );
}

void CDnCharPlateDlg::SetWithDrawOpen( bool bOpen )
{
	m_bWithDrawOpen = bOpen;

	CDnCharStatusDlg * pCharStatusDlg = (CDnCharStatusDlg *)(m_pParentDialog);

	if( m_bWithDrawOpen )
	{
		pCharStatusDlg->SetRadioButton( CDnCharStatusDlg::eRadioOpt_Glyph );

		m_pWithDrawGlyphText->Show( true );
		m_pWithDarwGlyphBackground->Show( true );
		
#ifdef PRE_ADD_SHORTCUT_HELP_DIALOG
		GetControl<CEtUIButton>("ID_BT_SMALLHELP_REMOVE")->Show(true);
		GetControl<CEtUIButton>("ID_BT_SMALLHELP_PLATE")->Show(false);
#endif
	}
	else
	{
		pCharStatusDlg->SetRadioButton( CDnCharStatusDlg::eRadioOpt_None );

		m_pWithDrawGlyphText->Show( false );
		m_pWithDarwGlyphBackground->Show( false );

#ifdef PRE_ADD_SHORTCUT_HELP_DIALOG
		GetControl<CEtUIButton>("ID_BT_SMALLHELP_REMOVE")->Show(false);
		GetControl<CEtUIButton>("ID_BT_SMALLHELP_PLATE")->Show(true);
#endif
	}
}

bool CDnCharPlateDlg::GetWithDrawOpen()
{
	return m_bWithDrawOpen;
}

#if defined(PRE_ADD_DRAGON_FELLOWSHIP_GLYPH)
void CDnCharPlateDlg::InitDragonGlyphSlot()
{
	if( 0 != CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::DragonFellowShipGlyph ) )
		return;

	for( DWORD itr = 0; itr < m_vecDragonGlyphGB.size(); ++itr )
		m_vecDragonGlyphGB[itr]->Show( false );

	for( int itr = GLYPH_DRAGON1; itr < GLYPHMAX; ++itr )
	{
		m_VecGlyphSlotButton[itr]->Show( false );
		m_VecGlyphCover[itr]->Show( false );
	}
}
#endif	// #if defined(PRE_ADD_DRAGON_FELLOWSHIP_GLYPH)
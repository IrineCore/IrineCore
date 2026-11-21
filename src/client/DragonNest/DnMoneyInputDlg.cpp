#include "StdAfx.h"
#include "DnMoneyInputDlg.h"
#include "DnItemTask.h"
#include "DnCashShopTask.h"
#ifdef PRE_MOD_PETAL_WRITE
#include "DnInterfaceString.h"
#endif // PRE_MOD_PETAL_WRITE



#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnMoneyControl::CDnMoneyControl( CEtUIDialog *pDialog )  
{
	m_eMoneyType = GOLD_TYPE;

	m_bEnablePetal = false;

	m_pDialog = pDialog;
	m_pCoinG = NULL;
	m_pCoinS = NULL;
	m_pCoinC = NULL;
	m_pCoinP = NULL;
	m_pCoinGOutline = NULL;
	m_pCoinSOutline = NULL;
	m_pCoinCOutline = NULL;
	m_pCoinPOutline = NULL;
	m_pEditBoxMoneyG = NULL;
	m_pEditBoxMoneyS = NULL;
	m_pEditBoxMoneyC = NULL;	
	m_pEditBoxMoneyP = NULL;	
	m_pButtonC = NULL;
	m_pButtonG = NULL;
	m_pButtonP = NULL;
	m_pButtonS = NULL;
	m_pImageC = NULL;
	m_pImageG = NULL;
	m_pImageP = NULL;
	m_pImageS = NULL;
}

void CDnMoneyControl::InitialUpdate( bool bEnablePetal )
{
	m_pCoinG = m_pDialog->GetControl<CEtUIStatic>("ID_COIN_G");
	m_pCoinS = m_pDialog->GetControl<CEtUIStatic>("ID_COIN_S");
	m_pCoinC = m_pDialog->GetControl<CEtUIStatic>("ID_COIN_C");

	m_pEditBoxMoneyG = m_pDialog->GetControl<CEtUIEditBox>("ID_EDITBOX_MONEY_G");
	m_pEditBoxMoneyS = m_pDialog->GetControl<CEtUIEditBox>("ID_EDITBOX_MONEY_S");
	m_pEditBoxMoneyC = m_pDialog->GetControl<CEtUIEditBox>("ID_EDITBOX_MONEY_C");

	m_pCoinGOutline = m_pDialog->GetControl<CEtUIStatic>("ID_STATIC_GOLD");
	m_pCoinSOutline = m_pDialog->GetControl<CEtUIStatic>("ID_STATIC_SILVER");
	m_pCoinCOutline = m_pDialog->GetControl<CEtUIStatic>("ID_STATIC_COPPER");
	
	m_pCoinGOutline->Enable( false );
	m_pCoinSOutline->Enable( false );
	m_pCoinCOutline->Enable( false );

	m_nMaxMoney = -1;

	if( bEnablePetal )
	{
		m_bEnablePetal = bEnablePetal;
		m_nMaxPetal = -1;

		m_pCoinP = m_pDialog->GetControl<CEtUIStatic>("ID_COIN_P");
		m_pEditBoxMoneyP = m_pDialog->GetControl<CEtUIEditBox>("ID_EDITBOX_MONEY_P");
		m_pCoinPOutline = m_pDialog->GetControl<CEtUIStatic>("ID_STATIC0");

		m_pCoinPOutline->Enable( false );

		m_pImageG = m_pDialog->GetControl<CEtUIStatic>("ID_STATIC3");
		m_pImageS = m_pDialog->GetControl<CEtUIStatic>("ID_STATIC4");
		m_pImageC = m_pDialog->GetControl<CEtUIStatic>("ID_STATIC5");
		m_pImageP = m_pDialog->GetControl<CEtUIStatic>("ID_STATIC_PETAL");

		m_pButtonG = m_pDialog->GetControl<CEtUIButton>("ID_BUTTON_DUMMY_G");
		m_pButtonS = m_pDialog->GetControl<CEtUIButton>("ID_BUTTON_DUMMY_S");
		m_pButtonC = m_pDialog->GetControl<CEtUIButton>("ID_BUTTON_DUMMY_C");
		m_pButtonP = m_pDialog->GetControl<CEtUIButton>("ID_BUTTON_DUMMY_P");

		SetMoneyType( m_eMoneyType );
	}
}

bool CDnMoneyControl::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	if( nCommand == EVENT_EDITBOX_CHANGE )
	{
		if( m_pDialog->IsCmdControl("ID_EDITBOX_MONEY_G" ) )
		{
			int nLen = m_pEditBoxMoneyG->GetTextLength();

			// When input exceeds 7 digits (via typing or pasting)
			if( nLen > 6 )
			{
				// Check length limit
				m_pEditBoxMoneyG->SetText( GetStrMoneyG() );
				return false;
			}

			// Get the value,
			INT64 nMoney = m_pEditBoxMoneyG->GetTextToInt64();

			// Discard previous value >= 10000,
			INT64 nBelow10000 = m_nMoney % 10000;

			// Register a new value between 100 and 10000
			m_nMoney = nMoney * 10000 + nBelow10000;

			// Only call UpdateMoney at the end when input comes from a UI control,
			// When calling ProcessCommand directly in code, it is likely an intermediate step, so do not call UPdateMoney.
			// That is, apply final adjustment only once at the end.
			if( bTriggeredByUser && UpdateMoney() )
			{
				m_pEditBoxMoneyG->SetText( GetStrMoneyG() );
				m_pCoinS->SetText( GetStrMoneyS() );
				m_pCoinC->SetText( GetStrMoneyC() );
			}
		}
		else if( m_pDialog->IsCmdControl("ID_EDITBOX_MONEY_S" ) )
		{
			int nLen = m_pEditBoxMoneyS->GetTextLength();

			// When input exceeds 3 digits (via typing or pasting)
			if( nLen > 2 )
			{
				// Check length limit
				std::wstring strGold = m_pCoinG->GetText();
				int nLenStrGold = (int)strGold.size();
				if( m_nMoney < 10000 ) nLenStrGold = 0;	// Exception handling
				if( nLenStrGold + nLen > 8 )
				{
					m_pEditBoxMoneyS->SetText( GetStrMoneyS() );
					return false;
				}

				// If within length limit, roll over digits beyond 3 places into silver.
				std::wstring strSilver = m_pEditBoxMoneyS->GetText();
				std::wstring strToGold = strSilver.substr(0, nLen-2);
				std::wstring strNewSilver = strSilver.substr(nLen-2);
				std::wstring strNewGold = m_pCoinG->GetText();
				if( m_nMoney < 10000 ) strNewGold = L"";
				strNewGold = strNewGold + strToGold;

				m_pEditBoxMoneyS->SetText( strNewSilver.c_str() );
				m_pEditBoxMoneyG->SetText( strNewGold.c_str() );

				m_pDialog->ProcessCommand( EVENT_EDITBOX_CHANGE, false, m_pEditBoxMoneyG, 0 );
				m_pDialog->ProcessCommand( EVENT_EDITBOX_RELEASEFOCUS, false, m_pEditBoxMoneyG, 0 );
			}

			// Get the value,
			INT64 nMoney = m_pEditBoxMoneyS->GetTextToInt64();
			if( nMoney < 0 ) nMoney = 0;

			// Discard previous value between 100 and 10000,
			INT64 nBelow100 = m_nMoney % 100;
			m_nMoney -= nBelow100;
			INT64 nBelow10000 = m_nMoney % 10000;
			m_nMoney -= nBelow10000;

			// Register a new value between 100 and 10000
			m_nMoney += nMoney * 100;

			// Re-register if under 100
			m_nMoney += nBelow100;

			if( bTriggeredByUser && UpdateMoney() )
			{
				m_pEditBoxMoneyS->SetText( GetStrMoneyS() );
				m_pCoinG->SetText( GetStrMoneyG() );
				m_pCoinC->SetText( GetStrMoneyC() );
			}
		}
		else if( m_pDialog->IsCmdControl("ID_EDITBOX_MONEY_C" ) )
		{
			int nLen = m_pEditBoxMoneyC->GetTextLength();

			// When input exceeds 3 digits (via typing or pasting)
			if( nLen > 2 )
			{
				// Check length limit - not handled properly when 0.
				std::wstring strGold = m_pCoinG->GetText();
				int nLenStrGold = (int)strGold.size();
				if( m_nMoney < 10000 ) nLenStrGold = 0;	// Exception handling

				std::wstring strSilver = m_pCoinS->GetText();
				int nLenStrSilver = (int)strSilver.size();
				if( m_nMoney < 100 ) nLenStrSilver = 0;

				if( nLenStrGold + nLenStrSilver + nLen > 10 )
				{
					m_pEditBoxMoneyC->SetText( GetStrMoneyC() );
					return false;
				}

				// If within length limit, roll over digits beyond 3 places into silver.
				std::wstring strCopper = m_pEditBoxMoneyC->GetText();
				std::wstring strToSilver = strCopper.substr(0, nLen-2);
				std::wstring strNewCopper = strCopper.substr(nLen-2);
				std::wstring strNewSilver = m_pCoinS->GetText();
				if( m_nMoney < 100 ) strNewSilver = L"";
				strNewSilver = strNewSilver + strToSilver;

				m_pEditBoxMoneyC->SetText( strNewCopper.c_str() );
				m_pEditBoxMoneyS->SetText( strNewSilver.c_str() );

				m_pDialog->ProcessCommand( EVENT_EDITBOX_CHANGE, false, m_pEditBoxMoneyS, 0 );
				m_pDialog->ProcessCommand( EVENT_EDITBOX_RELEASEFOCUS, false, m_pEditBoxMoneyS, 0 );
			}
			else if( nLen < 2 )
			{
				// When length becomes shorter than before (deleted via backspace or block selection)
				// When pressing 1 with nothing in the edit box, do not pull even if there is an amount in silver.
				//
				// Wait, is this really necessary? Exclude it for now after discussing with the designer.
				if( nLen < m_nPrevLenMoneyC )
				{

				}
			}

			// Remember the length.
			m_nPrevLenMoneyC = nLen;

			// Get the value,
			INT64 nMoney = m_pEditBoxMoneyC->GetTextToInt64();
			if( nMoney < 0 ) nMoney = 0;

			// Discard previous value under 100,
			INT64 nBelow100 = m_nMoney % 100;
			m_nMoney -= nBelow100;

			// Register a new value less than 100
			m_nMoney += nMoney;

			if( bTriggeredByUser && UpdateMoney() )
			{
				m_pEditBoxMoneyC->SetText( GetStrMoneyC() );
				m_pCoinS->SetText( GetStrMoneyS() );
				m_pCoinG->SetText( GetStrMoneyG() );
			}
		}
		else if( m_pDialog->IsCmdControl("ID_EDITBOX_MONEY_P" ) )
		{
			m_nPetal = m_pEditBoxMoneyP->GetTextToInt64();

			if( bTriggeredByUser && UpdatePetal() )
#ifdef PRE_MOD_PETAL_WRITE
				m_pEditBoxMoneyP->SetText( DN_INTERFACE::UTIL::GetAddCommaString( m_nPetal ).c_str() );
#else // PRE_MOD_PETAL_WRITE
				m_pEditBoxMoneyP->SetText( GetStrMoneyP() );
#endif // PRE_MOD_PETAL_WRITE
		}
	}
	else if( nCommand == EVENT_EDITBOX_FOCUS )
	{
		if( m_pDialog->IsCmdControl("ID_EDITBOX_MONEY_G" ) )
		{
			m_pCoinG->Show( false );
			m_pCoinGOutline->Enable( true );
			m_pEditBoxMoneyG->SetText( GetStrMoneyG(), true );
		}
		else if( m_pDialog->IsCmdControl("ID_EDITBOX_MONEY_S" ) )
		{
			m_pCoinS->Show( false );
			m_pCoinSOutline->Enable( true );
			m_pEditBoxMoneyS->SetText( GetStrMoneyS(), true );
		}
		else if( m_pDialog->IsCmdControl("ID_EDITBOX_MONEY_C" ) )
		{
			m_pCoinC->Show( false );
			m_pCoinCOutline->Enable( true );
			m_pEditBoxMoneyC->SetText( GetStrMoneyC(), true );
		}
		else if( m_pDialog->IsCmdControl("ID_EDITBOX_MONEY_P" ) )
		{
			m_pCoinP->Show( false );
			m_pCoinPOutline->Enable( true );
#ifdef PRE_MOD_PETAL_WRITE
			m_pEditBoxMoneyP->SetText( DN_INTERFACE::UTIL::GetAddCommaString( m_nPetal ).c_str(), true );
#else // PRE_MOD_PETAL_WRITE
			m_pEditBoxMoneyP->SetText( GetStrMoneyP(), true );
#endif // PRE_MOD_PETAL_WRITE
		}
	}
	else if( nCommand == EVENT_EDITBOX_RELEASEFOCUS )
	{
		if( m_pDialog->IsCmdControl("ID_EDITBOX_MONEY_G" ) )
		{
			m_pEditBoxMoneyG->ClearText();
			m_pCoinGOutline->Enable( false );
			m_pCoinG->SetText( GetStrMoneyG() );
			m_pCoinG->Show( true );
		}
		else if( m_pDialog->IsCmdControl("ID_EDITBOX_MONEY_S" ) )
		{
			m_pEditBoxMoneyS->ClearText();
			m_pCoinSOutline->Enable( false );
			m_pCoinS->SetText( GetStrMoneyS() );
			m_pCoinS->Show( true );
		}
		else if( m_pDialog->IsCmdControl("ID_EDITBOX_MONEY_C" ) )
		{
			m_pEditBoxMoneyC->ClearText();
			m_pCoinCOutline->Enable( false );
			m_pCoinC->SetText( GetStrMoneyC() );
			m_pCoinC->Show( true );
		}
		else if( m_pDialog->IsCmdControl("ID_EDITBOX_MONEY_P" ) )
		{
			m_pEditBoxMoneyP->ClearText();
			m_pCoinPOutline->Enable( false );
#ifdef PRE_MOD_PETAL_WRITE
			m_pCoinP->SetText( DN_INTERFACE::UTIL::GetAddCommaString( m_nPetal ).c_str() );
#else // PRE_MOD_PETAL_WRITE
			m_pCoinP->SetText( GetStrMoneyP() );
#endif // PRE_MOD_PETAL_WRITE
			m_pCoinP->Show( true );
		}
	}
	else if ( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( m_pDialog->IsCmdControl("ID_BUTTON_DUMMY_G" ) )
		{
			m_pDialog->RequestFocus( m_pEditBoxMoneyG );
			return false;
		}
		else if( m_pDialog->IsCmdControl("ID_BUTTON_DUMMY_S" ) )
		{
			m_pDialog->RequestFocus( m_pEditBoxMoneyS );
			return false;
		}
		else if( m_pDialog->IsCmdControl("ID_BUTTON_DUMMY_C" ) )
		{
			m_pDialog->RequestFocus( m_pEditBoxMoneyC );
			return false;
		}
		else if( m_pDialog->IsCmdControl("ID_BUTTON_DUMMY_P" ) )
		{
			m_pDialog->RequestFocus( m_pEditBoxMoneyP );
			return false;
		}
	}

	return true;
}

void CDnMoneyControl::SetMax()
{
	if( GOLD_TYPE == m_eMoneyType )
	{
		INT64 nMaxMoney;

		if( m_nMaxMoney == -1 )
			nMaxMoney = GetItemTask().GetCoin();
		else
			nMaxMoney = m_nMaxMoney;

		m_nMoney = nMaxMoney;
		m_pCoinG->SetText( GetStrMoneyG() );
		m_pCoinS->SetText( GetStrMoneyS() );
		m_pCoinC->SetText( GetStrMoneyC() );
	}
	else if( PETAL_TYPE == m_eMoneyType )
	{
		INT64 nMaxPetal;

		if( m_nMaxPetal == -1 )
			nMaxPetal = GetCashShopTask().GetUserReserve();
		else
			nMaxPetal = m_nMaxPetal;

		m_nPetal = nMaxPetal;
#ifdef PRE_MOD_PETAL_WRITE
		m_pCoinP->SetText( DN_INTERFACE::UTIL::GetAddCommaString( m_nPetal ).c_str() );
#else // PRE_MOD_PETAL_WRITE
		m_pCoinP->SetText( GetStrMoneyP() );
#endif // PRE_MOD_PETAL_WRITE
	}
}

void CDnMoneyControl::Init()
{
	m_nMoney = 0;
	
	m_pCoinG->SetIntToText( 0 );
	m_pCoinS->SetIntToText( 0 );
	m_pCoinC->SetIntToText( 0 );

	m_nPrevLenMoneyG = 0;
	m_nPrevLenMoneyS = 0;
	m_nPrevLenMoneyC = 0;

	if( m_bEnablePetal )
	{
		m_nPetal = 0;
		m_pCoinP->SetIntToText( 0 );
		m_nPrevLenMoneyP = 0;
	}
}

bool CDnMoneyControl::UpdateMoney()
{
	// Min clamping is always 0
	if( m_nMoney < 0 )
	{
		m_nMoney = 0;
		return true;
	}

	// Cap maximum adjustment at configured max value; if not set, use inventory gold amount.
	INT64 nMaxMoney;
	if( m_nMaxMoney == -1 )
		nMaxMoney = GetItemTask().GetCoin();
	else
		nMaxMoney = m_nMaxMoney;

	if( m_nMoney > nMaxMoney )
	{
		m_nMoney = nMaxMoney;
		return true;
	}

	return false;
}

bool CDnMoneyControl::UpdatePetal()
{
	if( m_nPetal < 0 )
	{
		m_nPetal = 0;
		return true;
	}

	INT64 nMaxPetal;
	if( m_nMaxPetal == -1 )
		nMaxPetal = GetCashShopTask().GetUserReserve();
	else
		nMaxPetal = m_nMaxPetal;

	if( m_nPetal > nMaxPetal )
	{
		m_nPetal = nMaxPetal;
		return true;
	}

	return false;
}

void CDnMoneyControl::Show( bool bShow )
{
	if( bShow && m_bEnablePetal )
	{
		SetMoneyType( m_eMoneyType );
	}
	else
	{
		Init();
	}
}

bool CDnMoneyControl::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( uMsg == WM_KEYDOWN )
	{
		if( wParam == VK_TAB )
		{
			if( m_pEditBoxMoneyC->IsFocus() )
			{
				m_pDialog->RequestFocus( m_pEditBoxMoneyG );
				return true;
			}
		}
	}

	if( uMsg == WM_MOUSEWHEEL )
	{
		POINT MousePoint;
		MousePoint.x = short( LOWORD( lParam ) );
		MousePoint.y = short( HIWORD( lParam ) );

		float fMouseX, fMouseY;
		m_pDialog->PointToFloat( MousePoint, fMouseX, fMouseY );

		if( m_pDialog->IsMouseInDlg() )
		{
			UINT uLines;
			SystemParametersInfo( SPI_GETWHEELSCROLLLINES, 0, &uLines, 0 );
			int nScrollAmount = int( ( short )HIWORD( wParam ) ) / WHEEL_DELTA * uLines;
			if( nScrollAmount > 0 )
			{
				if( m_pEditBoxMoneyG->IsFocus() )
				{
					// When using mouse wheel, increment by each unit.
					m_nMoney += 10000;
					UpdateMoney();
					m_pEditBoxMoneyG->SetText( GetStrMoneyG() );
					m_pCoinS->SetText( GetStrMoneyS() );
					m_pCoinC->SetText( GetStrMoneyC() );
				}
				else if( m_pEditBoxMoneyS->IsFocus() )
				{
					m_nMoney += 100;
					UpdateMoney();
					m_pEditBoxMoneyS->SetText( GetStrMoneyS() );
					m_pCoinG->SetText( GetStrMoneyG() );
					m_pCoinC->SetText( GetStrMoneyC() );
				}
				else if( m_pEditBoxMoneyC->IsFocus() )
				{
					m_nMoney += 1;
					UpdateMoney();
					m_pEditBoxMoneyC->SetText( GetStrMoneyC() );
					m_pCoinS->SetText( GetStrMoneyS() );
					m_pCoinG->SetText( GetStrMoneyG() );
				}
				else if( m_pEditBoxMoneyP && m_pEditBoxMoneyP->IsFocus() )
				{
					m_nMoney += 100;
					UpdatePetal();
#ifdef PRE_MOD_PETAL_WRITE
					m_pEditBoxMoneyP->SetText( DN_INTERFACE::UTIL::GetAddCommaString( m_nPetal ).c_str() );
#else // PRE_MOD_PETAL_WRITE
					m_pEditBoxMoneyP->SetText( GetStrMoneyP() );
#endif // PRE_MOD_PETAL_WRITE
				}
				return true;
			}
			else if( nScrollAmount < 0 )
			{
				if( m_pEditBoxMoneyG->IsFocus() )
				{
					// When using mouse wheel, increment by each unit.
					m_nMoney -= 10000;
					UpdateMoney();
					m_pEditBoxMoneyG->SetText( GetStrMoneyG() );
					m_pCoinS->SetText( GetStrMoneyS() );
					m_pCoinC->SetText( GetStrMoneyC() );
				}
				else if( m_pEditBoxMoneyS->IsFocus() )
				{
					m_nMoney -= 100;
					UpdateMoney();
					m_pEditBoxMoneyS->SetText( GetStrMoneyS() );
					m_pCoinG->SetText( GetStrMoneyG() );
					m_pCoinC->SetText( GetStrMoneyC() );
				}
				else if( m_pEditBoxMoneyC->IsFocus() )
				{
					m_nMoney -= 1;
					UpdateMoney();
					m_pEditBoxMoneyC->SetText( GetStrMoneyC() );
					m_pCoinS->SetText( GetStrMoneyS() );
					m_pCoinG->SetText( GetStrMoneyG() );
				}
				else if( m_pEditBoxMoneyP && m_pEditBoxMoneyP->IsFocus() )
				{
					m_nMoney -= 100;
					UpdatePetal();
#ifdef PRE_MOD_PETAL_WRITE
					m_pEditBoxMoneyP->SetText( DN_INTERFACE::UTIL::GetAddCommaString( m_nPetal ).c_str() );
#else // PRE_MOD_PETAL_WRIT
					m_pEditBoxMoneyP->SetText( GetStrMoneyP() );
#endif // PRE_MOD_PETAL_WRIT
				}
				return true;
			}
		}
	}
	return false;
}

bool CDnMoneyControl::IsEditBoxFocus()
{
	return (m_pEditBoxMoneyG->IsFocus() || m_pEditBoxMoneyS->IsFocus() || m_pEditBoxMoneyC->IsFocus() || ( m_pEditBoxMoneyP && m_pEditBoxMoneyP->IsFocus() ) );
}

void CDnMoneyControl::SetMoney( INT64 nMoney )
{
	m_nMoney = nMoney;
	UpdateMoney();

	m_pCoinG->SetText( GetStrMoneyG() );
	m_pCoinS->SetText( GetStrMoneyS() );
	m_pCoinC->SetText( GetStrMoneyC() );
	
	m_pEditBoxMoneyG->SetText( L"" );
	m_pEditBoxMoneyS->SetText( L"" );
	m_pEditBoxMoneyC->SetText( L"" );

	if( m_bEnablePetal )
	{
#ifdef PRE_MOD_PETAL_WRITE
		m_pCoinP->SetText( DN_INTERFACE::UTIL::GetAddCommaString( m_nPetal ).c_str() );
#else // PRE_MOD_PETAL_WRIT
		m_pCoinP->SetText( GetStrMoneyP() );
#endif // PRE_MOD_PETAL_WRITE
		m_pEditBoxMoneyP->SetText( L"" );
	}
}

void CDnMoneyControl::SetMoneyType( int eMoneyType )
{
	m_eMoneyType = eMoneyType;

	if( GOLD_TYPE == m_eMoneyType )
	{
		m_pCoinG->Show( true );
		m_pCoinS->Show( true );
		m_pCoinC->Show( true );
		m_pCoinP->Show( false );

		m_pCoinGOutline->Show( true );
		m_pCoinSOutline->Show( true );
		m_pCoinCOutline->Show( true );
		m_pCoinPOutline->Show( false );

		m_pEditBoxMoneyG->Show( true );
		m_pEditBoxMoneyS->Show( true );
		m_pEditBoxMoneyC->Show( true );
		m_pEditBoxMoneyP->Show( false );

		m_pImageG->Show( true );
		m_pImageS->Show( true );
		m_pImageC->Show( true );
		m_pImageP->Show( false );

		m_pButtonG->Show( true );
		m_pButtonS->Show( true );
		m_pButtonC->Show( true );
		m_pButtonP->Show( false );

		EnableFee( true );

		m_pDialog->RequestFocus( m_pEditBoxMoneyC );
	}
	else if( PETAL_TYPE == m_eMoneyType )
	{
		m_pCoinG->Show( false );
		m_pCoinS->Show( false );
		m_pCoinC->Show( false );
		m_pCoinP->Show( true );

		m_pCoinGOutline->Show( false );
		m_pCoinSOutline->Show( false );
		m_pCoinCOutline->Show( false );
		m_pCoinPOutline->Show( true );

		m_pEditBoxMoneyG->Show( false );
		m_pEditBoxMoneyS->Show( false );
		m_pEditBoxMoneyC->Show( false );
		m_pEditBoxMoneyP->Show( true );

		m_pImageG->Show( false );
		m_pImageS->Show( false );
		m_pImageC->Show( false );
		m_pImageP->Show( true );

		m_pButtonG->Show( false );
		m_pButtonS->Show( false );
		m_pButtonC->Show( false );
		m_pButtonP->Show( true );

		EnableFee( false );

		m_pDialog->RequestFocus( m_pEditBoxMoneyP );
	}
}

void CDnMoneyControl::EnableFee( bool bEnable )
{	
	m_pDialog->CONTROL( Static, ID_GOLD )->Enable( bEnable );
	m_pDialog->CONTROL( Static, ID_SILVER )->Enable( bEnable );
	m_pDialog->CONTROL( Static, ID_BRONZE )->Enable( bEnable );

	m_pDialog->CONTROL( Static, ID_GOLD )->SetText( L"" );
	m_pDialog->CONTROL( Static, ID_SILVER )->SetText( L"" );
	m_pDialog->CONTROL( Static, ID_BRONZE )->SetText( L"" );
}

//////////////////////////////////////////////////////////////////////////

CDnMoneyInputDlg::CDnMoneyInputDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  )
	: CEtUIDialog( dialogType, pParentDialog, nID, pCallback )
	, m_pStaticTitle(NULL)
	, m_pButtonOK(NULL)
	, m_pButtonCancel(NULL)
	, m_pButtonMax(NULL)
	, m_pButtonInit(NULL)
	, m_MoneyControl( this )
{
}

CDnMoneyInputDlg::~CDnMoneyInputDlg(void)
{
}

void CDnMoneyInputDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "MoneyInputDlg.ui" ).c_str(), bShow );
}

void CDnMoneyInputDlg::InitialUpdate()
{
	m_pStaticTitle = GetControl<CEtUIStatic>("ID_STATIC_TITLE");

	m_pButtonOK = GetControl<CEtUIButton>("ID_BUTTON_OK");
	m_pButtonCancel = GetControl<CEtUIButton>("ID_BUTTON_CANCEL");
	
	m_pButtonMax = GetControl<CEtUIButton>("ID_BUTTON_MAX");
	m_pButtonInit = GetControl<CEtUIButton>("ID_BUTTON_INIT");

	m_MoneyControl.InitialUpdate();
}

void CDnMoneyInputDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( !m_MoneyControl.ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg ) ) {
		return;
	}

	if ( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_BUTTON_MAX" ) )
		{
			m_MoneyControl.SetMax();
			return;
		}
		else if( IsCmdControl("ID_BUTTON_INIT" ) )
		{
			m_MoneyControl.Init();
			return;
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnMoneyInputDlg::SetInfo( LPCWSTR wszTitle, LPCWSTR wszOK )
{
	m_pStaticTitle->SetText( wszTitle );
	m_pButtonOK->SetText( wszOK );
}

void CDnMoneyInputDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	m_MoneyControl.Show( bShow );

	CEtUIDialog::Show( bShow );	
}

bool CDnMoneyInputDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
		return false;

	if( uMsg == WM_KEYDOWN )
	{
		if( wParam == VK_ESCAPE )
		{
			Show(false);
			return true;
		}
		else if( wParam == VK_RETURN )
		{
			if( m_MoneyControl.IsEditBoxFocus() )
			{
				RequestFocus( m_pButtonOK );
				return true;
			}
		}
	}

	if( m_MoneyControl.MsgProc( hWnd, uMsg, wParam, lParam ) ) {
		return true;
	}

	return CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );
}

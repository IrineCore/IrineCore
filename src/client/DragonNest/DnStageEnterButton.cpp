#include "StdAfx.h"
#include "DnStageEnterButton.h"
#include "DNTableDB.h"
#include "DNCommonDef.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnStageEnterButton::CDnStageEnterButton( CEtUIDialog *pParent )
: CEtUIRadioButton( pParent )
, m_nMapType(0)
, m_nMinLevel(0)
, m_nMaxLevel(0)
, m_bDarklairEnterButton( false )
, m_bMissionAchieve( false )
, m_nMapIndex(0)
, m_cPermit(0)
{
}

CDnStageEnterButton::~CDnStageEnterButton(void)
{
}

void CDnStageEnterButton::Clear()
{
	m_strDungeonName.clear();
	m_strPartyMaxCount.clear();
}

void CDnStageEnterButton::SetInfo( const wchar_t *wszDungeonName, const wchar_t *wszDungeonDesc, int nMinPartyCount, int nMaxPartyCount, int nMapType, int nMinLevel, int nMaxLevel, int nMapIndex, char cPermit )
{
	Clear();

	m_strDungeonName = wszDungeonName;
	m_strDungeonDesc = wszDungeonDesc;
	m_nMapType = nMapType;
	m_nMinLevel = nMinLevel;
	m_nMaxLevel = nMaxLevel;
	m_nMapIndex = nMapIndex;
	m_cPermit = cPermit;

//#define PRE_MOD_STAGE_ENTER_BUTTON
#ifdef PRE_MOD_STAGE_ENTER_BUTTON
	// Exception handling: cannot check for embedded newlines because this is a shared string used elsewhere.
	// Have no choice but to just truncate at a reasonable size.
	bool bCheck = false;
	SUICoord textCoord1;
	SUIElement *pElement = GetElement(1);
	// Get width directly from FontManager to keep it consistent regardless of dialog resolution and size.
	CEtFontMng::GetInstance().CalcTextRect( pElement->nFontIndex, pElement->nFontHeight, m_strDungeonName.c_str(), pElement->dwFontFormat, textCoord1, -1 );

	// And truncate the string at a reasonable point (currently beyond 100 pixels at 1024 resolution),
	// Display string in the unused PartyMaxCount slot.
	if( textCoord1.fWidth > (100.0f/DEFAULT_UI_SCREEN_WIDTH) )
	{
		int nTrail = 0;
		int nCaret = CEtFontMng::GetInstance().GetCaretFromCaretPos( m_strDungeonName.c_str(), pElement->nFontIndex, pElement->nFontHeight, 
			int(100.0f/DEFAULT_UI_SCREEN_WIDTH), nTrail );
		m_strPartyMaxCount = m_strDungeonName.substr( nCaret+nTrail );
		m_strDungeonName = m_strDungeonName.substr( 0, nCaret+nTrail );
	}

	// If party max count is set and the text is long, it gets clipped instead of rendering properly, so deciding whether to adopt it is deferred for now.
	// Skip for now and plan to add it later if it becomes truly necessary.
#endif	// PRE_MOD_STAGE_ENTER_BUTTON

	wchar_t wszTemp[64] = {0};
	if( nMinPartyCount != -1 && nMaxPartyCount != -1 )
		swprintf_s( wszTemp, _countof(wszTemp), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 6480 ), nMinPartyCount, nMaxPartyCount );
	else if( nMinPartyCount == -1 && nMaxPartyCount != -1 )
		swprintf_s( wszTemp, _countof(wszTemp), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2366 ), nMaxPartyCount );

	m_strPartyMaxCount = wszTemp;
}

void CDnStageEnterButton::Render( float fElapsedTime )
{
	m_CurrentState = UI_STATE_NORMAL;

	SUIElement *pElement(NULL);

	SUICoord textCoord1, textCoord2;
	{
		// Note: Calculate text coordinates first.
		//
		pElement = GetElement(1);
		m_pParent->CalcTextRect( m_strDungeonName.c_str(), pElement, textCoord1 );
		textCoord1.fX = m_Property.UICoord.fX + 0.046f;
		textCoord1.fY = m_Property.UICoord.fY + 0.054f;
		textCoord1.fX += pElement->fFontHoriOffset;
		textCoord1.fY += pElement->fFontVertOffset;

		pElement = GetElement(0);
		if( m_bDarklairEnterButton )
		{
			m_pParent->CalcTextRect( m_strDungeonDesc.c_str(), pElement, textCoord2 );
			textCoord2.fX = m_Property.UICoord.fX + 0.046f;
			textCoord2.fY = textCoord1.fY + textCoord1.fHeight + 0.004f;
		}
		else
		{
			m_pParent->CalcTextRect( m_strPartyMaxCount.c_str(), pElement, textCoord2 );
			textCoord2.fX = textCoord1.fX;
			textCoord2.fY = textCoord1.fY + textCoord1.fHeight + 0.004f;
		}
	}

	if( !IsShow() )
	{
		m_CurrentState = UI_STATE_HIDDEN;
	}
	else if( !IsEnable() )
	{
		m_CurrentState = UI_STATE_DISABLED;
	}
	else if( m_bPressed )
	{
		m_CurrentState = UI_STATE_PRESSED;
	}
	else if( m_bMouseEnter )
	{
		m_CurrentState = UI_STATE_MOUSEENTER;
	}
	else if( m_bFocus )
	{
		m_CurrentState = UI_STATE_FOCUS;
	}

	UpdateBlendRate();

	if( m_CurrentState == UI_STATE_HIDDEN )
	{
		pElement = GetElement(3);
		if( !pElement ) return;

		pElement->TextureColor.Blend( UI_STATE_NORMAL, fElapsedTime, m_fBlendRate );
		if( m_bExistTemplateTexture )
			m_pParent->DrawSprite( m_Template.m_hTemplateTexture, pElement->TemplateUVCoord, pElement->TextureColor.dwCurrentColor, m_ButtonCoord );
		else
			m_pParent->DrawSprite( pElement->UVCoord, pElement->TextureColor.dwCurrentColor, m_ButtonCoord );
		return;
	}

	UI_CONTROL_STATE SelectState1, SelectState2;

	if( m_bChecked )
	{
		SelectState1 = UI_STATE_HIDDEN;
		SelectState2 = m_CurrentState;
	}
	else
	{
		SelectState1 = m_CurrentState;
		SelectState2 = UI_STATE_HIDDEN;
	}

	static SUICoord uiCoord(m_Property.UICoord);
	m_Property.UICoord.fWidth = uiCoord.fWidth;

	float fMaxRight = max( textCoord1.Right(), textCoord2.Right() );
	float fMaxWidth = max( textCoord1.fWidth, textCoord2.fWidth );

	if( fMaxRight+0.02f > m_Property.UICoord.Right() )
	{
		m_Property.UICoord.fWidth = fMaxWidth + 0.074f;

		// Note : If larger than the texture size, split into 3 parts to render.
		//
		m_BaseCoordLeft.SetCoord( m_Property.UICoord.fX, m_Property.UICoord.fY, 60/(float)DEFAULT_UI_SCREEN_WIDTH, m_Property.UICoord.fHeight );
		m_BaseCoordMid.SetCoord( m_BaseCoordLeft.Right(), m_Property.UICoord.fY, m_Property.UICoord.fWidth-(60/(float)DEFAULT_UI_SCREEN_WIDTH*2.0f), m_Property.UICoord.fHeight );
		m_BaseCoordRight.SetCoord( m_BaseCoordMid.Right(), m_Property.UICoord.fY, 60/(float)DEFAULT_UI_SCREEN_WIDTH, m_Property.UICoord.fHeight );

		EtTextureHandle hTexture = m_pParent->GetUITexture();
		if( m_bExistTemplateTexture )
			hTexture = m_Template.m_hTemplateTexture;
		if( !hTexture ) return;

		int nTextureWidth = hTexture->Width();
		float fBaseWidth = 60 / (float)nTextureWidth;
		
		pElement = GetElement(0);
		if( !pElement ) return;
		{
			SUICoord UVCoord = pElement->UVCoord;
			if( m_bExistTemplateTexture )
				UVCoord = pElement->TemplateUVCoord;

			m_uvCoordLeft = UVCoord;
			m_uvCoordLeft.fWidth = fBaseWidth;

			m_uvCoordRight = UVCoord;
			m_uvCoordRight.fX = UVCoord.Right() - fBaseWidth;
			m_uvCoordRight.fWidth = fBaseWidth;

			m_uvCoordMid = UVCoord;
			m_uvCoordMid.fX = UVCoord.fX + fBaseWidth;
			m_uvCoordMid.fWidth = ((pElement->fTextureWidth-(60/(float)DEFAULT_UI_SCREEN_WIDTH*2.0f))*DEFAULT_UI_SCREEN_WIDTH) / nTextureWidth;
		}

		pElement->TextureColor.Blend( SelectState1, fElapsedTime, m_fBlendRate );

		m_pParent->DrawSprite( hTexture, m_uvCoordLeft, pElement->TextureColor.dwCurrentColor, m_BaseCoordLeft );
		m_pParent->DrawSprite( hTexture, m_uvCoordMid, pElement->TextureColor.dwCurrentColor, m_BaseCoordMid );
		m_pParent->DrawSprite( hTexture, m_uvCoordRight, pElement->TextureColor.dwCurrentColor, m_BaseCoordRight );

		pElement = GetElement(1);
		if( !pElement ) return;
		{
			SUICoord UVCoord = pElement->UVCoord;
			if( m_bExistTemplateTexture )
				UVCoord = pElement->TemplateUVCoord;

			m_uvCoordLeft = UVCoord;
			m_uvCoordLeft.fWidth = fBaseWidth;

			m_uvCoordRight = UVCoord;
			m_uvCoordRight.fX = UVCoord.Right() - fBaseWidth;
			m_uvCoordRight.fWidth = fBaseWidth;

			m_uvCoordMid = UVCoord;
			m_uvCoordMid.fX = UVCoord.fX + fBaseWidth;
			m_uvCoordMid.fWidth = ((pElement->fTextureWidth-(60/(float)DEFAULT_UI_SCREEN_WIDTH*2.0f))*DEFAULT_UI_SCREEN_WIDTH) / nTextureWidth;
		}

		pElement->TextureColor.Blend( SelectState2, fElapsedTime, m_fBlendRate );

		m_pParent->DrawSprite( hTexture, m_uvCoordLeft, pElement->TextureColor.dwCurrentColor, m_BaseCoordLeft );
		m_pParent->DrawSprite( hTexture, m_uvCoordMid, pElement->TextureColor.dwCurrentColor, m_BaseCoordMid );
		m_pParent->DrawSprite( hTexture, m_uvCoordRight, pElement->TextureColor.dwCurrentColor, m_BaseCoordRight );
	}
	else
	{
		pElement = GetElement(0);
		if( !pElement ) return;

		pElement->TextureColor.Blend( SelectState1, fElapsedTime, m_fBlendRate );
		if( m_bExistTemplateTexture )
			m_pParent->DrawSprite( m_Template.m_hTemplateTexture, pElement->TemplateUVCoord, pElement->TextureColor.dwCurrentColor, m_ButtonCoord );
		else
			m_pParent->DrawSprite( pElement->UVCoord, pElement->TextureColor.dwCurrentColor, m_ButtonCoord );

		pElement = GetElement(1);
		if( !pElement ) return;

		pElement->TextureColor.Blend( SelectState2, fElapsedTime, m_fBlendRate );
		if( m_bExistTemplateTexture )
			m_pParent->DrawSprite( m_Template.m_hTemplateTexture, pElement->TemplateUVCoord, pElement->TextureColor.dwCurrentColor, m_ButtonCoord );
		else
			m_pParent->DrawSprite( pElement->UVCoord, pElement->TextureColor.dwCurrentColor, m_ButtonCoord );
	}

	// Note : Display dungeon icon
	//
	pElement = GetElement( 3 + m_nMapType );
	if( pElement && m_nMapType != 0 )
	{
		pElement->TextureColor.Blend( m_CurrentState, fElapsedTime, m_fBlendRate );

		SUICoord iconCoord;
		iconCoord.fX = m_Property.UICoord.fX + 0.008f;
		iconCoord.fY = m_Property.UICoord.fY + 0.01f;
		iconCoord.fWidth = pElement->fTextureWidth;
		iconCoord.fHeight = pElement->fTextureHeight;
		if( m_bExistTemplateTexture )
			m_pParent->DrawSprite( m_Template.m_hTemplateTexture, pElement->TemplateUVCoord, pElement->TextureColor.dwCurrentColor, iconCoord );
		else
			m_pParent->DrawSprite( pElement->UVCoord, pElement->TextureColor.dwCurrentColor, iconCoord );
	}

	// Note : Display dungeon name and player limit
	//
	{
		pElement = GetElement(1);
		pElement->FontColor.Blend( m_CurrentState, fElapsedTime, m_fBlendRate );
		pElement->ShadowFontColor.Blend( m_CurrentState, fElapsedTime, m_fBlendRate );
		m_pParent->DrawDlgText( m_strDungeonName.c_str(), pElement, pElement->FontColor.dwCurrentColor, textCoord1 );

		pElement = GetElement(0);
		pElement->FontColor.Blend( m_CurrentState, fElapsedTime, m_fBlendRate );
		pElement->ShadowFontColor.Blend( m_CurrentState, fElapsedTime, m_fBlendRate );
		if( m_bDarklairEnterButton )
			m_pParent->DrawDlgText( m_strDungeonDesc.c_str(), pElement, pElement->FontColor.dwCurrentColor, textCoord2 );
		else
			m_pParent->DrawDlgText( m_strPartyMaxCount.c_str(), pElement, pElement->FontColor.dwCurrentColor, textCoord2 );
	}
}

void CDnStageEnterButton::MakeStringByPermit(std::wstring& str)
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMAPGATE );
	if (pSox == NULL)
		return;

	std::vector<int> nVecItemList;
	pSox->GetItemIDListFromField( "_MapIndex", m_nMapIndex, nVecItemList );

	// There must be only one.
	if (nVecItemList.size() != 1)
		return;

	int nItemID = nVecItemList[0];
	if (nItemID <= 0)
		return;

	DNTableFileFormat* pGateSox = GetDNTable( CDnTableDB::TMAPGATE );
	if (pGateSox == NULL)
		return;

	char* pszDateTime = pGateSox->GetFieldFromLablePtr( nItemID, "_DateTime" )->GetString();
	std::vector<string> vGateDateTime;
	TokenizeA(pszDateTime , vGateDateTime, ":");

	int nType = 0, nAttr1 = 0, nAttr2 = 0, nAttr3 = 0;
	if (vGateDateTime.size() > 0)
	{
		nType = atoi(vGateDateTime[0].c_str());
		if (nType == MapGateCondition::oDailyHours && vGateDateTime.size() == 3)	// Time
		{
			nAttr1 = atoi(vGateDateTime[1].c_str());
			nAttr2 = atoi(vGateDateTime[2].c_str());
			str += FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 100077 ), nAttr1, nAttr2 ); // UISTRING : Open daily from %d:00 to %d:00
			return;
		}

		if (nType == MapGateCondition::oDayOfWeek && vGateDateTime.size() == 2) // Day of week
		{
			nAttr1 = atoi(vGateDateTime[1].c_str());

			const WCHAR* wszDay = GetDayOfWeekString(nAttr1);
			if (!wszDay)
				return;

			str += FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 100078 ), wszDay ); // UISTRING : Open only on %s
			return;
			
		}
		else if (nType == MapGateCondition::oDayOfWeekAndHours && vGateDateTime.size() == 4) // Day of week & time
		{
			nAttr1 = atoi(vGateDateTime[1].c_str());
			nAttr2 = atoi(vGateDateTime[2].c_str());
			nAttr3 = atoi(vGateDateTime[3].c_str());

			const WCHAR* wszDay = GetDayOfWeekString(nAttr1);
			if (!wszDay)
				return;

			str += FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 100079 ), wszDay, nAttr2, nAttr3 ); // UISTRING : Opens on %s from %d:00 to %d:00	
			return;
		}
		if (nType == MapGateCondition::oDayOfWeekDuringFatigueTime && vGateDateTime.size() == 2) // Day-of-week fatigue time
		{
			nAttr1 = atoi(vGateDateTime[1].c_str());

			const WCHAR* wszDay = GetDayOfWeekString(nAttr1);
			if (!wszDay)
				return;

			DNTableFileFormat *pScheduleSox = GetDNTable( CDnTableDB::TSCHEDULE );
			if( !pScheduleSox )
				return;

			int nItemID = 2;	//Reset fatigue
			if( !pScheduleSox->IsExistItem( nItemID ) ) return;
			int nHour = pScheduleSox->GetFieldFromLablePtr( nItemID, "_Hour" )->GetInteger();
			int nMinute = pScheduleSox->GetFieldFromLablePtr( nItemID, "_Minute" )->GetInteger();

			str += FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 100081 ), wszDay, nHour, nMinute ); // UISTRING : Opens on %s at %d:%d.
			return;

		}
#if defined(PRE_ADD_CHALLENGE_DARKLAIR)
		else if (nType == MapGateCondition::cMonthlyHours && vGateDateTime.size() == 4) // Close only on that day & time every month
		{
			nAttr1 = atoi(vGateDateTime[1].c_str());
			nAttr2 = atoi(vGateDateTime[2].c_str());
			nAttr3 = atoi(vGateDateTime[3].c_str());

			//Need to add UI string
			str += FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 100080 ), nAttr1, nAttr2, nAttr3 ); // UISTRING : Usage is restricted on day %d of every month from %d:00 to %d:00
			return;
		}
#endif
	}
}

const WCHAR* CDnStageEnterButton::GetDayOfWeekString(int nDay)
{
	switch (nDay)
	{
	case 0: // Day
		return GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 114067 );
	case 1: // Month
		return GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 114061 );
	case 2: // Fire / Tue
		return GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 114062 );
	case 3: // Count
		return GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 114063 );
	case 4: // Neck
		return GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 114064 );
	case 5: // Gold / Fri
		return GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 114065 );
	case 6: // Sat
		return GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 114066 );
	}

	return NULL;
}

bool CDnStageEnterButton::HandleMouse( UINT uMsg, float fX, float fY, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
		return false;

	if( drag::IsValid() && !m_bEnableDragMode )
		return true;

	switch( uMsg )
	{
	case WM_MOUSEMOVE:
		{
			if( IsEnable() == false )
			{
				std::wstring str;
				if (m_cPermit & CDnWorld::PermitClose)
					MakeStringByPermit(str);
				else
				{
					if( m_nMinLevel != -1 )
						str += FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 920 ), m_nMinLevel ); // UISTRING : Cannot enter below level %d
					if( m_nMaxLevel != -1 ) 
					{
						if( !str.empty() ) str += L"\n";
						str += FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 919 ), m_nMaxLevel ); // UISTRING : Cannot enter above level %d
					}
				}

				if( m_bDarklairEnterButton && !m_bMissionAchieve )
				{
					if( !str.empty() ) str += L"\n";
					str += GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 832 ); // UISTRING : Opens after clearing the previous floor
				}

				if( str.length() > 0 )
					m_pParent->ShowTooltipDlg( this, true, str, textcolor::RED, true );
			}
		}
		break;
	}

	CEtUIRadioButton::HandleMouse(uMsg, fX, fY, wParam, lParam);

	return false;
}

#include "StdAfx.h"
#include "DnPVPZoneMapDlg.h"
#include "DnActor.h"
#include "DnPlayerActor.h"
#include "TaskManager.h"
#include "DnGameTask.h"
#include "DnPartyTask.h"
#include "DnCommonUtil.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnPVPZoneMapDlg::CDnPVPZoneMapDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  )
: CDnZoneMapDlg( dialogType, pParentDialog, nID, pCallback )
{
	m_IsAllPlayerShow = false;
}

CDnPVPZoneMapDlg::~CDnPVPZoneMapDlg(void)
{
	SAFE_RELEASE_SPTR( m_hMinimap );
}

void CDnPVPZoneMapDlg::CreatePartyControl()
{
	bool IsMyTeam = false;

	CDnPartyTask* pPartyTask = static_cast<CDnPartyTask*>(CTaskManager::GetInstancePtr()->GetTask( "PartyTask" ));
	if( !pPartyTask ) return;

	CDnPartyTask::PartyStruct* pParty(NULL);
	int nCount = pPartyTask->GetPartyCount();

	SUIControlProperty sUIProperty;	

	CEtUIButton *pButton(NULL);

	int iMyTeam = 0;

	if( CDnActor::s_hLocalActor )
		iMyTeam = CDnActor::s_hLocalActor->GetTeam(); 

	for( int i = 0; i<nCount; i++ )
	{
		pParty = pPartyTask->GetPartyData( i );
		if( !pParty ) continue;

		if( pParty->nSessionID == CDnBridgeTask::GetInstance().GetSessionID() )
			continue;

		if( pParty->hActor->GetTeam() != iMyTeam )//Enemy
		{
			m_pButtonEnemy->GetProperty( sUIProperty );

		}else
		{
			if (CommonUtil::IsValidCharacterClassId(pParty->cClassID) == false)
				continue;
			m_pButtonParty[pParty->cClassID - 1]->GetProperty(sUIProperty);
			IsMyTeam = true;
		}

		pButton = (CEtUIButton*)CEtUIDialog::CreateControl( &sUIProperty );

		pButton->Show( IsMyTeam );

		SZonePartyButtonInfo zonePartyButtonInfo;
		zonePartyButtonInfo.pButton = pButton;
		zonePartyButtonInfo.nSessionID = pParty->nSessionID;

		m_listButtonParty.push_back( zonePartyButtonInfo );
	}
}

void CDnPVPZoneMapDlg::ProcessParty( float fElapsedTime )
{
	if( !CDnWorld::IsActive() || !CDnWorld::GetInstance().GetGrid() ) 
		return;

	if ( !CDnWorld::GetInstance().GetGrid() )
		return;

	CDnPartyTask* pPartyTask = static_cast<CDnPartyTask*>(CTaskManager::GetInstancePtr()->GetTask( "PartyTask" ));
	if( !pPartyTask ) return;

	CDnPartyTask::PartyStruct* pParty(NULL);

	SUICoord textureCoord;
	m_pTextureMinimap->GetUICoord(textureCoord);

	float fMapWidth = CDnWorld::GetInstance().GetGrid()->GetGridWidth() * 50.0f;
	float fMapHeight = CDnWorld::GetInstance().GetGrid()->GetGridHeight() * 50.0f;

	CEtUIButton *pButton(NULL);
	CDnActor *pActor(NULL);

	std::list<SZonePartyButtonInfo>::iterator iter = m_listButtonParty.begin();
	for( ; iter != m_listButtonParty.end(); )
	{
		pActor = (CDnActor*)CDnActor::FindActorFromUniqueID( iter->nSessionID );
		pParty = pPartyTask->GetPartyDataFromSessionID( iter->nSessionID );

		if( !pParty )
		{
			DeleteControl( iter->pButton );
			iter = m_listButtonParty.erase( iter );
			continue;
		}

		EtVector3 *pNpcPos = ( pActor ) ? pActor->GetPosition() : &pParty->vPos;
		if( !pNpcPos ) continue;		

		float fNpcX = pNpcPos->x + fMapWidth;
		float fNpcY = fabs(pNpcPos->z - fMapHeight);

		SUICoord UVCoord;
		m_pTextureMinimap->GetUVCoord(UVCoord);

		fNpcX = (((fNpcX / fMapWidth )*0.5f - UVCoord.fX) / UVCoord.fWidth) * textureCoord.fWidth;
		fNpcY = (((fNpcY / fMapHeight )*0.5f - UVCoord.fY) / UVCoord.fHeight) * textureCoord.fHeight;


		SUICoord uiCoord;

		if( CDnActor::s_hLocalActor && pActor && ( pActor->GetTeam() != CDnActor::s_hLocalActor->GetTeam()) )
		{
			CDnPlayerActor* pPlayerActor = dynamic_cast<CDnPlayerActor*>(pActor);
			//Enemy
			if( pPlayerActor )
			{
				iter->pButton->Show((pPlayerActor->GetExposure() || m_IsAllPlayerShow));
				iter->pButton->GetUICoord(uiCoord);
				iter->pButton->SetPosition( textureCoord.fX + fNpcX - (uiCoord.fWidth/2.0f), textureCoord.fY + fNpcY - (uiCoord.fHeight/2.0f) );
				iter->pButton->SetTooltipText( pParty->wszCharacterName );								
			}
			++iter;
			continue;
		}
		
		iter->pButton->GetUICoord(uiCoord);
		iter->pButton->SetPosition( textureCoord.fX + fNpcX - (uiCoord.fWidth/2.0f), textureCoord.fY + fNpcY - (uiCoord.fHeight/2.0f) );
		iter->pButton->SetTooltipText( pParty->wszCharacterName );
		++iter;
	}
}

bool CDnPVPZoneMapDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
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
	}

	return CEtUIDialog::MsgProc(hWnd, uMsg, wParam, lParam);
}

void CDnPVPZoneMapDlg::Render( float fElapsedTime )
{
	CDnZoneMapDlg::Render( fElapsedTime );
#ifndef _FINAL_BUILD

	if( CGlobalValue::GetInstance().m_bEnableRespawnInfo && IsShow())
	{

		float fMapWidth = CDnWorld::GetInstance().GetGrid()->GetGridWidth() * 50.0f;
		float fMapHeight = CDnWorld::GetInstance().GetGrid()->GetGridHeight() * 50.0f;

		SUICoord textureCoord;
		m_pTextureMinimap->GetUICoord(textureCoord);
			
		for(UINT i = 0 ; i < CGlobalValue::GetInstance().m_vRespawnInfo.size()  ;i++)
		{
			EtVector3 vPosition = CGlobalValue::GetInstance().m_vRespawnInfo[i].Position;

			float fX = vPosition.x  + fMapWidth;
			float fY = fabs(vPosition.z  - fMapHeight);

			SUICoord UVCoord;
			m_pTextureMinimap->GetUVCoord(UVCoord);

			fX = (((fX / fMapWidth )*0.5f - UVCoord.fX) / UVCoord.fWidth) * textureCoord.fWidth;
			fY = (((fY / fMapHeight )*0.5f - UVCoord.fY) / UVCoord.fHeight) * textureCoord.fHeight;

			int nFontIndex = INT_MAX;			
			SFontDrawEffectInfo Info;
			Info.dwFontColor = 0xFFFFFFFF;
			wchar_t wszBuff[256]={0};
			MultiByteToWideChar(CP_UTF8, 0, CGlobalValue::GetInstance().m_vRespawnInfo[i].szName, -1, wszBuff, (int)strlen(CGlobalValue::GetInstance().m_vRespawnInfo[i].szName) );

			SUICoord screenCoord;

			CEtFontMng::GetInstance().CalcTextRect(nFontIndex,12,wszBuff,0xffffffff, screenCoord,-1);

			screenCoord.SetPosition(textureCoord.fX + fX + 0.15f, textureCoord.fY + fY );

			CEtFontMng::GetInstance().DrawTextW( nFontIndex, 12, wszBuff, 0xffffffff, screenCoord, -1, Info );

			/*
			SFontDrawEffectInfo Info;
			Info.dwFontColor = 0xFFFFFFFF;
			wchar_t wszBuff[256]={0};
			MultiByteToWideChar(CP_UTF8, 0, m_Property.szUIName, -1, wszBuff, (int)strlen(m_Property.szUIName) );

			SUICoord screenCoord;
			m_pParent->GetPosition(screenCoord.fX, screenCoord.fY);
			screenCoord.SetCoord(m_Property.UICoord.fX + screenCoord.fX, m_Property.UICoord.fY + screenCoord.fY, m_Property.UICoord.fWidth, m_Property.UICoord.fHeight);

			CEtFontMng::GetInstance().DrawTextW( pElement->nFontIndex, 16, wszBuff, 0xffffffff, screenCoord, -1, Info );
			*/

			//EternityEngine::DrawText2D( EtVector2( fX + textureCoord.fX + fWidthDot , fY + textureCoord.fY + fHeightDot ), CGlobalValue::GetInstance().m_vRespawnInfo[i].szName, D3DCOLOR_ARGB(255,255,0,0) );
		}
		
	}
	
#endif

}



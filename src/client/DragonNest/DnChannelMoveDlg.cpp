#include "StdAfx.h"
#include "DnChannelMoveDlg.h"
#include "DnPartyTask.h"
#include "DnWorld.h"
#include "DnSystemDlg.h"
#include "DnInterface.h"
#include "DnMainMenuDlg.h"
#include "DnTableDB.h"
#include "TaskManager.h"
#include "DnVillageTask.h"
#include "DnSimpleTooltipDlg.h"
#ifdef PRE_ADD_SECONDARY_SKILL
#include "DnLifeSkillCookingTask.h"
#endif // PRE_ADD_SECONDARY_SKILL
#include "DnLocalPlayerActor.h"

#ifdef PRE_MOD_POTENTIAL_JEWEL_RENEWAL
#include "DnPotentialJewelDlg.h"
#endif

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

const int NPC_UNION_ICON_TEXTURE_SIZE = 32;
#ifdef PRE_ADD_ACTIVEMISSION
const int NPC_UNION_MARK_TEXTURE_WIDTH = 256;
#endif PRE_ADD_ACTIVEMISSION

CDnChannelMoveDlg::CDnChannelMoveDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  )
	: CEtUIDialog( dialogType, pParentDialog, nID, pCallback  )
{
	m_pStaticChannel = NULL;
	m_pButtonChannel = NULL;
	m_pStaticMapName = NULL;
	m_pStaticDungeon = NULL;
	m_pStaticVillage = NULL;
	m_pStaticWorld = NULL;
	m_pStaticMeritMark = NULL;
	int i = 0;
	for (; i < _MAX_MARK_COUNT; ++i)
	{
		m_pMarks[i] = NULL;
		m_pMarksNoChannel[i] = NULL;
	}
}

CDnChannelMoveDlg::~CDnChannelMoveDlg(void)
{
	SAFE_RELEASE_SPTR(m_hMarkTexture);
}

void CDnChannelMoveDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "ChannelMoveDlg.ui" ).c_str(), bShow );
}

void CDnChannelMoveDlg::InitialUpdate()
{
	m_pStaticMapName = GetControl< CEtUIStatic >( "ID_STATIC_MAPNAME" );
	m_pStaticDungeon = GetControl< CEtUIStatic >( "ID_STATIC_DUNGEON" );
	m_pStaticVillage = GetControl< CEtUIStatic >( "ID_STATIC_VILLAGE" );
	m_pStaticWorld = GetControl< CEtUIStatic >( "ID_STATIC_WORLD" );
	m_pStaticMeritMark = GetControl<CEtUIStatic>("ID_STATIC_SPECIALCH");
	m_pStaticDungeon->Show( false );
	m_pStaticVillage->Show( false );
	m_pStaticWorld->Show( false );
	m_pStaticMeritMark->Show(false);

	m_pStaticChannel = GetControl< CEtUIStatic >( "ID_STATIC_CH" );
	m_pButtonChannel = GetControl< CEtUIButton >( "ID_BUTTON_CH" );

	int i = 0;
	std::string ctrlName;
	for (; i < _MAX_MARK_COUNT; ++i)
	{
		ctrlName = FormatA("ID_TEXTUREL_MARK%d", i);
		m_pMarks[i] = GetControl<CEtUITextureControl>(ctrlName.c_str());
		m_pMarks[i]->Show(false);

		ctrlName = FormatA("ID_TEXTUREL_MARK1_%d", i);
		m_pMarksNoChannel[i] = GetControl<CEtUITextureControl>(ctrlName.c_str());
		m_pMarksNoChannel[i]->Show(false);
	}

	SAFE_RELEASE_SPTR(m_hMarkTexture);
	m_hMarkTexture = LoadResource("Repute_SmallMark.dds", RT_TEXTURE, true);
}

void CDnChannelMoveDlg::Process( float fElapsedTime )
{
	CEtUIDialog::Process( fElapsedTime );

	if( !IsShow() )
		return;

	if( !CDnPartyTask::IsActive() ) return;

	if( CDnWorld::GetInstance().GetMapType() == CDnWorld::MapTypeDungeon )
	{
		Show( false );
	}
	else if( CDnWorld::GetInstance().GetMapType() == CDnWorld::MapTypeVillage )
	{
		m_pButtonChannel->Show(true);
		m_pStaticChannel->Show(true);

#ifdef PRE_PARTY_DB
		bool bIsPartyMember = false;
		if (CDnLocalPlayerActor::s_hLocalActor)
		{
			CDnLocalPlayerActor *pLocalActor = static_cast<CDnLocalPlayerActor*>(CDnLocalPlayerActor::s_hLocalActor.GetPointer());
			if (pLocalActor)
				bIsPartyMember = CDnPartyTask::GetInstance().IsPartyMember(pLocalActor->GetUniqueID());
		}

#ifdef PRE_MOD_POTENTIAL_JEWEL_RENEWAL
		CDnPotentialJewelDlg* pPotentialJewelDlg = GetInterface().GetItemPotentialDlg();
		bool bIsShowPotentialJewelDlg = false;
		if(pPotentialJewelDlg)
			bIsShowPotentialJewelDlg = pPotentialJewelDlg->IsShow();
#endif // PRE_MOD_POTENTIAL_JEWEL_RENEWAL

		if( bIsPartyMember
#ifdef PRE_ADD_SECONDARY_SKILL
			|| (CDnLifeSkillCookingTask::IsActive() && GetLifeSkillCookingTask().IsNowCooking())
#endif // PRE_ADD_SECONDARY_SKILL
#ifdef PRE_MOD_POTENTIAL_JEWEL_RENEWAL
			|| bIsShowPotentialJewelDlg
#endif // PRE_MOD_POTENTIAL_JEWEL_RENEWAL
			|| GetInterface().IsShowCostumeMixDlg()
			)
		{
			m_pButtonChannel->Enable(false);
		}
		else
		{
			m_pButtonChannel->Enable(true);
		}
#else
	#ifdef PRE_MOD_POTENTIAL_JEWEL_RENEWAL
		CDnPotentialJewelDlg* pPotentialJewelDlg = GetInterface().GetItemPotentialDlg();
		bool bIsShowPotentialJewelDlg = false;
		if(pPotentialJewelDlg)
			bIsShowPotentialJewelDlg = pPotentialJewelDlg->IsShow();
	#endif // PRE_MOD_POTENTIAL_JEWEL_RENEWAL

		if( CDnPartyTask::GetInstance().GetPartyRole() == CDnPartyTask::MEMBER 
	#ifdef PRE_ADD_SECONDARY_SKILL
			|| ( CDnLifeSkillCookingTask::IsActive() && GetLifeSkillCookingTask().IsNowCooking() )
	#endif // PRE_ADD_SECONDARY_SKILL
	#ifdef PRE_MOD_POTENTIAL_JEWEL_RENEWAL
			|| bIsShowPotentialJewelDlg
	#endif // PRE_MOD_POTENTIAL_JEWEL_RENEWAL
			|| GetInterface().IsShowCostumeMixDlg()
			)
		{
			m_pButtonChannel->Enable(false);
		}
		else
		{
			m_pButtonChannel->Enable(true);
		}
#endif // PRE_PARTY_DB
	}
	else
	{
		m_pButtonChannel->Show(false);
		m_pStaticChannel->Show(false);
		m_pStaticMeritMark->Show(false);
	}

	UpdateMarks(fElapsedTime);
}

void CDnChannelMoveDlg::UpdateMarks(float fElapsedTime)
{
	if (m_NoticeMarkHandler.IsUpdateMarks())
	{
		int i = 0;
		for (; i < _MAX_MARK_COUNT; ++i)
		{
			m_pMarks[i]->Show(false);
			m_pMarksNoChannel[i]->Show(false);
		}

		const std::list<CDnMarkInformer*>& markInfoList = m_NoticeMarkHandler.GetMarkInformerList();
		std::list<CDnMarkInformer*>::const_iterator iter = markInfoList.begin();
		for (i = 0; i < _MAX_MARK_COUNT && iter != markInfoList.end(); ++i, ++iter)
		{
			if (i >= (int)markInfoList.size())
				break;

			const CDnMarkInformer* pInformer = (*iter);
			if (pInformer)
			{
				CEtUITextureControl* pCtrl = (m_pButtonChannel->IsShow()) ? m_pMarks[i] : m_pMarksNoChannel[i];
				const int& iconIdx = pInformer->GetIconIdex();

#ifdef PRE_ADD_ACTIVEMISSION
				// Repute_SmallMark.dds ran out of horizontal space. Modified to extend vertically.
				int x = (iconIdx * NPC_UNION_ICON_TEXTURE_SIZE) % NPC_UNION_MARK_TEXTURE_WIDTH;
				int y = (iconIdx * NPC_UNION_ICON_TEXTURE_SIZE) / NPC_UNION_MARK_TEXTURE_WIDTH;
				pCtrl->SetTexture(m_hMarkTexture, x, y * NPC_UNION_ICON_TEXTURE_SIZE,
					NPC_UNION_ICON_TEXTURE_SIZE, NPC_UNION_ICON_TEXTURE_SIZE);
#else
				pCtrl->SetTexture(m_hMarkTexture, (iconIdx % NPC_UNION_ICON_TEXTURE_SIZE) * NPC_UNION_ICON_TEXTURE_SIZE, (iconIdx / NPC_UNION_ICON_TEXTURE_SIZE) * NPC_UNION_ICON_TEXTURE_SIZE,
					NPC_UNION_ICON_TEXTURE_SIZE, NPC_UNION_ICON_TEXTURE_SIZE);
#endif PRE_ADD_ACTIVEMISSION	

				/*pCtrl->SetTexture(m_hMarkTexture, (iconIdx % NPC_UNION_ICON_TEXTURE_SIZE) * NPC_UNION_ICON_TEXTURE_SIZE, (iconIdx / NPC_UNION_ICON_TEXTURE_SIZE) * NPC_UNION_ICON_TEXTURE_SIZE,
					NPC_UNION_ICON_TEXTURE_SIZE, NPC_UNION_ICON_TEXTURE_SIZE);*/
				pCtrl->SetTexture(m_hMarkTexture, (iconIdx % NPC_UNION_ICON_TEXTURE_SIZE) * NPC_UNION_ICON_TEXTURE_SIZE, (iconIdx / NPC_UNION_ICON_TEXTURE_SIZE) * NPC_UNION_ICON_TEXTURE_SIZE,
					NPC_UNION_ICON_TEXTURE_SIZE, NPC_UNION_ICON_TEXTURE_SIZE);
				pCtrl->Show(true);
			}
		}

		m_NoticeMarkHandler.SetUpdateMarksFlag(false);
	}

	m_NoticeMarkHandler.UpdateMarks(fElapsedTime, ((m_pButtonChannel->IsShow()) ? m_pMarks : m_pMarksNoChannel));
}

void CDnChannelMoveDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( GetInterface().IsShowPrivateMarketDlg() 
			|| GetInterface().IsShowCostumeMixDlg()
			)
			return;

		if( IsCmdControl("ID_BUTTON_CH" ) )
		{
			CDnSystemDlg *pSystemDlg = (CDnSystemDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::SYSTEM_DIALOG );
			if( pSystemDlg ) {
				pSystemDlg->OpenChannelDlg();
				drag::Command(UI_DRAG_CMD_CANCEL);
				drag::ReleaseControl();
			}
			return;			
		}
	}
}

void CDnChannelMoveDlg::SetChannelID( int nChannelID )
{
	if( !m_pStaticChannel )
		return;

	wchar_t szTemp[32] = {0};
#ifdef PRE_MOD_CONSTANT_TEXT_CONVERT_TO_TABLE_TEXT
	swprintf_s( szTemp, _countof(szTemp), L"%s%d", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7889 ), nChannelID ); // UISTRING : CH
#else
	swprintf_s( szTemp, _countof(szTemp), L"CH.%d", nChannelID );
#endif 
	m_pStaticChannel->SetText( szTemp );
}

void CDnChannelMoveDlg::RefreshMapInfo()
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMAP );
	if( !pSox ) return;

	int nID = CGlobalInfo::GetInstance().m_nCurrentMapIndex;
	int nStringTableID = pSox->GetFieldFromLablePtr( nID, "_MapNameID" )->GetInteger();

	m_pStaticMapName->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nStringTableID ) );

	m_pStaticDungeon->Show( false );	
	m_pStaticVillage->Show( false );
	m_pStaticWorld->Show( false );

	CDnWorld::MapTypeEnum mapType = CDnWorld::GetInstance().GetMapType();
	switch( mapType )
	{
		case CDnWorld::MapTypeDungeon:	m_pStaticDungeon->Show( true );		break;
		case CDnWorld::MapTypeVillage:	m_pStaticVillage->Show( true );		break;
		case CDnWorld::MapTypeWorldMap:	m_pStaticWorld->Show( true );		break;
		case CDnWorld::MapTypeUnknown:
		break;
	}

	CDnVillageTask *pVillageTask = (CDnVillageTask *)CTaskManager::GetInstance().GetTask( "VillageTask" );
	if (pVillageTask)
	{
		m_ChannelMeritDesc.clear();
		pVillageTask->GetMeritChannelText(m_ChannelMeritDesc, CDnActor::s_hLocalActor->GetLevel());
		m_pStaticMeritMark->Show(m_ChannelMeritDesc.empty() == false);
	}
	m_NoticeMarkHandler.SetUpdateMarksFlag(true);
}

bool CDnChannelMoveDlg::OnMouseOver(CEtUITextureControl** ppCtrl, float fMouseX, float fMouseY)
{
	if (ppCtrl == NULL)
		return true;

	int i = 0;
	bool bOnMouse = false;
	for (; i < _MAX_MARK_COUNT; ++i)
	{
		if (ppCtrl[i] && ppCtrl[i]->IsShow() && ppCtrl[i]->IsInside(fMouseX, fMouseY))
		{
			float fX, fY;
			GetPosition(fX, fY);

			m_NoticeMarkHandler.OnMouseOver(i, fX + fMouseX, fY + fMouseY);
			return true;
		}
	}

	return false;
}

bool CDnChannelMoveDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
	{
		return false;
	}

	bool bRet;
	bRet = CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );

	switch( uMsg )
	{
	case WM_MOUSEMOVE:
		{
			float fMouseX, fMouseY;
			POINT MousePoint;
			MousePoint.x = short( LOWORD( lParam ) );
			MousePoint.y = short( HIWORD( lParam ) );
			PointToFloat( MousePoint, fMouseX, fMouseY );

			if (m_pStaticMeritMark && m_pStaticMeritMark->IsShow() && m_pStaticMeritMark->IsInside(fMouseX, fMouseY) && m_ChannelMeritDesc.empty() == false)
			{
				CDnSimpleTooltipDlg* pTooltipDlg = GetInterface().GetSimpleTooltipDialog();
				pTooltipDlg->ShowTooltipDlg(m_pStaticMeritMark, true, m_ChannelMeritDesc, 0xffffffff, true);
				break;
			}

			CDnInterface::GetInstance().CloseRepUnionMarkTooltip();
#if defined(PRE_ADD_VIP_FARM)
			CDnInterface::GetInstance().CloseLifeUnionMarkTooltip();
#endif
			CDnInterface::GetInstance().CloseGuildWarMarkTooltip();
			CDnInterface::GetInstance().CloseGuildRewardMarkTooltip();

#ifdef PRE_ADD_WEEKLYEVENT
			CDnInterface::GetInstance().CloseWeeklyEventMarkTooltip();
#endif

#ifdef PRE_ADD_ACTIVEMISSION
			CDnInterface::GetInstance().CloseActiveMissionEventMarkTooltip();
#endif // PRE_ADD_ACTIVEMISSION


			bool bRetMarks0 = OnMouseOver(m_pMarks, fMouseX, fMouseY);
			if (bRetMarks0 == false)
				OnMouseOver(m_pMarksNoChannel, fMouseX, fMouseY);
		}
		break;
	}

	return bRet;
}

void CDnChannelMoveDlg::SetUnionMarks(std::vector<int>& unionMarkTableIds)
{
	m_NoticeMarkHandler.SetUnionMarks(unionMarkTableIds);
}

void CDnChannelMoveDlg::SetGuildWarMark(bool bShow, bool bNew)
{
	if (IsShow())
		m_NoticeMarkHandler.SetGuildWarMark(bShow, bNew);
}


void CDnChannelMoveDlg::SetGuildRewardMark(bool bShow, bool bNew)
{
	if (IsShow())
		m_NoticeMarkHandler.SetGuildRewardMark(bShow, bNew);
}

#if defined(PRE_ADD_WEEKLYEVENT)
void CDnChannelMoveDlg::SetWeeklyEventMark(bool bShow, bool bNew)
{
	if (IsShow())
		m_NoticeMarkHandler.SetWeeklyEventMark(bShow, bNew);
}
#endif

#if defined(PRE_ADD_ACTIVEMISSION)
void CDnChannelMoveDlg::SetActiveMissionEventMark( int acMissionID, std::wstring & str,bool bShow, bool bNew)
{
	m_NoticeMarkHandler.SetActiveMissionEventMark( acMissionID, str, bShow, bNew );
}
#endif // PRE_ADD_ACTIVEMISSION
#include "StdAfx.h"
#include "DnNoticeMarkHandler.h"
#include "DnInterface.h"
#include "DnTableDB.h"
//#define PRE_ADD_VIP_FARM
#if defined(PRE_ADD_VIP_FARM)
#include "DnLocalPlayerActor.h"
#endif

#ifdef PRE_ADD_ACTIVEMISSION
#include "DnDungeonInfoDlg.h"
#endif // PRE_ADD_ACTIVEMISSION

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif

void CDnMarkInformer::ResetBlink()
{
	m_bBlink = false;
	m_nBlinkPreventTimeBack = 0;
	m_fBlinkTime = 0.f;
}

void CDnMarkInformer::SetBlink(bool bBlink, float fBlinkTime)
{
	m_bBlink = bBlink;
	m_fBlinkTime = fBlinkTime;
	m_nBlinkPreventTimeBack = 0;
}

//////////////////////////////////////////////////////////////////////////

void CDnUnionMarkInformer::OnMouseOver(float fX, float fY)
{
	CDnInterface::GetInstance().ShowRepUnionMarkTooltip(m_TableId, fX, fY);
}

//////////////////////////////////////////////////////////////////////////

void CDnVIPFarmMarkInformer::OnMouseOver(float fX, float fY)
{
#if defined(PRE_ADD_VIP_FARM)
	CDnInterface::GetInstance().ShowLifeUnionMarkTooltip(fX, fY);
#endif
}

//////////////////////////////////////////////////////////////////////////

void CDnGuildWarMarkInformer::OnMouseOver(float fX, float fY)
{
	CDnInterface::GetInstance().ShowGuildWarMarkTooltip(fX, fY);
	SetBlink(false);
}

void CDnGuildRewardMarkInformer::OnMouseOver(float fX, float fY)
{
	CDnInterface::GetInstance().ShowGuildRewardMarkTooltip(fX, fY);
	SetBlink(false);
}
//////////////////////////////////////////////////////////////////////////

#if defined(PRE_ADD_WEEKLYEVENT)
void CDnWeeklyEventMarkInformer::OnMouseOver(float fX, float fY)
{
	CDnInterface::GetInstance().ShowWeeklyEventMarkTooltip(fX, fY);
	SetBlink(false);
}
#endif

#if defined(PRE_ADD_ACTIVEMISSION)
void CDnActiveMissionMarkInformer::OnMouseOver(float fX, float fY)
{
	GetInterface().ShowActiveMissionEventMarkTooltip( m_strTooltip, true, fX, fY );
	SetBlink(false);
}

void CDnActiveMissionMarkInformer::Process(float time)
{	
	// Display for 30 sec then close.
	if( m_bOpen )
	{
		m_CloseTime += time;
		if( m_CloseTime >= 30.0f )		
		{
			m_bOpen = false;
			m_CloseTime = 0.0f;

			GetInterface().CloseActiveMissionEventMarkTooltip();
		}
	}

	// Opens every 120 seconds.	
	else
	{		
		m_OpenTime += time;
		if( m_OpenTime >= 120.0f )		
		{
			m_bOpen = true;	
			m_OpenTime = 0.0f;

			if( m_fX == 0.0f && m_fY == 0.0f )
			{
				CDnDungeonInfoDlg * pDlg = GetInterface().GetDungeonInfoDlg();	
				if( pDlg )
					pDlg->GetActiveMarkPos( m_fX, m_fY );
			}			

			if( m_fX > 0.0f && m_fY > 0.0f )
				GetInterface().ShowActiveMissionEventMarkTooltip( m_strTooltip, true, m_fX, m_fY );
		}
	}
	
}
#endif // PRE_ADD_ACTIVEMISSION


CDnNoticeMarkHandler::CDnNoticeMarkHandler()
:m_bIsUpdateMarks(true) {}

CDnNoticeMarkHandler::~CDnNoticeMarkHandler()
{
	if (m_pMarkInfoList.empty() == false)
	{
		std::list<CDnMarkInformer*>::iterator iter = m_pMarkInfoList.begin();
		for (; iter != m_pMarkInfoList.end(); ++iter)
		{
			CDnMarkInformer* pInformer = (*iter);
			SAFE_DELETE(pInformer);
		}

		m_pMarkInfoList.clear();
	}
}

bool CDnNoticeMarkHandler::CompareInformers(const CDnMarkInformer* pMark1, const CDnMarkInformer* pMark2)
{
	return (pMark1->GetGroupType() < pMark2->GetGroupType());
}

void CDnNoticeMarkHandler::OnMouseOver(int slotIndex, float fX, float fY)
{
	std::list<CDnMarkInformer*>::iterator iter = m_pMarkInfoList.begin();
	std::advance(iter, slotIndex);

	if (iter != m_pMarkInfoList.end())
	{
		CDnMarkInformer* pInformer = (*iter);
		if (pInformer)
			pInformer->OnMouseOver(fX, fY);
	}
}

void CDnNoticeMarkHandler::UpdateMarks(float fElapsedTime, CEtUITextureControl** ppControls)
{
#if defined(PRE_ADD_VIP_FARM)
	ProcessVIPFarmMark();
#endif
	UpdateMarkBlink(fElapsedTime, ppControls);
}

void CDnNoticeMarkHandler::UpdateMarkBlink(float fElapsedTime, CEtUITextureControl** ppControls)
{
	if (GetInterface().IsOpenBlind() || GetInterface().IsFading())
		return;

	if (ppControls == NULL)
		return;

	int i = 0;
	std::list<CDnMarkInformer*>::iterator iter = m_pMarkInfoList.begin();
	for (; iter != m_pMarkInfoList.end() && i < _MAX_MARK_COUNT; ++iter, ++i)
	{
		CDnMarkInformer* pInfo = (*iter);
		CEtUITextureControl* pCtrl = ppControls[i];

		if (pInfo && pCtrl)
		{
			if (pInfo->IsNeedBlink())
			{
				if (pInfo->GetBlinkTime() <= 0.f)
				{
					pInfo->ResetBlink();
					pCtrl->Show(false);
				}
				else
				{
					int nBlinkTime = int(pInfo->GetBlinkTime() * 10.0f);

					if (((nBlinkTime % 3) == 0) && (pInfo->GetBlinkPreventTimeBack() != nBlinkTime))
					{
						pInfo->SetBlinkPreventTimeBack(nBlinkTime);
						pCtrl->Show(!pCtrl->IsShow());
					}
				}

				pInfo->ProcessBlinkElapsedTime(fElapsedTime);
			}
			else
			{
				pCtrl->Show(true);
			}
#ifdef PRE_ADD_ACTIVEMISSION
			pInfo->Process( fElapsedTime );
#endif // PRE_ADD_ACTIVEMISSION

		}
	}
}

void CDnNoticeMarkHandler::RemoveInfoByGroupType(eMarkGroupType type)
{
	std::list<CDnMarkInformer*>::iterator iter = m_pMarkInfoList.begin();
	for (; iter != m_pMarkInfoList.end();)
	{
		CDnMarkInformer* pInfo = (*iter);
		if (pInfo != NULL && pInfo->GetGroupType() == type) {
			SAFE_DELETE( pInfo );
			iter = m_pMarkInfoList.erase(iter);
		}
		else
			++iter;
	}
}

CDnMarkInformer * CDnNoticeMarkHandler::GetMarkInfo(eMarkGroupType type)
{
	std::list<CDnMarkInformer*>::iterator iter = m_pMarkInfoList.begin();
	for (; iter != m_pMarkInfoList.end(); iter++ )
	{
		CDnMarkInformer* pInfo = (*iter);
		if (pInfo != NULL && pInfo->GetGroupType() == type) {
			return pInfo;
		}		
	}

	return NULL;
}

bool CDnNoticeMarkHandler::IsMarkGroupTypeInInfoList(eMarkGroupType type) const
{
	std::list<CDnMarkInformer*>::const_iterator iter = m_pMarkInfoList.begin();
	for (; iter != m_pMarkInfoList.end(); ++iter)
	{
		const CDnMarkInformer* pInfo = (*iter);
		if (pInfo->GetGroupType() == type)
			return true;
	}

	return false;
}

void CDnNoticeMarkHandler::SetUnionMarks(std::vector<int>& unionMarkTableIds)
{
	DNTableFileFormat*  pBenefitTable = GetDNTable(CDnTableDB::TREPUTEBENEFIT);
	if (pBenefitTable == NULL)
		return;

	if ((int)unionMarkTableIds.size() > NpcReputation::UnionType::Etc)
		return;

	m_pMarkInfoList.sort(CompareInformers);

	RemoveInfoByGroupType(MARK_REPUTE);

	std::vector<int>::const_iterator curIter = unionMarkTableIds.begin();
	for (; curIter != unionMarkTableIds.end(); ++curIter)
	{
		const int& tableId = (*curIter);
		if (tableId != 0)
		{
			int iconIdx = pBenefitTable->GetFieldFromLablePtr(tableId, "_UnionIconID")->GetInteger();
			if (iconIdx >= 0)
			{
				CDnMarkInformer* pCurrentInformer = new CDnUnionMarkInformer(MARK_REPUTE, iconIdx, tableId);
				m_pMarkInfoList.push_back(pCurrentInformer);

				m_pMarkInfoList.sort(CompareInformers);
			}
		}
	}

	m_bIsUpdateMarks = true;
}

#if defined(PRE_ADD_VIP_FARM)
void CDnNoticeMarkHandler::ProcessVIPFarmMark()
{
	if (!CDnLocalPlayerActor::s_hLocalActor)
		return;

	CDnLocalPlayerActor *pLocalActor = static_cast<CDnLocalPlayerActor *>(CDnLocalPlayerActor::s_hLocalActor.GetPointer());
	if (!pLocalActor )
		return;

	bool bIsVipFarm = pLocalActor->IsVipFarm();
	bool bIsVipFarmMark = IsMarkGroupTypeInInfoList(MARK_VIP_FARM);

	if( bIsVipFarm == bIsVipFarmMark )
		return;

	RemoveInfoByGroupType(MARK_VIP_FARM);

	if (bIsVipFarm)
	{
		CDnMarkInformer* pCurrentInformer = new CDnVIPFarmMarkInformer(MARK_VIP_FARM);
		m_pMarkInfoList.push_back(pCurrentInformer);
	}

	m_pMarkInfoList.sort(CompareInformers);
	m_bIsUpdateMarks = true;
}
#endif // PRE_ADD_VIP_FARM


void CDnNoticeMarkHandler::SetGuildWarMark(bool bShow, bool bNew)
{
	RemoveInfoByGroupType(MARK_GUILDWAR_ALERT);
	
	if (bShow)
	{
		CDnMarkInformer* pCurrentInformer = new CDnGuildWarMarkInformer(MARK_GUILDWAR_ALERT);
		pCurrentInformer->SetBlink(bNew);
		m_pMarkInfoList.push_back(pCurrentInformer);
	}

	m_pMarkInfoList.sort(CompareInformers);
	m_bIsUpdateMarks = true;
}

void CDnNoticeMarkHandler::SetGuildRewardMark(bool bShow, bool bNew)
{
	RemoveInfoByGroupType(MARK_GUILDREWARD_ALERT);

	if (bShow)
	{
		CDnMarkInformer* pCurrentInformer = new CDnGuildRewardMarkInformer(MARK_GUILDREWARD_ALERT);
		pCurrentInformer->SetBlink(bNew);
		m_pMarkInfoList.push_back(pCurrentInformer);
	}

	m_pMarkInfoList.sort(CompareInformers);
	m_bIsUpdateMarks = true;
}

#if defined(PRE_ADD_WEEKLYEVENT)
void CDnNoticeMarkHandler::SetWeeklyEventMark( bool bShow, bool bNew )
{
	RemoveInfoByGroupType( MARK_WEEKLYEVENT_ALERT );

	if (bShow)
	{
		CDnMarkInformer* pCurrentInformer = new CDnWeeklyEventMarkInformer( MARK_WEEKLYEVENT_ALERT );
		pCurrentInformer->SetBlink( bNew );
		m_pMarkInfoList.push_back( pCurrentInformer );
	}

	m_pMarkInfoList.sort( CompareInformers );
	m_bIsUpdateMarks = true;
}
#endif

#ifdef PRE_ADD_ACTIVEMISSION
void CDnNoticeMarkHandler::SetActiveMissionEventMark( int acMissionID, std::wstring & str, bool bShow, bool bNew )
{
	/*// If it is an already existing active mission, no need to change.
	CDnMarkInformer * pMarkInfo = GetMarkInfo( MARK_ACTIVEMISSION_ALERT );
	if( pMarkInfo )
	{
		CDnActiveMissionMarkInformer * pAcMark = dynamic_cast<CDnActiveMissionMarkInformer*>( pMarkInfo );
		if( pAcMark && acMissionID == pAcMark->GetActivemissionID() )
			return;
	}*/

	RemoveInfoByGroupType( MARK_ACTIVEMISSION_ALERT );

	if( bShow )
	{
		CDnActiveMissionMarkInformer * pCurrentInformer = new CDnActiveMissionMarkInformer( MARK_ACTIVEMISSION_ALERT );
		pCurrentInformer->SetActiveMissionID( acMissionID );		 
		pCurrentInformer->SetActiveMissionTooltip( str );
		pCurrentInformer->SetBlink( bNew );
		m_pMarkInfoList.push_back( pCurrentInformer );
	}

	m_pMarkInfoList.sort( CompareInformers );
	m_bIsUpdateMarks = true;
}
#endif // PRE_ADD_ACTIVEMISSION
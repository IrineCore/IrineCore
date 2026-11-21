#include "StdAfx.h"
#include "DnLifeSkillContent.h"
#include "DnSkillTreeDlg.h"
#include "DnLifeSkillItem.h"
#include "DnLifeSkillCookingTask.h"
#include "DnLifeSkillFishingTask.h"
#include "TaskManager.h"
#include "DnSkillTask.h"
#include "DnTableDB.h"
#include "SecondarySkill.h"
#include "DnLifeSkillItem.h"
#include "DnInterface.h"
#include "DnPlayerActor.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnLifeSkillContent::CDnLifeSkillContent() : m_pSkillTreeDlg( NULL )
{
}

CDnLifeSkillContent::~CDnLifeSkillContent()
{
}

void CDnLifeSkillContent::RefreshInfo()
{
	if( m_pSkillTreeDlg == NULL || m_pSkillTreeDlg->GetSkillListBox() == NULL )
		return;

	m_pSkillTreeDlg->GetSkillListBox()->RemoveAllItems();
	CSecondarySkillRepository& pLifeSkillRepository = GetSkillTask().GetLifeSkillRepository();

	std::map<int,CSecondarySkill*>::iterator iter = pLifeSkillRepository.GetSecondarySkillMap().begin();
	for( ; iter != pLifeSkillRepository.GetSecondarySkillMap().end(); iter++ )
	{
		AddLifeSkill( (*iter).second );
	}
}

void CDnLifeSkillContent::AddLifeSkill( CSecondarySkill* pAddSecondarySkill )
{
	if( pAddSecondarySkill == NULL )
		return;

	DNTableFileFormat* pSoxSkill = GetDNTable( CDnTableDB::TSecondarySkill );
	DNTableFileFormat* pSoxSkillLevel = GetDNTable( CDnTableDB::TSecondarySkillLevel );
	if( pSoxSkill == NULL || pSoxSkillLevel == NULL )
	{
		DN_ASSERT( 0, "Invalid SecondarySkillType" );
		return;
	}

	std::vector<int> vecID;
	for( int i=0; i<pSoxSkillLevel->GetItemCount(); i++ )
		vecID.push_back( pSoxSkillLevel->GetItemID( i ) );

	int nSkillLevelIndex = 0;
	std::vector<int>::iterator iter = vecID.begin();
	for( ; iter!= vecID.end(); iter++ )
	{
		if( pAddSecondarySkill->GetSkillID() == pSoxSkillLevel->GetFieldFromLablePtr( (*iter), "_SecondarySkillID" )->GetInteger() )
		{
			SecondarySkill::Grade::eType eSkillGrade = (SecondarySkill::Grade::eType)pSoxSkillLevel->GetFieldFromLablePtr( (*iter), "_SecondarySkillClass" )->GetInteger();
			int nSkillLevel = pSoxSkillLevel->GetFieldFromLablePtr( (*iter), "_SecondarySkillLevel" )->GetInteger();

			if( eSkillGrade == pAddSecondarySkill->GetGrade() && nSkillLevel == pAddSecondarySkill->GetLevel() )
			{
				nSkillLevelIndex = (*iter);
				break;
			}
		}
	}

	CDnLifeSkillItem *pItem = m_pSkillTreeDlg->GetSkillListBox()->AddItem<CDnLifeSkillItem>();

	stLifeSkillInfoData stInfoData;

	stInfoData.m_nSkillID = pAddSecondarySkill->GetSkillID();
	int nNameIndex = pSoxSkill->GetFieldFromLablePtr( pAddSecondarySkill->GetSkillID(), "_SecondarySkillName" )->GetInteger();
	stInfoData.m_strName = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nNameIndex );
	int nDescriptionIndex = pSoxSkillLevel->GetFieldFromLablePtr( nSkillLevelIndex, "_SecondarySkillExplanation" )->GetInteger();
	stInfoData.m_strDescription = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nDescriptionIndex );
	stInfoData.m_eGrade = pAddSecondarySkill->GetGrade();
	stInfoData.m_nLevel = pAddSecondarySkill->GetLevel();
	stInfoData.m_nExp = pAddSecondarySkill->GetExp();

	SetCustomSkillFunction( pSoxSkill, &stInfoData );

	pItem->SetInfo( stInfoData );
}

void CDnLifeSkillContent::SetCustomSkillFunction( DNTableFileFormat* pSoxSkill, stLifeSkillInfoData* pLifeSkillInfoData )
{
	if( pSoxSkill == NULL || pLifeSkillInfoData == NULL )
		return;

	SecondarySkill::SubType::eType SubType;
	SubType = (SecondarySkill::SubType::eType)pSoxSkill->GetFieldFromLablePtr( pLifeSkillInfoData->m_nSkillID, "_SecondarySkillType" )->GetInteger();

	switch( SubType )
	{
		case SecondarySkill::SubType::CookingSkill:
			{
				pLifeSkillInfoData->m_fpDoSkill = &CDnSkillTask::DoSkillCooking;
				pLifeSkillInfoData->m_strDoSkill = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7310 );
			}
			break;
		case SecondarySkill::SubType::CultivationSkill:
			{
				pLifeSkillInfoData->m_fpDoSkill = NULL;
				pLifeSkillInfoData->m_strDoSkill = L"";
			}
			break;
		case SecondarySkill::SubType::FishingSkill:
			{
				pLifeSkillInfoData->m_fpDoSkill = &CDnSkillTask::DoSkillFishing;
				pLifeSkillInfoData->m_strDoSkill = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7311 );
			}
			break;
		default:
			{
				pLifeSkillInfoData->m_fpDoSkill = &CDnSkillTask::DoSkillNone;
				pLifeSkillInfoData->m_strDoSkill = L"";
			}
			break;
	}
}

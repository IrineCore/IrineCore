#include "StdAfx.h"
#include "DnItemChoiceDlg.h"
#include "DnPartyTask.h"
#include "DnItemTask.h"
#include "DnInterfaceString.h"
#include "DnLocalPlayerActor.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CDnItemChoiceDlg::CDnItemChoiceDlg(UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback)
	: CDnCustomDlg(dialogType, pParentDialog, nID, pCallback)
{
	m_pItemSlotBtn	= NULL;
	m_pItemName		= NULL;
	m_pItemLevel	= NULL;
	m_pItemClass	= NULL;
	m_pWarning		= NULL;

	m_pProgressBar	= NULL;
	m_pAcceptBtn	= NULL;
	m_pGiveUpBtn	= NULL;

	m_pItemCache	= NULL;
	m_DropItemUniqueID = 0;

	m_RemainTime = 0.0f;
	m_TotalTime = 0.0f;
}

CDnItemChoiceDlg::~CDnItemChoiceDlg(void)
{
	SAFE_DELETE(m_pItemCache);
}

void CDnItemChoiceDlg::Initialize(bool bShow)
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "ItemChoiceDlg.ui" ).c_str(), bShow );
}

void CDnItemChoiceDlg::InitialUpdate()
{
	m_pItemName		= GetControl<CEtUIStatic>("ID_STATIC_TEXT");
	m_pItemLevel	= GetControl<CEtUIStatic>("ID_STATIC_LEVEL");
	m_pItemClass	= GetControl<CEtUIStatic>("ID_STATIC_TYPE");
	m_pWarning		= GetControl<CEtUIStatic>("ID_STATIC1");

	m_pProgressBar	= GetControl<CEtUIProgressBar>("ID_PROGRESSBAR0");
	m_pAcceptBtn	= GetControl<CEtUIButton>("ID_OK");
	m_pGiveUpBtn	= GetControl<CEtUIButton>("ID_CANCEL");
}

void CDnItemChoiceDlg::InitCustomControl(CEtUIControl *pControl)
{
	if (pControl == NULL || !strstr(pControl->GetControlName(), "ID_BUTTON_SYMBOL"))
		return;

	m_pItemSlotBtn = static_cast<CDnItemSlotButton*>(pControl);
	m_pItemSlotBtn->SetSlotType(ST_INVENTORY);
	//m_pItemSlotBtn->SetSlotIndex(0);
}

void CDnItemChoiceDlg::Process(float fElapsedTime)
{
	if (IsShow() == false)
		return;

	if( m_RemainTime <= 0.0f )
	{
		OnSelectCancel();
		Show(false);
	}
	else
	{
		m_RemainTime -= fElapsedTime;

		if( m_TotalTime != 0.0f)
		{
			m_pProgressBar->SetProgress( (m_RemainTime / m_TotalTime) * 100.0f );
		}
		else
		{
			m_pProgressBar->SetProgress( 0.0f );
		}
	}

	CDnCustomDlg::Process(fElapsedTime);
}

void CDnItemChoiceDlg::OnSelectCancel()
{
	if (CDnActor::s_hLocalActor && m_pItemCache)
	{
		CDnLocalPlayerActor* pPlayer = static_cast<CDnLocalPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());
		if (pPlayer && pPlayer->IsSummonPet())
			pPlayer->AddPetNotEatItemList(m_DropItemUniqueID);
	}

	GetPartyTask().ReqPartyJoinGetReversionItem(false);
}

void CDnItemChoiceDlg::ProcessCommand(UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg)
{
	SetCmdControlName( pControl->GetControlName() );

	if (nCommand == EVENT_BUTTON_CLICKED)
	{
		if (IsCmdControl("ID_OK"))
		{
			GetPartyTask().ReqPartyJoinGetReversionItem(true);
			Show(false);
		}

		if (IsCmdControl("ID_CANCEL"))
		{
			OnSelectCancel();
			Show(false);
		}
	}

	CDnCustomDlg::ProcessCommand(nCommand, bTriggeredByUser, pControl, uMsg);
}

void CDnItemChoiceDlg::SetItem(const TItem& itemInfo)
{
	SAFE_DELETE(m_pItemCache);

	TItemInfo infoForCreate; 
	infoForCreate.cSlotIndex = 0;
	infoForCreate.Item = itemInfo;
	m_pItemCache = GetItemTask().CreateItem(infoForCreate);
	if (m_pItemCache == NULL)
	{
		_ASSERT(0);
		return;
	}

	m_pItemSlotBtn->SetItem(m_pItemCache, m_pItemCache->GetOverlapCount());
	//m_pItemSlotBtn->SetShowTooltip(false);
	m_pItemName->SetText(m_pItemCache->GetName());

	std::wstring strTemp;
	int levelLimit = m_pItemCache->GetLevelLimit();
	strTemp = (levelLimit != 0) ? FormatW(L"%s%d", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 2240), levelLimit)
								: FormatW(L"%s%s", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 2240), GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 93));	// UISTRING : Level Limit : , None
	m_pItemLevel->SetText(strTemp.c_str());

	DWORD dwLevelColor = textcolor::WHITE;
	if (levelLimit > CDnActor::s_hLocalActor->GetLevel())
		dwLevelColor = textcolor::RED;
	m_pItemLevel->SetTextColor(dwLevelColor);

	DWORD dwClassColor = textcolor::WHITE;
	strTemp = L"";
	if( m_pItemCache->IsNeedJob() )
	{
		WCHAR szTemp[256] = {0, };
		WCHAR szTempSub1[256] = {0, };

		int nNeedJobCount = m_pItemCache->GetNeedJobCount();

		swprintf_s( szTemp, 256, L"" );
		for( int i = 0; i < nNeedJobCount; ++i ) {
			int nJobID = m_pItemCache->GetNeedJob(i);
			swprintf_s( szTempSub1, 256, L"%s", DN_INTERFACE::STRING::GetJobString( nJobID ) );

			if( szTemp[0] == L'\0' )
				swprintf_s( szTemp, 256, L"%s", szTempSub1 );
			else
				swprintf_s( szTemp, 256, L"%s, %s", szTemp, szTempSub1 );
		}

		std::vector<int> nVecJobList;
		if (CDnActor::s_hLocalActor)
		{
			((CDnPlayerActor*)CDnActor::s_hLocalActor.GetPointer())->GetJobHistory( nVecJobList );
			if(m_pItemCache->IsPermitPlayer( nVecJobList ) == false)
				dwClassColor = textcolor::RED;
		}

		strTemp = szTemp;
	}

	std::wstring formatString = (strTemp.empty()) ? L"%s %s" : L" / %s %s";
	if (m_pItemCache->GetItemType() == ITEMTYPE_WEAPON)
	{
		CDnWeapon *pWeapon = dynamic_cast<CDnWeapon *>(m_pItemCache);
		if( pWeapon )
			strTemp += FormatW(formatString.c_str(), DN_INTERFACE::STRING::ITEM::GetEquipString(pWeapon->GetEquipType()), GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 7009));	// UISTRING : Type
	}
	else if( m_pItemCache->GetItemType() == ITEMTYPE_PARTS )
	{
		const CDnParts *pParts = dynamic_cast<const CDnParts *>(m_pItemCache);
		if( pParts )
		{
			if( m_pItemCache->IsCashItem() )
				strTemp += FormatW(formatString.c_str(), DN_INTERFACE::STRING::ITEM::GetCashPartsString(pParts->GetPartsType()), GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 7009));	// UISTRING : Type
			else
				strTemp += FormatW(formatString.c_str(), DN_INTERFACE::STRING::ITEM::GetPartsString( pParts->GetPartsType() ), GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 7009));	// UISTRING : Type
		}
	}

	m_pItemClass->SetTextColor(dwClassColor);
	m_pItemClass->SetText(strTemp.c_str());

	std::wstring soulBoundStr = (itemInfo.bSoulbound) ? GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 2111) : GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 4130);	// UISTRING : Seal/Bind
	strTemp = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3625), soulBoundStr.c_str()); // UISTRING : This item is %s. Do you want to roll the dice?
	m_pWarning->SetText(strTemp.c_str());
}

void CDnItemChoiceDlg::Open(const TItem& itemInfo, float progressTotalTime, const DWORD& dropItemUniqueID)
{
	SetItem(itemInfo);
	m_TotalTime = m_RemainTime = progressTotalTime;
	m_pProgressBar->SetProgress(100.f);

	m_DropItemUniqueID = dropItemUniqueID;

	Show(true);
}

void CDnItemChoiceDlg::Close()
{
// 	if ( IsCmdControl("ID_CANCEL"))
// 		GetPartyTask().ReqPartyJoinGetReversionItem(false);

	Show(false);
}

void CDnItemChoiceDlg::Show(bool bShow)
{
	if (bShow == m_bShow)
		return;

	if (m_pItemCache == NULL && bShow)
		return;

	CDnCustomDlg::Show(bShow);
}
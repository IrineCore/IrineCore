#include "StdAfx.h"
#include "DnCharmItemProgressDlg.h"
#include "DnWorld.h"
#include "ItemSendPacket.h"
#include "DnInterface.h"
#include "DnTradeTask.h"
#include "DnTradeMail.h"

#include "DnLocalPlayerActor.h"
#include "DnTableDB.h"
#include "DnItemTask.h"
//#include "DnCashInventory.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CDnCharmItemProgressDlg::CDnCharmItemProgressDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, true )
, m_pCancelButton(NULL)
, m_pProgressBarTime(NULL)
, m_fTimer(0.f)
, m_fMaxTimer(0.f)
{
	m_cFlag = 0;
	m_cInvenType = ITEMPOSITION_INVEN;
	m_sInvenIndex = -1;
	m_biInvenSerial = 0;
	m_cKeyInvenIndex = -1;
	m_nKeyItemID = 0;
	m_biKeyItemSerial = 0;

	m_pTempItem = NULL;

#if defined(PRE_FIX_43986)
	m_nSoundIndex = -1;
	m_pItemSlot = NULL;
	m_pStaticText = NULL;
#endif // PRE_FIX_43986
}

CDnCharmItemProgressDlg::~CDnCharmItemProgressDlg(void)
{
	SAFE_DELETE(m_pTempItem);

	//m_hSound->Release();
}

void CDnCharmItemProgressDlg::Initialize( bool bShow )
{
#if defined(PRE_FIX_43986)
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "CharmItemOpen.ui" ).c_str(), bShow );
#else
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "RandomItemDlg.ui" ).c_str(), bShow );
#endif // PRE_FIX_43986
	
}

void CDnCharmItemProgressDlg::InitialUpdate()
{
	m_pCancelButton = GetControl<CEtUIButton>("ID_BUTTON_CANCEL");
	m_pProgressBarTime = GetControl<CEtUIProgressBar>("ID_PROGRESSBAR_TIME");

#if defined(PRE_FIX_43986)
	m_pItemSlot = GetControl<CDnItemSlotButton>("ID_BT_ITEM");
	m_pStaticText = GetControl<CEtUIStatic>("ID_TEXT_ITEMNAME");

	if (m_pItemSlot)
		m_pItemSlot->SetShowTooltip(false);

	m_nSoundIndex = CEtSoundEngine::GetInstance().LoadSound( "UI_CharmItemOpen_Loop.ogg", false, false );
#endif // PRE_FIX_43986
}

void CDnCharmItemProgressDlg::Process( float fElapsedTime )
{
	CEtUIDialog::Process( fElapsedTime );

	if( !IsShow() ) return;
	switch( m_cFlag ) {
		case 1:
			{
#ifdef PRE_ADD_AUTOUNPACK
				// Stop auto-opening if moving while item auto-open is active.
				if( GetInterface().IsOpenAutoUnPackDlg() )
				{
					CDnLocalPlayerActor *pLocalActor = static_cast<CDnLocalPlayerActor *>(CDnLocalPlayerActor::s_hLocalActor.GetPointer());
					if (pLocalActor != NULL)
					{
						if ((CDnActor::s_hLocalActor->IsMove() && CDnActor::s_hLocalActor->IsMovable()) || 
							pLocalActor->IsAutoRun() || pLocalActor->IsFollowing() )
							{
								SendCancelCharmItem();
								Show(false);
								break;
						    }
					}	
				}
#endif
				m_fTimer -= fElapsedTime;
				if( m_fTimer <= 0.f ) {
					m_fTimer = 0.f;
					m_pCancelButton->Enable( false );
					m_pProgressBarTime->Enable( false );
					m_cFlag = 0;

					SendCompleteCharmItem( m_cInvenType, m_sInvenIndex, m_biInvenSerial, m_cKeyInvenIndex, m_nKeyItemID, m_biKeyItemSerial );
					Show( false );

					//Stop sound.
#if defined(PRE_FIX_43986)
					if (m_hSound)
						m_hSound->GetChannel()->stop();
#endif // PRE_FIX_43986
				}
				m_pProgressBarTime->SetProgress( 100.f - ( 100.f / m_fMaxTimer * m_fTimer ) );


#if defined(PRE_FIX_43986)
				m_CurItemDelay -= fElapsedTime;
				if (m_CurItemDelay <= 0.0f)
				{
					m_CurItemDelay = m_ItemDelayTime;
					ChangeItemInfo();
				}
#endif // PRE_FIX_43986
			}
			break;
	}
}

void CDnCharmItemProgressDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_BUTTON_CANCEL" ) ) {
			SendCancelCharmItem();

			Show(false);

			CDnTradeMail& tradeMail = GetTradeTask().GetTradeMail();
			if (tradeMail.IsOnMailMode())
				tradeMail.LockMailDlg(false);

			GetInterface().CloseMovieDlg();
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnCharmItemProgressDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	if (bShow == false)
	{
		//Stop sound.
#if defined(PRE_FIX_43986)
		if (m_hSound)
		{
			m_hSound->GetChannel()->stop();
		}
#endif // PRE_FIX_43986
	}

	CEtUIDialog::Show( bShow );
}

void CDnCharmItemProgressDlg::ShowEx( bool bShow, char cInvenType, short sInvenIndex, INT64 biInvenSerial, BYTE cKeyInvenIndex, int nKeyItemID, INT64 biKeyItemSerial, float fTimer, float fDelayTime )
{
	Show( bShow );

	if( bShow ) {
		m_cFlag = 1;
#ifdef PRE_ADD_AUTOUNPACK
		m_fMaxTimer = m_fTimer = fTimer;
#else
		m_fMaxTimer = m_fTimer = fTimer = 3.0f;
#endif

		if( m_fMaxTimer == 0.0f ) m_fMaxTimer = 1.0f;

		m_pCancelButton->Show( true );
		m_pProgressBarTime->Show( true );

		m_pCancelButton->Enable( true );
		m_cInvenType = cInvenType;
		m_sInvenIndex = sInvenIndex;
		m_biInvenSerial = biInvenSerial;
		m_cKeyInvenIndex = cKeyInvenIndex;
		m_nKeyItemID = nKeyItemID;
		m_biKeyItemSerial = biKeyItemSerial;

#if defined(PRE_FIX_43986)
		MakeItemList();
		
		m_ItemDelayTime = fDelayTime;
		m_CurItemDelay = 0.0f;

		//Output sound (as loop??...)
		if (m_nSoundIndex != -1)
		{
			m_hSound = CEtSoundEngine::GetInstance().PlaySound( "2D", m_nSoundIndex, true );
		}
#endif // PRE_FIX_43986
	}
}

#if defined(PRE_FIX_43986)
void CDnCharmItemProgressDlg::MakeItemList()
{
	m_OrigCharmItemList.clear();

	CDnItem *pCharmItem = NULL;
	if (m_cInvenType == ITEMPOSITION_CASHINVEN)
		pCharmItem = GetItemTask().GetCashInventory().GetItem(m_sInvenIndex);
	else
		pCharmItem = GetItemTask().GetCharInventory().GetItem(m_sInvenIndex);
	

#if defined( PRE_ADD_EASYGAMECASH )
#ifdef PRE_ADD_GETITEM_WITH_RANDOMVALUE
	if (pCharmItem == NULL || (pCharmItem->GetItemType() != ITEMTYPE_CHARM && 
							   pCharmItem->GetItemType() != ITEMTYPE_ALLGIVECHARM && 
							   pCharmItem->GetItemType() != ITEMTYPE_CHARMRANDOM))
		return;
#else
	if (pCharmItem == NULL || (pCharmItem->GetItemType() != ITEMTYPE_CHARM && pCharmItem->GetItemType() != ITEMTYPE_ALLGIVECHARM))
		return;
#endif

#else
	if (pCharmItem == NULL || pCharmItem->GetItemType() != ITEMTYPE_CHARM)
		return;
#endif
	
	int nCurItemCharmNum = pCharmItem->GetTypeParam(0);

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TCHARMITEM );
	if (pSox)
	{
		int nTableCount = pSox->GetItemCount();
		for (int i = 0; i < nTableCount; ++i)
		{
			int nTableItemID = pSox->GetItemID(i);
			
			const DNTableCell *pCharmNumField = pSox->GetFieldFromLablePtr(nTableItemID, "_CharmNum");
			if (pCharmNumField == NULL)
				continue;

			int nCharmNo = pCharmNumField->GetInteger();
			if (nCharmNo != nCurItemCharmNum) continue;

			const DNTableCell *pItemIDField = pSox->GetFieldFromLablePtr(nTableItemID, "_ItemID");
			if (pItemIDField == NULL)
				continue;

#if defined(PRE_ADD_54166)
			//#54166 2nd Pandora Box system overhaul - Added feature to show selectable contents
			const DNTableCell *pLookField = pSox->GetFieldFromLablePtr(nTableItemID, "_Look");
			if (pLookField == NULL || pLookField->GetBool() == false)
				continue;
#endif // PRE_ADD_54166

			int nItemID = pItemIDField->GetInteger();
			m_OrigCharmItemList.insert(std::make_pair(nItemID, nItemID));
		}
	}

	//Build the list and copy to the display list..
	m_CharmItemList = m_OrigCharmItemList;

	m_nCurItemIndex = -1;
}

void CDnCharmItemProgressDlg::ChangeItemInfo()
{
	int nListCount = (int)m_CharmItemList.size();
	if (nListCount == 0)
		m_nCurItemIndex = -1;
	else
		m_nCurItemIndex = rand() % nListCount;

	if (m_nCurItemIndex == -1)
		return;
	
	std::map<int, int>::iterator findIter = m_CharmItemList.begin();
	for (int i = 0; i < m_nCurItemIndex; ++i)
		findIter++;
	
	int nSelectedItemID = findIter->second;

	if (m_pTempItem == NULL)
	{
		m_pTempItem = CDnItem::CreateItem( nSelectedItemID, 0 );

		if (m_pItemSlot && m_pTempItem)
			m_pItemSlot->SetItem( m_pTempItem, CDnSlotButton::ITEM_ORIGINAL_COUNT );
	}

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TITEM );
	if (pSox)
	{
		int nIconIndex = -1;
		eItemRank itemRank = ITEMRANK_D;
		eItemTypeEnum itemType = ITEMTYPE_NORMAL;

		const DNTableCell* pIconImageField = pSox->GetFieldFromLablePtr( nSelectedItemID, "_IconImageIndex" );
		if (pIconImageField)
			nIconIndex = pIconImageField->GetInteger();

		const DNTableCell* pRankField = pSox->GetFieldFromLablePtr( nSelectedItemID, "_Rank" );
		if (pRankField)
			itemRank = (eItemRank)pRankField->GetInteger();

		const DNTableCell* pItmeTypeField = pSox->GetFieldFromLablePtr( nSelectedItemID, "_Type" );
		if (pItmeTypeField)
			itemType = (eItemTypeEnum)pItmeTypeField->GetInteger();


		m_pTempItem->ChangeIconIndex(nIconIndex);
		m_pTempItem->ChangeRank(itemRank);
		m_pTempItem->ChangeItemType(itemType);


		m_pItemSlot->SetItem(m_pTempItem, CDnSlotButton::ITEM_ORIGINAL_COUNT);
		
		if (m_pStaticText)
		{
			tstring szName = CDnItem::GetItemFullName(nSelectedItemID);

			m_pStaticText->SetText(szName.c_str());
		}
	}

	//Remove an item from the list once it has been displayed.
	if (findIter != m_CharmItemList.end())
		m_CharmItemList.erase(findIter);


	//Once all items are displayed within the window duration, no more items are shown, so
	//When the list is completely empty, recreate the list..
	if (m_CharmItemList.empty())
		m_CharmItemList = m_OrigCharmItemList;
}
#endif // PRE_FIX_43986
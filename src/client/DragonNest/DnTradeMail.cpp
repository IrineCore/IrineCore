#include "StdAfx.h"
#include "DnTradeMail.h"
#include "TradeSendPacket.h"
#include "DnMailDlg.h"
#include "DnTradeTask.h"
#include "DnTradeMail.h"
#include "DnMainDlg.h"
#include "DnInterface.h"
#include "DnMainMenuDlg.h"
#include "DnTableDB.h"
#include "DnCommonUtil.h"
#include "DnItemTask.h"
#ifdef PRE_ADD_CADGE_CASH
#include "DnCashShopDlg.h"
#include "DnCashShopTask.h"
#endif // PRE_ADD_CADGE_CASH



#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnTradeMail::CDnTradeMail(void)
	: m_nTotalMailCount(0)
	, m_nUnreadMailCount(0)
	, m_nItemMailCount(0)
	, m_pMailDialog(NULL)
	, m_bDelMailSoReqLastPage(false)
	, m_RecvAttachItemSerialCache(-1)
{
	m_nDaySendEnableCount = 0;
	m_nRecvAttachGoldSoundIdx = -1;
}

CDnTradeMail::~CDnTradeMail(void)
{
	ClearMailBoxInfoList();
}

bool CDnTradeMail::Initialize()
{
	const char *szFileName = CDnTableDB::GetInstance().GetFileName( 10003 );
	if( strlen( szFileName ) > 0 )
		m_nRecvAttachGoldSoundIdx = CEtSoundEngine::GetInstance().LoadSound( CEtResourceMng::GetInstance().GetFullName( szFileName ).c_str(), false, false );

	return true;
}

void CDnTradeMail::Finalize()
{
	CEtSoundEngine::GetInstance().RemoveSound( m_nRecvAttachGoldSoundIdx );
}

void CDnTradeMail::RequestMailBox( int nPageNum )
{
	if (IsLockedDlgs())
		return;

	SendMailBox(nPageNum);

	LockMailDlg(true);
}

bool CDnTradeMail::IsLockedDlgs()
{
	if (m_pMailDialog->IsLockedDlgs())
	{
		GetInterface().AddChatMessage(CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1989), true);	// UISTRING : Processing mail. Please wait a moment.
		return true;
	}

	return false;
}

void CDnTradeMail::RequestMailSend( const SSendMailInfo &sendMailInfo )
{
	if (IsLockedDlgs())
		return;

	TAttachMailItem AttachMailItem[MAILATTACHITEMMAX];
	ZeroMemory(AttachMailItem, _countof(AttachMailItem));

	int i = 0, nCount = 0;
	for (; i < MAILATTACHITEMMAX; ++i)
	{
		const SSendAttachItemInfo& info = sendMailInfo.attachItems[i];
		if (info.index == -1) continue;

		AttachMailItem[nCount].nInvenIndex = info.index;
		AttachMailItem[nCount].nItemID = info.itemClassID;
		AttachMailItem[nCount].nCount = info.count;
		AttachMailItem[nCount].biSerial = info.biSerial;
		nCount++;
	}
	SendMailSend(sendMailInfo.strReceiver.c_str(), 
				sendMailInfo.strTitle.c_str(), 
				sendMailInfo.strText.c_str(), 
				(int)sendMailInfo.money, 
				sendMailInfo.isPremium,
				nCount,
				AttachMailItem
				);

	LockMailDlg(true);
}

void CDnTradeMail::RequestMailRead( int nMailDBID )
{
	if (IsLockedDlgs())
		return;

	SMailBoxInfo *pMailBoxInfo = FindMailBoxInfo( nMailDBID );
	if( !pMailBoxInfo ) return;

#ifdef PRE_ADD_CADGE_CASH
	if( pMailBoxInfo->mailKind == MailType::Cadge )
		SendCadgeMailRead( nMailDBID );
	else
		SendMailRead( nMailDBID );
#else // PRE_ADD_CADGE_CASH
	SendMailRead( nMailDBID );
#endif // PRE_ADD_CADGE_CASH

	LockMailDlg(true);
}

void CDnTradeMail::RequestMailDelete(int* mailDBIDList, int delCount)
{
	if (IsLockedDlgs())
		return;

	m_bDelMailSoReqLastPage = (delCount >= (int)m_vecMailBoxInfoList.size()) ? (GetCurrentPage() == GetMaxPage()) : false;

	SendMailDelete(mailDBIDList);

	LockMailDlg(true);
}

void CDnTradeMail::RequestMailAttachAll(int* mailDBIDList)
{
	if (IsLockedDlgs())
		return;

	SendAttachAllMail(mailDBIDList);

	LockMailDlg(true);
}

void CDnTradeMail::RequestMailAttach(int mailDBID, INT64 attachItemSerial /* = -1*/, int attachSlotIdx /* = -1*/)
{
	if (IsLockedDlgs())
		return;

	SendAttachMail(mailDBID, attachSlotIdx);
	m_RecvAttachItemSerialCache = attachItemSerial;
	LockMailDlg(true);
}

void CDnTradeMail::OnRecvMailBox( SCMailBox *pPacket )
{
	LockMailDlg(false);

	if (pPacket->nRetCode == ERROR_NONE)
	{
		ClearMailBoxInfoList();

		SMailBoxInfo mailBoxInfo;

		for( int i=0; i<(int)pPacket->cMailBoxCount; i++ )
		{
			mailBoxInfo.Assign(pPacket->MailBox[i]);
			m_vecMailBoxInfoList.push_back(mailBoxInfo);
		}

		m_nTotalMailCount = pPacket->wTotalCount;
		m_nUnreadMailCount = pPacket->wUnreadCount;
		SetDaySendEnableCount(pPacket->cDailyMailCount);

		m_pMailDialog->RefreshMailBoxList(m_vecMailBoxInfoList);
	}
	else
		GetInterface().ServerMessageBox(pPacket->nRetCode);
}

void CDnTradeMail::OnRecvMailSend( SCSendMail *pPacket )
{
	LockMailDlg(false);

	if (pPacket->nRet == ERROR_NONE)
	{
		GetInterface().MessageBox(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1967 ), MB_OK);	// UISTRING : Sent successfully.
		SetDaySendEnableCount(pPacket->cDailyMailCount);
		m_pMailDialog->OpenMailWriteDialog(_T(""), _T(""), true);
		return;
	}
	else
	{
		if( ERROR_GENERIC_LEVELLIMIT == pPacket->nRet )
		{
			WCHAR wszMessage[512];
			int iWeight = (int)CGlobalWeightTable::GetInstance().GetValue(CGlobalWeightTable::LimitLevel_SendMail);
			swprintf_s( wszMessage, _countof(wszMessage), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1995 ), iWeight );	// Mail writing is available from level %d.

			GetInterface().MessageBox( wszMessage );

			return;
		}
		GetInterface().ServerMessageBox(pPacket->nRet);
	}
}

void CDnTradeMail::OnRecvMailRead( SCReadMail *pPacket )
{
	LockMailDlg(false);

	if (pPacket->nRet == ERROR_NONE)
	{
		SMailBoxInfo *pMailBoxInfo = FindMailBoxInfo( pPacket->nMailDBID );
		if( !pMailBoxInfo ) return;

		SReadMailInfo& info = m_CurReadMailInfoCache;
		info.Clear();

		info.nMailID	= pMailBoxInfo->nMailID;
		info.strSender	= pMailBoxInfo->strName;
		info.strTitle	= pMailBoxInfo->strTitle;

		time_t sendDate = pPacket->tSendDate;
		DnLocalTime_s(&info.date, &sendDate);

		info.strText	= pPacket->wszText;
		info.money		= pPacket->nAttachCoin;

		info.bPremium = (pPacket->cDeliveryType == MailType::Premium);
		info.mailKind = pMailBoxInfo->mailKind;
		info.bNewFlag	= pPacket->bNewFlag;

		int i = 0;
		for (; i < pPacket->cAttachCount; ++i)
		{
			const TItemInfo& packetItem = pPacket->AttachItem[i];
			info.attachItems[i] = packetItem;
		}

		if (info.mailKind != MailType::MissionMail && info.bPremium == false && pPacket->cAttachCount > 1)
		{
			GetInterface().MessageBox(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1990 )); // UISTRING : Failed to load mail. Please reopen the mailbox.
			return;
		}

		m_pMailDialog->RefreshMailReadDialog(info);
		m_pMailDialog->OpenMailReadDialog();

		m_nUnreadMailCount = pPacket->wNotReadMailCount;

		pMailBoxInfo->bRead = MailType::Read;
		m_pMailDialog->RefreshMailBoxList(m_vecMailBoxInfoList);
	}
	else
	{
		GetInterface().ServerMessageBox(pPacket->nRet);
	}
}

#ifdef PRE_ADD_CADGE_CASH

void CDnTradeMail::OnRecvCadgeMailRead( SCReadCadgeMail* pPacket )
{
	LockMailDlg( false );

	if( pPacket->nRet == ERROR_NONE )
	{
		SMailBoxInfo *pMailBoxInfo = FindMailBoxInfo( pPacket->nMailDBID );
		if( !pMailBoxInfo ) return;

		SReadMailInfo& info = m_CurReadMailInfoCache;
		info.Clear();

		info.nMailID = pMailBoxInfo->nMailID;
		info.strSender = pMailBoxInfo->strName;

		info.bCadgeComplete = pPacket->bCadgeComplete;
		info.strTitle = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4921 );

		time_t sendDate = pPacket->tSendDate;
		DnLocalTime_s(&info.date, &sendDate);

		info.strText = pPacket->wszText;
		info.mailKind = pMailBoxInfo->mailKind;
		info.bNewFlag = pPacket->bNewFlag;

		m_pMailDialog->RefreshMailReadDialog( info );
		m_pMailDialog->OpenMailReadDialog();

		m_nUnreadMailCount = pPacket->wNotReadMailCount;

		pMailBoxInfo->bRead = MailType::Read;
		m_pMailDialog->RefreshMailBoxList( m_vecMailBoxInfoList );

		GetInterface().GetCashShopDlg()->SetReserveCadgeItemList( info.strSender, info.nMailID, pPacket->nPackageSN, pPacket->cWishListCount, pPacket->WishList );
	}
	else
	{
		GetInterface().ServerMessageBox( pPacket->nRet );
	}
}

#endif // PRE_ADD_CADGE_CASH

void CDnTradeMail::OnRecvMailDelete( SCMailResult *pPacket )
{
	LockMailDlg(false);

	if (pPacket && pPacket->nRet == ERROR_NONE)
	{
		int pageBeforeDel = GetCurrentPage();
		int reqPageNum = m_bDelMailSoReqLastPage ? pageBeforeDel - 1 : pageBeforeDel;
		if (reqPageNum < 0)
			reqPageNum = 0;
		RequestMailBox(reqPageNum);

		m_pMailDialog->ClearMailReadDialog();
		m_pMailDialog->OpenMailListDialog();

		ClearCurReadMailInfoCache();
		return;
	}

	if (m_pMailDialog)
	{
 		m_pMailDialog->RefreshMailBoxList(m_vecMailBoxInfoList);
	}
}

void CDnTradeMail::OnRecvMailAttachAll( SCAttachAllMailResult *pPacket )
{
	LockMailDlg(false);

	if (pPacket->nRet == ERROR_NONE)
	{
		int i = 0;
		for (; i < MAILPAGEMAX; ++i)
		{
			int curMailID = pPacket->nMailDBID[i];
			if (curMailID > 0)
			{
				SMailBoxInfo* info = FindMailBoxInfo(curMailID);
				if (info != NULL)
				{
					info->bItem = false;
				}

				if (GetCurReadMailID() == curMailID)
				{
					m_CurReadMailInfoCache.ClearAttachItems();
					m_pMailDialog->RefreshMailReadDialog(m_CurReadMailInfoCache);
				}
			}
		}

		if (m_pMailDialog)
			m_pMailDialog->RefreshMailBoxList(m_vecMailBoxInfoList);

		GetInterface().MessageBox(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1968 ), MB_OK); // UISTRING : Collected attached items and money.
		CEtSoundEngine::GetInstance().PlaySound("2D", m_nRecvAttachGoldSoundIdx);
	}
	else
	{
		eError err = (eError)pPacket->nRet;

		if (err == ERROR_ITEM_INVENTORY_NOTENOUGH ||
			err == ERROR_ITEM_OVERFLOWMONEY)
		{
			RequestMailBox(GetCurrentPage());
		}

		GetInterface().ServerMessageBox(err);
	}
}

void CDnTradeMail::OnRecvMailAttach(SCMailResult *pPacket)
{
	LockMailDlg(false);

	if (pPacket->nRet == ERROR_NONE)
	{
		int i = 0;
		ITEMCLSID attachItemId = ITEMCLSID_NONE;
		for (; i < (int)m_CurReadMailInfoCache.attachItems.size(); ++i)
		{
			if (m_CurReadMailInfoCache.attachItems[i].Item.nItemID > 0)
			{
				attachItemId = m_CurReadMailInfoCache.attachItems[i].Item.nItemID;
				break;
			}
		}

		if (attachItemId != ITEMCLSID_NONE && CDnItemTask::IsActive())
		{
			GetItemTask().PlayItemSound(attachItemId, CDnItemTask::ItemSound_Drag);
		}
		else
		{
			if (m_CurReadMailInfoCache.money != 0 && m_nRecvAttachGoldSoundIdx != -1)
				CEtSoundEngine::GetInstance().PlaySound("2D", m_nRecvAttachGoldSoundIdx);
		}

		if (m_RecvAttachItemSerialCache == -1)
			m_pMailDialog->ClearMailReadAttach();
		else
			m_pMailDialog->RemoveReadDialogAttachItem(m_RecvAttachItemSerialCache, m_CurReadMailInfoCache.money > 0);
	}
	else
		GetInterface().ServerMessageBox(pPacket->nRet);

	m_RecvAttachItemSerialCache = -1;
}

void CDnTradeMail::OnRecvMailNotify(SCNotifyMail* pPacket)
{
	m_nUnreadMailCount	= pPacket->wNotReadMailCount;
	m_nTotalMailCount	= pPacket->wTotalMailCount;


	if (m_nUnreadMailCount > 0 && pPacket->bNewMail) {
		CDnMainDlg *pMainBarDlg = GetInterface().GetMainBarDialog();
		if( pMainBarDlg ) 
		{
			pMainBarDlg->OnMailNotify();
#ifdef PRE_ADD_MAIL_UI_IMPROVEMENTS
			pMainBarDlg->BlinkMenuButton( CDnMainMenuDlg::MAIL_DIALOG );
#endif
		}

	}

	if (pPacket->bExpiration)
		GetInterface().AddChatMessage(CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1959 ), false); // UISTRING : You have unread mail about to expire. Please check your mailbox.
}

CDnTradeMail::SMailBoxInfo* CDnTradeMail::FindMailBoxInfo( int nMailID )
{
	int nSize = (int)m_vecMailBoxInfoList.size();
	for( int i=0; i<nSize; i++ )
	{
		if( m_vecMailBoxInfoList[i].nMailID == nMailID )
		{
			return &m_vecMailBoxInfoList[i];
		}
	}

	CDebugSet::ToLogFile( "CDnTradeMail::FindMailBoxInfo, Mail(%d) not found!", nMailID );
	return NULL;
}

void CDnTradeMail::DeleteMailBoxInfo( int nMailID )
{
	int nSize = (int)m_vecMailBoxInfoList.size();
	for( int i=0; i<nSize; i++ )
	{
		if( m_vecMailBoxInfoList[i].nMailID == nMailID )
		{
			m_vecMailBoxInfoList.erase( m_vecMailBoxInfoList.begin()+i );
			return;
		}
	}

	CDebugSet::ToLogFile( "CDnTradeMail::DeleteMailBoxInfo, Mail(%d) not found!", nMailID );
}

//void CDnTradeMail::SetCheckMail( int nMailID, bool bCheck )
//{
//	int nSize = (int)m_vecMailBoxInfoList.size();
//	for( int i=0; i<nSize; i++ )
//	{
//		if( m_vecMailBoxInfoList[i].nMailID == nMailID )
//		{
//			m_vecMailBoxInfoList[i].bChecked = bCheck;
//			break;
//		}
//	}
//}

int CDnTradeMail::GetMaxPage() const
{
	int nTotalCount = m_nTotalMailCount;
	if( nTotalCount == 0 )
		return 1;

	int nMaxPage = nTotalCount/MAILPAGEMAX;
	if (nTotalCount%MAILPAGEMAX)
		nMaxPage++;

	return nMaxPage;
}

int CDnTradeMail::GetCurrentPage() const
{
	if (m_vecMailBoxInfoList.size() > 0)
	{
		const SMailBoxInfo& firstInfo = m_vecMailBoxInfoList[0];
		return (firstInfo.nIndex / MAILPAGEMAX) + 1;
	}

	return -1;
}

int CDnTradeMail::GetMailBoxIndex(int nMailID) const
{
	int nSize = (int)m_vecMailBoxInfoList.size();
	for (int i=0; i < nSize; i++)
	{
		if (m_vecMailBoxInfoList[i].nMailID == nMailID)
			return m_vecMailBoxInfoList[i].nIndex;
	}

	return -1;
}

bool CDnTradeMail::IsOnMailMode() const
{
	return (m_pMailDialog && m_pMailDialog->IsShow());
}

//	todo : extract to interface or view layer. Avoid controlling dlg directly in task.
void CDnTradeMail::LockMailDlg(bool bLock)
{
	if (IsOnMailMode())
		m_pMailDialog->LockDlgs(bLock);
}

int	CDnTradeMail::GetCurReadMailID() const
{
	return m_CurReadMailInfoCache.nMailID;
}

void CDnTradeMail::ClearCurReadMailInfoCache()
{
	m_CurReadMailInfoCache.Clear();
}

void CDnTradeMail::SetDaySendEnableCount(int count)
{
	//if (count < 0 || count > MAILDAYSENDCOUNTMAX)
	//{
	//	_ASSERT(0);
	//	m_nDaySendEnableCount = 0;
	//	return;
	//}

	m_nDaySendEnableCount = count;
}

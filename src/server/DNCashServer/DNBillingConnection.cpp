#include "stdafx.h"
#include "DNBillingConnection.h"
#include "DNUserRepository.h"
#include "DNSQLMembership.h"

CDNBillingConnection *g_pBillingConnection = NULL;

CDNBillingConnection::CDNBillingConnection(void)
{
}

CDNBillingConnection::~CDNBillingConnection(void)
{
}

int CDNBillingConnection::OnBalanceInquiry(CDNUserRepository *pUser)
{
	if (!pUser)
		return ERROR_GENERIC_INVALIDREQUEST;

	CDNSQLMembership *pMembershipDB = pUser->GetMembershipDB();
	if (!pMembershipDB)
	{
		pUser->SendBalanceInquiry(ERROR_GENERIC_INVALIDREQUEST, 0);
		return ERROR_GENERIC_INVALIDREQUEST;
	}

	int nCashBalance = 0;
	int nRet = pMembershipDB->QueryGetCashBalance(pUser->GetAccountDBID(), NULL, nCashBalance);
	if (nRet != ERROR_NONE)
	{
		pUser->SendBalanceInquiry(nRet, 0);
		return nRet;
	}

	pUser->SendBalanceInquiry(ERROR_NONE, nCashBalance);
	return ERROR_NONE;
}

int CDNBillingConnection::OnBuy(CDNUserRepository *pUser)
{
	pUser->OnRecvBillingBuyItem(ERROR_NONE, 0, true);
	return ERROR_NONE;
}

int CDNBillingConnection::OnPackageBuy(CDNUserRepository *pUser)
{
	pUser->OnRecvBillingBuyItem(ERROR_NONE, 0, false);
	return ERROR_NONE;
}

int CDNBillingConnection::OnGift(CDNUserRepository *pUser)
{
	pUser->OnRecvBillingBuyItem(ERROR_NONE, 0, true);
	return ERROR_NONE;
}

int CDNBillingConnection::OnPackageGift(CDNUserRepository *pUser)
{
	pUser->OnRecvBillingBuyItem(ERROR_NONE, 0, false);
	return ERROR_NONE;
}

int CDNBillingConnection::OnVIPBuy(CDNUserRepository *pUser)
{
	pUser->OnRecvBillingBuyItem(ERROR_NONE, 0, false);
	return ERROR_NONE;
}

int CDNBillingConnection::OnCoupon(CDNUserRepository *pUser)
{
	return ERROR_GENERIC_INVALIDREQUEST;
}

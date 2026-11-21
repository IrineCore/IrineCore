#pragma once

class CDNUserRepository;
class CDNBillingConnection
{
public:
	CDNBillingConnection(void);
	virtual ~CDNBillingConnection(void);

	int OnBalanceInquiry(CDNUserRepository *pUser);
	int OnBuy(CDNUserRepository *pUser);
	int OnPackageBuy(CDNUserRepository *pUser);
	int OnGift(CDNUserRepository *pUser);
	int OnPackageGift(CDNUserRepository *pUser);
	int OnVIPBuy(CDNUserRepository *pUser);
	int OnCoupon(CDNUserRepository *pUser);
};

extern CDNBillingConnection *g_pBillingConnection;

#pragma once
#include "DNPacket.h"

#if defined(PRE_SPECIALBOX) 
// Rotha - #63016 Special Storage

class CDnSpecialBox
{

protected:

public:
	CDnSpecialBox(void);
	virtual ~CDnSpecialBox(void);

public:
	void TestTabListFunc();
	void TestRewardListFunc();

	// Receive Func
	void OnRecvSpecialBoxList( SCSpecialBoxList *pPacket );
	void OnRecvSpecialBoxItemList( SCSpecialBoxItemList *pPacket );
	void OnRecvSpecialBoxReceiveItem( SCReceiveSpecialBoxItem *pPacket );
	void OnRecvSpecialBoxNotify( SCNotifySpecialBox *pPacket );
};

#endif
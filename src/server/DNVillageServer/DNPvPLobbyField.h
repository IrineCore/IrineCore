
#pragma once

#include "DNField.h"

// PvP lobby
class CDNPvPLobbyField:public CDNField
{
public:

	CDNPvPLobbyField();
	virtual ~CDNPvPLobbyField();

	virtual bool bIsPvPVillage(){ return false; }
	virtual bool bIsPvPLobby(){ return true; }
	virtual int SendFieldMessage(CDNBaseObject *pSender, USHORT wMsg, TBaseData *pSenderData, TParamData *pParamData);

private:

};

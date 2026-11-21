
#pragma once

#include "DNField.h"


//Farm village
class CDNFarmField : public CDNField
{
public:
	CDNFarmField();
	~CDNFarmField();

	virtual bool bIsPvPVillage(){ return false; }
	virtual bool bIsPvPLobby(){ return false; }
	virtual bool bIsDarkLairVillage(){ return false; }
	virtual bool bIsFarmVillage() { return true; }
	virtual GameTaskType::eType GetGameTaskType(){ return GameTaskType::Farm; }

private:

};

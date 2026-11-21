
#pragma once

#include "DNField.h"

// Dark Lair village
class CDNDarkLairField : public CDNField
{
public:

	CDNDarkLairField();
	virtual ~CDNDarkLairField();

	virtual bool bIsPvPVillage(){ return false; }
	virtual bool bIsPvPLobby(){ return false; }
	virtual bool bIsDarkLairVillage(){ return true; }
	virtual GameTaskType::eType GetGameTaskType(){ return GameTaskType::DarkLair; }

private:

};

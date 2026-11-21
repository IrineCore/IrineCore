#pragma once

#include <vector>

class CDNUserSession;
class CDNGameRoom;

namespace AIBot
{
	enum class RoleType
	{
		Tank = 0,
		Healer,
		IceStacker,
		Support,
		Dps,
	};

	struct BotJobSpec
	{
		BYTE cSecondJob;
		BYTE cFirstJob;
		BYTE cBaseClass;
		RoleType Role;
		char cMainWeaponType;
		char cSubWeaponType;
		const WCHAR* pDefaultName;
	};

	RoleType GetJobRole(BYTE cJobID);
	const BotJobSpec* FindJobSpec(BYTE cJobID);

	void SelectSquadSpecs(BYTE cHumanJobID, int nBotCount, std::vector<const BotJobSpec*>& outSpecs);
	bool PopulateBotSession(CDNUserSession* pBotSession, const BotJobSpec* pSpec, int nBotSlotIndex, int nTargetLevel = 60);
}

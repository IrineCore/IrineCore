
#pragma once

namespace AICommand
{
	enum Command
	{
		CMD_USESKILL = 0,				// Use skill
		CMD_REMOVEBLOW_BYSKILLINDEX,	// Remove BLOWs caused by this skill
		CMD_MAX,
	};

	typedef struct tagUseSkill
	{
		int iSkillIndex;
	}SUseSkill;

	typedef SUseSkill	SRemoveBlowBySkillIndex;
};

class MAAiScript;

class MAAiCommand:public TBoostMemoryPool<MAAiCommand>
{
public:

	void MsgProc( MAAiScript* pAIScript, const UINT uiMsg, void* pParam );
};

#include "stdafx.h"
#include "DnCheckKeyProcessor.h"
#include "DnPlayerActor.h"
#include "IDnSkillUsableChecker.h"
#include "DnSkill.h"

#if !defined(_GAMESERVER)
#include "DnPetActor.h"
#endif	// #if !defined(_GAMESERVER)

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnCheckKeyProcess::CDnCheckKeyProcess(DnActorHandle hActor, const char* pActionName) : IDnSkillProcessor(hActor),
m_strActionName(pActionName)
{
	m_iType = CHECK_KEY;

	TokenizeA(pActionName, strTokens, ";");
	OutputDebug("%s -> %s", __FUNCTION__, pActionName);
}

CDnCheckKeyProcess::~CDnCheckKeyProcess(void)
{
}

void CDnCheckKeyProcess::CopyFrom(IDnSkillProcessor* pProcessor)
{
	if (NULL == pProcessor)
		return;

	if (GetType() != pProcessor->GetType())
		return;

	CDnCheckKeyProcess* pSource = static_cast<CDnCheckKeyProcess*>(pProcessor);
	m_strActionName = pSource->m_strActionName;
}


void CDnCheckKeyProcess::OnBegin(LOCAL_TIME LocalTime, float fDelta, DnSkillHandle hParentSkill)
{
	if (m_hHasActor)
	{
#if !defined(_GAMESERVER)
		CDnPlayerActor * pActor = (CDnPlayerActor *)(m_hHasActor.GetPointer());
		if (!pActor )
		{
			OutputDebug("CDnCheckKeyProcess::OnBegin() -> No Actor Found.");
			return;
		}
		//Skill_SwiftShot_EX_Back
		//if (strTokens.size()>0)
		//{
		std::string sNewSkill = FormatA("%s_Back", pActor->GetCurrentAction());// , strTokens[0]);
			if(pActor->IsExistAction(sNewSkill.c_str()))
				pActor->SetActionQueue(sNewSkill.c_str());

	//		OutputDebug("%s -> %s   -> %s\n", __FUNCTION__, pActor->GetCurrentAction(), sNewSkill);
		//}
#endif	// #if !defined(_GAMESERVER)


	}
}


void CDnCheckKeyProcess::Process(LOCAL_TIME LocalTime, float fDelta)
{
	m_fTimeLength -= fDelta;
	if (m_fTimeLength <= 0.0f)
		m_fTimeLength = 0.0f;
}


bool CDnCheckKeyProcess::IsFinished(void)
{
	return true;
}


void CDnCheckKeyProcess::OnEnd(LOCAL_TIME LocalTime, float fDelta)
{

}
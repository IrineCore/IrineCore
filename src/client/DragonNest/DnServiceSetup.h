#pragma once

#include "MessageListener.h"

class IServiceSetup : public MessageListener
{
public:
	//Common defines
	enum eServiceInfo{
		GameStart = 1,				//Start game main function 
		Server_Certification = 2,	//Passed server authentication
		InVillage = 3,				//Arrived at village (including Colosseum)
		Ingame = 4,					//Arrived in game (including PvP)
		Game_Exit = 5,				//Normal termination 
		JoinChannel = 6,
		ExitChannel = 7,
		SecurityModule_Failed = 99 , 		//Security module failed
	};

	IServiceSetup() { }
	virtual ~IServiceSetup() { }

	virtual int	PreInitialize(void* pCustomData);
	virtual int	Initialize(void* pCustomData) { return  0; }
	virtual void* GetCustomData() { return NULL; }

	virtual LRESULT MsgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) { return 0; }
	virtual int OnEvent(int nType, void* pExtra) { return 0; }
	virtual void OnDispatchMessage(int iMainCmd, int iSubCmd, char * pData, int iLen) { }
	virtual int Release() { return  0; }

	virtual bool WriteStageLog_( INT32 nStage , LPCTSTR szComment ){return false;};
	virtual bool WriteErrorLog_( INT32 nStage , LPCTSTR szComment ){return false;};
};

// Create service after creating the window. 
class DnServiceSetup
{
public:
	// Handle release on your own. 
	static IServiceSetup* Create();
};

extern IServiceSetup*	g_pServiceSetup;
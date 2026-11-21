#pragma once

#include "Timer.h"
#include "Singleton.h"

class CInputDevice;
class CTaskManager;
class CDnTableDB;
class CTask;
class CEtSoundEngine;
class CClientSessionManager;
class CDnActionBase;
class CDnWorldImp;
class CEtResourceMng;

class CDnMainFrame : public CSingleton<CDnMainFrame> {
public:
	CDnMainFrame();
	~CDnMainFrame();

	// Things to do before creating Window, such as hacking tool checks, etc.
	bool PreInitialize();

	// Create Engine
	bool InitializeDevice();
	bool FinalizeDevice();

	bool PreloadResource();

	// Create game
	bool Initialize();
	bool Finalize();

	//SetMultiCommmon
	void InitializeMultiCommon(int iCnt, std::vector <unsigned int> * vList);			//Set thread count and store IDs in the container
	void SetMultiThreadID(unsigned int iID);														//

	// Kernel main loop

protected:
	CDnTableDB *m_pTableDB;
	CEtResourceMng *m_pResMng;
	CEtUIXML *m_pUIXML;
#ifdef PRE_ADD_EXCEPTIONAL_MAKESTRING
	CEtExceptionalUIXML* m_pExceptionalUIXML;
#endif // PRE_ADD_EXCEPTIONAL_MAKESTRING

	std::map<int, std::string> m_mapCodePageString;

	std::vector<EtResourceHandle> m_hVecEngineResList;
	std::vector<CDnActionBase *> m_pVecActionList;
	std::vector<CDnWorldImp *> m_pVecWorldList;

protected:
	void InitializeLocalize();
	void PreloadSkinFile( int nTableID, char *szLable );
	void PreloadAniFile( int nTableID, char *szLable );
	void PreloadActFile( int nTableID, char *szLable );
	void FinalizePreloadFile();

	void PreloadSkinFile( const char *szFileName );
	void PreloadAniFile( const char *szFileName );
	void PreloadActFile( const char *szFileName );

protected:
};
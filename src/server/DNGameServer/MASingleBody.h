#pragma once

#include "Timer.h"
#include "MAActorRenderBase.h"

class MASingleBody : virtual public MAActorRenderBase {
public:
	MASingleBody();
	virtual ~MASingleBody();

protected:

	std::string m_szSkinFileName;

public:
	virtual void LoadSkin( const char *szSkinName, const char *szAniName );
	//bool LoadSkin( const char *szSkinName, const char *szAniName );	
	virtual bool bIsSingleBody(){ return true; }

	// DnRenderBase 
};

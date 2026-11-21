#pragma once
#include "dncutsceneworldprop.h"

class CDnCutSceneLightProp : public CDnCutSceneWorldProp
{
private:

	EtLightHandle		m_LightHandle;

public:
	CDnCutSceneLightProp(void);
	virtual ~CDnCutSceneLightProp(void);

	bool Initialize( CEtWorldSector *pParentSector, const char *szPropName, EtVector3 &vPos, EtVector3 &vRotate, EtVector3 &vScale );
};

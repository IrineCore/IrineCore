#pragma once

// Just a class that draws for debugging..
class CDnDebugRender : public CEtCustomRender
{
public:
	CDnDebugRender();
	virtual ~CDnDebugRender();

	virtual void RenderCustom( float fElapsedTime );

	EtTextureHandle m_hTexture;
};

// Constructor must be called later than CEtCustomRender static variables, so create/delete with new.
extern CDnDebugRender* g_pDnDebugRender;
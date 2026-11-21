#pragma once

#include "EtUIControl.h"

// This class is no longer used after implementing video playback with CustomControl.
class CEtUIMovieControl : public CEtUIControl
{
public:
	CEtUIMovieControl( CEtUIDialog *pParent );
	virtual ~CEtUIMovieControl(void);

protected:

	// Leaving the texture handle as is since we'll fetch the texture when playing video.
	EtTextureHandle m_hTexture;

	SUICoord m_TextureCoord;
	SUICoord m_TextureUV;
	SUICoord m_ControlCoord;

public:
	//EtTextureHandle GetTextrue() { return m_hTexture; }
	//void SetTexture( EtTextureHandle hTextureHandle, int nX, int nY, int nWidth, int nHeight );
	//void DeleteTexture();

public:
	virtual void Initialize( SUIControlProperty *pProperty );
	virtual void Render( float fElapsedTime );
	virtual void UpdateRects();
	virtual void Process( float fElapsedTime );
};
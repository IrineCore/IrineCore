#pragma once

#include "EtUIControl.h"

// Originally designed so the program passes the texture handle directly (using SetTexture member function)
// Internally, it used the texture as received without directly deleting or modifying it.
// So without setting textures directly in the tool, it was used by just creating the frame.
//
// As a result, there were too many Static textures, so when trying to store the filename in the texture property and use it,
// In this case, the texture handle had to be managed directly.
//
// Create and use a bool variable to distinguish from the above case.
class CEtUITextureControl : public CEtUIControl
{
public:
	CEtUITextureControl( CEtUIDialog *pParent );
	virtual ~CEtUITextureControl(void);

protected:
	EtTextureHandle m_hTexture;

	/*SUICoord	m_TextureCoord;*/
	SUICoord	m_TextureUV;
	SUICoord	m_ControlCoord;
	
	SUICoord	m_NewUICoord;

	float		m_Scale;
	bool		m_bManualControlColor;

	float		m_fAddX;
	float		m_fAddY;

	// Created because a TextureFile name existed in the Property passed to Initialize.
	// In this case, run the code that deletes the existing texture when a new texture setting arrives.
	bool m_bCreatedFromPropertyTextureFile;

public:
	EtTextureHandle GetTextrue()	{ return m_hTexture; }
	void	SetTexture( EtTextureHandle hTextureHandle );
	void	SetTexture( EtTextureHandle hTextureHandle, int nX, int nY, int nWidth, int nHeight );
	void	SetFlipHTexture( EtTextureHandle hTextureHandle, int nX, int nY, int nWidth, int nHeight, bool bFlipH = false);
	void	SetScale(float scale)		{ m_Scale = scale; }
	void	SetTexturePosition(float fX, float fY);
	void	SetManualControlColor(bool bManual)		{ m_bManualControlColor = bManual; }
	void	DeleteTexture();
	DWORD	GetTextureColor();
	float	GetScale() const						{ return m_Scale; }
	void	SetTextureColor(DWORD dwColor);
	void GetUVCoord( SUICoord &Coord ) {Coord = m_TextureUV;}

public:
	virtual void Initialize( SUIControlProperty *pProperty );
	virtual void Render( float fElapsedTime );
	virtual void UpdateRects();
	virtual void Process( float fElapsedTime );
	virtual void Show( bool bShow );

	virtual void GetUICoord( SUICoord &Coord ) { Coord = m_NewUICoord; }
};
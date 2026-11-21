#pragma once

// Define interface for rendered objects
class IDnCutSceneRenderObject
{
private:
	

public:
	IDnCutSceneRenderObject(void);
	virtual ~IDnCutSceneRenderObject(void);

	virtual int GetAniIndex( const char* pAniName ) = 0;
	virtual void ChangeAnimation( int iAniIndex, float fFrame, float fBlendFrame = 0.0f )  {}; 
};

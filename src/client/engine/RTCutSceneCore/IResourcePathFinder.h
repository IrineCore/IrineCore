#pragma once


// Resource path manager wrapper interface
// Resolved this way because RTCutSceneCore cannot have a path manager or similar..
class IResourcePathFinder
{
public:
	IResourcePathFinder(void);
	virtual ~IResourcePathFinder(void);

	virtual void GetFullPath( const char* pFileName, /*IN OUT*/string& strFullPath ) { };
};

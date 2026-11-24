#pragma once

#include "EtQuadtree.h"

class CFileStream;
class CEtWorldEventControl;

class CEtWorldEventArea
{
public:
	CEtWorldEventArea( CEtWorldEventControl *pControl );
	virtual ~CEtWorldEventArea();

	static int s_nDummySize;
protected:
	std::string m_szName;
	EtVector3 m_vMin;
	EtVector3 m_vMax;
	bool m_bActive;
	void *m_pData;
	float m_fRotate;
	SOBB m_ObbBox;
	int m_nCreateUniqueID;
	CEtWorldEventControl *m_pControl;

	std::vector<int> m_nVec2Index;
	std::vector<int> m_nVec3Index;
	std::vector<int> m_nVec4Index;
	std::vector<int> m_nVecStrIndex;
#if defined(WIN64) || defined(__x86_64__) || defined(_M_X64) || defined(__LP64__)
	std::vector<int> m_nVec2Index64;
	std::vector<int> m_nVec3Index64;
	std::vector<int> m_nVec4Index64;
	std::vector<int> m_nVecStrIndex64;
#endif

	CEtQuadtreeNode<CEtWorldEventArea *> *m_pCurrentNode;
	SCircle m_Circle;

protected:
#if defined(WIN64) || defined(__x86_64__) || defined(_M_X64) || defined(__LP64__)
	char IsPointerTable( int nIndex );
	void IncreasePointerTableIndex( int nValue );
#endif

public:
	virtual bool Load( CStream *pStream );
	void SetName( const char *szName ) { m_szName = szName; }
	void SetMin( EtVector3 &vPos ) { m_vMin = vPos; }
	void SetMax( EtVector3 &vPos ) { m_vMax = vPos; }

	void *GetData() { return m_pData; }

	const char *GetName() { return m_szName.c_str(); }
	EtVector3 *GetMin() { return &m_vMin; }
	EtVector3 *GetMax() { return &m_vMax; }

	float GetRotate() { return m_fRotate; }
	void SetRotate( float fValue ) { m_fRotate = fValue; }

	void SetActive( bool bValue ) { m_bActive = bValue; }
	bool IsActive() { return m_bActive; }

	void CalcOBB();
	virtual bool CheckArea( EtVector3 &vPosition, bool bIgnoreY = false );
	SOBB *GetOBB() { return &m_ObbBox; }

	int GetCreateUniqueID() { return m_nCreateUniqueID; }
	void SetCreateUniqueID( int nValue ) { m_nCreateUniqueID = nValue; }
	CEtWorldEventControl *GetControl() { return m_pControl; }

	void SetCurQuadtreeNode( CEtQuadtreeNode<CEtWorldEventArea *> *pNode ) { m_pCurrentNode = pNode; }
	CEtQuadtreeNode<CEtWorldEventArea *> *GetCurQuadtreeNode() { return m_pCurrentNode; }

	void GetBoundingCircle( SCircle &Circle );

};
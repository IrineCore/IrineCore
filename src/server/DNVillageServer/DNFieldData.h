#pragma once

#include "EtWorld.h"

class CDNField;
class CDNFieldData
{
private:
	string m_szMapName;
	int m_nMapIndex;
	CEtWorld m_World;

	struct AreaInfo {
		int nIndex;
		TPosition Min;
		TPosition Max;
		float fAngle;
		int nType;
	};
	std::vector<AreaInfo> m_VecGateList;
	std::vector<AreaInfo> m_VecStartPositionList;
	bool m_bHasCollisionHeight;

protected:
	void CalcGatePosition();

public:
	CDNFieldData(void);
	virtual ~CDNFieldData(void);

	bool Init(char *pszMapName, int nMapIndex);
	void GetWidthHeight(UINT &nWidth, UINT &nHeight);

	int CheckGateArea(TPosition &Position);
	bool GetStartPosition(char cGateNo, TPosition &Pos);
	void GetStartPositionAngle( char cGateNo, float &fAngle );

	void GetRandomStartPosition(TPosition &Pos);	// Temporarily extract coordinates from a random gate
	char GetRandomGateNo();

	void LoadCreateNpc(CDNField *pField);

	float GetCollisionHeight(float fX, float fZ);
	float GetTerrainHeight(float fX, float fZ);
	inline bool HasCollisionHeight() const { return m_bHasCollisionHeight; }
	DWORD GetCurrentTileSize();

	int GetGateType(TPosition &Position);
	bool GetGateAreaBounds(int nGateIndex, float& outMinX, float& outMinZ, float& outMaxX, float& outMaxZ, float& outAngle) const;
	bool GetStartPositionCenter(int nGateIndex, float& outX, float& outY, float& outZ, float& outAngle);
	bool IsInsideAnyGateTrigger(float fX, float fZ, float fExtraMargin) const;

	inline int GetGridX() { return m_World.GetGridX(); }
	inline int GetGridY() { return m_World.GetGridY(); }
	inline int GetMapIndex() { return m_nMapIndex; }
};

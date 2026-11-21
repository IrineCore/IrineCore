#pragma once

const int MAX_WEIGHT = static_cast<unsigned int>(-1)/2;

struct PathInfo
{
	int nGateIndex;
	int nMapIndex;
};

typedef std::deque<PathInfo>	PathResult;


class CDnQuestPathFinder
{
public:
	CDnQuestPathFinder();
	virtual ~CDnQuestPathFinder();

	bool Init();
	bool GetPathOneWay( int nStartMapIndex, int nEndMapIndex, OUT PathResult& path_result, bool bInverseResult = false );
	bool GetPath( int nStartMapIndex, int nEndMapIndex, OUT PathResult& path_result, bool bBothWay = false );
	bool GetPathDijkstra( int nStartMapIndex, int nEndMapIndex, OUT PathResult& path_result);
	// Are the two maps linked via a gate? Find which gate number can be used to exit. -1 if impossible
	bool IsLinkedMap(int nCurrentMapIndex, int nTargeMapIndex, OUT int& nLinkedGate);

// #77323, #77324 Dragon Expedition quest notifier error.
	void GetGatePath( int nStartMapIndex, int nEndMapIndex, bool bInverseResult, OUT std::vector< int > & beforeMap, OUT PathResult & path_result );
//

private:
	// Get actual map index from adjacency matrix array index.
	inline int GetMapIndex(int nArrayIndex) { return m_AdjMatrixMapIndex[nArrayIndex]; }

private:

	struct GateInfo 
	{
		int nGateIndex;				// Gate index, gates 1~8
		int nLinkedMapIndex;		// Which map ID this gate leads to.
		//int nLinkedStartIndex; // Which start position exiting through this gate leads to.
	};

	struct MapNode 
	{
		MapNode()
		{
			nMapIndex = nAdjMatrixIndex = 0;
		}
		int	nMapIndex;
		int nAdjMatrixIndex;
		std::vector<GateInfo> gates;

		inline bool IsLinked(int nMapIndex, OUT int& nLinkedGate)
		{
			nLinkedGate = -1;
			for ( size_t i = 0 ; i < gates.size() ; i++ )
			{
				if ( gates[i].nLinkedMapIndex == nMapIndex )
				{
					nLinkedGate = gates[i].nGateIndex ;
					return true;
				}
			}

			return false;
		}
	};

	// Weighted adjacency matrix
	std::vector< std::vector<int> > m_AdjMatrixWeight;
	// Actual game map index paired with the weighted adjacency matrix
	std::vector<int>				m_AdjMatrixMapIndex;	
	// Map containing each gate info 
	// key - mapindex
	std::map<int, MapNode>			m_MapNode;
};
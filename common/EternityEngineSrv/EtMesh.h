#pragma once
#include "EtResource.h"
//#include "EtCustomMeshStream.h"
#include "EtPrimitive.h"
#include "EtCollisionPrimitive.h"

#define MESH_FILE_STRING	"Eternity Engine Mesh File 0.12"
#define MESH_FILE_VERSION	12
#define MESH_HEADER_RESERVED	( 1024 - sizeof( SMeshFileHeader ) )

struct SMeshFileHeader
{
	char szHeaderString[ 256 ];
	int nVersion;
	int nSubMeshCount;
	int nLODCount;
	bool bUVAni;
	char cSpace[ 3 ];
	EtVector3 MaxVec;
	EtVector3 MinVec;
	int nBoneCount;
	int nCollisionPrimitiveCount;
	int nDummyCount;
};

class CEtSubMesh;
struct SAABox;
class CEtMesh;
typedef CSmartPtr< CEtMesh > EtMeshHandle;

class CEtMesh : public CEtResource, public TBoostMemoryPool< CEtMesh >
{
public:
	CEtMesh( CMultiRoom *pRoom );
	virtual ~CEtMesh();
	void Clear();

protected:
	SMeshFileHeader m_MeshHeader;
	SSphere m_BoundingSphere;

	std::vector< std::string > m_vecBoneName;
	std::vector< EtMatrix > m_vecInvWorldMat;
	std::vector< SCollisionPrimitive * > m_vecCollisionPrimitive;
	std::vector< std::string > m_vecCollisionPrimitiveParentName;
	std::vector< int > m_vecCollisionPrimitiveParentIndex;

public:

	std::vector< EtMatrix > &GetInvWorldMatList() { return m_vecInvWorldMat; }

	bool HasCollisionBone();
	void LinkToAni( EtResourceHandle hAni );

	int LoadResource( CStream *pStream );

	void GetExtent( EtVector3 &Origin, EtVector3 &Extent );
	void GetBoundingBox( SAABox &Box );
	void GetBoundingSphere( SSphere &Sphere, bool bActorSize = false ){ Sphere = m_BoundingSphere; }

	int GetCollisionPrimitiveCount() { return ( int )m_vecCollisionPrimitive.size(); }

	std::vector< SCollisionPrimitive * > *GetCollisionPrimitive() { return &m_vecCollisionPrimitive; }
	std::vector< int > *GetCollisionPrimitiveParentIndex() { return &m_vecCollisionPrimitiveParentIndex; }
};

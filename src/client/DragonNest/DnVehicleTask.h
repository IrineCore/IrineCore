#pragma once

#include "Task.h"
#include "MessageListener.h"
#include "DNProtocol.h"
#include "DNPacket.h"


// Rotha 

// Mount packets are mostly handled in ItemTask because mounts are closely tied to items. 
// However, mount info differs from how the existing ItemTask works, so putting all mount exceptions into ItemTask would get too complicated
// Managed separately in the mount's Task; if additional messages other than items arise, handle them here.
// Currently configured so that Vehicle_Task holds the mount list.


// Rotha: Add vehicle system TASK

class CDnVehicleTask : public CTask, public CTaskListener, public CSingleton<CDnVehicleTask>
{
public:
	typedef std::vector<TVehicleCompact>		    VEHICLE_LIST_VECTOR;
	typedef VEHICLE_LIST_VECTOR::iterator		    VEHICLE_LIST_VECTOR_ITER;
	typedef VEHICLE_LIST_VECTOR::const_iterator	    VEHICLE_LIST_VECTOR_ITER_CONST;

	CDnVehicleTask();
	~CDnVehicleTask();

	bool Initialize();
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnDispatchMessage( int nMainCmd, int nSubCmd, char *pData, int nSize ); // Message handling when sent only to mount: none yet 

	void OnConnectTcp() {}
	void OnDisconnectTcp(bool bValidDisconnect) {}

protected:
	void OnRecvVehicleItemMessage( int nSubCmd, char *pData, int nSize ); 

	void OnRecvVehicleEquipListMessage(SCVehicleEquipList *pData); // Receive and store mount information separately.
	void OnRecvVehicleInvenListMessage(SCVehicleInvenList *pData);
	void OnRecvChangeVehiclePartsMessage(SCChangeVehicleParts *pData);
	void OnRecvChangeVehicleColorMessagse(SCChangeVehicleColor *pData);
	void OnRecvRefreshVehicleInvenMessage(SCRefreshVehicleInven *pData);
	void OnRecvPetEquipListMessage( SCVehicleEquipList* pData );
	void ClearVehicleInfo();

public:
	void RemoveInstantVehicleItem();
	void SetRide(DnActorHandle hActor,int nVehicleID);
	void SetUnRide(DnActorHandle hActor);
	
	bool IsExistVehicle(INT64 nItemSerial);
	int GetVehicleIndexFromSerial(INT64 nSerialID);
	TVehicleCompact* GetVehicleInfoFromSerial( INT64 nSerialID );
	VEHICLE_LIST_VECTOR GetVehicleList(){return m_VehicleInfoList;}
	
protected:
	VEHICLE_LIST_VECTOR m_VehicleInfoList; // Holds the list of mounts. <index/mount info>
	INT64 m_nCurrentVehicleSerial;         // Remember the serial of the mount currently being ridden.
	UINT m_nCurrentVehicleID;
};

#define GetVehicleTask()		CDnVehicleTask::GetInstance()

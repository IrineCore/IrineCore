#pragma once

/********************************************************************
	created:	2011/03/23
	filename: 	DnStateBlowSignalProcessor.h
	author:		semozz
	
purpose: Separates OnSignal handling from the state effect Process function to be handled here
*********************************************************************/
class CDnStateBlowSignalProcessor
{
public:
	CDnStateBlowSignalProcessor();
	~CDnStateBlowSignalProcessor();

	// Add new Blow to the registration list
	void AddBlow(DnBlowHandle hBlow);
	// Add Blow to be removed to the removal list
	void RemoveBlow(DnBlowHandle hBlow);
	
	// Add items to add, iterate the main list, and remove items to remove
	void Process(LOCAL_TIME localTime, float fDelta);

protected:
	// Move Blow from the pending list to the main list
	void AddListProcess();
	// Find Blow from the removal list in the main list and delete it
	void RemoveListProcess();

	// Iterate through the main list and call Blow's SignalProcess
	void OnSignal(LOCAL_TIME localTime, float fDelta);

private:
	typedef std::map<DWORD, DnBlowHandle> STATE_BLOW_LIST;
	
	STATE_BLOW_LIST m_AddList;
	STATE_BLOW_LIST m_MainList;
	STATE_BLOW_LIST m_DeleteList;

};
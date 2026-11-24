#pragma once

class AcquireOwnerShip
{
public:
	AcquireOwnerShip() {}
	virtual ~AcquireOwnerShip() {}

public:
	virtual void ReleaseOwnerShip() = 0;
};

class GiveOwnerShip
{
private:
	std::vector< AcquireOwnerShip* > m_OwnerShipList;

public:
	GiveOwnerShip() {}

	virtual ~GiveOwnerShip() 
	{
		for (AcquireOwnerShip *pElement : m_OwnerShipList) {
			pElement->ReleaseOwnerShip();
		}
	}

	void AddOwnerShip( AcquireOwnerShip *pOwnerShip ) 
	{
		m_OwnerShipList.push_back( pOwnerShip); 
	}
	 
	void RemoveOwnerShip( AcquireOwnerShip *pOwnerShip )
	{
		m_OwnerShipList.erase( std::remove( m_OwnerShipList.begin(), m_OwnerShipList.end(), pOwnerShip ), m_OwnerShipList.end() );
	}
};

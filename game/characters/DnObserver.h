#pragma once

class IDnObserverNotifyEvent;


//------------------------------------------------------------------------------
/**
Receive and handle event notification
*/
//------------------------------------------------------------------------------
class CDnObserver
{
private:
	

public:
	virtual void OnEvent( boost::shared_ptr<IDnObserverNotifyEvent> pEvent ) = 0;

};

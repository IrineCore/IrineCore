#pragma once


// Note: Hangi - Interface declaration for messages sent to Entity in a specific state
class IStateMessage
{
public:
	enum
	{
		SCAN_ACTOR,
		MESSAGE_COUNT,
	};

protected:
	int m_iType;

public:
	IStateMessage(void);
	virtual ~IStateMessage(void);
};

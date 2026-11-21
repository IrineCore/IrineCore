#pragma once


// Object that detects rapid repeated key input.
class CDnComboCalculator
{
public:
	enum
	{
		LINEAR_CHECK,
		CIRCULAR_CHECK,
	};

private:
	int m_iCheckMethod;
	vector<BYTE> m_vlKeysToCheck;
	vector<BYTE> m_vlPadsToCheck;
	
	int m_iLastKeyIndex;
	int m_iLastPadIndex;
	float m_fPadRatio;
	float m_fPadAccum;

private:
	CDnComboCalculator( void ) {};

public:
	CDnComboCalculator( int iCheckMethod );
	~CDnComboCalculator(void);

	void SetPadsToCheck( std::vector<BYTE>& vlPadsToCheck, float DurationTime );
	void SetKeysToCheck( std::vector<BYTE>& vlKeysToCheck )		{ m_vlKeysToCheck = vlKeysToCheck; }

	// Return true if the configured condition matches. Also returns the combo count.
	bool Process( LOCAL_TIME LocalTime, float fDelta, /*OUT*/ int* piComboCount );
};

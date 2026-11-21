#include "StdAfx.h"
#include "DnComboCalculator.h"
#include "InputWrapper.h"
#include "DnLocalPlayerActor.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


CDnComboCalculator::CDnComboCalculator( int iCheckMethod ) : m_iCheckMethod( iCheckMethod ),
															 m_iLastKeyIndex( -1 ),
															 m_iLastPadIndex( -1 ),
															 m_fPadRatio( 0.0f ),
															 m_fPadAccum( 0.0f )
{
}

CDnComboCalculator::~CDnComboCalculator(void)
{

}


void CDnComboCalculator::SetPadsToCheck( std::vector<BYTE>& vlPadsToCheck, float DurationTime )
{ 
	m_vlPadsToCheck = vlPadsToCheck; 
	
	m_iLastPadIndex = -1; 
	
	m_fPadAccum = .0f;
	m_fPadRatio =  DurationTime / (float)vlPadsToCheck.size();
}


bool CDnComboCalculator::Process( LOCAL_TIME LocalTime, float fDelta, int* piComboCount )
{
	bool bComboSatisfy = false;
	*piComboCount = 0;

	//CInputJoyPad* pJoypad = static_cast<CInputJoyPad*>(CInputDevice::GetInstancePtr()->GetDeviceObject( CInputDevice::JOYPAD ));

	CInputKeyboard* pKeyboard = static_cast<CInputKeyboard*>(CInputDevice::GetInstancePtr()->GetDeviceObject( CInputDevice::KEYBOARD ));
	vector<BYTE>* pvlNowKeyEvents = pKeyboard->GetEventKeyList();
	// Can check via IsPushKey()..

#ifndef _FINAL_BUILD
	if( false == pvlNowKeyEvents->empty() )
	{
		OutputDebug( "[Freezing - KeyEventList(%d)] ", (int)pvlNowKeyEvents->size() );
		for( int i = 0; i < (int)pvlNowKeyEvents->size(); ++i )
		{
			if( pKeyboard->IsPushKey(pvlNowKeyEvents->at(i)) )
				OutputDebug( "%d ", pvlNowKeyEvents->at(i) );
		}
		OutputDebug( "\n" );
	}
#endif

	switch( m_iCheckMethod )
	{
		case CIRCULAR_CHECK:
			{
				// Joypad handling
				if( CDnActor::s_hLocalActor ) 
				{
					if( -1 == m_iLastPadIndex )
						m_iLastPadIndex = 0;

					CDnLocalPlayerActor * localActor = ((CDnLocalPlayerActor *)(CDnActor::s_hLocalActor.GetPointer()));
					std::vector<BYTE>::iterator Itor;
					if( localActor->IsJoypad() && !m_vlPadsToCheck.empty() )
					{
						bool bComboFailed = false;

						for( std::vector<BYTE>::iterator Itor = m_vlPadsToCheck.begin(); Itor != m_vlPadsToCheck.end(); ++Itor )
						{
							if ( localActor->IsPushJoypadButton( IW_PAD( (*Itor) ) ) )
							{
								// Must not equal the previous value.
								if( m_iLastPadIndex != (*Itor) )
								{
									m_iLastPadIndex = (*Itor);
									m_vlPadsToCheck.erase(Itor);

									m_fPadAccum += m_fPadRatio;

									if( m_fPadAccum > 1.0f )
									{
										m_fPadAccum = 1.0f - m_fPadAccum;

										(*piComboCount) = 1;
									}
									return true;
								}
							}
						}
					}
				}

				if( -1 == m_iLastKeyIndex )
					m_iLastKeyIndex = 0;

				int iNumNowEventKeys = (int)pvlNowKeyEvents->size();
				for( int iKey = 0; iKey < iNumNowEventKeys; ++iKey )
				{
					BYTE KeyEvent = pvlNowKeyEvents->at( iKey );
					bool bComboFailed = false;

					// Pass Key Up events
					if( false == pKeyboard->IsPushKey(KeyEvent) )
						continue;

					// Skip keys unrelated to combo calculation.
					vector<BYTE>::iterator iter = find( m_vlKeysToCheck.begin(), m_vlKeysToCheck.end(), KeyEvent );
					if( m_vlKeysToCheck.end() != iter )
					{
						if( m_vlKeysToCheck.at(m_iLastKeyIndex) == KeyEvent )
						{		
							// Combo succeeded, proceed
							m_iLastKeyIndex = (m_iLastKeyIndex+1) % (int)m_vlKeysToCheck.size();

							if( piComboCount && m_iLastKeyIndex == 0 )
							{
								bComboSatisfy = true;
								++(*piComboCount);
							}
						}
						else
							bComboFailed = true;
					}
					else
					{
						bComboFailed = true;
					}

					if( bComboFailed )
					{
						// Return immediately if a wrong key is pressed. Must restart combo from the beginning.
						m_iLastKeyIndex = -1;
						return bComboSatisfy;
					}
				}
				
				//vector<BYTE>::iterator iter = find( m_vlKeysToCheck.begin(), m_vlKeysToCheck.end(), LastKey );
				//if( m_vlKeysToCheck.end() != iter )
				//	m_iLastKeyIndex = distance( m_vlKeysToCheck.begin(), iter );
			}
			break;
	}

	return bComboSatisfy;
}
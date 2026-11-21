#pragma once

#include "EtUIDialog.h"


#define MAX_CREATE_PW 4
#define DELETE_BTN_DELAY 0.8f

class CDnSecurityBase
{
public:

	CDnSecurityBase();
	virtual ~CDnSecurityBase();

	

	struct SecondPassStruct {
		int nSeed;
		int nValue[4];

		void Clear( bool bClearSeed = true ) {
			if( bClearSeed ) nSeed = 0;
			memset( nValue, -1, sizeof(nValue) );
		};
		void SetSeed( int nA ) { nSeed = nA; }
		void AddValue( int nA ) {
			if( nValue[0] == -1 ) nValue[0] = nA;
			else if( nValue[1] == -1 ) nValue[1] = nA;
			else if( nValue[2] == -1 ) nValue[2] = nA;
			else if( nValue[3] == -1 ) nValue[3] = nA;
		};
		void RemoveValue()
		{
			if( nValue[3] != -1 ) nValue[3] = -1;
			else if( nValue[2] != -1 ) nValue[2] = -1;
			else if( nValue[1] != -1 ) nValue[1] = -1;
			else if( nValue[0] != -1 ) nValue[0] = -1;
		}
		bool IsEmpty()
		{
			if( nValue[0] == -1 && nValue[1] == -1 && nValue[2] == -1 && nValue[3] == -1 ) return true;
			return false;
		}
		std::wstring GetStr( bool bPass = true )
		{
			std::wstring wszStr;
			if( bPass ) {
				if( nValue[0] != -1 ) wszStr += L"*";
				if( nValue[1] != -1 ) wszStr += L"*";
				if( nValue[2] != -1 ) wszStr += L"*";
				if( nValue[3] != -1 ) wszStr += L"*";
			}
			else {
			}
			return wszStr;
		}
		int GetSize() {
			int nSize = 0;
			if( nValue[0] != -1 ) nSize++;
			if( nValue[1] != -1 ) nSize++;
			if( nValue[2] != -1 ) nSize++;
			if( nValue[3] != -1 ) nSize++;
			return nSize;
		}
		bool operator == ( const SecondPassStruct &rhs ) {
			return ( MakeSecondPasswordString( nSeed, nValue ) == MakeSecondPasswordString( rhs.nSeed, rhs.nValue ) );

			/*
			if( nValue[0] == rhs.nValue[0] && nValue[1] == rhs.nValue[1] && nValue[2] == rhs.nValue[2] && nValue[3] == rhs.nValue[3] ) return true;
			return false;
			*/
		};
		std::string MakeSecondPasswordString( const int nSeed, const int nValue[] ) {
			/*
			std::wstring szResult;
			if( nSeed == 0 ) return szResult;
			if( nValue[0] == -1 || nValue[1] == -1 || nValue[2] == -1 || nValue[3] == -1 ) return szResult;

			CMtRandom Random;

			std::vector<std::string> tokens;
			TokenizeA(std::string(szVersion), tokens, ".");

			int nKey = atoi(tokens[0].c_str()) + atoi(tokens[1].c_str());
			if( nKey % 2 ) nKey = -nKey;

			int nTempSeed = nSeed + nKey;
			Random.srand( nTempSeed );
			std::vector<int> nVecList, nVecList2;
			int nArray[10] = { 7, 8, 9, 4, 5, 6, 1, 2, 3, 0 };
			for( int i=0; i<10; i++ ) nVecList2.push_back(nArray[i]);

			while( !nVecList2.empty() ) {
				int nOffset = Random.rand( (int)nVecList2.size() );
				nVecList.push_back( nVecList2[nOffset] );
				nVecList2.erase( nVecList2.begin() + nOffset );
			}

			for( int i=0; i<4; i++ ) {
				WCHAR wszTemp[4] = { 0, };
				_itow_s( nVecList[nValue[i]], wszTemp, 4, 10 );
				szResult += wszTemp;
			}
			return szResult;
			*/
			std::string szResultStr;
			if( nSeed == 0 ) return szResultStr;
			if( nValue[0] == -1 || nValue[1] == -1 || nValue[2] == -1 || nValue[3] == -1 ) return szResultStr;

			std::vector<std::string> tokens;
			TokenizeA(std::string(SecondPassword::GetVersion()), tokens, ".");
			int nCount = rand()%( atoi(tokens[1].c_str())/20);

			if( nCount % 6 == 0 ) RevertNumber1( nSeed, nValue, nCount, szResultStr );
			else if( nCount % 6 == 1 ) RevertNumber2( nSeed, nValue, nCount, szResultStr );
			else if( nCount % 6 == 2 ) RevertNumber3( nSeed, nValue, nCount, szResultStr );
			else if( nCount % 6 == 3 ) RevertNumber4( nSeed, nValue, nCount, szResultStr );
			else if( nCount % 6 == 4 ) RevertNumber5( nSeed, nValue, nCount, szResultStr );
			else if( nCount % 6 == 5 ) RevertNumber6( nSeed, nValue, nCount, szResultStr );
			return szResultStr;
		}
	};

	std::vector<std::wstring> m_vecNumberStr;
	std::vector<CEtUIButton*> m_pBtnNumPad;

	int SuffleNumber1( std::vector<std::wstring> &vecNumberStr, std::vector<CEtUIButton*> &vecBtnNumPad, int &nCountNum, int *nResultSeed );
	int SuffleNumber2( std::vector<std::wstring> &vecNumberStr, std::vector<CEtUIButton*> &vecBtnNumPad, int &nCountNum, int *nResultSeed );
	int SuffleNumber3( std::vector<std::wstring> &vecNumberStr, std::vector<CEtUIButton*> &vecBtnNumPad, int &nCountNum, int *nResultSeed );
	int SuffleNumber4( std::vector<std::wstring> &vecNumberStr, std::vector<CEtUIButton*> &vecBtnNumPad, int &nCountNum, int *nResultSeed );
	int SuffleNumber5( std::vector<std::wstring> &vecNumberStr, std::vector<CEtUIButton*> &vecBtnNumPad, int &nCountNum, int *nResultSeed );
	int SuffleNumber6( std::vector<std::wstring> &vecNumberStr, std::vector<CEtUIButton*> &vecBtnNumPad, int &nCountNum, int *nResultSeed );

	static int RevertNumber1( const int nSeed, const int nValue[], int &nCountNum, std::string &szResultValue );
	static int RevertNumber2( const int nSeed, const int nValue[], int &nCountNum, std::string &szResultValue );
	static int RevertNumber3( const int nSeed, const int nValue[], int &nCountNum, std::string &szResultValue );
	static int RevertNumber4( const int nSeed, const int nValue[], int &nCountNum, std::string &szResultValue );
	static int RevertNumber5( const int nSeed, const int nValue[], int &nCountNum, std::string &szResultValue );
	static int RevertNumber6( const int nSeed, const int nValue[], int &nCountNum, std::string &szResultValue );

	bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

protected:
	std::wstring m_EditBox_0;
	std::wstring m_EditBox_1;
	std::wstring m_EditBox_2;

	bool CheckMaxCountPw(SecondPassStruct &Pass);
	bool CheckCompareWord(SecondPassStruct &Pass1, SecondPassStruct &Pass2);
	bool MouseLClickPress;
	float m_fTime;
	CEtUIButton* m_pBtnDelete;
#ifdef PRE_MOD_SECURITYNUMPAD_RENEWAL
	CEtUIButton* m_pBtnShuffle;
#endif 
};

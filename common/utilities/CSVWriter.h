#pragma once
#include "locale.h"

// Rotha 
// Microsoft Excel: CSV export utility, made to extract and inspect data.
// If you need anything later, feel free to add or reuse it.

class CSVWriter
{
public:
	CSVWriter();
	~CSVWriter();

protected:
	FILE *m_stream;
	std::wstring m_strFileName;
	std::wstring m_strColumname;

	bool m_bEnableWrite;
	
public:
	bool Initialize( const wchar_t *FileName , std::wstring strColumname );
	bool CheckExistFile();
	void WriteColumnName();

	void AddColumn(const wchar_t *str);
	void AddField( int nCount = 1);

	void StartWrite();
	void EndWrite();

	FILE* GetStream() { return m_stream; } 
};
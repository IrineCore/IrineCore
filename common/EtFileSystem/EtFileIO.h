#pragma once

class CEtPackingFile;

class CEtFileIO
{
public:
	enum FileIOType
	{
		// Normal file I/O 
		ET_STD_FILE = 0,
		ET_PACK_FILE = 1
	};

	enum AccessType
	{
		ET_READ = 0,
		ET_WRITE,
		ET_ALL
	};
	enum SeekType
	{
		// Same value as FILE_BEGIN = 0 in winbase.h, only the name differs; do not change the number!
		ET_SEEK_BEGIN = 0,		// Relative to beginning of file
		ET_SEEK_CURRENT = 1,	// Relative to current file position
		ET_SEEK_END = 2			// Relative to end of file 
	};

public:
	
	CEtFileIO() { }
	virtual ~CEtFileIO() { }

	virtual bool	Open(const TCHAR* szFileName, AccessType nAccessMode) = 0;
	virtual bool	Close() = 0;

	virtual int		Read(void* pBuffer, int nLen) = 0;
	virtual int		Write(const void* pBuffer, int nLen ) = 0;
	virtual int		Seek(int nOffset, SeekType nPosition) = 0;
	virtual int		Tell() = 0;

public:
	
	// Creates and returns a file I/O object matching the type (caller is responsible for freeing).
	static	CEtFileIO*	CreateFileIO(FileIOType opt);

};

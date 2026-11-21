#pragma once


class DnFirstPatchInfo
{
public:
	DnFirstPatchInfo(void);
	virtual ~DnFirstPatchInfo(void);
	static DnFirstPatchInfo& DnFirstPatchInfo::GetInstance();

	enum 
	{
		EM_FIRSTPATCH_OK = 0,
		EM_FIRSTPATCH_VERSION_FAIL ,
		EM_FIRSTPATCH_ING ,
		EM_FIRSTPATCH_NOT
	};

private:
	CString						m_strSavePostPathPath;
	CString						m_strPostPatchList;			// PostPatch.txt file path.
	std::vector<std::string>	m_vecCopyList;				// Copy file list
	std::vector<std::string>	m_vecDeleteList;			// Delete file list

	int							m_nLocalModuleVersion;		// Current module version.
	int							m_nServerModuleVersion;		// Server's module version.

	bool						m_bIsChangedLauncherFile;	// Was the launcher file modified?

	int							m_nFirstPatchStatus;		// FirstPatch Status
	int							m_nIsFirstPatchProcess;		// Is patching?

public:
	void			SetLocalModuleVersion( int nVersion )		{ m_nLocalModuleVersion  = nVersion;  }
	void			SetServerModuleVersion( int nVersion )		{ m_nServerModuleVersion = nVersion; }
	int				GetLocalModuleVersion()						{ return m_nLocalModuleVersion;  }
	int				GetServerModuleVersion()					{ return m_nServerModuleVersion; }

	void			SetIsChangedLauncherFile(bool bFlag)		{ m_bIsChangedLauncherFile = bFlag; }
	bool			GetIsChangedLauncherFile()					{ return m_bIsChangedLauncherFile;	}

	int				GetFirstPatchStatus()						{ return m_nFirstPatchStatus;	 }
	void			SetFirstPatchStatus(int nStatus)			{ m_nFirstPatchStatus = nStatus; }

	int				IsFirstPatchProcess()						{ return m_nIsFirstPatchProcess;	 }
	void			SetFirstPatchProcess(int nStatus)			{ m_nIsFirstPatchProcess = nStatus; }
	
	int				SetPatchInfo(); // Manages patches prior to the client patch (launcher & modules).
};
#define DNFIRSTPATCHINFO DnFirstPatchInfo::GetInstance()
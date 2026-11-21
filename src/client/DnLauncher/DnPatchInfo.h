//
//	DnPatchInfo.h Patch-related info storage class
//

#pragma once

struct stLoginServerSet
{
	CString	m_strLoginUrl;	// Login server IP address
	WORD	m_wLoginPort;	// Login server Port
};

struct stPartitionListSet
{
	CString m_strGuidepageUrl;	// Launcher guide page URL
	CString m_strHomepageUrl;	// Homepage URL
	CString m_strPartitionName;	// Partition name
	CString m_strVersionUrl;	// URL of PatchInfoServer.cfg file
	CString m_strPatchUrl;		// CDN URL to download patch from
	CString m_strStateTestUrl;	// Webpage URL to check partition status
	WORD	m_wPartitionId;		// Partition ID
	WORD	m_wNew;				// New partition display flag
	WORD	m_wOpen;			// Flag to use during server maintenance
	
	std::vector<stLoginServerSet> m_vecLoginServerList;	// Login server list

	stPartitionListSet() : m_wPartitionId( 0 ), m_wNew( 0 ), m_wOpen( 1 ) {}
	~stPartitionListSet() { m_vecLoginServerList.clear(); }
};

struct stChannelListSet
{
	CString m_strChannelName;	// Channel name
	std::vector<stPartitionListSet> m_vecPartitionList;	// Partition info list

	~stChannelListSet() { m_vecPartitionList.clear(); }
};


//////////////////////////////////////////////////////////////////////////
// Substructure of PatchConfigList.xml
// Channel
//		L Partition
//				L LoginIpList
//////////////////////////////////////////////////////////////////////////

// PatchInfo Class
class CDnPatchInfo
{
public:
	CDnPatchInfo();
	virtual ~CDnPatchInfo();

	static CDnPatchInfo& CDnPatchInfo::GetInstance();
	HRESULT			Init();
	void			SetClientPath( CString strClientPath );
	CString			GetClientPath()		{ return m_strClientPath; }
	BOOL			SetPatchInfo();		// Set patch info
	BOOL			IsSetPatchInfo()	{ return m_bSetPatchInfo; }
	void			LoadClientVersion();// Read client version info
	void			LoadServerVersion();// Read server version info

	void			SetClientVersion( int nClientVersion ) { m_nClientVersion = nClientVersion; }
	void			SetServerVersion( int nServerVersion ) { m_nServerVersion = nServerVersion; }
	int				GetClientVersion() { return m_nClientVersion; }
	int				GetServerVersion() { return m_nServerVersion; }


	CString			GetGuidepageUrl()		{ return m_strGuidepageUrl; }
	CString			GetHomepageUrl()		{ return m_strHomepageUrl; }
	CString			GetPartitionName()		{ return m_strPartitionName; }
	CString			GetPatchUrl()			{ return m_strPatchUrl; }
	CString			GetTotalParameter()		{ return m_strTotalParam; }
	CString			GetOptionParameter()	{ return m_strOptionParam; }
	CString			GetVersionURL()			{ return m_strVersionUrl; }
	
	std::vector<int>&				GetSkipVersion() { return m_vecSkipVersion; }
	std::vector<stChannelListSet>&	GetChannelList() { return m_vecChannelList; }

#ifdef _USE_AUTOUPDATE
	int GetNextVersion() { return m_nNextVersion; }
#endif // _USE_AUTOUPDATE
	WORD GetPartitionOpen() { return m_wOpen; }

#ifdef _USE_PARTITION_SELECT
	int		GetPartitionId( int nChannelNum, int nPartitionNum );
	CString GetBaseGuidePageUrl();
#endif // _USE_PARTITION_SELECT

	int		GetLanguageOffset() { return m_nLanguageOffset; }
	BOOL	SetParameter();							// Set parameters

private:
	HRESULT DownLoadPatchConfigList();		// Download PatchConfigList.xml
	HRESULT ParsingPatchConfigList();		// Parse PatchConfigList.xml
	void	LoadPatchVersionInfo();					// Read patch version info
	void	SetPartitionList(	stChannelListSet& channellistset, WORD dwNew, WORD wPartitionId, WORD wOpen, CString strGuidepageUrl, CString strHomepageUrl, 
								CString strPartitionName, CString strVersionUrl, CString strPatchUrl, CString strStateTestUrl, std::vector<stLoginServerSet>& vecLoginServerList );

	void	ResetPatchInfo();						// Initialize patch info
	BOOL	SetBasePatchInfo();						// Set default patch info
	BOOL	SetFirstPatchInfo();					// Set FIrstPatch info
	int		LoadVersion( CString& szFilePath );		// Read version info from file
	int		LoadVersion( CString& szFilePath, CString szTypeName );
	BOOL	ReadConfig();								// Read config file locally
	void	ParseCommandLineDev( LPCTSTR szCommandLine );

public:
	bool	LoadVersionData();

	
	
#ifdef _USE_SINGLE_CLIENT
	void LoadClientVersionFromResourceFile();
#else // _USE_SINGLE_CLIENT
	void LoadClientVersionFromVersionFile();
#endif // _USE_SINGLE_CLIENT

private:
	BOOL							m_bSetPatchInfo;
	std::vector<stChannelListSet>	m_vecChannelList;
	std::vector<int>				m_vecSkipVersion; //Version to skip

	// Patch info of the selected partition
	CString m_strGuidepageUrl;	// Launcher guide page URL
	CString m_strHomepageUrl;	// Homepage URL
	CString m_strPartitionName;	// Partition name
	CString m_strVersionUrl;	// URL of PatchInfoServer.cfg file
	CString m_strPatchUrl;		// CDN URL to download patch from

	std::vector<stLoginServerSet> m_vecLoginServerList;	// Login server list

	int		m_nClientVersion;			// Current client version
	int		m_nServerVersion;			// Current server version


#ifdef _USE_AUTOUPDATE
	int		m_nNextVersion;		// Next patch version for auto-update
#endif // _USE_AUTOUPDATE

	int		m_nLanguageOffset;	// Per-language offset in Resource.h
	WORD	m_wOpen;			// Partition open flag

	CString m_strClientPath;	// Client installation path
	CString m_strIPList;		// Login IP list for passing parameters (ex: 192.168.0.1;192.168.0.2;...)
	CString m_strPortList;		// Login Port list for passing parameters (ex: 4006;4007;...)
	CString m_strOptionParam;	// Option parameter
	CString m_strTotalParam;	// All parameters (parameters passed when launching client)

};

#define DNPATCHINFO CDnPatchInfo::GetInstance()
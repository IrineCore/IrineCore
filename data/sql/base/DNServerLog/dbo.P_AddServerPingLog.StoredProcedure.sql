USE [dnserverlog]
GO
/****** Object:  StoredProcedure [dbo].[P_AddServerPingLog]    Script Date: 11/14/2026 8:58:01 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2011-07-20
description : ??? Ping ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
2 = ?? ? VIEW DROP ?? ??     by 20111006.wgkim
3 = 15?? ????? ?? by ??? at 2012-11-22
**/

CREATE PROCEDURE [dbo].[P_AddServerPingLog]
	  @intServerType int --// ?? ??
	, @intWorldID int --// (optional) ?? ID
	, @intAccountID int --// (optional) ?? ID
	, @inbCharacterID bigint --// (optional) ??? ID
	, @intSessionID int --// (optional) ?? ID
	, @nvcLogText nvarchar(2000) --// ?? String
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dt2Now datetime2(0) = GETDATE()
	, @nvcTableName nvarchar(128)
	, @nvcTableNameSuffix nchar(8)
	, @intServiceManagerID int
	, @vchIP varchar(15)
	, @intTCPPing int
	, @intTCPTotalLatancy int
	, @intTCPCount int
	, @intUDPPing int
	, @intUDPTotalLatancy int
	, @intUDPCount int
	, @nvcStmt nvarchar(4000);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SET @nvcTableNameSuffix = CONVERT(nchar(8), @dt2Now, 112)
	SET @nvcTableName = N'ServerPingLogs' + @nvcTableNameSuffix;

	IF OBJECT_ID(@nvcTableName, N'U') IS NULL
	BEGIN
		SET @nvcStmt = N'
CREATE TABLE dbo.ServerPingLogs' + @nvcTableNameSuffix + N' (
	LogDate datetime2(0) NOT NULL CONSTRAINT CK_ServerPingLogs' + @nvcTableNameSuffix + N'_LogDate CHECK (LogDate >= ''' + CONVERT(nchar(10), @dt2Now, 121) + N''' AND LogDate < ''' + CONVERT(nchar(10), DATEADD(DAY, 1, @dt2Now), 121) + N'''),	
	ServerType int NOT NULL,
	WorldID int NULL,
	AccountID int NULL,
	CharacterID bigint NULL,
	SessionID int NULL,
	ServiceManagerID int NULL,
	IP varchar(15) NULL,
	TCPPing int NULL,
	TCPTotalLatancy int NULL,
	TCPCount int NULL,
	UDPPing int NULL,
	UDPTotalLatancy int NULL,
	UDPCount int NULL	
);

CREATE CLUSTERED INDEX IX_NC_' + @nvcTableName + N'_LogDate ON dbo.' + @nvcTableName + N' (LogDate);
CREATE INDEX IX_NN_' + @nvcTableName + N'_AccountID ON dbo.' + @nvcTableName + N' (AccountID);
CREATE INDEX IX_NN_' + @nvcTableName + N'_CharacterID ON dbo.' + @nvcTableName + N' (CharacterID);
CREATE INDEX IX_NN_' + @nvcTableName + N'_SessionID ON dbo.' + @nvcTableName + N' (SessionID);
'

		EXEC (@nvcStmt);

		IF OBJECT_ID(N'V_ServerPingLogs', N'V') IS NOT NULL
			DROP VIEW dbo.V_ServerPingLogs;

		SET @nvcStmt = N'
CREATE VIEW dbo.V_ServerPingLogs
AS
';

		SELECT @nvcStmt += N'SELECT * FROM dbo.' + [name] + N' UNION ALL' + CHAR(13) + CHAR(10)
		FROM sys.tables
		WHERE [name] LIKE N'ServerPingLogs%' AND [name] >= N'ServerPingLogs' + CONVERT(nchar(8), DATEADD(DAY, -15, @dt2Now), 112)
		ORDER BY [name];

		SET @nvcStmt = REVERSE(STUFF(REVERSE(@nvcStmt), 1, 11, N''));

		EXEC (@nvcStmt);

		SET @nvcStmt = N'';

		SELECT @nvcStmt += N'DROP TABLE dbo.' + [name] + N';' + CHAR(13) + CHAR(10)
		FROM sys.tables
		WHERE [name] LIKE N'ServerPingLogs%' AND [name] < N'ServerPingLogs' + CONVERT(nchar(8), DATEADD(DAY, -15, @dt2Now), 112)
		ORDER BY [name];

		EXEC (@nvcStmt);
	END

	/** ??? ServerLog Text ???? **/
	SET @intServiceManagerID = CAST(SUBSTRING(@nvcLogText,CHARINDEX(N'[',@nvcLogText,0) + 1,CHARINDEX(N']',@nvcLogText,0) - 2) AS int) 
	SET @vchIP = CAST(SUBSTRING(@nvcLogText,CHARINDEX(N'IP=',@nvcLogText,0) + 3,CHARINDEX(N'TcpPing',@nvcLogText,0) - 16) AS varchar(15)) 
	SET @intTCPPing = CAST(SUBSTRING(@nvcLogText,CHARINDEX(N'TcpPing=',@nvcLogText,0) + 8,CHARINDEX(N'ms',@nvcLogText,0) - CHARINDEX(N'TcpPing=',@nvcLogText,0) - 8) As int) 
	SET @intTCPTotalLatancy = CAST(SUBSTRING(@nvcLogText,CHARINDEX(N'(',@nvcLogText,0) + 1,CHARINDEX(N'/',@nvcLogText,0) - CHARINDEX(N'(',@nvcLogText,0) - 1) As int) 
	SET @intTCPCount = CAST(SUBSTRING(@nvcLogText,CHARINDEX(N'/',@nvcLogText,0) + 1,CHARINDEX(N')',@nvcLogText,0) - CHARINDEX(N'/',@nvcLogText,0) - 1) As int) 


	IF CHARINDEX(N'UdpPing=',@nvcLogText,0) > 0
	BEGIN
		SET @intUDPPing = CAST(SUBSTRING(@nvcLogText,CHARINDEX(N'UdpPing=',@nvcLogText,0) + 8,CHARINDEX(N'ms',@nvcLogText,CHARINDEX(N'UdpPing=',@nvcLogText,0)) - CHARINDEX(N'UdpPing=',@nvcLogText,0) - 8) As int) 
		SET @intUDPTotalLatancy = CAST(SUBSTRING(@nvcLogText,CHARINDEX(N'(',@nvcLogText,CHARINDEX(N'UdpPing=',@nvcLogText,0)) + 1,CHARINDEX(N'/',@nvcLogText,CHARINDEX(N'UdpPing=',@nvcLogText,0)) - CHARINDEX(N'(',@nvcLogText,CHARINDEX(N'UdpPing=',@nvcLogText,0)) - 1) As int) 
		SET @intUDPCount = CAST(SUBSTRING(@nvcLogText,CHARINDEX(N'/',@nvcLogText,CHARINDEX(N'UdpPing=',@nvcLogText,0)) + 1,CHARINDEX(N')',@nvcLogText,CHARINDEX(N'UdpPing=',@nvcLogText,0)) - CHARINDEX(N'/',@nvcLogText,CHARINDEX(N'UdpPing=',@nvcLogText,0)) - 1) As int) 
	END


	SET @nvcStmt = N'INSERT dbo.' + @nvcTableName + N' (LogDate, ServerType, WorldID, AccountID, CharacterID, SessionID, ServiceManagerID, IP, TCPPing, TCPTotalLatancy, TCPCount, UDPPing, UDPTotalLatancy, UDPCount) VALUES (@dt2Now, @intServerType, @intWorldID, @intAccountID, @inbCharacterID, @intSessionID, @intServiceManagerID, @vchIP, @intTCPPing, @intTCPTotalLatancy, @intTCPCount, @intUDPPing, @intUDPTotalLatancy, @intUDPCount);';

	EXEC sp_executesql @nvcStmt
		, N'@dt2Now datetime2(0), @intServerType int, @intWorldID int, @intAccountID int, @inbCharacterID bigint, @intSessionID int, @intServiceManagerID int, @vchIP varchar, @intTCPPing int, @intTCPTotalLatancy int, @intTCPCount int, @intUDPPing int, @intUDPTotalLatancy int, @intUDPCount int'
		, @dt2Now = @dt2Now		
		, @intServerType = @intServerType
		, @intWorldID = @intWorldID
		, @intAccountID = @intAccountID
		, @inbCharacterID = @inbCharacterID
		, @intSessionID = @intSessionID
		, @intServiceManagerID = @intServiceManagerID
		, @vchIP = @vchIP
		, @intTCPPing = @intTCPPing
		, @intTCPTotalLatancy = @intTCPTotalLatancy
		, @intTCPCount = @intTCPCount
		, @intUDPPing = @intUDPPing
		, @intUDPTotalLatancy = @intUDPTotalLatancy
		, @intUDPCount = @intUDPCount;		

END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 2714 AND ERROR_MESSAGE() LIKE N'%ServerPingLogs%'
	BEGIN
		EXEC @intReturnValue = dbo.P_AddServerPingLog			  
			  @intServerType = @intServerType
			, @intWorldID = @intWorldID
			, @intAccountID = @intAccountID
			, @inbCharacterID = @inbCharacterID
			, @intSessionID = @intSessionID
			, @nvcLogText = @nvcLogText;

		RETURN (@intReturnValue);
	END
	GOTO ErrorHandler;
END CATCH;

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

IF @intReturnValue IS NULL OR @intReturnValue = 0
	EXEC @intReturnValue = dbo.P_AddErrorLog;

RETURN @intReturnValue;
GO

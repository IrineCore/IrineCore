USE [dnserverlog]
GO
/****** Object:  StoredProcedure [dbo].[P_AddCommonLog]    Script Date: 11/14/2026 8:58:01 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 16
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-25
description : ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103102 = ???? ???? ????.
history :
10 = ??? ? ?? ?? ?? ?? at 2010-09-13
11 = ?? ?? ??? 7?? ?? at 2010-12-07
15 = input param. @intServerID ?? by ??? at 2012-04-30
**/

CREATE PROCEDURE [dbo].[P_AddCommonLog]
	  @inyLogType tinyint --// ?? ??
	, @intServerType int --// ?? ??
	, @intWorldID int --// (optional) ?? ID
	, @intAccountID int --// (optional) ?? ID
	, @inbCharacterID bigint --// (optional) ??? ID
	, @intSessionID int --// (optional) ?? ID
	, @nvcLogText nvarchar(2000) --// ?? String
	, @intServerID int = NULL --// ?? ID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dt2Now datetime2(0) = GETDATE()
	, @nvcTableName nvarchar(128)
	, @nvcTableNameSuffix nchar(8)
	, @nvcStmt nvarchar(4000)
	, @bitAddColumnFlag bit = 0
	, @nvcCharacterName nvarchar(30);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	IF @inyLogType = 7
		RETURN 0;

	SET @nvcTableNameSuffix = CONVERT(nchar(8), @dt2Now, 112)
	SET @nvcTableName = N'CommonLogs' + @nvcTableNameSuffix;

	IF OBJECT_ID(@nvcTableName, N'U') IS NOT NULL
	BEGIN
		IF EXISTS (SELECT * FROM sys.columns WHERE object_id = OBJECT_ID(@nvcTableName) AND name = N'ServerID')
			SET @bitAddColumnFlag = 1;
	END

	ELSE
	BEGIN
		SET @bitAddColumnFlag = 1;

		SET @nvcStmt = N'
CREATE TABLE dbo.CommonLogs' + @nvcTableNameSuffix + N' (
	LogDate datetime2(0) NOT NULL CONSTRAINT CK_CommonLogs' + @nvcTableNameSuffix + N'_LogDate CHECK (LogDate >= ''' + CONVERT(nchar(10), @dt2Now, 121) + N''' AND LogDate < ''' + CONVERT(nchar(10), DATEADD(DAY, 1, @dt2Now), 121) + N'''),
	LogType tinyint NOT NULL,
	ServerType int NOT NULL,
	WorldID int NULL,
	AccountID int NULL,
	CharacterID bigint NULL,
	SessionID int NULL,
	LogText nvarchar(2000) NOT NULL,
	ServerID int NULL
);

CREATE CLUSTERED INDEX IX_NC_' + @nvcTableName + N'_LogDate ON dbo.' + @nvcTableName + N' (LogDate);
CREATE INDEX IX_NN_' + @nvcTableName + N'_AccountID ON dbo.' + @nvcTableName + N' (AccountID);
CREATE INDEX IX_NN_' + @nvcTableName + N'_CharacterID ON dbo.' + @nvcTableName + N' (CharacterID);
CREATE INDEX IX_NN_' + @nvcTableName + N'_SessionID ON dbo.' + @nvcTableName + N' (SessionID);
'

		EXEC (@nvcStmt);

		IF OBJECT_ID(N'V_CommonLogs', N'V') IS NOT NULL
			DROP VIEW dbo.V_CommonLogs;

		SET @nvcStmt = N'
CREATE VIEW dbo.V_CommonLogs
AS
';

		SELECT @nvcStmt += N'SELECT LogDate, LogType, ServerType, WorldID, AccountID, CharacterID, SessionID, LogText FROM dbo.' + [name] + N' UNION ALL' + CHAR(13) + CHAR(10)
		FROM sys.tables
		WHERE [name] LIKE N'CommonLogs%' AND [name] >= N'CommonLogs' + CONVERT(nchar(8), DATEADD(DAY, -15, @dt2Now), 112)
		ORDER BY [name];

		SET @nvcStmt = REVERSE(STUFF(REVERSE(@nvcStmt), 1, 11, N''));

		EXEC (@nvcStmt);

		SET @nvcStmt = N'';

		SELECT @nvcStmt += N'DROP TABLE dbo.' + [name] + N';' + CHAR(13) + CHAR(10)
		FROM sys.tables
		WHERE [name] LIKE N'CommonLogs%' AND [name] < N'CommonLogs' + CONVERT(nchar(8), DATEADD(DAY, -15, @dt2Now), 112)
		ORDER BY [name];

		EXEC (@nvcStmt);
	END

	SET @nvcStmt = N'INSERT dbo.' + @nvcTableName + N' (LogDate, LogType, ServerType, WorldID, AccountID, CharacterID, SessionID, LogText' + CASE WHEN @intServerID IS NULL OR @bitAddColumnFlag = 0 THEN N'' ELSE N', ServerID' END + N')
VALUES (@dt2Now, @inyLogType, @intServerType, @intWorldID, @intAccountID, @inbCharacterID, @intSessionID, @nvcLogText' + CASE WHEN @intServerID IS NULL OR @bitAddColumnFlag = 0 THEN N'' ELSE N', @intServerID' END + N');';

	EXEC sp_executesql @nvcStmt
		, N'@dt2Now datetime2(0), @inyLogType tinyint, @intServerType int, @intWorldID int, @intAccountID int, @inbCharacterID bigint, @intSessionID int, @nvcLogText nvarchar(2000), @intServerID int'
		, @dt2Now = @dt2Now
		, @inyLogType = @inyLogType
		, @intServerType = @intServerType
		, @intWorldID = @intWorldID
		, @intAccountID = @intAccountID
		, @inbCharacterID = @inbCharacterID
		, @intSessionID = @intSessionID
		, @nvcLogText = @nvcLogText
		, @intServerID = @intServerID;

	-- SpeedHack Log
	IF @inyLogType = 4 AND @nvcLogText LIKE N'SpeedHack%'
	BEGIN
		SET @nvcCharacterName = SUBSTRING(@nvcLogText, CHARINDEX(N'CharName=', @nvcLogText) + 9, 30);
		EXEC P_AddSpeedHackLogs @intWorldID, @intAccountID, @inbCharacterID, @nvcCharacterName;
	END
	-- ServerPing Log
	IF @inyLogType = 35 AND @nvcLogText LIKE N'%TcpPing%'
		EXEC P_AddServerPingLog @intServerType, @intWorldID, @intAccountID, @inbCharacterID, @intSessionID, @nvcLogText;

END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 2714 AND ERROR_MESSAGE() LIKE N'%CommonLogs%'
	BEGIN
		EXEC @intReturnValue = dbo.P_AddCommonLog
			  @inyLogType = @inyLogType
			, @intServerType = @intServerType
			, @intWorldID = @intWorldID
			, @intAccountID = @intAccountID
			, @inbCharacterID = @inbCharacterID
			, @intSessionID = @intSessionID
			, @nvcLogText = @nvcLogText
			, @intServerID = @intServerID;

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

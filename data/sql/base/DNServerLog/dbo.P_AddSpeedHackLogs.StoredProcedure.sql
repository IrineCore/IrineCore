USE [dnserverlog]
GO
/****** Object:  StoredProcedure [dbo].[P_AddSpeedHackLogs]    Script Date: 11/14/2026 8:58:01 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-11-22
description : ??? ? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_AddSpeedHackLogs]
	  @intWorldID int --// ?? ID
	, @intAccountID int --// ?? ID
	, @inbCharacterID bigint --// ??? ID
	, @nvcCharacterName nvarchar(30) --// ??? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dt2Now datetime2(0) = GETDATE()
	, @nvcTableName nvarchar(128)
	, @nvcTableNameSuffix nchar(8)
	, @nvcStmt nvarchar(4000)


/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SET @nvcTableNameSuffix = CONVERT(nchar(8), @dt2Now, 112)
	SET @nvcTableName = N'SpeedHackLogs' + @nvcTableNameSuffix;

	IF OBJECT_ID(@nvcTableName, N'U') IS NULL
	BEGIN
		SET @nvcStmt = N'
CREATE TABLE dbo.SpeedHackLogs' + @nvcTableNameSuffix + N' (
	LogDate datetime2(0) NOT NULL CONSTRAINT CK_SpeedHackLogs' + @nvcTableNameSuffix + N'_LogDate CHECK (LogDate >= ''' + CONVERT(nchar(10), @dt2Now, 121) + N''' AND LogDate < ''' + CONVERT(nchar(10), DATEADD(DAY, 1, @dt2Now), 121) + N'''),	
	WorldID int NOT NULL,
	AccountID int NULL,
	CharacterID bigint NULL,
	CharacterName nvarchar(60) NULL,
);

CREATE CLUSTERED INDEX IX_NC_' + @nvcTableName + N'_LogDate ON dbo.' + @nvcTableName + N' (LogDate);
'

		EXEC (@nvcStmt);

		IF OBJECT_ID(N'V_SpeedHackLogs', N'V') IS NOT NULL
			DROP VIEW dbo.V_SpeedHackLogs;

		SET @nvcStmt = N'
CREATE VIEW dbo.V_SpeedHackLogs
AS
';

		SELECT @nvcStmt += N'SELECT * FROM dbo.' + [name] + N' UNION ALL' + CHAR(13) + CHAR(10)
		FROM sys.tables
		WHERE [name] LIKE N'SpeedHackLogs%' AND [name] >= N'SpeedHackLogs' + CONVERT(nchar(8), DATEADD(DAY, -15, @dt2Now), 112)
		ORDER BY [name];

		SET @nvcStmt = REVERSE(STUFF(REVERSE(@nvcStmt), 1, 11, N''));

		EXEC (@nvcStmt);

		SET @nvcStmt = N'';

		SELECT @nvcStmt += N'DROP TABLE dbo.' + [name] + N';' + CHAR(13) + CHAR(10)
		FROM sys.tables
		WHERE [name] LIKE N'SpeedHackLogs%' AND [name] < N'SpeedHackLogs' + CONVERT(nchar(8), DATEADD(DAY, -15, @dt2Now), 112)
		ORDER BY [name];

		EXEC (@nvcStmt);
	END


	SET @nvcStmt = N'INSERT dbo.' + @nvcTableName + N' (LogDate, WorldID, AccountID, CharacterID, CharacterName) VALUES (@dt2Now, @intWorldID, @intAccountID, @inbCharacterID, @nvcCharacterName);';

	EXEC sp_executesql @nvcStmt
		, N'@dt2Now datetime2(0), @intWorldID int, @intAccountID int, @inbCharacterID bigint, @nvcCharacterName nvarchar(50)'
		, @dt2Now = @dt2Now		

		, @intWorldID = @intWorldID
		, @intAccountID = @intAccountID
		, @inbCharacterID = @inbCharacterID
		, @nvcCharacterName = @nvcCharacterName

END TRY
BEGIN CATCH
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

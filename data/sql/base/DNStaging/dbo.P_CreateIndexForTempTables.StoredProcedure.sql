USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_CreateIndexForTempTables]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 7
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-02-20
description : ETL ?? ? ?? ???? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
5 = History ?? ?? by ??? at (2011-11-07)
5 = TempAccounts ?? by ??? at 2012-11-20
**/

CREATE PROCEDURE [dbo].[P_CreateIndexForTempTables]
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	IF NOT EXISTS (SELECT * FROM sys.indexes WHERE name = N'IX_UC_TempCharacters1_CharacterID')
		CREATE UNIQUE CLUSTERED INDEX IX_UC_TempCharacters1_CharacterID ON dbo.TempCharacters1 (CharacterID);

	IF NOT EXISTS (SELECT * FROM sys.indexes WHERE name = N'IX_UC_TempCharacters2_CharacterID')
		CREATE UNIQUE CLUSTERED INDEX IX_UC_TempCharacters2_CharacterID ON dbo.TempCharacters2 (CharacterID);

	IF NOT EXISTS (SELECT * FROM sys.indexes WHERE name = N'IX_UC_TempDeletedCharacters_CharacterID')
	BEGIN
		SELECT DISTINCT CharacterID, DeleteDate, CharacterName, DeletePrivateIP, DeletePublicIP
		INTO #t1
		FROM dbo.TempDeletedCharacters;

		TRUNCATE TABLE dbo.TempDeletedCharacters;

		INSERT dbo.TempDeletedCharacters (CharacterID, DeleteDate, CharacterName, DeletePrivateIP, DeletePublicIP)
		SELECT CharacterID, DeleteDate, CharacterName, DeletePrivateIP, DeletePublicIP
		FROM #t1;

		DROP TABLE #t1;

		CREATE UNIQUE CLUSTERED INDEX IX_UC_TempDeletedCharacters_CharacterID ON dbo.TempDeletedCharacters (CharacterID);
	END
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

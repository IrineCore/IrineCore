USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_EmptyTempTables]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 7
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-02-20
description : ETL ??? ?? ?? ???? ????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
4 = CharactersBuffer, DormantCharactersBuffer ??? by ??? at (2011-11-07)
5 = CharactersBuffer, DormantCharactersBuffer ??? by ??? at 2011-11-30
6 = TempAccounts ??? by ??? at 2012-11-20
**/

CREATE PROCEDURE [dbo].[P_EmptyTempTables]
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
	TRUNCATE TABLE dbo.TempCharacters1;
	TRUNCATE TABLE dbo.TempCharacters2;
	TRUNCATE TABLE dbo.TempDeletedCharacters;
	TRUNCATE TABLE dbo.TempAccounts;

	IF EXISTS (SELECT * FROM sys.indexes WHERE name = N'IX_UC_TempCharacters1_CharacterID')
		DROP INDEX dbo.TempCharacters1.IX_UC_TempCharacters1_CharacterID;

	IF EXISTS (SELECT * FROM sys.indexes WHERE name = N'IX_UC_TempCharacters2_CharacterID')
		DROP INDEX dbo.TempCharacters2.IX_UC_TempCharacters2_CharacterID;

	IF EXISTS (SELECT * FROM sys.indexes WHERE name = N'IX_UC_TempDeletedCharacters_CharacterID')
		DROP INDEX dbo.TempDeletedCharacters.IX_UC_TempDeletedCharacters_CharacterID;

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

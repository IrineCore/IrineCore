USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListConvertCharacters]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetListConvertCharacters]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetListConvertCharacters] AS' 
END
GO
/**
version : 1
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-08-10
description : ???ID, ????, ???? ? ?? ?? ???? ???? ??? ????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

ALTER PROCEDURE [dbo].[P_GetListConvertCharacters]
	  @xmlCharacterNames xml = NULL--// <root><CH CharacterName="aaa" /><CH CharacterName="bbb" /></root>
	, @xmlCharacterIDs xml = NULL --// <root><CH CharacterID="123" /><CH CharacterID="456" /></root>
	, @xmlAccountNames xml = NULL --// <root><AC AccountName="aaa" /><AC AccountName="bbb" /></root>
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @intDoc int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	IF @xmlCharacterNames IS NOT NULL
	BEGIN
		EXEC sys.sp_xml_preparedocument @intDoc OUTPUT, @xmlCharacterNames;

		WITH C AS (
			SELECT CharacterName
			FROM OPENXML (@intDoc, 'root/CH')
			WITH (CharacterName nvarchar(30) '@CharacterName')
		)
		SELECT DISTINCT CH.WorldID, CH.CharacterID, CH.CharacterName, AC.AccountName
		FROM C
			INNER JOIN dbo.Characters CH WITH (NOLOCK, INDEX(IX_UNF_Characters_CharacterName), FORCESEEK) ON C.CharacterName = CH.CharacterName
			INNER JOIN dbo.Accounts AC WITH (NOLOCK, INDEX(PK_Accounts), FORCESEEK) ON CH.AccountID = AC.AccountID				
		WHERE CH.DeleteFlag = CAST(0 AS bit);

		EXEC sys.sp_xml_removedocument @intDoc;
	END
	ELSE IF @xmlCharacterIDs IS NOT NULL
	BEGIN
		EXEC sys.sp_xml_preparedocument @intDoc OUTPUT, @xmlCharacterIDs;

		WITH C AS (
			SELECT CharacterID
			FROM OPENXML (@intDoc, 'root/CH')
			WITH (CharacterID nvarchar(30) '@CharacterID')
		)
		SELECT DISTINCT CH.WorldID, CH.CharacterID, CH.CharacterName, AC.AccountName
		FROM C
			INNER JOIN dbo.Characters CH WITH (NOLOCK, INDEX(PK_Characters), FORCESEEK) ON C.CharacterID = CH.CharacterID		
			INNER JOIN dbo.Accounts AC WITH (NOLOCK, INDEX(PK_Accounts), FORCESEEK) ON CH.AccountID = AC.AccountID
		WHERE CH.DeleteFlag = CAST(0 AS bit);

		EXEC sys.sp_xml_removedocument @intDoc;
	END
	ELSE IF @xmlAccountNames IS NOT NULL
	BEGIN
		EXEC sys.sp_xml_preparedocument @intDoc OUTPUT, @xmlAccountNames;

		WITH A AS (
			SELECT AccountName
			FROM OPENXML (@intDoc, 'root/AC')
			WITH (AccountName nvarchar(30) '@AccountName')
		)
		SELECT DISTINCT CH.WorldID, CH.CharacterID, CH.CharacterName, AC.AccountName
		FROM A
			INNER JOIN dbo.Accounts AC WITH (NOLOCK, INDEX(IX_UN_Accounts_AccountName), FORCESEEK) ON A.AccountName = AC.AccountName	
			INNER JOIN dbo.Characters CH WITH (NOLOCK, INDEX(IX_NN_Characters_AccountID), FORCESEEK) ON AC.AccountID = CH.AccountID
		WHERE CH.DeleteFlag = CAST(0 AS bit);
		
		EXEC sys.sp_xml_removedocument @intDoc;
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

USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListAccountByCharacterID]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetListAccountByCharacterID]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetListAccountByCharacterID] AS' 
END
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-10-07
description : CharacterID? Account ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

ALTER PROCEDURE [dbo].[P_GetListAccountByCharacterID]
	  @xmlCharacterIDs xml --// <root><CH CharacterID="1" /><CH CharacterID="2" /></root>
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
	EXEC sys.sp_xml_preparedocument @intDoc OUTPUT, @xmlCharacterIDs;

	WITH C AS (
		SELECT CharacterID
		FROM OPENXML (@intDoc, 'root/CH')
		WITH (CharacterID bigint '@CharacterID')
	)
	SELECT DISTINCT AC.AccountID, AC.AccountName
	FROM C
		INNER JOIN dbo.Characters CH WITH (NOLOCK) ON C.CharacterID = CH.CharacterID
		INNER JOIN dbo.Accounts AC WITH (NOLOCK) ON CH.AccountID = AC.AccountID;

	EXEC sys.sp_xml_removedocument @intDoc;
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

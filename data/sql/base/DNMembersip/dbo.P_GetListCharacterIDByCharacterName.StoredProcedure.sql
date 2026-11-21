USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListCharacterIDByCharacterName]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetListCharacterIDByCharacterName]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetListCharacterIDByCharacterName] AS' 
END
GO
/**
version : 2
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-01-10
description : ??? ?? ???? ???ID? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

ALTER PROCEDURE [dbo].[P_GetListCharacterIDByCharacterName]
	  @xmlCharacterNames xml --// <root><CH CharacterName="aaa" /><CH CharacterName="bbb" /></root>
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @intDoc int;;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	EXEC sys.sp_xml_preparedocument @intDoc OUTPUT, @xmlCharacterNames;

	WITH C AS (
		SELECT CharacterName
		FROM OPENXML (@intDoc, 'root/CH')
		WITH (CharacterName nvarchar(30) '@CharacterName')
	)
	SELECT DISTINCT CH.WorldID, CH.CharacterID, CH.CharacterName
	FROM C
		INNER JOIN dbo.Characters CH WITH (NOLOCK) ON C.CharacterName = CH.CharacterName;		

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

USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DLS_ValidateCharacterIDsForDNGirl]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : dailykim
e-mail : dailykim@eyedentitygames.com
created date : 2013-08-08
description : ??? ID? ???? ?????. (????)
result set : ErrorType (1=??? ???, 2=?? ???, 3=?? ???? ???), CharacterID
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_DLS_ValidateCharacterIDsForDNGirl]
	    @xmlCharacterIDs xml = NULL --// ???ID XML ??? (<root><CH CharacterID="123" /><CH CharacterID="456" /></root>) 	  
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	  , @intDoc int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	/**_# xml? ???? ??? ID? ?????.*/
	EXEC sys.sp_xml_preparedocument @intDoc OUTPUT, @xmlCharacterIDs;

	WITH C AS (
		SELECT CharacterID
		FROM OPENXML (@intDoc, 'root/CH')
		WITH (CharacterID nvarchar(30) '@CharacterID')
	)				
	SELECT CAST(1 AS tinyint) ErrorType, CH.CharacterID
	FROM C
		INNER JOIN dbo.Characters CH WITH (NOLOCK) ON C.CharacterID = CH.CharacterID
	WHERE CH.DeleteFlag = CAST(1 AS bit)
	UNION ALL
	SELECT CAST(2 AS tinyint), C.CharacterID
	FROM C
	WHERE NOT EXISTS (
		SELECT * FROM dbo.Characters WITH (NOLOCK) WHERE CharacterID = C.CharacterID
	)	
	UNION ALL
	SELECT CAST(3 AS tinyint), CH.CharacterID
	FROM C
		INNER JOIN dbo.Characters CH WITH (NOLOCK) ON C.CharacterID = CH.CharacterID
	WHERE CH.AccountLevelCode = 120
	ORDER BY ErrorType, CharacterID;

	EXEC sys.sp_xml_removedocument @intDoc;	
		
END TRY
BEGIN CATCH
	IF XACT_STATE() <> 0
		ROLLBACK TRANSACTION;

	IF @intReturnValue IS NULL OR @intReturnValue = 0
		EXEC @intReturnValue = dbo.P_AddErrorLog @intPROCID = @@PROCID;

	ELSE
		GOTO ErrorHandler;

	RETURN @intReturnValue;
END CATCH;

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

RETURN @intReturnValue;
GO

USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DLS_ModAccountLevelForXML]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : dailykim
e-mail : dailykim@eyedentitygames.com
created date : 2013-08-08
description : XML ???? ???? ??????? AccountLevelCode? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103417 = ???ID? ???? ???? ??? ???????.
**/

CREATE PROCEDURE [dbo].[P_DLS_ModAccountLevelForXML]
	  @xmlCharacterIDs xml = NULL --// ???ID XML ??? (<root><CH CharacterID="123" /><CH CharacterID="456" /></root>) 
	, @xmlCharacterNames xml = NULL --// ???? XML ??? (<root><CH CharacterName="aaa" /><CH CharacterName="bbb" /></root>) 	
	, @inyAccountLevelCode tinyint --// 0=?? ??, 10=??(??), 20=????(??), 30=???(??), 99=QA, 100=???, 110=DWC???, 120=???
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
	IF @xmlCharacterIDs IS NULL AND @xmlCharacterNames IS NULL
	BEGIN
		SET @intReturnValue = 103417;
		GOTO ErrorHandler;
	END
	
	IF @xmlCharacterIDs IS NOT NULL
	BEGIN
		/**_# xml? ???? ???ID? ???? ???? AccountLevelCode ? UPDATE???.*/
		EXEC sys.sp_xml_preparedocument @intDoc OUTPUT, @xmlCharacterIDs;

		WITH C AS (
			SELECT CharacterID
			FROM OPENXML (@intDoc, 'root/CH')
			WITH (CharacterID nvarchar(30) '@CharacterID')
		)
		UPDATE dbo.Characters
		SET AccountLevelCode = @inyAccountLevelCode
		FROM C INNER JOIN dbo.Characters CH WITH (NOLOCK) ON C.CharacterID = CH.CharacterID;
		
		EXEC sys.sp_xml_removedocument @intDoc;
	END
	ELSE IF @xmlCharacterNames IS NOT NULL
	BEGIN
		/**_# xml? ???? ?????? ???? ???? AccountLevelCode ? UPDATE???.*/
		
		EXEC sys.sp_xml_preparedocument @intDoc OUTPUT, @xmlCharacterNames;

		WITH C AS (
			SELECT CharacterName
			FROM OPENXML (@intDoc, 'root/CH')
			WITH (CharacterName nvarchar(30) '@CharacterName')
		)	
		UPDATE dbo.Characters
		SET AccountLevelCode = @inyAccountLevelCode
		FROM C INNER JOIN dbo.Characters CH WITH (NOLOCK) ON C.CharacterName = CH.CharacterName
		WHERE CH.DeleteFlag = 0;
		
		EXEC sys.sp_xml_removedocument @intDoc;
	END
	
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

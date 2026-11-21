USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListCharacterByAccountName]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-06-09
description : ?? ???? ???? ?????.
result set : AccountName/nvarchar, CharacterID/bigint, CharacterName/nvarchar, CharacterLevel/tinyint, JobCode/tinyint
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListCharacterByAccountName]
	  @xmlAccountNames xml --// <root><Characters AccountName="kimbab"/></root>
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @intDoc int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	EXEC sp_xml_preparedocument @intDoc OUTPUT, @xmlAccountNames;

	WITH S (AccountName) AS (
		SELECT AccountName
		FROM OPENXML (@intDoc, N'/root/Characters')
		WITH (
			AccountName nvarchar(50) '@AccountName'
		)
	)
	SELECT CH.AccountName, CH.CharacterID, CH.CharacterName, CHS.CharacterLevel, CHS.JobCode
	FROM S
		INNER JOIN dbo.Characters CH WITH (NOLOCK, INDEX(IX_NN_Characters_AccountName), FORCESEEK) ON S.AccountName = CH.AccountName
		INNER JOIN dbo.CharacterStatus CHS WITH (NOLOCK, INDEX(PK_CharacterStatus), FORCESEEK) ON CH.CharacterID = CHS.CharacterID
	WHERE CH.DeleteFlag = CAST(0 AS bit);

	EXEC sp_xml_removedocument @intDoc;
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

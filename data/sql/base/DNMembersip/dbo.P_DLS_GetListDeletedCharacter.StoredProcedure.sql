USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_DLS_GetListDeletedCharacter]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_DLS_GetListDeletedCharacter]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_DLS_GetListDeletedCharacter] AS' 
END
GO
/**
version : 1
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-03-27
description : ??? ???? ??? ???? ?????.
result set : AccountID/int, AccountName/nvarchar(50), CharacterID/bigint, CharacterName/nvarchar(50), WorldID/int, CreateDate/datetime2(0), DeleteDate/datetime2(0), LastLoginDate/datetime2(0)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

ALTER PROCEDURE [dbo].[P_DLS_GetListDeletedCharacter]
	  @nvcCharacterName nvarchar(30) --// ??? ??
--WITH EXECUTE AS 'SPExecutor'
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
	SELECT C.AccountID, C.AccountName, A.CharacterID, A.CharacterName, B.WorldID, B.CreateDate, A.DeleteDate, C.LastLoginDate 
	FROM dbo.DeletedCharacters A WITH(NOLOCK, INDEX(IX_NN_DeletedCharacters_CharacterName), FORCESEEK) 
		INNER JOIN dbo.Characters B WITH(NOLOCK, INDEX(PK_Characters), FORCESEEK) ON A.CharacterID = B.CharacterID
		INNER JOIN dbo.Accounts C WITH(NOLOCK, INDEX(PK_Accounts), FORCESEEK) ON B.AccountID = C.AccountID
	WHERE A.CharacterName = @nvcCharacterName
	OPTION (KEEPFIXED PLAN);
END TRY
BEGIN CATCH
	GOTO ErrorHandler;
END CATCH;

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

IF @intReturnValue IS NULL OR @intReturnValue = 0
	EXEC @intReturnValue = dbo.P_AddErrorLog @intPROCID = @@PROCID;

RETURN @intReturnValue;
GO

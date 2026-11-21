USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModAccountLevel]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-03-22
description : ??? ??? ?????. (??ID or ???? or ???ID or ??????? ?? ??)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103102 = ???? ???? ????.
**/

CREATE PROCEDURE [dbo].[P_ModAccountLevel]
	  @intAccountID int = NULL --// ?? ID
	, @nvcAccountName nvarchar(50) = NULL --// ?? ??
	, @inbCharacterID bigint = NULL --// ??? ID
	, @nvcCharacterName nvarchar(30) = NULL --// ??? ??
	, @inyAccountLevelCode tinyint --// 0=?? ??, 10=??(??), 20=????(??), 30=???(??), 99=QA, 100=???
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
	IF @intAccountID IS NOT NULL
		UPDATE dbo.Characters
		SET AccountLevelCode = @inyAccountLevelCode
		WHERE AccountID = @intAccountID

	ELSE IF @nvcAccountName IS NOT NULL
		UPDATE dbo.Characters
		SET AccountLevelCode = @inyAccountLevelCode
		WHERE AccountName = @nvcAccountName

	ELSE IF @inbCharacterID IS NOT NULL
		UPDATE dbo.Characters
		SET AccountLevelCode = @inyAccountLevelCode
		WHERE CharacterID = @inbCharacterID

	ELSE IF @nvcCharacterName IS NOT NULL
		UPDATE dbo.Characters
		SET AccountLevelCode = @inyAccountLevelCode
		WHERE CharacterName = @nvcCharacterName AND DeleteFlag = CAST(0 AS bit)

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103102;
		GOTO ErrorHandler;
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

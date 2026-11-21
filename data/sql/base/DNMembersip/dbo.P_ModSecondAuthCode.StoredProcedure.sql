USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_ModSecondAuthCode]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_ModSecondAuthCode]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_ModSecondAuthCode] AS' 
END
GO
/**
version : 3
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-18
description : 2? ?? ???? ????? ?????.
	SecondAuthCode ? ??
	1 -> 3 : P_ModSecondAuthPassphrase ? ?? ???? ??? ?? ??
	2 -> 3 : ? SP? ??
	3 -> 2 : ? SP? ??
	2, 3 -> 1 : ?? ??
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101115 = ??? ???? ????.
101128 = @inySecondAuthCode ?? ?? ????.
**/

ALTER PROCEDURE [dbo].[P_ModSecondAuthCode]
	  @intAccountID int --// ??? ?? ID
	, @inySecondAuthCode tinyint --// 2=2? ?? ???? ??, 3=2? ?? ???
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @binSecondPassphrase binary(20), @inySecondAuthCodeBefore tinyint;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	IF @inySecondAuthCode NOT IN (2, 3)
	BEGIN
		SET @intReturnValue = 101128;
		GOTO ErrorHandler;
	END

	UPDATE dbo.Accounts
	SET @inySecondAuthCodeBefore = SecondAuthCode
		, SecondAuthCode = @inySecondAuthCode
		, SecondAuthLockFlag = CASE @inySecondAuthCode WHEN 2 THEN 0 ELSE 1 END
	WHERE AccountID = @intAccountID;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 101115;
		GOTO ErrorHandler;
	END

	IF @inySecondAuthCodeBefore <> @inySecondAuthCode
		INSERT dbo.SecondAuthLogs (LogDate, AccountID, SecondAuthCode)
		VALUES (GETDATE(), @intAccountID, @inySecondAuthCode);
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

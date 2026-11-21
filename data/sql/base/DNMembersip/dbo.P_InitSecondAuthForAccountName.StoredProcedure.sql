USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_InitSecondAuthForAccountName]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_InitSecondAuthForAccountName]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_InitSecondAuthForAccountName] AS' 
END
GO
/**
version : 2
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-11-23
description : ?? ??? 2? ?? ??? ??? ???.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

ALTER PROCEDURE [dbo].[P_InitSecondAuthForAccountName]
	  @nvcAccountName nvarchar(50) --// ??? ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @inySecondAuthCodeBefore tinyint, @intAccountID int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# [Accounts] ???? UPDATE???.*/
	UPDATE dbo.Accounts
	SET @inySecondAuthCodeBefore = SecondAuthCode
		, @intAccountID = AccountID
		, SecondAuthPassphrase = NULL
		, SecondAuthFailCount = 0
		, SecondAuthCode = 1
		, SecondAuthLockFlag = 0
		, LastSecondAuthNotifyDate = NULL
		, SecondAuthResetDate = NULL
	WHERE AccountName = @nvcAccountName;

	/**_# ??? ???? ??? 101115? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 101115;
		GOTO ErrorHandler;
	END

	IF @inySecondAuthCodeBefore <> 1
		INSERT dbo.SecondAuthLogs (LogDate, AccountID, SecondAuthCode)
		VALUES (GETDATE(), @intAccountID, 1);
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

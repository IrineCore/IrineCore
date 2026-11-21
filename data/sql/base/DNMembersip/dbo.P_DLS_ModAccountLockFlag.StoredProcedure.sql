USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_DLS_ModAccountLockFlag]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_DLS_ModAccountLockFlag]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_DLS_ModAccountLockFlag] AS' 
END
GO
/**
version : 3
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-08-30
description : ?? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101115 = ??? ???? ????.
**/

ALTER PROCEDURE [dbo].[P_DLS_ModAccountLockFlag]
	  @intAccountID int --// ??ID
	, @bitLockFlag bit --// 0=????, 1=??
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
	/**_# [Accounts] ???? UPDATE???.*/
	UPDATE dbo.Accounts
	SET LockFlag = @bitLockFlag
	WHERE AccountID = @intAccountID;
	
	/**_## ??? ???? ??? 101115? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 101119;
		GOTO ErrorHandler;
	END

	INSERT dbo.AccountLockLogs(LogDate, AccountID, LockFlag)
	VALUES (GETDATE(), @intAccountID, @bitLockFlag)

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

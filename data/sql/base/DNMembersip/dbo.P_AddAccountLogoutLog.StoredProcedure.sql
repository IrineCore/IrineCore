USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_AddAccountLogoutLog]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_AddAccountLogoutLog]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_AddAccountLogoutLog] AS' 
END
GO
/**
version : 7
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-06
description : ?? ???? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

ALTER PROCEDURE [dbo].[P_AddAccountLogoutLog]
	  @dt2LogoutDate datetime2(0) --// ???? ??
	, @intAccountID int --// ??? ?? ID
	, @dt2LoginDate datetime2(0) --// ??? ??
	, @intLoginIP int --// ??? IP ??
	, @intSessionID int --// ?? ID
	, @binMachineID binary(16) = NULL --// PC ???? ???
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @intLogTableSuffix int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SET @intLogTableSuffix = dbo.FN_LogTableSuffix(@dt2LogoutDate);

	/**_# [AccountLogoutLogs0] ???? INSERT???.*/
	IF @intLogTableSuffix = 0
		INSERT dbo.AccountLogoutLogs0 (LogoutDate, AccountID, LoginDate, LoginIP, SessionID, MachineID)
		VALUES (@dt2LogoutDate, @intAccountID, @dt2LoginDate, @intLoginIP, @intSessionID, @binMachineID);

	ELSE IF @intLogTableSuffix = 1
		INSERT dbo.AccountLogoutLogs1 (LogoutDate, AccountID, LoginDate, LoginIP, SessionID, MachineID)
		VALUES (@dt2LogoutDate, @intAccountID, @dt2LoginDate, @intLoginIP, @intSessionID, @binMachineID);

	ELSE
		INSERT dbo.AccountLogoutLogs2 (LogoutDate, AccountID, LoginDate, LoginIP, SessionID, MachineID)
		VALUES (@dt2LogoutDate, @intAccountID, @dt2LoginDate, @intLoginIP, @intSessionID, @binMachineID);
END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 2601 AND ERROR_MESSAGE() LIKE N'%IX_UN_AccountLogoutLogs%'
		RETURN 0;
		

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

USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_AddMobileAuthenticationLog]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_AddMobileAuthenticationLog]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_AddMobileAuthenticationLog] AS' 
END
GO
/**
version : 1
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-11-09
description : ??? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

ALTER PROCEDURE [dbo].[P_AddMobileAuthenticationLog]
	  @dt2LogDate datetime2(0) --// ????
	, @intAccountID int --// ??ID
	, @inyAuthenticationLogCode tinyint --// ?????? (0=??????,1=??????,2=??????)
	, @intAccountAuthenticationKey int --// ??? ?? ?? ?
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
	SET @intLogTableSuffix = dbo.FN_LogTableSuffix(@dt2LogDate);

	/**_# [MobileAuthenticationLogs0] ???? INSERT???.*/
	IF @intLogTableSuffix = 0
		INSERT dbo.MobileAuthenticationLogs0 (LogDate, AccountID, AuthenticationLogCode, AccountAuthenticationKey)
		VALUES (@dt2LogDate, @intAccountID, @inyAuthenticationLogCode, @intAccountAuthenticationKey);
	ELSE IF @intLogTableSuffix = 1
		INSERT dbo.MobileAuthenticationLogs1 (LogDate, AccountID, AuthenticationLogCode, AccountAuthenticationKey)
		VALUES (@dt2LogDate, @intAccountID, @inyAuthenticationLogCode, @intAccountAuthenticationKey);
	ELSE
		INSERT dbo.MobileAuthenticationLogs2 (LogDate, AccountID, AuthenticationLogCode, AccountAuthenticationKey)
		VALUES (@dt2LogDate, @intAccountID, @inyAuthenticationLogCode, @intAccountAuthenticationKey);

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

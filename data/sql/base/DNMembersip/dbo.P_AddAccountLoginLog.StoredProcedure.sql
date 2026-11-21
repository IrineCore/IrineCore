USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_AddAccountLoginLog]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_AddAccountLoginLog]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_AddAccountLoginLog] AS' 
END
GO
/**
version : 6
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-02-12
description : ?? ??? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
4 = (?? 26402) ???? ?? ??? ?? ? ?? ??. by wgkim (2010.12.23)
**/

ALTER PROCEDURE [dbo].[P_AddAccountLoginLog]
	  @dt2LoginDate datetime2(0) --// ??? ??
	, @intAccountID int --// ??? ?? ID
	, @intLoginIP int --// IP ??
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
	SET @intLogTableSuffix = dbo.FN_LogTableSuffix(@dt2LoginDate);

	/**_# [AccountLoginLogs] ???? INSERT???.*/
	IF @intLogTableSuffix = 0
		INSERT dbo.AccountLoginLogs0 (LoginDate, AccountID, LoginIP, SessionID, MachineID)
		VALUES (@dt2LoginDate, @intAccountID, @intLoginIP, @intSessionID, @binMachineID);

	ELSE IF @intLogTableSuffix = 1
		INSERT dbo.AccountLoginLogs1 (LoginDate, AccountID, LoginIP, SessionID, MachineID)
		VALUES (@dt2LoginDate, @intAccountID, @intLoginIP, @intSessionID, @binMachineID);

	ELSE
		INSERT dbo.AccountLoginLogs2 (LoginDate, AccountID, LoginIP, SessionID, MachineID)
		VALUES (@dt2LoginDate, @intAccountID, @intLoginIP, @intSessionID, @binMachineID);

	/**_# ???? ?? ??? ?? ? ?? ?? */
	MERGE dbo.LeavePreventionLoginTerm T
	USING (SELECT @intAccountID, @dt2LoginDate) S (AccountID, LoginDate)
	ON T.AccountID = S.AccountID
	WHEN MATCHED THEN
		UPDATE SET LastLoginDate = S.LoginDate
			 , Term1  = CASE WHEN (CONVERT(CHAR(8),S.LoginDate, 112) > CONVERT(CHAR(8), LastLoginDate, 112)) THEN DATEDIFF(DAY, LastLoginDate, S.LoginDate) ELSE Term1 END 
			 , Term2  = CASE WHEN (CONVERT(CHAR(8),S.LoginDate, 112) > CONVERT(CHAR(8), LastLoginDate, 112)) THEN Term1 ELSE Term2  END 
			 , Term3  = CASE WHEN (CONVERT(CHAR(8),S.LoginDate, 112) > CONVERT(CHAR(8), LastLoginDate, 112)) THEN Term2 ELSE Term3  END 
			 , Term4  = CASE WHEN (CONVERT(CHAR(8),S.LoginDate, 112) > CONVERT(CHAR(8), LastLoginDate, 112)) THEN Term3 ELSE Term4  END 
			 , Term5  = CASE WHEN (CONVERT(CHAR(8),S.LoginDate, 112) > CONVERT(CHAR(8), LastLoginDate, 112)) THEN Term4 ELSE Term5  END 
			 , Term6  = CASE WHEN (CONVERT(CHAR(8),S.LoginDate, 112) > CONVERT(CHAR(8), LastLoginDate, 112)) THEN Term5 ELSE Term6  END 
			 , Term7  = CASE WHEN (CONVERT(CHAR(8),S.LoginDate, 112) > CONVERT(CHAR(8), LastLoginDate, 112)) THEN Term6 ELSE Term7  END 
			 , Term8  = CASE WHEN (CONVERT(CHAR(8),S.LoginDate, 112) > CONVERT(CHAR(8), LastLoginDate, 112)) THEN Term7 ELSE Term8  END 
			 , Term9  = CASE WHEN (CONVERT(CHAR(8),S.LoginDate, 112) > CONVERT(CHAR(8), LastLoginDate, 112)) THEN Term8 ELSE Term9  END 
			 , Term10 = CASE WHEN (CONVERT(CHAR(8),S.LoginDate, 112) > CONVERT(CHAR(8), LastLoginDate, 112)) THEN Term9 ELSE Term10 END 
	WHEN NOT MATCHED THEN
		INSERT (AccountID, LastLoginDate, Term1, Term2, Term3, Term4, Term5, Term6, Term7, Term8, Term9, Term10)
		VALUES (S.AccountID, S.LoginDate, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		
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

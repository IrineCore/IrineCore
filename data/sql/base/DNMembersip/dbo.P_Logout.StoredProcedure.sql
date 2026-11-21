USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_Logout]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_Logout]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_Logout] AS' 
END
GO
/**
version : 10
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-06
description : ???? ???. - LastLogoutDate? ???? ???? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101115 = ??? ???? ????.
history :
7 = 6? ???? ?? (5??? ??) by wgkim (2010.12.23)
8 = (26402) ???? ?? ?? Duration Time (?) ?? ??(dbo.AccountID / TotalDT). by wgkim (2010.12.23)
9 = (44912) IP ? ?? ?? ???? ??? ?? by ??? at 2011-11-09
10 = (49590) IP ?? ?? ?? by ??? at 2012-01-13
**/

ALTER PROCEDURE [dbo].[P_Logout]
	  @intAccountID int --// ??? ?? ID
	, @intSessionID int --// ?? ID
	, @binMachineID binary(16) = NULL --// PC ???? ???
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dt2Now datetime2(0) = GETDATE()
	, @dt2LoginDate datetime2(0)
	, @intLoginIP int
	, @intLoginSessionID int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# Accounts.LastLogoutDate? UPDATE???.*/
	UPDATE dbo.Accounts
	SET @dt2LoginDate = LastLoginDate
		, @intLoginIP = LastLoginIP
		, @intLoginSessionID = LastSessionID
		, LastLogoutDate = @dt2Now
		, TotalDT = ISNULL(TotalDT, 0) + DATEDIFF(MI, LastLoginDate, @dt2Now)
	WHERE AccountID = @intAccountID;

	/**_# ??? ???? ??? 101115? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 101115;
		GOTO ErrorHandler;
	END

	/**_# [ClientIPConnectCounts] ???? UPDATE???.*/
	DELETE dbo.ClientIPConnectCounts
	FROM dbo.ClientIPConnectCounts WITH (INDEX(IX_UC_ClientIPConnectCounts_IP_AccountID), FORCESEEK)
	WHERE IP = @intLoginIP AND AccountID = @intAccountID
	OPTION (KEEPFIXED PLAN);
	/**_# [P_AddAccountLogoutLog]? ???? ???? ??? ?????.*/
	IF @intLoginSessionID = @intSessionID AND @dt2LoginDate < @dt2Now
		EXEC @intReturnValue = dbo.P_AddAccountLogoutLog
			  @dt2LogoutDate = @dt2Now
			, @intAccountID = @intAccountID
			, @dt2LoginDate = @dt2LoginDate
			, @intLoginIP = @intLoginIP
			, @intSessionID = @intSessionID
			, @binMachineID = @binMachineID;

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

USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListAccountLogoutLog]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetListAccountLogoutLog]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetListAccountLogoutLog] AS' 
END
GO
/**
version : 3
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-02-14
description : ?? ?? ?? ??? ?????.
result set : LogoutDate, AccountID, LoginDate, LoginIP, SessionID
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
3	MachineID ??? ????? ?? at 2010-05-07
**/

ALTER PROCEDURE [dbo].[P_GetListAccountLogoutLog]
	@dtmBasisDate datetime --// ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dt2BasisDate datetime2(0) = @dtmBasisDate
	, @intLogTableSuffix int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SET @intLogTableSuffix = dbo.FN_LogTableSuffix(@dt2BasisDate);

	IF @intLogTableSuffix = 0
		SELECT LogoutDate, AccountID, LoginDate, LoginIP, SessionID, MachineID
		FROM dbo.AccountLogoutLogs0 WITH (NOLOCK)
		WHERE LogoutDate >= @dt2BasisDate AND LogoutDate < DATEADD(DAY, 1, @dt2BasisDate)

	ELSE IF @intLogTableSuffix = 1
		SELECT LogoutDate, AccountID, LoginDate, LoginIP, SessionID, MachineID
		FROM dbo.AccountLogoutLogs1 WITH (NOLOCK)
		WHERE LogoutDate >= @dt2BasisDate AND LogoutDate < DATEADD(DAY, 1, @dt2BasisDate)

	ELSE
		SELECT LogoutDate, AccountID, LoginDate, LoginIP, SessionID, MachineID
		FROM dbo.AccountLogoutLogs2 WITH (NOLOCK)
		WHERE LogoutDate >= @dt2BasisDate AND LogoutDate < DATEADD(DAY, 1, @dt2BasisDate)

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

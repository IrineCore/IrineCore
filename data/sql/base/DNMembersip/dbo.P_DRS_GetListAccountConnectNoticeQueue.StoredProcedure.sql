USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_GetListAccountConnectNoticeQueue]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_DRS_GetListAccountConnectNoticeQueue]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_DRS_GetListAccountConnectNoticeQueue] AS' 
END
GO
/**
version : 2
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-11-01
description : ?? ?? ?? ?? ?????.
result set : AccountID/int, AccountName/nvarchar(50), Logindate/datetime2
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

ALTER PROCEDURE [dbo].[P_DRS_GetListAccountConnectNoticeQueue]	 
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

	/**_# PushQueue ????? ?? ??? ?? ???? ?????.*/
	CREATE TABLE #ReturnQueue (
		AccountID int NOT NULL,
	    LoginDate datetime2(0) NOT NULL
	);

	/**_# PushQueue ???? ?? DELETE ??? ??? Row? ?????? ????.*/
	DELETE dbo.AccountConnectNoticeQueue
	OUTPUT deleted.AccountID, deleted.RegistedDate
	INTO #ReturnQueue (AccountID, LoginDate);

	/**_# ?? ???? ?? ??? ?????.*/
	SELECT Q.AccountID, AC.AccountName, Q.LoginDate
	FROM #ReturnQueue Q
		INNER JOIN dbo.Accounts AC WITH (NOLOCK) ON Q.AccountID = AC.AccountID;

	DROP TABLE #ReturnQueue;

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

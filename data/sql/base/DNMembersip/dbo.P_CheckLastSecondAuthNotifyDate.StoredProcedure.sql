USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_CheckLastSecondAuthNotifyDate]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_CheckLastSecondAuthNotifyDate]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_CheckLastSecondAuthNotifyDate] AS' 
END
GO
/**
version : 5
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-02-09
description : ?? 2? ?? ?? ??? ??? ???? ?? ??? ?????? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101139 = 2? ?? ??? ?? ??? ???????.
**/

ALTER PROCEDURE [dbo].[P_CheckLastSecondAuthNotifyDate]
	  @intAccountID int --// ??? ?? ID
	, @intPeriod int = 3 --// ??? ?? (?)
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
	SET @intReturnValue =
		CASE
			WHEN (SELECT DATEDIFF(DAY, LastSecondAuthNotifyDate, GETDATE()) FROM dbo.Accounts WITH (NOLOCK) WHERE AccountID = @intAccountID) <= @intPeriod THEN 0
			ELSE 101139
		END;

	IF @intReturnValue <> 0
		GOTO ErrorHandler;
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

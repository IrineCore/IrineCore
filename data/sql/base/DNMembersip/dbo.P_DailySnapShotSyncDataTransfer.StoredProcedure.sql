USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_DailySnapShotSyncDataTransfer]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_DailySnapShotSyncDataTransfer]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_DailySnapShotSyncDataTransfer] AS' 
END
GO
/**
version : 1
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2011-09-20
description : DailySnapShot?? Accounts ??? ????
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

ALTER PROCEDURE [dbo].[P_DailySnapShotSyncDataTransfer]

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
-- ??? ?? : dbo.SecondAuthLogs
DECLARE @dtmStartChangeDate DATETIME
	  , @dtmEndChangeDate DATETIME;
BEGIN TRY
    -- ??? ??? ??? ?? ??? ????. (StatusCode = 1)
    SELECT @dtmStartChangeDate = StartChangeDate
         , @dtmEndChangeDate = EndChangeDate
    FROM dbo.SecondAuthLogsChangeTracking WITH (NOLOCK)
    WHERE StatusCode = 1;

    -- Source ?????? ?? ?? ? ???? ????.
    SELECT DISTINCT
           SAL.AccountID
         , A.SecondAuthCode
    FROM dbo.SecondAuthLogs SAL WITH (NOLOCK)
          LEFT OUTER JOIN dbo.Accounts A WITH (NOLOCK) ON (SAL.AccountID = A.AccountID)
    WHERE SAL.LogDate > @dtmStartChangeDate
       AND SAL.LogDate <= @dtmEndChangeDate
    OPTION (FORCE ORDER, LOOP JOIN, MAXDOP 1);

    -- ?? ??
    UPDATE dbo.SecondAuthLogsChangeTracking
    SET StatusMessage = N'dbo.SecondAuthLogs Success!'
    WHERE StatusCode = 1;

END TRY
BEGIN CATCH
    -- ?? ??
    UPDATE dbo.SecondAuthLogsChangeTracking
    SET StatusMessage = N'dbo.SecondAuthLogs Fail.. ' + ERROR_MESSAGE(), StatusCode = 2
    WHERE StatusCode = 1;
	
	GOTO ErrorHandler
END CATCH

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

IF @intReturnValue IS NULL OR @intReturnValue = 0
	EXEC @intReturnValue = dbo.P_AddErrorLog;

RETURN @intReturnValue;
GO

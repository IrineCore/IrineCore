USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_DailySnapShotSyncScope]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_DailySnapShotSyncScope]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_DailySnapShotSyncScope] AS' 
END
GO
/**
version : 2
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2011-09-20
description : DailySnapShot ??? ??? Setting???
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

ALTER PROCEDURE [dbo].[P_DailySnapShotSyncScope]

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

    DECLARE @dtmStartChangeDate DATETIME
          , @dtmEndChangeDate DATETIME;

    -- ??? ??? ??? ?? ??? ????.
    SELECT @dtmStartChangeDate = MAX(EndChangeDate)
         , @dtmEndChangeDate = GETDATE()
    FROM dbo.SecondAuthLogsChangeTracking WITH (NOLOCK)
    WHERE StatusCode = 0;

    -- ?? ?? ?? ?? (?? ???)
    SELECT @dtmStartChangeDate = ISNULL(@dtmStartChangeDate, DATEADD(DAY, -1, GETDATE()))
         , @dtmEndChangeDate = ISNULL(@dtmEndChangeDate, GETDATE());

    -- [dbo.SecondAuthLogsChangeTracking]? ????? ???(StatusCode = 1)? ?? ??? ???? (???? Row ? ?? ? ?? ??)
    IF EXISTS (SELECT * FROM dbo.SecondAuthLogsChangeTracking WITH (NOLOCK) WHERE StatusCode = 1)
    BEGIN
        UPDATE dbo.SecondAuthLogsChangeTracking
           SET StatusMessage = N'StatusCode : 1'
             , StatusCode = 3
         WHERE StatusCode = 1;
    END

    -- [dbo.SecondAuthLogsChangeTracking]? ????? ???(StatusCode = 1)? Row ??
    INSERT dbo.SecondAuthLogsChangeTracking (StartChangeDate, EndChangeDate, StatusCode, RegistrationDate)
    VALUES (@dtmStartChangeDate, @dtmEndChangeDate, 1, GETDATE());
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

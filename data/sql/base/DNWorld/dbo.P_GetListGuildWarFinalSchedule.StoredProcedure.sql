USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListGuildWarFinalSchedule]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : wgkim@eyedentitygames
created date : 2011-03-03

description :
    ??? ?? ?? ??
result set :
    MatchTypeCode	tinyint
    MatchStartTime	smalldatetime
    MatchEndTime	smalldatetime
return value :
    0 = ??? ????.
    1 = ????? Commit? ? ?? ?????. ????? Rollback???.
    100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
    2 = ?? ??
**/

CREATE PROCEDURE [dbo].[P_GetListGuildWarFinalSchedule]
    @insGuildWarScheduleID smallint --// ??? ??
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

    /**_# ?? ??? ?????. */
    SELECT MatchTypeCode -- 1=??, 2=4?, 3=8?, 4=16?
         , MatchStartTime
         , MatchEndTime
      FROM dbo.GuildWarFinalSchedule WITH (NOLOCK)
     WHERE GuildWarScheduleID = @insGuildWarScheduleID
     ORDER BY MatchTypeCode, MatchStartTime, MatchEndTime

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

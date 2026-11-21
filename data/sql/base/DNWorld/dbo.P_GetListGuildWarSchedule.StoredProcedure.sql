USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListGuildWarSchedule]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 6
author : ???
e-mail : wgkim@eyedentitygames
created date : 2011-03-10

description :
    ??? ?? ??
result set : 
    GuildWarScheduleID	smallint
    ApplyStartDate	smalldatetime
    ApplyEndDate	smalldatetime
    OpeningStartDate	smalldatetime
    OpeningEndDate	smalldatetime
    RewardStartDate	smalldatetime
    RewardEndDate	smalldatetime
    WinnersWeightRate	smallint
    WinningTeamColorCode	tinyint
	FinalProgressFlag	bit
	RewardExpirationDate	smalldatetime
	FinalCancelFlag	bit
return value :
    0 = ??? ????.
    1 = ????? Commit? ? ?? ?????. ????? Rollback???.
    100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
    3 = ?? ??
	4 = ???? ?? ? ?? ???? ??     by 20110919.wgkim
	5 = ?? ??? ??     by ??? at 2011-12-07
	6 = ?? ?? ??? ??     by ??? at 2012-11-07
**/

CREATE PROCEDURE [dbo].[P_GetListGuildWarSchedule]
	  @sdtStartDate smalldatetime --// ?? ???
	, @sdtEndDate smalldatetime --// ?? ???
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

    /**_# ?? ??? ???? ?? ??? ???? ??? ??? ?????. */
	;WITH CG AS (
		SELECT DISTINCT GuildWarScheduleID	
		  FROM dbo.CanceledGuildWarFinalResults WITH (NOLOCK)
	)
    SELECT GS.GuildWarScheduleID, GS.ApplyStartDate, GS.ApplyEndDate, GS.OpeningStartDate, GS.OpeningEndDate, GS.RewardStartDate, GS.RewardEndDate, 
		   GS.WinnersWeightRate, GS.WinningTeamColorCode, GS.FinalProgressFlag, GS.RewardExpirationDate,
	       CASE WHEN CG.GuildWarScheduleID IS NULL THEN CAST(0 AS bit) ELSE CAST(1 AS bit) END AS FinalCancelFlag
      FROM dbo.GuildWarSchedule GS WITH (NOLOCK) LEFT OUTER JOIN CG ON GS.GuildWarScheduleID = CG.GuildWarScheduleID
     WHERE GS.ApplyStartDate >= @sdtStartDate AND GS.ApplyStartDate <= @sdtEndDate
     ORDER BY GS.ApplyStartDate

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

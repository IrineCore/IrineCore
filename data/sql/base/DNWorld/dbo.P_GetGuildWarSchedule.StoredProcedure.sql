USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetGuildWarSchedule]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 13
author : ???
e-mail : wgkim@eyedentitygames
created date : 2011-01-06

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
return value :
    0 = ??? ????.
    1 = ????? Commit? ? ?? ?????. ????? Rollback???.
    100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
    9 = ?? ??
	10 = ???? ?? ? ?? ???? ??     by 20110919.wgkim
	11~12 = ?? ??? ??     by ??? at 2011-12-07
	13 = ?? ???? ?? ??? ?? ?? ??? ??? ????? ??     by ??? at 2011-12-08
**/

CREATE PROCEDURE [dbo].[P_GetGuildWarSchedule]
	  @insGuildWarScheduleID smallint OUTPUT --// ??? ??
	, @sdtApplyStartDate smalldatetime OUTPUT --// ???? (???)
	, @sdtApplyEndDate smalldatetime OUTPUT --// ???? (???)
	, @sdtOpeningStartDate smalldatetime OUTPUT --// ???? (???)
	, @sdtOpeningEndDate smalldatetime OUTPUT --// ???? (???)
	, @sdtRewardStartDate smalldatetime OUTPUT --// ???? (???)
	, @sdtRewardEndDate smalldatetime OUTPUT --// ???? (???)
	, @insWinnersWeightRate smallint OUTPUT --// ?? ??? ???
	, @insFinalProgressFlag bit OUTPUT --// ?? ???? (0=???, 1=??)
	, @sdtRewardExpirationDate smalldatetime = NULL OUTPUT --// ?? ???? (0=???, 1=??)
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

    /**_# ???? ???? ?? ?? ?? / ??? ???? ?? ?? ??? ?????. */
    SELECT TOP 1 @insGuildWarScheduleID = GuildWarScheduleID, @sdtApplyStartDate = ApplyStartDate, @sdtApplyEndDate = ApplyEndDate, @sdtOpeningStartDate = OpeningStartDate, @sdtOpeningEndDate = OpeningEndDate, @sdtRewardStartDate = RewardStartDate, @sdtRewardEndDate = RewardEndDate, @insWinnersWeightRate = WinnersWeightRate, @insFinalProgressFlag = FinalProgressFlag
      FROM dbo.GuildWarSchedule WITH (NOLOCK)
     WHERE RewardEndDate >= GETDATE()
     ORDER BY RewardEndDate;

    /**_## ?? ???? ?? ??? ?? ?? ??? ??? ?????. */
	SELECT TOP 1 @sdtRewardExpirationDate = RewardExpirationDate
	  FROM dbo.GuildWarSchedule WITH (NOLOCK)
	 WHERE RewardExpirationDate >= GETDATE()
	 ORDER BY RewardExpirationDate;

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

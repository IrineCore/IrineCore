USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddGuildWarScheduleForQA]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : wgkim@eyedentitygames
created date : 2011-11-23

description :
    ??? ?? ?? (QA ??)
return value :
    0 = ??? ????.
    1 = ????? Commit? ? ?? ?????. ????? Rollback???.
    100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
    1 = ?? ??
	2 = ?? ??? ??     by ??? at 2012-01-04
**/

CREATE PROCEDURE [dbo].[P_AddGuildWarScheduleForQA]
	  @insGuildWarScheduleID smallint OUTPUT --// ??? ?? (??? ???? null ? ??)
	, @sdtApplyStartDate smalldatetime --// ???? (???)
	, @sdtApplyEndDate smalldatetime --// ???? (???)
	, @sdtOpeningStartDate smalldatetime --// ???? (???)
	, @sdtOpeningEndDate smalldatetime --// ???? (???)
	, @sdtRewardStartDate smalldatetime --// ???? (???)
	, @sdtRewardEndDate smalldatetime --// ???? (???)
	, @insWinnersWeightRate smallint --// ?? ??? ???
	, @inyWinningTeamColorCode tinyint = NULL --// ?? ??? (1=??, 2=??) (???? ?? ???? ??? ??? ?? ??)
	, @bitFinalProgressFlag bit = 1 --// ?? ???? (0=???, 1=??)
	, @sdtRewardExpirationDate smalldatetime --// ?????
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
      , @sdtNowDate smalldatetime;

SET @sdtNowDate = GETDATE()

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	/**_# @insGuildWarScheduleID IS NOT NULL */
    /**_## ??? ?? ??? ??? ?????. */
    IF @insGuildWarScheduleID IS NOT NULL
    BEGIN
	    UPDATE dbo.GuildWarSchedule
	    SET ApplyStartDate = @sdtApplyStartDate
		    , ApplyEndDate = @sdtApplyEndDate
		    , OpeningStartDate = @sdtOpeningStartDate
		    , OpeningEndDate = @sdtOpeningEndDate
		    , RewardStartDate = @sdtRewardStartDate
		    , RewardEndDate = @sdtRewardEndDate
		    , WinnersWeightRate = @insWinnersWeightRate
		    , WinningTeamColorCode = ISNULL(@inyWinningTeamColorCode, WinningTeamColorCode)
		    , ChangeDate = GETDATE()
		    , FinalProgressFlag = @bitFinalProgressFlag
			, RewardExpirationDate = @sdtRewardExpirationDate
	    WHERE GuildWarScheduleID = @insGuildWarScheduleID;

    END
    ELSE
    BEGIN

        /**_# @insGuildWarScheduleID IS NULL */
	    /**_## ??? ??? ?????. */
	    INSERT dbo.GuildWarSchedule (ApplyStartDate, ApplyEndDate, OpeningStartDate, OpeningEndDate, RewardStartDate, RewardEndDate, WinnersWeightRate, RegisterDate, ChangeDate, FinalProgressFlag, RewardExpirationDate)
	    VALUES (@sdtApplyStartDate, @sdtApplyEndDate, @sdtOpeningStartDate, @sdtOpeningEndDate, @sdtRewardStartDate, @sdtRewardEndDate, @insWinnersWeightRate, GETDATE(), GETDATE(), 1, @sdtRewardExpirationDate);

        SET @insGuildWarScheduleID = SCOPE_IDENTITY()

	    /**_## ??? ?? ??? ????? ?????. */
	    INSERT dbo.GuildWarFinalSchedule (GuildWarScheduleID, MatchTypeCode, MatchStartTime, MatchEndTime, RegisterDate, ChangeDate)
        SELECT @insGuildWarScheduleID
             , 4
             , CONVERT(SMALLDATETIME, CONVERT(CHAR(10), @sdtRewardEndDate, 120) + ' 18:00:00')
             , CONVERT(SMALLDATETIME, CONVERT(CHAR(10), @sdtRewardEndDate, 120) + ' 18:40:00')
             , @sdtNowDate
             , @sdtNowDate
	    INSERT dbo.GuildWarFinalSchedule (GuildWarScheduleID, MatchTypeCode, MatchStartTime, MatchEndTime, RegisterDate, ChangeDate)
        SELECT @insGuildWarScheduleID
             , 3
             , CONVERT(SMALLDATETIME, CONVERT(CHAR(10), @sdtRewardEndDate, 120) + ' 19:00:00')
             , CONVERT(SMALLDATETIME, CONVERT(CHAR(10), @sdtRewardEndDate, 120) + ' 19:40:00')
             , @sdtNowDate
             , @sdtNowDate

	    INSERT dbo.GuildWarFinalSchedule (GuildWarScheduleID, MatchTypeCode, MatchStartTime, MatchEndTime, RegisterDate, ChangeDate)
        SELECT @insGuildWarScheduleID
             , 2
             , CONVERT(SMALLDATETIME, CONVERT(CHAR(10), @sdtRewardEndDate, 120) + ' 20:00:00')
             , CONVERT(SMALLDATETIME, CONVERT(CHAR(10), @sdtRewardEndDate, 120) + ' 20:40:00')
             , @sdtNowDate
             , @sdtNowDate

	    INSERT dbo.GuildWarFinalSchedule (GuildWarScheduleID, MatchTypeCode, MatchStartTime, MatchEndTime, RegisterDate, ChangeDate)
        SELECT @insGuildWarScheduleID
             , 1
             , CONVERT(SMALLDATETIME, CONVERT(CHAR(10), @sdtRewardEndDate, 120) + ' 21:00:00')
             , CONVERT(SMALLDATETIME, CONVERT(CHAR(10), @sdtRewardEndDate, 120) + ' 21:40:00')
             , @sdtNowDate
             , @sdtNowDate

    END

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

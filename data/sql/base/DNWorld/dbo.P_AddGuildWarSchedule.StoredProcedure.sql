USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddGuildWarSchedule]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 22
author : ???
e-mail : wgkim@eyedentitygames
created date : 2011-03-08

description :
    ??? ?? ??
return value :
    0 = ??? ????.
    1 = ????? Commit? ? ?? ?????. ????? Rollback???.
    100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
	103312 = ??? ?? ??? ??? ????.
history :
    10 = ?? ??
	14 = return value ?? : GuildWarScheduleID     by 20110721.wgkim
	15 = ?? ?? ?? ??     by 20110727.wgkim
	16 = ?? ?? ?? ??     by 20110801.wgkim
	17 = ??? ?? ?? ?? (19? -> 18?)
	18 = ???? ?? ? ?? ???? ??     by 20110919.wgkim
	19 = ?? ?? ?? ?? ?? ??? ???? ?? by ??? at 2011-11-07
	20 = ???, ???? @sdtNowDate ????? ??     by ??? at 2011-12-06
	21 = ?? ??? ??     by ??? at 2011-12-07
	22 = ?? ??? ?? ??? ?? ??     by ??? at 2012-06-21
**/

CREATE PROCEDURE [dbo].[P_AddGuildWarSchedule]
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
	, @sdtRewardExpirationDate smalldatetime = NULL --// ?? ???
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
      , @sdtNowDate smalldatetime = GETDATE();

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	-- ?? ??
	IF (@sdtApplyStartDate > @sdtApplyEndDate)
	OR (@sdtApplyEndDate > @sdtOpeningStartDate)
	OR (@sdtOpeningStartDate > @sdtOpeningEndDate)
	OR (@sdtOpeningEndDate > @sdtRewardStartDate)
	OR (@sdtRewardStartDate > @sdtRewardEndDate)
	BEGIN
		SET @intReturnValue = 103312;
		GOTO ErrorHandler;
	END

	IF @insGuildWarScheduleID IS NOT NULL
	BEGIN
		IF EXISTS (SELECT * FROM dbo.GuildWarSchedule WITH (NOLOCK) WHERE GuildWarScheduleID < @insGuildWarScheduleID AND RewardEndDate >= @sdtApplyStartDate)
		BEGIN
			SET @intReturnValue = 103312;
			GOTO ErrorHandler;
		END
		IF EXISTS (SELECT * FROM dbo.GuildWarSchedule WITH (NOLOCK) WHERE GuildWarScheduleID > @insGuildWarScheduleID AND ApplyStartDate <= @sdtRewardEndDate)
		BEGIN
			SET @intReturnValue = 103312;
			GOTO ErrorHandler;
		END
	END
	ELSE
	BEGIN
		IF @sdtApplyStartDate < (SELECT MAX(RewardEndDate) FROM dbo.GuildWarSchedule WITH (NOLOCK))
		BEGIN
			SET @intReturnValue = 103312;
			GOTO ErrorHandler;
		END
	END

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
		    , ChangeDate = @sdtNowDate
		    , FinalProgressFlag = @bitFinalProgressFlag
		    , RewardExpirationDate = ISNULL(@sdtRewardExpirationDate, RewardExpirationDate)
	    WHERE GuildWarScheduleID = @insGuildWarScheduleID;

    END
    ELSE
    BEGIN

        /**_# @insGuildWarScheduleID IS NULL */
	    /**_## ??? ??? ?????. */
	    INSERT dbo.GuildWarSchedule (ApplyStartDate, ApplyEndDate, OpeningStartDate, OpeningEndDate, RewardStartDate, RewardEndDate, WinnersWeightRate, RegisterDate, ChangeDate, FinalProgressFlag, RewardExpirationDate)
	    VALUES (@sdtApplyStartDate, @sdtApplyEndDate, @sdtOpeningStartDate, @sdtOpeningEndDate, @sdtRewardStartDate, @sdtRewardEndDate, @insWinnersWeightRate, @sdtNowDate, @sdtNowDate, 1, @sdtRewardExpirationDate);

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

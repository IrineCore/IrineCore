USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddGuildWarRewardForGuild]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : wgkim@eyedentitygames
created date : 2011-03-21

description :
    ??? ???? ???? ??
return value :
    0 = ??? ????.
    1 = ????? Commit? ? ?? ?????. ????? Rollback???.
    100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
	103302=?? ?? ??? ???????.
history :
    4 = ?? ??
	5 = input parameter (?? ????) ??     by ??? at 2012-06-21
**/

CREATE PROCEDURE [dbo].[P_AddGuildWarRewardForGuild]
	  @insGuildWarScheduleID smallint --// ??? ??
	, @intGuildID int --// ?? ID
	, @inyRewardCode tinyint --// ?? ?? ?? (1=?? ?? ??(???), 2=?? ?? ??, 3=?? ?? ??, 4=?? ?? ???, 5=?? ?? ???)
	, @sdtRewardExpirationDate smalldatetime = NULL --// ??????
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
      , @bitRewardSkill bit
      , @bitRewardItem bit
      , @bitRewardMail bit
      , @bitRewardPointByOpening bit
      , @bitRewardPointByFinal bit;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	/**_# ?? ????? ?? ???. */
	IF @inyRewardCode = 1 AND @sdtRewardExpirationDate IS NOT NULL
	BEGIN
		UPDATE dbo.GuildWarSchedule
		   SET RewardExpirationDate = @sdtRewardExpirationDate
		 WHERE GuildWarScheduleID = @insGuildWarScheduleID;
	END

    /**_# ??? ???? ????? ?????. */
    IF NOT EXISTS (SELECT * FROM dbo.GuildWarRewardForGuild WITH (NOLOCK) WHERE GuildWarScheduleID = @insGuildWarScheduleID AND GuildID = @intGuildID)
    BEGIN
	    INSERT dbo.GuildWarRewardForGuild (GuildWarScheduleID, GuildID, RewardSkill, RewardItem, RewardMail, RewardPointByOpening, RewardPointByFinal)
	    VALUES (@insGuildWarScheduleID, @intGuildID, CONVERT(BIT, 0), CONVERT(BIT, 0), CONVERT(BIT, 0), CONVERT(BIT, 0), CONVERT(BIT, 0));
    END

    IF @inyRewardCode = 1
    BEGIN
        SET @bitRewardSkill = 1;
        SET @bitRewardItem = 0;
        SET @bitRewardMail = 0;
        SET @bitRewardPointByOpening = 0;
        SET @bitRewardPointByFinal = 0;

        UPDATE dbo.GuildWarRewardForGuild
           SET RewardSkill = @bitRewardSkill
         WHERE GuildWarScheduleID = @insGuildWarScheduleID
           AND GuildID = @intGuildID
           AND RewardSkill <> @bitRewardSkill;
    END
    ELSE IF @inyRewardCode = 2
    BEGIN
        SET @bitRewardSkill = 0;
        SET @bitRewardItem = 1;
        SET @bitRewardMail = 0;
        SET @bitRewardPointByOpening = 0;
        SET @bitRewardPointByFinal = 0;

        UPDATE dbo.GuildWarRewardForGuild
           SET RewardItem = @bitRewardItem
         WHERE GuildWarScheduleID = @insGuildWarScheduleID
           AND GuildID = @intGuildID
           AND RewardItem <> @bitRewardItem;
    END
    ELSE IF @inyRewardCode = 3
    BEGIN
        SET @bitRewardSkill = 0;
        SET @bitRewardItem = 0;
        SET @bitRewardMail = 1;
        SET @bitRewardPointByOpening = 0;
        SET @bitRewardPointByFinal = 0;

        UPDATE dbo.GuildWarRewardForGuild
           SET RewardMail = @bitRewardMail
         WHERE GuildWarScheduleID = @insGuildWarScheduleID
           AND GuildID = @intGuildID
           AND RewardMail <> @bitRewardMail;
    END
    ELSE IF @inyRewardCode = 4
    BEGIN
        SET @bitRewardSkill = 0;
        SET @bitRewardItem = 0;
        SET @bitRewardMail = 0;
        SET @bitRewardPointByOpening = 1;
        SET @bitRewardPointByFinal = 0;

        UPDATE dbo.GuildWarRewardForGuild
           SET RewardPointByOpening = @bitRewardPointByOpening
         WHERE GuildWarScheduleID = @insGuildWarScheduleID
           AND GuildID = @intGuildID
           AND RewardPointByOpening <> @bitRewardPointByOpening;
    END
    ELSE IF @inyRewardCode = 5
    BEGIN
        SET @bitRewardSkill = 0;
        SET @bitRewardItem = 0;
        SET @bitRewardMail = 0;
        SET @bitRewardPointByOpening = 0;
        SET @bitRewardPointByFinal = 1;

        UPDATE dbo.GuildWarRewardForGuild
           SET RewardPointByFinal = @bitRewardPointByFinal
         WHERE GuildWarScheduleID = @insGuildWarScheduleID
           AND GuildID = @intGuildID
           AND RewardPointByFinal <> @bitRewardPointByFinal;
    END

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103302;
		GOTO ErrorHandler;
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

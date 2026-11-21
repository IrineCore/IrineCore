USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddGuildWarRewardForCharacter]    Script Date: 11/14/2026 8:54:37 AM ******/
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
    5 = ?? ??
**/

CREATE PROCEDURE [dbo].[P_AddGuildWarRewardForCharacter]
	  @insGuildWarScheduleID smallint --// ??? ??
	, @inbCharacterID bigint --// ??? ID
	, @inyRewardCode tinyint --// ?? ?? ?? (1=?? ?? ??, 2=?? ?? ??, 3=?? ?? ???)
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
      , @bitRewardItem bit
      , @bitRewardMail bit
      , @bitRewardPointByOpening bit;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

    /**_# ??? ???? ????? ?????. */
    IF NOT EXISTS (SELECT * FROM dbo.GuildWarRewardForCharacter WITH (NOLOCK) WHERE GuildWarScheduleID = @insGuildWarScheduleID AND CharacterID = @inbCharacterID)
    BEGIN
	    INSERT dbo.GuildWarRewardForCharacter (GuildWarScheduleID, CharacterID, RewardItem, RewardMail, RewardPointByOpening)
	    VALUES (@insGuildWarScheduleID, @inbCharacterID, CONVERT(BIT, 0), CONVERT(BIT, 0), CONVERT(BIT, 0));
    END

    IF @inyRewardCode = 1
    BEGIN
        SET @bitRewardItem = 1;
        SET @bitRewardMail = 0;
        SET @bitRewardPointByOpening = 0;

        UPDATE dbo.GuildWarRewardForCharacter
           SET RewardItem = @bitRewardItem
         WHERE GuildWarScheduleID = @insGuildWarScheduleID
           AND CharacterID = @inbCharacterID
           AND RewardItem <> @bitRewardItem;
    END
    ELSE IF @inyRewardCode = 2
    BEGIN
        SET @bitRewardItem = 0;
        SET @bitRewardMail = 1;
        SET @bitRewardPointByOpening = 0;

        UPDATE dbo.GuildWarRewardForCharacter
           SET RewardMail = @bitRewardMail
         WHERE GuildWarScheduleID = @insGuildWarScheduleID
           AND CharacterID = @inbCharacterID
           AND RewardMail <> @bitRewardMail;
    END
    ELSE IF @inyRewardCode = 3
    BEGIN
        SET @bitRewardItem = 0;
        SET @bitRewardMail = 0;
        SET @bitRewardPointByOpening = 1;

        UPDATE dbo.GuildWarRewardForCharacter
           SET RewardPointByOpening = @bitRewardPointByOpening
         WHERE GuildWarScheduleID = @insGuildWarScheduleID
           AND CharacterID = @inbCharacterID
           AND RewardPointByOpening <> @bitRewardPointByOpening;
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

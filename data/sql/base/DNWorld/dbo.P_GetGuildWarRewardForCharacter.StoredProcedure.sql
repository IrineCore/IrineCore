USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetGuildWarRewardForCharacter]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : wgkim@eyedentitygames
created date : 2011-03-21

description :
    ??? ???? ???? ??
result set :
	@bitRewardItem						bit OUTPUT
	@bitRewardMail						bit OUTPUT
	@bitRewardPointByOpening			bit OUTPUT
return value :
    0 = ??? ????.
    1 = ????? Commit? ? ?? ?????. ????? Rollback???.
    100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
    2 = ?? ??
**/

CREATE PROCEDURE [dbo].[P_GetGuildWarRewardForCharacter]
	  @insGuildWarScheduleID smallint --// ??? ??
	, @inbCharacterID bigint --// ??? ID
	, @bitRewardItem bit OUTPUT --// ??? ?? ?? ?? (0=?? ???, 1=?? ??)
	, @bitRewardMail bit OUTPUT --// ??? ?? ?? ?? (0=?? ???, 1=?? ??)
	, @bitRewardPointByOpening bit OUTPUT --// ??? ?? ?? ??? (0=?? ???, 1=?? ??)
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

	/**_# ??? ???? ????? ?????. */
	SELECT @bitRewardItem = RewardItem, @bitRewardMail = RewardMail, @bitRewardPointByOpening = RewardPointByOpening
	FROM dbo.GuildWarRewardForCharacter WITH (NOLOCK)
	WHERE GuildWarScheduleID = @insGuildWarScheduleID AND CharacterID = @inbCharacterID;

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

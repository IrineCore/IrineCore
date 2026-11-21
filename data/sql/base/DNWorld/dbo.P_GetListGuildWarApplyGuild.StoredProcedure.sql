USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListGuildWarApplyGuild]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2012-06-11

description :
	??? ?? ?? ?? ?? ??
result set : 
	GuildWarScheduleID / smallint
	GuildID / int
	GuildName / nvarchar(50)
	GuildLevel / tinyint
	TeamColorCode / tinyint
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListGuildWarApplyGuild]
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
      , @insGuildWarScheduleID smallint;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

    /**_# ?? ???? ??? ??? ?????. */
	SELECT @insGuildWarScheduleID = GuildWarScheduleID
	  FROM dbo.GuildWarSchedule WITH (NOLOCK)
	 WHERE ApplyStartDate <= GETDATE()
	   AND GETDATE() <= RewardEndDate;

    /**_# ?? ??? ??? ?? ??? ?????. */
	SELECT GuildWarScheduleID, GuildID, GuildName, GuildLevel, TeamColorCode
	  FROM dbo.Guilds WITH (NOLOCK)
	 WHERE GuildWarScheduleID = @insGuildWarScheduleID
	   AND CloseFlag = CONVERT(bit, 0);

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

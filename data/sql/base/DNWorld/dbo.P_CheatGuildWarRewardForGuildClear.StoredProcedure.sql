USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_CheatGuildWarRewardForGuildClear]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : wgkim@eyedentitygames
created date : 2011-04-22

description :
    ??? ?? ?? ?? (QA ? ?? ??)
return value :
    0 = ??? ????.
    1 = ????? Commit? ? ?? ?????. ????? Rollback???.
    100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
    1 = ?? ??
**/

CREATE PROCEDURE [dbo].[P_CheatGuildWarRewardForGuildClear]
	  @nvcGuildName	nvarchar(50) = NULL --// ???
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @intGuildID int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	SELECT @intGuildID = GuildID FROM dbo.Guilds WITH (NOLOCK) WHERE GuildName = @nvcGuildName;

    /**_# ?? ???? ?? ???? ?????. */
	DELETE dbo.GuildWarRewardForGuild WHERE GuildID = @intGuildID;

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

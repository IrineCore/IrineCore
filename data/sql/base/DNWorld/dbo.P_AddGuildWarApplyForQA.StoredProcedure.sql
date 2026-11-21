USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddGuildWarApplyForQA]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : wgkim@eyedentitygames
created date : 2011-11-23

description :
    ?? ??? ??? ?? (QA ??)
return value :
    0 = ??? ????.
    1 = ????? Commit? ? ?? ?????. ????? Rollback???.
    100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
    1~3 = ?? ??
**/

CREATE PROCEDURE [dbo].[P_AddGuildWarApplyForQA]
	  @insGuildWarScheduleID smallint --// ??? ??
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

	WHILE (1 = 1)
	BEGIN
		SELECT TOP 1 @intGuildID = GuildID
		  FROM dbo.Guilds WITH (NOLOCK)
		 WHERE GuildID > @intGuildID
		 ORDER BY GuildID;

		IF @@ROWCOUNT = 0 BREAK;

		UPDATE dbo.Guilds
		   SET GuildWarScheduleID = @insGuildWarScheduleID
			 , TeamColorCode = (@intGuildID % 2) + 1
		 WHERE GuildID = @intGuildID;
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

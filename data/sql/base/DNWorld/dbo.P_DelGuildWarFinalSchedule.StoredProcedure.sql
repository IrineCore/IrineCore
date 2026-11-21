USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DelGuildWarFinalSchedule]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : wgkim@eyedentitygames
created date : 2011-03-11

description :
    ??? ?? ?? ??
return value :
    0 = ??? ????.
    1 = ????? Commit? ? ?? ?????. ????? Rollback???.
    100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
    3 = ?? ??
**/

CREATE PROCEDURE [dbo].[P_DelGuildWarFinalSchedule]
	  @insGuildWarScheduleID smallint --// ??? ??
	, @inyMatchTypeCode tinyint --// 1=??, 2=4?, 3=8?, 4=16?
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

	/**_# ??? ?? ??? ?? ???. */
	DELETE dbo.GuildWarFinalSchedule
	WHERE GuildWarScheduleID = @insGuildWarScheduleID AND MatchTypeCode = @inyMatchTypeCode;

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

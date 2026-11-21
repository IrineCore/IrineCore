USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddGuildWarFinalSchedule]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : wgkim@eyedentitygames
created date : 2011-03-08

description :
    ??? ?? ?? ??
return value :
    0 = ??? ????.
    1 = ????? Commit? ? ?? ?????. ????? Rollback???.
    100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
	103312 = ??? ?? ??? ??? ????.
history :
    2 = ?? ??
	3 = ?? ?? ?? ??     by 20110727.wgkim
**/

CREATE PROCEDURE [dbo].[P_AddGuildWarFinalSchedule]
	  @insGuildWarScheduleID smallint --// ??? ??
	, @inyMatchTypeCode tinyint --// 1=??, 2=4?, 3=8?, 4=16?
	, @sdtMatchStartTime smalldatetime --// ?? ?? ??
	, @sdtMatchEndTime smalldatetime --// ?? ?? ??
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

	-- ?? ??
	IF (SELECT OpeningEndDate FROM dbo.GuildWarSchedule WITH (NOLOCK) WHERE GuildWarScheduleID = @insGuildWarScheduleID) > @sdtMatchStartTime
	OR (@sdtMatchStartTime > @sdtMatchEndTime)
	BEGIN
		SET @intReturnValue = 103312;
		GOTO ErrorHandler;
	END

	/**_# ???? ?? ??? ?? ?? ??? ?????. */
    UPDATE dbo.GuildWarFinalSchedule
       SET MatchStartTime = @sdtMatchStartTime
         , MatchEndTime = @sdtMatchEndTime
         , ChangeDate = GETDATE()
     WHERE GuildWarScheduleID = @insGuildWarScheduleID
       AND MatchTypeCode = @inyMatchTypeCode;

    IF @@ROWCOUNT = 0
    BEGIN
	    /**_# ?? ??? ??? ?? ??(?? ??)? ?????. */
        INSERT dbo.GuildWarFinalSchedule (GuildWarScheduleID, MatchTypeCode, MatchStartTime, MatchEndTime, RegisterDate, ChangeDate)
        VALUES (@insGuildWarScheduleID, @inyMatchTypeCode, @sdtMatchStartTime, @sdtMatchEndTime, GETDATE(), GETDATE());
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

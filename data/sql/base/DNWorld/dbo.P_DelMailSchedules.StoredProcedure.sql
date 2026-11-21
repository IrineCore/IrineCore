USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DelMailSchedules]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 8
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2011-05-23

description :
	???? ??
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
	101172 = ?? ??? ??? ???? ????.
history :
	3 = ?? ??
	4~5 = ??? ???? ???? ?? ??     by 20110923.wgkim
	6 = ??? ','? ??     by 20110926.wgkim
	7 = ?? ??? ??? ???? DeleteFlag ????? ??     by 20110929.wgkim
	8 = ?? ??? ??     by 20111011.wgkim
**/

CREATE PROCEDURE [dbo].[P_DelMailSchedules]
	  @intMailScheduleID int --// ????ID
	, @inbCharacterID bigint = NULL --// ???ID
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

	-- ???? ??
	IF @inbCharacterID IS NULL
	BEGIN
		UPDATE dbo.MailSchedules
		   SET DeleteFlag = 1
		 WHERE MailScheduleID = @intMailScheduleID;

	END
	-- ???? ???? ??
	ELSE IF @inbCharacterID IS NOT NULL
	BEGIN
		UPDATE dbo.MailSchedulesTarget
		   SET DeleteFlag = 1
		 WHERE MailScheduleID = @intMailScheduleID
		   AND CharacterID = @inbCharacterID;

		IF @@ROWCOUNT = 0
		BEGIN
			SET @intReturnValue = 101172;
			GOTO ErrorHandler;
		END
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

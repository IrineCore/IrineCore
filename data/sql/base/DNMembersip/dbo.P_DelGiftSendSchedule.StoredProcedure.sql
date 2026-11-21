USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_DelGiftSendSchedule]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_DelGiftSendSchedule]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_DelGiftSendSchedule] AS' 
END
GO
/**
version : 5
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2011-05-23

description :
	??? ?? ??
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
	101172 = ?? ??? ??? ???? ????.
history :
	1 = ?? ??
	2 = ??? ???? ???? ?? ??     by 20110923.wgkim
	3 = ??? ','? ??     by 20110926.wgkim
	4 = ???? ??? ??? ???? DeleteFlag ????? ??     by 20110929.wgkim
	5 = ?? ??? ??     by 20111011.wgkim
**/

ALTER PROCEDURE [dbo].[P_DelGiftSendSchedule]
	  @intGiftSendScheduleID int --// ?? ?? ID
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
		UPDATE dbo.GiftSendSchedule
		   SET DeleteFlag = 1
		 WHERE GiftSendScheduleID = @intGiftSendScheduleID;
	END
	-- ???? ???? ??
	ELSE IF @inbCharacterID IS NOT NULL
	BEGIN
		UPDATE dbo.GiftSendScheduleTarget
		   SET DeleteFlag = 1
		 WHERE GiftSendScheduleID = @intGiftSendScheduleID
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

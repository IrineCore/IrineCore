USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModEventFatigueSchedule]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2011-12-27
description : ??? ??? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103321 = ?? ID? ??? ?? ??? ????.
**/

CREATE PROCEDURE [dbo].[P_ModEventFatigueSchedule]
	  @intEventFatigueScheduleID int --// ??? ??? ??ID
	, @dtmGiveDate datetime = NULL --// ????
	, @intFatigueValue int = NULL --// ?? ???
	, @bitGiveFlag bit = NULL --// ??? ?? ??	
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
	/**_# [EventFatigueSchedule] ???? UPDATE ???.*/
	UPDATE dbo.EventFatigueSchedule SET
		GiveDate = ISNULL(@dtmGiveDate, GiveDate),
		FatigueValue = ISNULL(@intFatigueValue, FatigueValue),
		GiveFlag = ISNULL(@bitGiveFlag, GiveFlag)
	WHERE EventFatigueScheduleID = @intEventFatigueScheduleID;

	IF @@ROWCOUNT = 0 
	BEGIN
		SET @intReturnValue = 103321;
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

USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddFairyLevelLog]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-04-24
description : ?? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_AddFairyLevelLog]
	  @dt2LogDate datetime2(7) --// ??? ??
	, @inbFairyItemSerial bigint --// ?? ??? ???
	, @inyFairyLevelBefore tinyint --// ?? ?? ??
	, @inyFairyLevelCurrent tinyint --// ?? ?? ??
	, @intChangedStatus1 int --// ??? ?? ??1
	, @intChangedStatus2 int --// ??? ?? ??2
	, @intChangedStatus3 int --// ??? ?? ??3
	, @intChangedStatus4 int --// ??? ?? ??4
	, @intChangedStatus5 int --// ??? ?? ??5
	, @intChangedStatus6 int --// ??? ?? ??6
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @intLogTableSuffix int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	SET @intLogTableSuffix = dbo.FN_LogTableSuffix(@dt2LogDate);

	/**_# [FairyLevelLogs] ???? INSERT???.*/
	IF @intLogTableSuffix = 0
		INSERT dbo.FairyLevelLogs0 (LogDate, FairyItemSerial, FairyLevelBefore, FairyLevelCurrent, ChangedStatus1, ChangedStatus2, ChangedStatus3, ChangedStatus4, ChangedStatus5, ChangedStatus6)
		VALUES (@dt2LogDate, @inbFairyItemSerial, @inyFairyLevelBefore, @inyFairyLevelCurrent, @intChangedStatus1, @intChangedStatus2, @intChangedStatus3, @intChangedStatus4, @intChangedStatus5, @intChangedStatus6);

	ELSE IF @intLogTableSuffix = 1
		INSERT dbo.FairyLevelLogs1 (LogDate, FairyItemSerial, FairyLevelBefore, FairyLevelCurrent, ChangedStatus1, ChangedStatus2, ChangedStatus3, ChangedStatus4, ChangedStatus5, ChangedStatus6)
		VALUES (@dt2LogDate, @inbFairyItemSerial, @inyFairyLevelBefore, @inyFairyLevelCurrent, @intChangedStatus1, @intChangedStatus2, @intChangedStatus3, @intChangedStatus4, @intChangedStatus5, @intChangedStatus6);

	ELSE
		INSERT dbo.FairyLevelLogs2 (LogDate, FairyItemSerial, FairyLevelBefore, FairyLevelCurrent, ChangedStatus1, ChangedStatus2, ChangedStatus3, ChangedStatus4, ChangedStatus5, ChangedStatus6)
		VALUES (@dt2LogDate, @inbFairyItemSerial, @inyFairyLevelBefore, @inyFairyLevelCurrent, @intChangedStatus1, @intChangedStatus2, @intChangedStatus3, @intChangedStatus4, @intChangedStatus5, @intChangedStatus6);

END TRY
BEGIN CATCH
	GOTO ErrorHandler;
END CATCH;

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

IF @intReturnValue IS NULL OR @intReturnValue = 0
	EXEC @intReturnValue = dbo.P_AddErrorLog @intPROCID = @@PROCID;

RETURN @intReturnValue;
GO

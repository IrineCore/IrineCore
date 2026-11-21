USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListFaityLevelLogForETL]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-04-25
description : ?? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListFaityLevelLogForETL]
	  @dtmBasisDate datetime --// ????	  
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @intLogTableSuffix int = dbo.FN_LogTableSuffix(@dtmBasisDate);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	IF @intLogTableSuffix = 0
		SELECT LogDate, FairyItemSerial, FairyLevelBefore, FairyLevelCurrent, ChangedStatus1, ChangedStatus2, ChangedStatus3, ChangedStatus4, ChangedStatus5, ChangedStatus6
		FROM dbo.FairyLevelLogs0 WITH (NOLOCK)
		WHERE LogDate >= CAST(@dtmBasisDate AS datetime2(0)) AND LogDate < CAST(@dtmBasisDate + 1 AS datetime2(0))
		OPTION (MAXDOP 1);

	ELSE IF @intLogTableSuffix = 1
		SELECT LogDate, FairyItemSerial, FairyLevelBefore, FairyLevelCurrent, ChangedStatus1, ChangedStatus2, ChangedStatus3, ChangedStatus4, ChangedStatus5, ChangedStatus6
		FROM dbo.FairyLevelLogs1 WITH (NOLOCK)
		WHERE LogDate >= CAST(@dtmBasisDate AS datetime2(0)) AND LogDate < CAST(@dtmBasisDate + 1 AS datetime2(0))
		OPTION (MAXDOP 1);

	ELSE
		SELECT LogDate, FairyItemSerial, FairyLevelBefore, FairyLevelCurrent, ChangedStatus1, ChangedStatus2, ChangedStatus3, ChangedStatus4, ChangedStatus5, ChangedStatus6
		FROM dbo.FairyLevelLogs2 WITH (NOLOCK)
		WHERE LogDate >= CAST(@dtmBasisDate AS datetime2(0)) AND LogDate < CAST(@dtmBasisDate + 1 AS datetime2(0))
		OPTION (MAXDOP 1);;

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

USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListFieldLog]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2011-04-19

description :
	??? ??? ?????. (DailyETLJob)
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
	2 = ?? ??
	3~4 = ?? ??? ??     by 20110914.wgkim
**/

CREATE PROCEDURE [dbo].[P_GetListFieldLog]
	  @dtmBasisDate datetime --// ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @dt2LogDate datetime2(0), @intLogTableSuffix int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	SET @dt2LogDate = CONVERT(datetime2(0), CONVERT(date, @dtmBasisDate));
	SET @intLogTableSuffix = dbo.FN_LogTableSuffix(@dt2LogDate);

	IF @intLogTableSuffix = 0
		SELECT LogDate, FarmID, FieldIndex, CharacterID, ItemID, PlantDate, ElapsedTime, HarvestFlag
		  FROM dbo.FieldDeleteLogs0 WITH (NOLOCK)
		 WHERE LogDate >= CONVERT(datetime, @dt2LogDate) AND LogDate < DATEADD(DAY, 1, CONVERT(datetime, @dt2LogDate));

	ELSE IF @intLogTableSuffix = 1
		SELECT LogDate, FarmID, FieldIndex, CharacterID, ItemID, PlantDate, ElapsedTime, HarvestFlag
		  FROM dbo.FieldDeleteLogs1 WITH (NOLOCK)
		 WHERE LogDate >= CONVERT(datetime, @dt2LogDate) AND LogDate < DATEADD(DAY, 1, CONVERT(datetime, @dt2LogDate));

	ELSE
		SELECT LogDate, FarmID, FieldIndex, CharacterID, ItemID, PlantDate, ElapsedTime, HarvestFlag
		  FROM dbo.FieldDeleteLogs2 WITH (NOLOCK)
		 WHERE LogDate >= CONVERT(datetime, @dt2LogDate) AND LogDate < DATEADD(DAY, 1, CONVERT(datetime, @dt2LogDate));

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

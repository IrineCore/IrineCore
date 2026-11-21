USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListFieldAttachmentLog]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2011-09-14

description :
	???? ??? ??? ??? ?????. (DailyETLJob)
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
	1 = ?? ??
**/

CREATE PROCEDURE [dbo].[P_GetListFieldAttachmentLog]
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
		SELECT LogDate, FarmID, FieldIndex, CharacterID, Attachments, HarvestFlag
		  FROM dbo.FieldAttachmentDeleteLogs0 WITH (NOLOCK)
		 WHERE LogDate >= CONVERT(datetime, @dt2LogDate) AND LogDate < DATEADD(DAY, 1, CONVERT(datetime, @dt2LogDate));

	ELSE IF @intLogTableSuffix = 1
		SELECT LogDate, FarmID, FieldIndex, CharacterID, Attachments, HarvestFlag
		  FROM dbo.FieldAttachmentDeleteLogs1 (NOLOCK)
		 WHERE LogDate >= CONVERT(datetime, @dt2LogDate) AND LogDate < DATEADD(DAY, 1, CONVERT(datetime, @dt2LogDate));

	ELSE
		SELECT LogDate, FarmID, FieldIndex, CharacterID, Attachments, HarvestFlag
		  FROM dbo.FieldAttachmentDeleteLogs2 WITH (NOLOCK)
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

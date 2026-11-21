USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_DLS_GetListDNEvent]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_DLS_GetListDNEvent]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_DLS_GetListDNEvent] AS' 
END
GO
/**
version : 6
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2012-11-08

description :
	??? ??
result set : 
	DNEventID / int
	DNEventDetailID / int
	WorldID / tinyint
	EnableFlag / bit
	RegistrationDate / smalldatetime
	EventName / nvarchar(50)
	EventType1 / tinyint
	EventType2 / tinyint
	EventType3 / tinyint
	EventAtt1 / int
	EventAtt2 / int
	EventAtt3 / int
	EventAtt4 / int
	EventAtt5 / int
	DeleteFlag / bit
	EventStartDate / smalldatetime
	EventEndDate / smalldatetime
	DisplayFlag / bit
	MapIDs / nvarchar(100)
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
	4 = ?? ??
**/

ALTER PROCEDURE [dbo].[P_DLS_GetListDNEvent]
	  @inyEventScheduleCode tinyint = 0 --// ??? ?? ?? (0=??, 1=??, 2=???, 3=??)
	, @inyWorldID tinyint = NULL --// ?? ID (0=??)
	, @bitEnableFlag bit = NULL --// 0=???, 1=??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @dtsNow smalldatetime = GETDATE();

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	/**_# ??? ?? */
	IF @inyEventScheduleCode = 0
	BEGIN
		SELECT E.DNEventID, E.DNEventDetailID, E.WorldID, E.EnableFlag, E.RegistrationDate, ED.EventName, ED.EventType1, ED.EventType2, ED.EventType3, ED.EventAtt1, ED.EventAtt2, ED.EventAtt3, ED.EventAtt4, ED.EventAtt5, ED.DeleteFlag, MIN(ES.EventStartDate) AS EventStartDate, MAX(ES.EventEndDate) AS EventEndDate, E.DisplayFlag, ED.MapIDs
		  FROM dbo.DNEvents E WITH (NOLOCK)
		       INNER JOIN dbo.DNEventDetails ED WITH (NOLOCK) ON (E.DNEventDetailID = ED.DNEventDetailID)
			   INNER JOIN dbo.DNEventSchedules ES WITH (NOLOCK) ON (E.DNEventID = ES.DNEventID)
		 WHERE (@inyWorldID IS NULL OR WorldID = @inyWorldID)
		   AND (@bitEnableFlag IS NULL OR EnableFlag = @bitEnableFlag)
		 GROUP BY E.DNEventID, E.DNEventDetailID, E.WorldID, E.EnableFlag, E.RegistrationDate, ED.EventName, ED.EventType1, ED.EventType2, ED.EventType3, ED.EventAtt1, ED.EventAtt2, ED.EventAtt3, ED.EventAtt4, ED.EventAtt5, ED.DeleteFlag, E.DisplayFlag, ED.MapIDs
		 ORDER BY MIN(ES.EventStartDate);
	END
	ELSE IF @inyEventScheduleCode = 1
	BEGIN
		;WITH T AS (
			SELECT DISTINCT DNEventID
			  FROM dbo.DNEventSchedules WITH (NOLOCK)
			 WHERE EventEndDate > @dtsNow
			   AND EventStartDate > @dtsNow
		)
		SELECT E.DNEventID, E.DNEventDetailID, E.WorldID, E.EnableFlag, E.RegistrationDate, ED.EventName, ED.EventType1, ED.EventType2, ED.EventType3, ED.EventAtt1, ED.EventAtt2, ED.EventAtt3, ED.EventAtt4, ED.EventAtt5, ED.DeleteFlag, MIN(ES.EventStartDate) AS EventStartDate, MAX(ES.EventEndDate) AS EventEndDate, E.DisplayFlag, ED.MapIDs
		  FROM T
			   INNER JOIN dbo.DNEvents E WITH (NOLOCK) ON (T.DNEventID = E.DNEventID)
		       INNER JOIN dbo.DNEventDetails ED WITH (NOLOCK) ON (E.DNEventDetailID = ED.DNEventDetailID)
			   INNER JOIN dbo.DNEventSchedules ES WITH (NOLOCK) ON (E.DNEventID = ES.DNEventID)
		 WHERE (@inyWorldID IS NULL OR WorldID = @inyWorldID)
		   AND (@bitEnableFlag IS NULL OR EnableFlag = @bitEnableFlag)
		 GROUP BY E.DNEventID, E.DNEventDetailID, E.WorldID, E.EnableFlag, E.RegistrationDate, ED.EventName, ED.EventType1, ED.EventType2, ED.EventType3, ED.EventAtt1, ED.EventAtt2, ED.EventAtt3, ED.EventAtt4, ED.EventAtt5, ED.DeleteFlag, E.DisplayFlag, ED.MapIDs
		 ORDER BY MIN(ES.EventStartDate);
	END
	ELSE IF @inyEventScheduleCode = 2
	BEGIN
		;WITH T AS (
			SELECT DISTINCT DNEventID
			  FROM dbo.DNEventSchedules WITH (NOLOCK)
			 WHERE EventEndDate > @dtsNow
			   AND EventStartDate <= @dtsNow
		)
		SELECT E.DNEventID, E.DNEventDetailID, E.WorldID, E.EnableFlag, E.RegistrationDate, ED.EventName, ED.EventType1, ED.EventType2, ED.EventType3, ED.EventAtt1, ED.EventAtt2, ED.EventAtt3, ED.EventAtt4, ED.EventAtt5, ED.DeleteFlag, MIN(ES.EventStartDate) AS EventStartDate, MAX(ES.EventEndDate) AS EventEndDate, E.DisplayFlag, ED.MapIDs
		  FROM T
			   INNER JOIN dbo.DNEvents E WITH (NOLOCK) ON (T.DNEventID = E.DNEventID)
		       INNER JOIN dbo.DNEventDetails ED WITH (NOLOCK) ON (E.DNEventDetailID = ED.DNEventDetailID)
			   INNER JOIN dbo.DNEventSchedules ES WITH (NOLOCK) ON (E.DNEventID = ES.DNEventID)
		 WHERE (@inyWorldID IS NULL OR WorldID = @inyWorldID)
		   AND (@bitEnableFlag IS NULL OR EnableFlag = @bitEnableFlag)
		 GROUP BY E.DNEventID, E.DNEventDetailID, E.WorldID, E.EnableFlag, E.RegistrationDate, ED.EventName, ED.EventType1, ED.EventType2, ED.EventType3, ED.EventAtt1, ED.EventAtt2, ED.EventAtt3, ED.EventAtt4, ED.EventAtt5, ED.DeleteFlag, E.DisplayFlag, ED.MapIDs
		 ORDER BY MIN(ES.EventStartDate);
	END
	ELSE IF @inyEventScheduleCode = 3
	BEGIN
		;WITH T AS (
			SELECT DISTINCT DNEventID
			  FROM dbo.DNEventSchedules WITH (NOLOCK)
			 WHERE EventEndDate <= @dtsNow
		)
		SELECT E.DNEventID, E.DNEventDetailID, E.WorldID, E.EnableFlag, E.RegistrationDate, ED.EventName, ED.EventType1, ED.EventType2, ED.EventType3, ED.EventAtt1, ED.EventAtt2, ED.EventAtt3, ED.EventAtt4, ED.EventAtt5, ED.DeleteFlag, MIN(ES.EventStartDate) AS EventStartDate, MAX(ES.EventEndDate) AS EventEndDate, E.DisplayFlag, ED.MapIDs
		  FROM T
			   INNER JOIN dbo.DNEvents E WITH (NOLOCK) ON (T.DNEventID = E.DNEventID)
		       INNER JOIN dbo.DNEventDetails ED WITH (NOLOCK) ON (E.DNEventDetailID = ED.DNEventDetailID)
			   INNER JOIN dbo.DNEventSchedules ES WITH (NOLOCK) ON (E.DNEventID = ES.DNEventID)
		 WHERE (@inyWorldID IS NULL OR WorldID = @inyWorldID)
		   AND (@bitEnableFlag IS NULL OR EnableFlag = @bitEnableFlag)
		 GROUP BY E.DNEventID, E.DNEventDetailID, E.WorldID, E.EnableFlag, E.RegistrationDate, ED.EventName, ED.EventType1, ED.EventType2, ED.EventType3, ED.EventAtt1, ED.EventAtt2, ED.EventAtt3, ED.EventAtt4, ED.EventAtt5, ED.DeleteFlag, E.DisplayFlag, ED.MapIDs
		 ORDER BY MIN(ES.EventStartDate);
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

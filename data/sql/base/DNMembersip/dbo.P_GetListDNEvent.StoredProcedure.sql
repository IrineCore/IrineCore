USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListDNEvent]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetListDNEvent]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetListDNEvent] AS' 
END
GO
/**
version : 9
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2012-11-08

description :
	??? ?? ??
result set : 
	@intVersion = 1 : DNEventDetailID/int, WorldID/tinyint, EventStartDate/smalldatetime, EventEndDate/smalldatetime, EventType1/tinyint, EventType2/tinyint, EventType3/tinyint, EventAtt1/int, EventAtt2/int, EventAtt3/int, EventAtt4/int, EventAtt5/int
	@intVersion = 6 : DNEventDetailID/int, WorldID/tinyint, EventStartDate/smalldatetime, EventEndDate/smalldatetime, EventType1/tinyint, EventType2/tinyint, EventType3/tinyint, EventAtt1/int, EventAtt2/int, EventAtt3/int, EventAtt4/int, EventAtt5/int, DisplayFlag/bit
	@intVersion = 9 : DNEventDetailID/int, WorldID/tinyint, EventStartDate/smalldatetime, EventEndDate/smalldatetime, EventType1/tinyint, EventType2/tinyint, EventType3/tinyint, EventAtt1/int, EventAtt2/int, EventAtt3/int, EventAtt4/int, EventAtt5/int, DisplayFlag/bit, MapIDs/nvarchar(100)
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
	5 = ?? ??
	6 = result set ?? (DisplayFlag), intput parameter ?? (@intVersion)     by ??? at 2013-03-26
	8 = result set ? DisplayFlag ?? ??     by ??? at 2013-05-08
**/

ALTER PROCEDURE [dbo].[P_GetListDNEvent]
	  @intVersion int = 1 --// ??
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

	/**_# ??? ?? ?? */

	IF @intVersion = 1
	BEGIN
		SELECT E.DNEventDetailID, E.WorldID, DS.EventStartDate, DS.EventEndDate
			 , ED.EventType1, ED.EventType2, ED.EventType3, ED.EventAtt1, ED.EventAtt2, ED.EventAtt3, ED.EventAtt4, ED.EventAtt5
		  FROM dbo.DNEventSchedules DS WITH (NOLOCK)
			   INNER JOIN dbo.DNEvents E WITH (NOLOCK) ON (DS.DNEventID = E.DNEventID)
			   INNER JOIN dbo.DNEventDetails ED WITH (NOLOCK) ON (E.DNEventDetailID = ED.DNEventDetailID)
		 WHERE DS.EventEndDate >= GETDATE()
		   AND DS.EventStartDate <= DATEADD(DAY, 10, GETDATE())
		   AND E.EnableFlag = CONVERT(BIT, 1);
	END
	ELSE IF @intVersion = 6
	BEGIN
		SELECT E.DNEventDetailID, E.WorldID, DS.EventStartDate, DS.EventEndDate
			 , ED.EventType1, ED.EventType2, ED.EventType3, ED.EventAtt1, ED.EventAtt2, ED.EventAtt3, ED.EventAtt4, ED.EventAtt5, E.DisplayFlag
		  FROM dbo.DNEventSchedules DS WITH (NOLOCK)
			   INNER JOIN dbo.DNEvents E WITH (NOLOCK) ON (DS.DNEventID = E.DNEventID)
			   INNER JOIN dbo.DNEventDetails ED WITH (NOLOCK) ON (E.DNEventDetailID = ED.DNEventDetailID)
		 WHERE DS.EventEndDate >= GETDATE()
		   AND DS.EventStartDate <= DATEADD(DAY, 10, GETDATE())
		   AND E.EnableFlag = CONVERT(BIT, 1);
	END
	ELSE IF @intVersion = 9
	BEGIN
		SELECT E.DNEventDetailID, E.WorldID, DS.EventStartDate, DS.EventEndDate
			 , ED.EventType1, ED.EventType2, ED.EventType3, ED.EventAtt1, ED.EventAtt2, ED.EventAtt3, ED.EventAtt4, ED.EventAtt5, E.DisplayFlag, ED.MapIDs
		  FROM dbo.DNEventSchedules DS WITH (NOLOCK)
			   INNER JOIN dbo.DNEvents E WITH (NOLOCK) ON (DS.DNEventID = E.DNEventID)
			   INNER JOIN dbo.DNEventDetails ED WITH (NOLOCK) ON (E.DNEventDetailID = ED.DNEventDetailID)
		 WHERE DS.EventEndDate >= GETDATE()
		   AND DS.EventStartDate <= DATEADD(DAY, 10, GETDATE())
		   AND E.EnableFlag = CONVERT(BIT, 1);
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

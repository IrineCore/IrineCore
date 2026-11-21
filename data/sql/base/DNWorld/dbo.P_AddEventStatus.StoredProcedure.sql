USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddEventStatus]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-02-17
description : ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_AddEventStatus]
	  @intEventID int = NULL --// ??? ID
	, @intWorldID int --// ?? ID
	, @nvcMapIDs nvarchar(50) --// ? ID
	, @dtmEventStartDate datetime --// ??? ?? ??
	, @dtmEventEndDate datetime --// ??? ?? ??
	, @intEventType1 int --// ??? ??1
	, @intEventType2 int --// ??? ??2
	, @intEventType3 int --// ??? ??3
	, @intEventAtt1 int --// ??? ??1
	, @intEventAtt2 int --// ??? ??2
	, @intEventAtt3 int --// ??? ??3
	, @intEventAtt4 int --// ??? ??4
	, @intEventAtt5 int --// ??? ??5
	, @intEventGroupID int = NULL --// ?????ID
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
	/**_# [EventStatus] ???? INSERT???.*/
	INSERT dbo.EventStatus (EventID, WorldID, MapIDs, EventStartDate, EventEndDate, EventType1, EventType2, EventType3, EventAtt1, EventAtt2, EventAtt3, EventAtt4, EventAtt5, EventGroupID)
	VALUES (@intEventID, @intWorldID, @nvcMapIDs, @dtmEventStartDate, @dtmEventEndDate, @intEventType1, @intEventType2, @intEventType3, @intEventAtt1, @intEventAtt2, @intEventAtt3, @intEventAtt4, @intEventAtt5, @intEventGroupID);
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

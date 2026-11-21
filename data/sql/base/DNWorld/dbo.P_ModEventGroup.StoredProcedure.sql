USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModEventGroup]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2011-07-06
description : ??? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_ModEventGroup]
	  @intEventGroupID int --// ??? ?? ID
	, @nvcEventName nvarchar(200) = NULL --// ????
	, @dtmEventStartDate datetime = NULL --// ??? ????
	, @dtmEventEndDate datetime = NULL --// ??? ????
	, @nvcEventDescription nvarchar(max) = NULL --// ??? ??
	
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
	/**_# [EventGroups] ???? ?????.*/
	IF NOT EXISTS ( SELECT * FROM dbo.EventGroups WITH (NOLOCK) WHERE EventGroupID = @intEventGroupID )
	BEGIN
		SET @intReturnValue = 103177;
		GOTO ErrorHandler;
	END
	
	UPDATE dbo.EventGroups SET
		EventName = COALESCE(@nvcEventName, EventName),
		EventDescription = COALESCE(@nvcEventDescription, EventDescription),
		EventStartDate = COALESCE(@dtmEventStartDate, EventStartDate),
		EventEndDate = COALESCE(@dtmEventEndDate, EventEndDate)
	WHERE EventGroupID = @intEventGroupID;

	DELETE dbo.EventStatus WHERE EventGroupID = @intEventGroupID;
		
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

USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_AddEvent]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-05-03
description : ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_AddEvent]
	  @nvcEventName nvarchar(50) --// ??? ??
	, @nvcEventMemo nvarchar(max) --// ??? ??, ??
	, @datEventStartDate date --// ??? ?? ??
	, @datEventEndDate date --// ??? ?? ??
	, @intEventID int = NULL OUTPUT --// ??? ID
WITH EXECUTE AS 'SPExecutor'
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
	/**_# [Events] ???? INSERT???.*/
	INSERT dbo.Events (EventName, EventMemo, EventStartDate, EventEndDate, RegisterDate)
	VALUES (@nvcEventName, @nvcEventMemo, @datEventStartDate, @datEventEndDate, GETDATE());

	SET @intEventID = SCOPE_IDENTITY();
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

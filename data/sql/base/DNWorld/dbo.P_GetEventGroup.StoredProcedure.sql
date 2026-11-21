USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetEventGroup]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2011-07-08
description : ??? ?? ??? ?????.
result set : EventGroupID/int, EventName/nvarchar, EventStartDate/datetime, EventEndDate/datetime, EventDescription/nvarchar, RegisterID/int, RegisteDate/datetime
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103177 = ??? ???? ????.
**/

CREATE PROCEDURE [dbo].[P_GetEventGroup]
	  @intEventGroupID int --// ??? ?? ID
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

	SELECT EventGroupID, EventName, EventStartDate, EventEndDate, EventDescription, RegisterID, RegisteDate 
	  FROM dbo.EventGroups WITH (NOLOCK) 
	 WHERE EventGroupID = @intEventGroupID

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103177;
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

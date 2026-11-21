USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListEventStatus]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-31
description : ??? ??? ?????.
result set : EventID/int, WorldID/int, MapIDs/int, EventStartDate/datetime, EventEndDate/datetime, EventType1/int, EventType2/int, EventType3/int, EventAtt1/int, EventAtt2/int, EventAtt3/int, EventAtt4/int, EventAtt5/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103177 = ??? ???? ????.
**/

CREATE PROCEDURE [dbo].[P_GetListEventStatus]
	  @intWorldID int --// ?? ID
    , @intEventGroupID int = NULL --// ??? ?? ID
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
	IF @intEventGroupID IS NULL
		SELECT EventID, WorldID, MapIDs, EventStartDate, EventEndDate, EventType1, EventType2, EventType3, EventAtt1, EventAtt2, EventAtt3, EventAtt4, EventAtt5
		FROM dbo.EventStatus WITH (NOLOCK);
	--	WHERE WorldID = 0 OR WorldID = @intWorldID;
	ELSE
		SELECT EventID, WorldID, MapIDs, EventStartDate, EventEndDate, EventType1, EventType2, EventType3, EventAtt1, EventAtt2, EventAtt3, EventAtt4, EventAtt5
		FROM dbo.EventStatus WITH (NOLOCK)
		WHERE EventGroupID = @intEventGroupID;

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

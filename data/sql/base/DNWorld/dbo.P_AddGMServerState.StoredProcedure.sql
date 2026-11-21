USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddGMServerState]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-02-02
description : 
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_AddGMServerState]
	  @intProcessIndex int --// 
	, @insProcessGubun smallint --// 
	, @intNetLauncherID int --// 
	, @nvcNetLauncherStatus nvarchar(30) --// 
	, @intPartitionID int --// 
	, @nvcProcessType nvarchar(30) --// 
	, @intAssignedLauncherID int --// 
	, @nvcPStatus nvarchar(30) --// 
	, @nvcSStatus nvarchar(30) --// 
	, @nvcVersion nvarchar(30) --// 
	, @nvcResource nvarchar(30) --// 
	, @intSID int --// 
	, @intUserCount int --// 
	, @intRoomCount int --// 
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
	/**_# [GMServerState] ???? INSERT???.*/
	INSERT dbo.GMServerState (ProcessIndex, ProcessGubun, NetLauncherID, NetLauncherStatus, PartitionID, ProcessType, AssignedLauncherID, PStatus, SStatus, [Version], [Resource], [SID], UserCount, RoomCount)
	VALUES (@intProcessIndex, @insProcessGubun, @intNetLauncherID, @nvcNetLauncherStatus, @intPartitionID, @nvcProcessType, @intAssignedLauncherID, @nvcPStatus, @nvcSStatus, @nvcVersion, @nvcResource, @intSID, @intUserCount, @intRoomCount);
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

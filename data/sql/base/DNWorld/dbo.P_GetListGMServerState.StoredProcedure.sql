USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListGMServerState]    Script Date: 11/14/2026 8:54:38 AM ******/
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
result set : 
	@intSelectClauseCode = 1 : ProcessIndex, ProcessGubun, NetLauncherID, NetLauncherStatus, PartitionID, ProcessType, AssignedLauncherID, PStatus, SStatus, Version, Resource, SID, UserCount, RoomCount
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
102110 = ???? ???? ????. [GMServerState]
**/

CREATE PROCEDURE [dbo].[P_GetListGMServerState]
	  @intSelectClauseCode int --// ?? ?? ??.
	, @intProcessIndex int = NULL --// 
	, @insProcessGubun smallint = NULL --// 
	, @intNetLauncherID int = NULL --// 
	, @nvcNetLauncherStatus nvarchar(30) = NULL --// 
	, @intPartitionID int = NULL --// 
	, @nvcProcessType nvarchar(30) = NULL --// 
	, @intAssignedLauncherID int = NULL --// 
	, @nvcPStatus nvarchar(30) = NULL --// 
	, @nvcSStatus nvarchar(30) = NULL --// 
	, @nvcVersion nvarchar(30) = NULL --// 
	, @nvcResource nvarchar(30) = NULL --// 
	, @intSID int = NULL --// 
	, @intUserCount int = NULL --// 
	, @intRoomCount int = NULL --// 
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE
	@intReturnValue int,
	@nvcStmt nvarchar(max),
	@nvcSelectStmt nvarchar(max),
	@inbNullBitmap bigint,
	@intColumnCount int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# ?? ??? ?????. (SELECT ?).*/
	SET @nvcSelectStmt = 
		CASE
			WHEN @intSelectClauseCode = 1 THEN N'ProcessIndex, ProcessGubun, NetLauncherID, NetLauncherStatus, PartitionID, ProcessType, AssignedLauncherID, PStatus, SStatus, Version, Resource, SID, UserCount, RoomCount'
		END;

	SET @intColumnCount = 13;

	SET @inbNullBitmap = 
		CASE WHEN @intProcessIndex IS NOT NULL THEN POWER(2, 0) ELSE 0 END + 
		CASE WHEN @insProcessGubun IS NOT NULL THEN POWER(2, 1) ELSE 0 END + 
		CASE WHEN @intNetLauncherID IS NOT NULL THEN POWER(2, 2) ELSE 0 END + 
		CASE WHEN @nvcNetLauncherStatus IS NOT NULL THEN POWER(2, 3) ELSE 0 END + 
		CASE WHEN @intPartitionID IS NOT NULL THEN POWER(2, 4) ELSE 0 END + 
		CASE WHEN @nvcProcessType IS NOT NULL THEN POWER(2, 5) ELSE 0 END + 
		CASE WHEN @intAssignedLauncherID IS NOT NULL THEN POWER(2, 6) ELSE 0 END + 
		CASE WHEN @nvcPStatus IS NOT NULL THEN POWER(2, 7) ELSE 0 END + 
		CASE WHEN @nvcSStatus IS NOT NULL THEN POWER(2, 8) ELSE 0 END + 
		CASE WHEN @nvcVersion IS NOT NULL THEN POWER(2, 9) ELSE 0 END + 
		CASE WHEN @nvcResource IS NOT NULL THEN POWER(2, 10) ELSE 0 END + 
		CASE WHEN @intSID IS NOT NULL THEN POWER(2, 11) ELSE 0 END + 
		CASE WHEN @intUserCount IS NOT NULL THEN POWER(2, 12) ELSE 0 END + 
		CASE WHEN @intRoomCount IS NOT NULL THEN POWER(2, 13) ELSE 0 END;

	/**_# ?? ??? ?????. (FROM, WHERE ?).*/
	SET @nvcStmt = N'
SELECT ' + @nvcSelectStmt + N'
FROM dbo.GMServerState WITH (NOLOCK)
'		+ dbo.FN_DynamicWhereClause2(N'ProcessIndex', N'@intProcessIndex', @inbNullBitmap, 1, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'ProcessGubun', N'@insProcessGubun', @inbNullBitmap, 2, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'NetLauncherID', N'@intNetLauncherID', @inbNullBitmap, 3, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'NetLauncherStatus', N'@nvcNetLauncherStatus', @inbNullBitmap, 4, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'PartitionID', N'@intPartitionID', @inbNullBitmap, 5, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'ProcessType', N'@nvcProcessType', @inbNullBitmap, 6, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'AssignedLauncherID', N'@intAssignedLauncherID', @inbNullBitmap, 7, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'PStatus', N'@nvcPStatus', @inbNullBitmap, 8, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'SStatus', N'@nvcSStatus', @inbNullBitmap, 9, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'Version', N'@nvcVersion', @inbNullBitmap, 10, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'Resource', N'@nvcResource', @inbNullBitmap, 11, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'SID', N'@intSID', @inbNullBitmap, 12, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'UserCount', N'@intUserCount', @inbNullBitmap, 13, @intColumnCount, '=')		+ dbo.FN_DynamicWhereClause2(N'RoomCount', N'@intRoomCount', @inbNullBitmap, 14, @intColumnCount, '=')
 + N'
ORDER BY ProcessIndex';

	/**_# ?? ??? ?????.*/
	EXEC dbo.sp_executesql @nvcStmt
		, N'@intProcessIndex int, @insProcessGubun smallint, @intNetLauncherID int, @nvcNetLauncherStatus nvarchar(30), @intPartitionID int, @nvcProcessType nvarchar(30), @intAssignedLauncherID int, @nvcPStatus nvarchar(30), @nvcSStatus nvarchar(30), @nvcVersion nvarchar(30), @nvcResource nvarchar(30), @intSID int, @intUserCount int, @intRoomCount int'
		, @intProcessIndex = @intProcessIndex, @insProcessGubun = @insProcessGubun, @intNetLauncherID = @intNetLauncherID, @nvcNetLauncherStatus = @nvcNetLauncherStatus, @intPartitionID = @intPartitionID, @nvcProcessType = @nvcProcessType, @intAssignedLauncherID = @intAssignedLauncherID, @nvcPStatus = @nvcPStatus, @nvcSStatus = @nvcSStatus, @nvcVersion = @nvcVersion, @nvcResource = @nvcResource, @intSID = @intSID, @intUserCount = @intUserCount, @intRoomCount = @intRoomCount;

	/**_## ??? ???? ?? ???? ???, 102110? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 102110;
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

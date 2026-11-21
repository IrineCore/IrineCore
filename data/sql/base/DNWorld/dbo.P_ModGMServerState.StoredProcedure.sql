USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModGMServerState]    Script Date: 11/14/2026 8:54:39 AM ******/
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
102110 = ???? ???? ????. [GMServerState]
**/

CREATE PROCEDURE [dbo].[P_ModGMServerState]
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

DECLARE
	  @nvcTableName nvarchar(128)		-- The name of table.
	, @inyOperationCode tinyint			-- 1=Create, 2=Retrieve, 3=Update, 4=Delete.
	, @nvcColumnNames nvarchar(max)		-- The names of the columns affected by a operation. It was delimitated by [|].
	, @nvcBeforeImages nvarchar(max)	-- The contents of the columns before operation. It was delimitated by [|].
	, @dtmOccurredDate datetime;		-- The date the modification occurred.

DECLARE @tblBeforeImages table (beforeImage nvarchar(max));

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# AUDIT? ?? ???? ?? ???? ?????.*/
	SET @nvcTableName = N'dbo.GMServerState';
	SET @inyOperationCode = 3;
	SET @nvcColumnNames = N'ProcessGubun|NetLauncherID|NetLauncherStatus|PartitionID|ProcessType|AssignedLauncherID|PStatus|SStatus|Version|Resource|SID|UserCount|RoomCount';
	SET @dtmOccurredDate = GETDATE();

	BEGIN TRANSACTION;
		/**_# ?? UPDATE?? ?? ? ???? ??? ??? ?? ?????.*/
		UPDATE dbo.GMServerState
		SET ProcessGubun = @insProcessGubun
			, NetLauncherID = @intNetLauncherID
			, NetLauncherStatus = @nvcNetLauncherStatus
			, PartitionID = @intPartitionID
			, ProcessType = @nvcProcessType
			, AssignedLauncherID = @intAssignedLauncherID
			, PStatus = @nvcPStatus
			, SStatus = @nvcSStatus
			, [Version] = @nvcVersion
			, [Resource] = @nvcResource
			, [SID] = @intSID
			, UserCount = @intUserCount
			, RoomCount = @intRoomCount
		OUTPUT ISNULL(CAST(DELETED.ProcessGubun AS nvarchar(max)), N'[NULL]') + N'|' + ISNULL(CAST(DELETED.NetLauncherID AS nvarchar(max)), N'[NULL]') + N'|' + ISNULL(CAST(DELETED.NetLauncherStatus AS nvarchar(max)), N'[NULL]') + N'|' + ISNULL(CAST(DELETED.PartitionID AS nvarchar(max)), N'[NULL]') + N'|' + ISNULL(CAST(DELETED.ProcessType AS nvarchar(max)), N'[NULL]') + N'|' + ISNULL(CAST(DELETED.AssignedLauncherID AS nvarchar(max)), N'[NULL]') + N'|' + ISNULL(CAST(DELETED.PStatus AS nvarchar(max)), N'[NULL]') + N'|' + ISNULL(CAST(DELETED.SStatus AS nvarchar(max)), N'[NULL]') + N'|' + ISNULL(CAST(DELETED.Version AS nvarchar(max)), N'[NULL]') + N'|' + ISNULL(CAST(DELETED.Resource AS nvarchar(max)), N'[NULL]') + N'|' + ISNULL(CAST(DELETED.SID AS nvarchar(max)), N'[NULL]') + N'|' + ISNULL(CAST(DELETED.UserCount AS nvarchar(max)), N'[NULL]') + N'|' + ISNULL(CAST(DELETED.RoomCount AS nvarchar(max)), N'[NULL]')
			INTO @tblBeforeImages
		WHERE ProcessIndex = @intProcessIndex;

		/**_## UPDATE? ?? ???, 102110? ?????.*/
		IF @@ROWCOUNT = 0
		BEGIN
			SET @intReturnValue = 102110;
			GOTO ErrorHandler;
		END

		/**_# ?? ? ???? [TableAuditLogs] ???? INSERT???.*/
		SELECT @nvcBeforeImages = beforeImage FROM @tblBeforeImages;

		EXEC @intReturnValue = dbo.P_AddAuditLog
			  @nvcTableName = @nvcTableName
			, @inyOperationCode = @inyOperationCode
			, @nvcColumnNames = @nvcColumnNames
			, @nvcBeforeImages = @nvcBeforeImages
			, @nvcModifier = N''
			, @dtmOccurredDate = @dtmOccurredDate;

		IF @intReturnValue <> 0 GOTO ErrorHandler;
	COMMIT TRANSACTION;
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

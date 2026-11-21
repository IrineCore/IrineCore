USE [dngsm]
GO
/****** Object:  StoredProcedure [dbo].[P_DelGMServerState]    Script Date: 11/14/2026 8:47:09 AM ******/
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

CREATE PROCEDURE [dbo].[P_DelGMServerState]
	  @intProcessIndex int --// 
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
	SET @inyOperationCode = 4;
	SET @nvcColumnNames = N'ProcessIndex|ProcessGubun|NetLauncherID|NetLauncherStatus|PartitionID|ProcessType|AssignedLauncherID|PStatus|SStatus|Version|Resource|SID|UserCount|RoomCount';
	SET @dtmOccurredDate = GETDATE();

	BEGIN TRANSACTION;
		/**_# ?? ????, ?? ? ???? ??? ??? ?? ?????.*/
		DELETE dbo.GMServerState
		OUTPUT ISNULL(CAST(DELETED.ProcessIndex AS nvarchar(max)), N'[NULL]') + N'|' + ISNULL(CAST(DELETED.ProcessGubun AS nvarchar(max)), N'[NULL]') + N'|' + ISNULL(CAST(DELETED.NetLauncherID AS nvarchar(max)), N'[NULL]') + N'|' + ISNULL(CAST(DELETED.NetLauncherStatus AS nvarchar(max)), N'[NULL]') + N'|' + ISNULL(CAST(DELETED.PartitionID AS nvarchar(max)), N'[NULL]') + N'|' + ISNULL(CAST(DELETED.ProcessType AS nvarchar(max)), N'[NULL]') + N'|' + ISNULL(CAST(DELETED.AssignedLauncherID AS nvarchar(max)), N'[NULL]') + N'|' + ISNULL(CAST(DELETED.PStatus AS nvarchar(max)), N'[NULL]') + N'|' + ISNULL(CAST(DELETED.SStatus AS nvarchar(max)), N'[NULL]') + N'|' + ISNULL(CAST(DELETED.Version AS nvarchar(max)), N'[NULL]') + N'|' + ISNULL(CAST(DELETED.Resource AS nvarchar(max)), N'[NULL]') + N'|' + ISNULL(CAST(DELETED.SID AS nvarchar(max)), N'[NULL]') + N'|' + ISNULL(CAST(DELETED.UserCount AS nvarchar(max)), N'[NULL]') + N'|' + ISNULL(CAST(DELETED.RoomCount AS nvarchar(max)), N'[NULL]')
			INTO @tblBeforeImages
		WHERE ProcessIndex = @intProcessIndex;

		/**_## ??? ?? ???, 102110? ?????.*/
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

USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddAuditLog]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
author : doeyull.kim
e-mail : purumae@gmail.com
created date : 2008-05-01
description : Inserts into the [TableAuditLogs] table.
return value :
1 = The transaction is in an uncommittable state. Rolling back transaction.
100 = System error occurred.
**/

CREATE PROCEDURE [dbo].[P_AddAuditLog]
	@nvcTableName nvarchar(128), --// ??? ??
	@inyOperationCode tinyint, --// 1=Create, 2=Retrieve, 3=Update, 4=Delete.
	@nvcColumnNames nvarchar(max), --// ?? ??
	@nvcBeforeImages nvarchar(max), --// ?? ? ??? ???
	@nvcModifier nvarchar(50), --// ??? ??? ???
	@dtmOccurredDate datetime --// ?? ??
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

/**_# Register the modified data.*/
BEGIN TRY
	INSERT dbo.TableAuditLogs (tableName, operationCode, columnNames, beforeImages, modifier, occurredDate)
	VALUES (@nvcTableName, @inyOperationCode, @nvcColumnNames, @nvcBeforeImages, @nvcModifier, @dtmOccurredDate);
END TRY
BEGIN CATCH
	GOTO ErrorHandler;
END CATCH

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

IF @intReturnValue IS NULL OR @intReturnValue = 0
	EXEC @intReturnValue = dbo.P_AddAuditLog;

RETURN @intReturnValue;
GO

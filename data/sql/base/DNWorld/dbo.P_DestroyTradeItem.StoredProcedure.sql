USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DestroyTradeItem]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-11-18
description : ??? ???? ?? ???? ?????. (??? ??)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103287 = ??? ???? ?? ??? ???????. ?? & ??? ? ????.
**/

CREATE PROCEDURE [dbo].[P_DestroyTradeItem]
	  @intTradeID int --// ?? ID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @inbItemSerial bigint, @inyTradeStatusCode tinyint;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	BEGIN TRANSACTION;

	SELECT @inbItemSerial = ItemSerial, @inyTradeStatusCode = TradeStatusCode
	FROM dbo.Trade WITH (UPDLOCK)
	WHERE TradeID = @intTradeID;
	IF @@ROWCOUNT = 0 OR @inyTradeStatusCode <> 0
	BEGIN
		SET @intReturnValue = 103287;
		GOTO ErrorHandler;
	END

	EXEC @intReturnValue = dbo.P_DelMaterializedItem
		  @inbItemSerial = @inbItemSerial
		, @intAuditLogID = 0;

	IF @intReturnValue <> 0
		GOTO ErrorHandler;

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

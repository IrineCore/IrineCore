USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_SetRollbackPurchaseOrderResult]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_SetRollbackPurchaseOrderResult]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_SetRollbackPurchaseOrderResult] AS' 
END
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-02-19
description : ?? ?? ?? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101144 = @inyOrderStatusCode ? ??
101145 = ??? ?? ??? ???? ????.
**/

ALTER PROCEDURE [dbo].[P_SetRollbackPurchaseOrderResult]
	  @inbPurchaseOrderID bigint --// ??? ?? ?? ID
	, @inyOrderStatusCode tinyint --// 5=?? ?? ??, 6=?? ?? ??
	, @intOrderRollbackResult int --// ????? ??? ?? ?? ?? ?
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
	IF @inyOrderStatusCode NOT IN (5, 6)
	BEGIN
		SET @intReturnValue = 101144;
		GOTO ErrorHandler;
	END

	UPDATE dbo.PurchaseOrders
	SET OrderStatusCode = @inyOrderStatusCode
		, OrderRollbackResult = @intOrderRollbackResult
	WHERE PurchaseOrderID = @inbPurchaseOrderID AND OrderStatusCode = 4;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 101145;
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

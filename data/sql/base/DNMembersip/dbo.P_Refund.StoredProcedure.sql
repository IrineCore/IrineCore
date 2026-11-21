USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_Refund]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_Refund]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_Refund] AS' 
END
GO
/**
version : 4
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-11-04
description : ?? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101182 = ?? ??? ??? ???? ????.
101183 = ?? ?? ?? ??? ???????.
**/

ALTER PROCEDURE [dbo].[P_Refund]
	  @inbPurchaseOrderDetailID bigint --// ?? ?? ?? ID
	, @intRefundLimitPeriod int --// ?? ??? ??? ??. ?? : Day
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dt2OrderDate datetime2(0)
	, @intProductID int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	BEGIN TRANSACTION;

	SELECT @dt2OrderDate = PO.OrderDate
		, @intProductID = PD.ProductID
	FROM dbo.PurchaseOrders PO WITH (NOLOCK)
		INNER JOIN dbo.PurchaseOrderDetails PD WITH (UPDLOCK) ON PO.PurchaseOrderID = PD.PurchaseOrderID
	WHERE PD.PurchaseOrderDetailID = @inbPurchaseOrderDetailID
		AND PD.RefundableFlag = CAST(1 AS bit)
		AND PD.RefundFlag = CAST(0 AS bit);

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 101182;
		GOTO ErrorHandler;
	END

	IF DATEADD(MINUTE, 5, DATEADD(DAY, @intRefundLimitPeriod, @dt2OrderDate)) < GETDATE()
	BEGIN
		SET @intReturnValue = 101183;
		GOTO ErrorHandler;
	END

	UPDATE dbo.PurchaseOrderDetails
	SET RefundableFlag = 0
		, RefundFlag = 1
		, RefundDate = GETDATE()
	WHERE PurchaseOrderDetailID = @inbPurchaseOrderDetailID;

	UPDATE dbo.LimitedProducts
	SET SaleCount -= 1
	WHERE ProductID = @intProductID;

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

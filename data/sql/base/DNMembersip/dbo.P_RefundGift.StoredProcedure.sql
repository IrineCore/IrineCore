USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_RefundGift]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_RefundGift]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_RefundGift] AS' 
END
GO
/**
version : 5
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-08-25
description : ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101169 = ??? ?? ??? ????.
history :
2 = ?? ??? ?? ??? ?????. at 2010-10-11
3 = ?? ?? ?? ?? at 2010-11-02
4 = RefundFlag, RefundDate ??? ??? ?? ?? at 2010-11-02
5 = ??? ?? ??? ?? ????? ??? PayMethodCode ??     by 20110413.wgkim
**/

ALTER PROCEDURE [dbo].[P_RefundGift]
	  @inbPurchaseOrderID bigint --// ??? ?? ID
	, @intRefundLimitDay int = 365 --// ???? ???? ??? ??? ?? (?? : 7?? ??)
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
	/**_# PurchaseOrders ???? RefundFlag? RefundDate ??? UPDATE???.*/
	UPDATE dbo.PurchaseOrderDetails
	SET RefundFlag = 1, RefundDate = GETDATE()
	FROM dbo.PurchaseOrderDetails PD
		INNER JOIN dbo.PurchaseOrders PO ON PD.PurchaseOrderID = PO.PurchaseOrderID
	WHERE PD.PurchaseOrderID = @inbPurchaseOrderID
		AND PO.PayMethodCode IN (2, 5, 14)
		AND PO.OrderStatusCode = 2
		AND PD.RefundFlag = CAST(0 AS bit)
		AND PO.OrderDate > DATEADD(DAY, - @intRefundLimitDay, GETDATE());

	/**_# ??? ?? ??? ???? ??? 101169? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 101169;
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

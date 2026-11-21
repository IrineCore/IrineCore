USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListRefundableProducts]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetListRefundableProducts]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetListRefundableProducts] AS' 
END
GO
/**
version : 8
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-11-04
description : ?? ??? (?? ????)? ?? ??? ?????.
result set : 
	version = 1 : PurchaseOrderDetailID/bigint, OrderKey/varchar, ProductID/int, ListPrice/int, SelectedItemIDs/varchar, ExpireFlag/bit, OrderDate/datetime2
	version = 2 : PurchaseOrderDetailID/bigint, OrderKey/varchar, ProductID/int, ListPrice/int, SelectedItemIDs/varchar, SelectedItemIDs/varchar, ExpireFlag/bit, OrderDate/datetime2
	version = 3 : version 2 + RefundDate/datetime, RefundFlag/bit
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101180 = ?? ???? ?? ????.
history :
5 = input param. @intVersion ?? & ?? ?? SelectedItemIDs ?? at 2011-10-04
**/

ALTER PROCEDURE [dbo].[P_GetListRefundableProducts]
	  @inbCharacterID bigint --// ??? ID
	, @intRefundLimitPeriod int --// ?? ??? ??? ??. ?? : Day
	, @intVersion int = 1 --// ??? SP? ??
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
	IF @intVersion = 1
		SELECT PD.PurchaseOrderDetailID, PO.OrderKey, PD.ProductID, PD.ListPrice, STUFF((
				SELECT ',' + CAST(ItemID AS varchar(10))
				FROM dbo.SelectedItems WITH (NOLOCK)
				WHERE PurchaseOrderDetailID = PD.PurchaseOrderDetailID
				FOR XML PATH('')
			), 1, 1, '') AS SelectedItemIDs
			, CASE WHEN DATEADD(DAY, @intRefundLimitPeriod, PO.OrderDate) < GETDATE() THEN CAST(1 AS bit) ELSE CAST(0 AS bit) END AS ExpireFlag
			, PO.OrderDate
		FROM dbo.PurchaseOrders PO WITH (NOLOCK)
			INNER JOIN dbo.PurchaseOrderDetails PD WITH (NOLOCK) ON PO.PurchaseOrderID = PD.PurchaseOrderID
		WHERE PO.CharacterID = @inbCharacterID AND PO.OrderStatusCode = 2 AND PO.PayMethodCode = 2 AND PD.RefundableFlag = CAST(1 AS bit)
		ORDER BY PD.PurchaseOrderDetailID;

	ELSE IF @intVersion = 2
		SELECT PD.PurchaseOrderDetailID
			, PO.OrderKey
			, PD.ProductID
			, PD.ListPrice
			, STUFF((
				SELECT ',' + CAST(ItemID AS varchar(10))
				FROM dbo.SelectedItems WITH (NOLOCK)
				WHERE PurchaseOrderDetailID = PD.PurchaseOrderDetailID
				FOR XML PATH('')
			), 1, 1, '') AS SelectedItemIDs
			, STUFF((
				SELECT ',' + CAST(ItemOption AS varchar(10))
				FROM dbo.SelectedItems WITH (NOLOCK)
				WHERE PurchaseOrderDetailID = PD.PurchaseOrderDetailID
				FOR XML PATH('')
			), 1, 1, '') AS SelectedItemOptions
			, CASE WHEN DATEADD(DAY, @intRefundLimitPeriod, PO.OrderDate) < GETDATE() THEN CAST(1 AS bit) ELSE CAST(0 AS bit) END AS ExpireFlag
			, PO.OrderDate
		FROM dbo.PurchaseOrders PO WITH (NOLOCK)
			INNER JOIN dbo.PurchaseOrderDetails PD WITH (NOLOCK) ON PO.PurchaseOrderID = PD.PurchaseOrderID
		WHERE PO.CharacterID = @inbCharacterID AND PO.OrderStatusCode = 2 AND PO.PayMethodCode = 2 AND PD.RefundableFlag = CAST(1 AS bit)
		ORDER BY PD.PurchaseOrderDetailID;

	ELSE IF @intVersion = 3
		SELECT PD.PurchaseOrderDetailID
			, PO.OrderKey
			, PD.ProductID
			, PD.ListPrice
			, STUFF((
				SELECT ',' + CAST(ItemID AS varchar(10))
				FROM dbo.SelectedItems WITH (NOLOCK)
				WHERE PurchaseOrderDetailID = PD.PurchaseOrderDetailID
				FOR XML PATH('')
			), 1, 1, '') AS SelectedItemIDs
			, STUFF((
				SELECT ',' + CAST(ItemOption AS varchar(10))
				FROM dbo.SelectedItems WITH (NOLOCK)
				WHERE PurchaseOrderDetailID = PD.PurchaseOrderDetailID
				FOR XML PATH('')
			), 1, 1, '') AS SelectedItemOptions
			, CASE WHEN DATEADD(DAY, @intRefundLimitPeriod, PO.OrderDate) < GETDATE() THEN CAST(1 AS bit) ELSE CAST(0 AS bit) END AS ExpireFlag
			, PO.OrderDate, PD.RefundDate, PD.RefundFlag
		FROM dbo.PurchaseOrders PO WITH (NOLOCK)
			INNER JOIN dbo.PurchaseOrderDetails PD WITH (NOLOCK) ON PO.PurchaseOrderID = PD.PurchaseOrderID
		WHERE PO.CharacterID = @inbCharacterID AND PO.OrderStatusCode = 2 AND PO.PayMethodCode = 2 AND (PD.RefundableFlag = CAST(1 AS bit) OR (PD.RefundableFlag = CAST(0 AS bit) AND PD.RefundFlag = 1))
		ORDER BY PD.PurchaseOrderDetailID;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 101180;
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

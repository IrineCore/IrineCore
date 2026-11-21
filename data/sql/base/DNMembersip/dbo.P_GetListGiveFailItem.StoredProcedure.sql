USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListGiveFailItem]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetListGiveFailItem]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetListGiveFailItem] AS' 
END
GO
/**
version : 14
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-03-10
description : ?? ??? ??? ??? ?????.
result set : 
	version 5 : PurchaseOrderID/bigint, SenderCharacterID/bigint, GiftFlag/bit, ProductID/int, ItemID/int
	version 7 : PurchaseOrderDetailID/bigint, SenderCharacterID/bigint, GiftFlag/bit, ProductID/int, ItemID/int
	version 13 : PurchaseOrderDetailID/bigint, SenderCharacterID/bigint, GiftFlag/bit, ProductID/int, ItemID/int, ItemOption/tinyint
	version 14 : PurchaseOrderDetailID/bigint, SenderCharacterID/bigint, GiftFlag/bit, ProductID/int, ItemID/int, ItemOption/tinyint, PaidCashAmount/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101148 = ?? ?? ???? ????.
history :
6 = ??? ??? ??? ?? ?? ?? at 2010-10-14
7 = ???? ??? PurchaseOrderDetailID/bigint, SenderCharacterID/bigint, GiftFlag/bit, ProductID/int, ItemID/int ? ?? at 2010-10-27
8 = ???? ??? SelectedItemID/bigint, SenderCharacterID/bigint, GiftFlag/bit, ProductID/int, ItemID/int ? ?? at 2010-11-04
9 = version 8? ?? at 2010-11-04
10 = ?? ??? at 2011-03-23
12 = indexed view ????? ??? at 2011-04-15
13 = ?? ?? ItemOption ?? at 2011-10-04
14 = ?? ?? PaidCashAmount ?? by ??? at 2012-10-24
**/

ALTER PROCEDURE [dbo].[P_GetListGiveFailItem]
	  @inbCharacterID bigint --// ??? ID
	, @inyVersion tinyint = 5 --// SP ??
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
	IF @inyVersion <= 5
		SELECT PO.PurchaseOrderID, CASE PO.GiftFlag WHEN 1 THEN PO.CharacterID ELSE NULL END AS SenderCharacterID, PO.GiftFlag, PD.ProductID, SI.ItemID
		FROM dbo.VIX_GiveFailItems SI WITH (NOEXPAND, NOLOCK)
			INNER JOIN dbo.PurchaseOrderDetails PD WITH (NOLOCK) ON PD.PurchaseOrderDetailID = SI.PurchaseOrderDetailID
			INNER JOIN dbo.PurchaseOrders PO WITH (NOLOCK) ON PO.PurchaseOrderID = PD.PurchaseOrderID
		WHERE SI.CharacterID = @inbCharacterID AND PO.GiftFlag = CAST(0 AS bit)
		UNION ALL
		SELECT PO.PurchaseOrderID, CASE PO.GiftFlag WHEN 1 THEN PO.CharacterID ELSE NULL END AS SenderCharacterID, PO.GiftFlag, PD.ProductID, SI.ItemID
		FROM dbo.VIX_GiveFailItems SI WITH (NOEXPAND, NOLOCK)
			INNER JOIN dbo.PurchaseOrderDetails PD WITH (NOLOCK) ON PD.PurchaseOrderDetailID = SI.PurchaseOrderDetailID
			INNER JOIN dbo.PurchaseOrders PO WITH (NOLOCK) ON PO.PurchaseOrderID = PD.PurchaseOrderID
		WHERE SI.ReceiverCharacterID = @inbCharacterID AND PO.GiftFlag = CAST(1 AS bit);

	ELSE IF @inyVersion = 7
		SELECT PD.PurchaseOrderDetailID, CASE PO.GiftFlag WHEN 1 THEN PO.CharacterID ELSE NULL END AS SenderCharacterID, PO.GiftFlag, PD.ProductID, SI.ItemID
		FROM dbo.VIX_GiveFailItems SI WITH (NOEXPAND, NOLOCK)
			INNER JOIN dbo.PurchaseOrderDetails PD WITH (NOLOCK) ON PD.PurchaseOrderDetailID = SI.PurchaseOrderDetailID
			INNER JOIN dbo.PurchaseOrders PO WITH (NOLOCK) ON PO.PurchaseOrderID = PD.PurchaseOrderID
		WHERE SI.CharacterID = @inbCharacterID
			AND (PO.GiftFlag = CAST(0 AS bit) OR (PO.GiftFlag = CAST(1 AS bit) AND PO.GiftRejectFlag = CAST(1 AS bit)))
		UNION ALL
		SELECT PD.PurchaseOrderDetailID, CASE PO.GiftFlag WHEN 1 THEN PO.CharacterID ELSE NULL END AS SenderCharacterID, PO.GiftFlag, PD.ProductID, SI.ItemID
		FROM dbo.VIX_GiveFailItems SI WITH (NOEXPAND, NOLOCK)
			INNER JOIN dbo.PurchaseOrderDetails PD WITH (NOLOCK) ON PD.PurchaseOrderDetailID = SI.PurchaseOrderDetailID
			INNER JOIN dbo.PurchaseOrders PO WITH (NOLOCK) ON PO.PurchaseOrderID = PD.PurchaseOrderID
		WHERE SI.ReceiverCharacterID = @inbCharacterID
			AND PO.GiftFlag = CAST(1 AS bit)
			AND PO.GiftRejectFlag = CAST(0 AS bit);

	ELSE IF @inyVersion = 13
		SELECT PD.PurchaseOrderDetailID, CASE PO.GiftFlag WHEN 1 THEN PO.CharacterID ELSE NULL END AS SenderCharacterID, PO.GiftFlag, PD.ProductID, SI.ItemID, SI.ItemOption
		FROM dbo.VIX_GiveFailItems SI WITH (NOEXPAND, NOLOCK)
			INNER JOIN dbo.PurchaseOrderDetails PD WITH (NOLOCK) ON PD.PurchaseOrderDetailID = SI.PurchaseOrderDetailID
			INNER JOIN dbo.PurchaseOrders PO WITH (NOLOCK) ON PO.PurchaseOrderID = PD.PurchaseOrderID
		WHERE SI.CharacterID = @inbCharacterID
			AND (PO.GiftFlag = CAST(0 AS bit) OR (PO.GiftFlag = CAST(1 AS bit) AND PO.GiftRejectFlag = CAST(1 AS bit)))
		UNION ALL
		SELECT PD.PurchaseOrderDetailID, CASE PO.GiftFlag WHEN 1 THEN PO.CharacterID ELSE NULL END AS SenderCharacterID, PO.GiftFlag, PD.ProductID, SI.ItemID, SI.ItemOption
		FROM dbo.VIX_GiveFailItems SI WITH (NOEXPAND, NOLOCK)
			INNER JOIN dbo.PurchaseOrderDetails PD WITH (NOLOCK) ON PD.PurchaseOrderDetailID = SI.PurchaseOrderDetailID
			INNER JOIN dbo.PurchaseOrders PO WITH (NOLOCK) ON PO.PurchaseOrderID = PD.PurchaseOrderID
		WHERE SI.ReceiverCharacterID = @inbCharacterID
			AND PO.GiftFlag = CAST(1 AS bit)
			AND PO.GiftRejectFlag = CAST(0 AS bit);

	ELSE IF @inyVersion = 14
		SELECT PD.PurchaseOrderDetailID, CASE PO.GiftFlag WHEN 1 THEN PO.CharacterID ELSE NULL END AS SenderCharacterID, PO.GiftFlag, PD.ProductID, SI.ItemID, SI.ItemOption, ISNULL(PO.PaidCashAmount, PO.Price) AS PaidCashAmount
		FROM dbo.VIX_GiveFailItems SI WITH (NOEXPAND, NOLOCK)
			INNER JOIN dbo.PurchaseOrderDetails PD WITH (NOLOCK) ON PD.PurchaseOrderDetailID = SI.PurchaseOrderDetailID
			INNER JOIN dbo.PurchaseOrders PO WITH (NOLOCK) ON PO.PurchaseOrderID = PD.PurchaseOrderID
		WHERE SI.CharacterID = @inbCharacterID
			AND (PO.GiftFlag = CAST(0 AS bit) OR (PO.GiftFlag = CAST(1 AS bit) AND PO.GiftRejectFlag = CAST(1 AS bit)))
		UNION ALL
		SELECT PD.PurchaseOrderDetailID, CASE PO.GiftFlag WHEN 1 THEN PO.CharacterID ELSE NULL END AS SenderCharacterID, PO.GiftFlag, PD.ProductID, SI.ItemID, SI.ItemOption, ISNULL(PO.PaidCashAmount, PO.Price) AS PaidCashAmount
		FROM dbo.VIX_GiveFailItems SI WITH (NOEXPAND, NOLOCK)
			INNER JOIN dbo.PurchaseOrderDetails PD WITH (NOLOCK) ON PD.PurchaseOrderDetailID = SI.PurchaseOrderDetailID
			INNER JOIN dbo.PurchaseOrders PO WITH (NOLOCK) ON PO.PurchaseOrderID = PD.PurchaseOrderID
		WHERE SI.ReceiverCharacterID = @inbCharacterID
			AND PO.GiftFlag = CAST(1 AS bit)
			AND PO.GiftRejectFlag = CAST(0 AS bit);
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 101148;
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

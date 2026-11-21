USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListGiftBox]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetListGiftBox]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetListGiftBox] AS' 
END
GO
/**
version : 11
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-04-01
description : ???? ???? ?? ??? ?????.
result set : 
	version <= 5 : PayMethodCode/tinyint, PurchaseOrderID/bigint, SenderCharacterID/bigint, SenderCharacterName/nvarchar, ProductID/int, ItemIDs/nvarchar, OrderDate/datetime2, GiftMemo/nvarchar, GiftExpireDate/smalldatetime
	version >= 6 : PayMethodCode/tinyint, PurchaseOrderID/bigint, SenderCharacterID/bigint, SenderCharacterName/nvarchar, ProductID/int, ItemIDs/nvarchar, OrderDate/datetime2, GiftMemo/nvarchar, GiftExpireDate/smalldatetime, GiftRejectFlag/bit
	version = 8 : PayMethodCode/tinyint, PurchaseOrderID/bigint, SenderCharacterID/bigint, SenderCharacterName/nvarchar, ProductID/int, ItemIDs/nvarchar, OrderDate/datetime2, GiftMemo/nvarchar, GiftExpireDate/smalldatetime, GiftRejectFlag/bit, NewFlag/bit
	version = 9 : PayMethodCode/tinyint, PurchaseOrderID/bigint, SenderCharacterID/bigint, SenderCharacterName/nvarchar, ProductID/int, ItemIDs/nvarchar, ItemOptions/nvarchar, OrderDate/datetime2, GiftMemo/nvarchar, GiftExpireDate/smalldatetime, GiftRejectFlag/bit, NewFlag/bit
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101153 = ???? ?? ????.
history :
4 = ??? ?? ?? ??, PayMethodCode ??, ??? ??? ?? ItemID? ,? ?????? String?? ????? ?? at 2010-04-09
5 = ??? ??? ???? ????? ?? at 2010-08-25
6 = ?? ??? GiftRejectFlag ?? at 2010-10-14
7 = RefundFlag, RefundDate ??? ??? ?? ?? at 2010-11-02
8 = ?? ??? NewFlag ?? at 2011-04-06
9 = ?? ??? ItemOptions ?? at 2011-10-04
10 = ?? ?? PaidCashAmount ?? by ??? at 2012-10-24
11 = ver.9 ? ?? by ??? at 2012-10-25
**/

ALTER PROCEDURE [dbo].[P_GetListGiftBox]
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
		WITH C1 AS (
			SELECT PO.PurchaseOrderID, PD.ProductID, STUFF((
					SELECT ',' + CAST(ItemID AS varchar(10))
					FROM dbo.SelectedItems WITH (NOLOCK)
					WHERE PurchaseOrderDetailID = PD.PurchaseOrderDetailID
					FOR XML PATH ('')
				), 1, 1, '') AS ItemIDs
			FROM dbo.PurchaseOrders PO WITH (NOLOCK)
				INNER JOIN dbo.PurchaseOrderDetails PD WITH (NOLOCK) ON PO.PurchaseOrderID = PD.PurchaseOrderID
			WHERE PO.ReceiverCharacterID = @inbCharacterID
				AND PO.GiftFlag = CAST(1 AS bit)
				AND PO.OrderStatusCode = 2
				AND PO.GiftReceiveFlag = CAST(0 AS bit)
				AND (PO.GiftExpireDate IS NULL OR PO.GiftExpireDate > GETDATE())
				AND PD.RefundFlag = CAST(0 AS bit)
			GROUP BY PO.PurchaseOrderID, PD.PurchaseOrderDetailID, PD.ProductID
		)
		SELECT PO.PayMethodCode, PO.PurchaseOrderID, CH.CharacterID AS SenderCharacterID, COALESCE(CH.CharacterName, DC.CharacterName, PO.GiftSenderName) AS SenderCharacterName
			, C1.ProductID, CAST(C1.ItemIDs AS nvarchar(4000)) AS ItemIDs, PO.OrderDate, PO.GiftMemo, PO.GiftExpireDate
		FROM C1
			INNER JOIN dbo.PurchaseOrders PO WITH (NOLOCK) ON C1.PurchaseOrderID = PO.PurchaseOrderID
			LEFT OUTER JOIN dbo.Characters CH WITH (NOLOCK) ON PO.CharacterID = CH.CharacterID
			LEFT OUTER JOIN dbo.DeletedCharacters DC WITH (NOLOCK) ON CH.CharacterID = DC.CharacterID
		ORDER BY PO.PurchaseOrderID;

	ELSE IF @inyVersion <= 7
		WITH C1 AS (
			SELECT PO.PurchaseOrderID, PD.ProductID, STUFF((
					SELECT ',' + CAST(ItemID AS varchar(10))
					FROM dbo.SelectedItems WITH (NOLOCK)
					WHERE PurchaseOrderDetailID = PD.PurchaseOrderDetailID
					FOR XML PATH ('')
				), 1, 1, '') AS ItemIDs
			FROM dbo.PurchaseOrders PO WITH (NOLOCK)
				INNER JOIN dbo.PurchaseOrderDetails PD WITH (NOLOCK) ON PO.PurchaseOrderID = PD.PurchaseOrderID
			WHERE ((PO.ReceiverCharacterID = @inbCharacterID AND PO.GiftRejectFlag = CAST(0 AS bit)) OR (PO.CharacterID = @inbCharacterID AND PO.GiftRejectFlag = CAST(1 AS bit)))
				AND PO.GiftFlag = CAST(1 AS bit)
				AND PO.OrderStatusCode = 2
				AND PO.GiftReceiveFlag = CAST(0 AS bit)
				AND (PO.GiftExpireDate IS NULL OR PO.GiftExpireDate > GETDATE())
				AND PD.RefundFlag = CAST(0 AS bit)
			GROUP BY PO.PurchaseOrderID, PD.PurchaseOrderDetailID, PD.ProductID
		)
		SELECT PO.PayMethodCode, PO.PurchaseOrderID, CH.CharacterID AS SenderCharacterID, COALESCE(CH.CharacterName, DC.CharacterName, PO.GiftSenderName) AS SenderCharacterName
			, C1.ProductID, CAST(C1.ItemIDs AS nvarchar(4000)) AS ItemIDs, PO.OrderDate, PO.GiftMemo, PO.GiftExpireDate, ISNULL(PO.GiftRejectFlag, CAST(0 AS bit)) AS GiftRejectFlag
		FROM C1
			INNER JOIN dbo.PurchaseOrders PO WITH (NOLOCK) ON C1.PurchaseOrderID = PO.PurchaseOrderID
			LEFT OUTER JOIN dbo.Characters CH WITH (NOLOCK) ON PO.CharacterID = CH.CharacterID
			LEFT OUTER JOIN dbo.DeletedCharacters DC WITH (NOLOCK) ON CH.CharacterID = DC.CharacterID
		ORDER BY PO.PurchaseOrderID;

	ELSE IF @inyVersion = 8
		WITH C1 AS (
			SELECT PO.PurchaseOrderID, PD.ProductID, STUFF((
					SELECT ',' + CAST(ItemID AS varchar(10))
					FROM dbo.SelectedItems WITH (NOLOCK)
					WHERE PurchaseOrderDetailID = PD.PurchaseOrderDetailID
					FOR XML PATH ('')
				), 1, 1, '') AS ItemIDs
			FROM dbo.PurchaseOrders PO WITH (NOLOCK)
				INNER JOIN dbo.PurchaseOrderDetails PD WITH (NOLOCK) ON PO.PurchaseOrderID = PD.PurchaseOrderID
			WHERE ((PO.ReceiverCharacterID = @inbCharacterID AND PO.GiftRejectFlag = CAST(0 AS bit)) OR (PO.CharacterID = @inbCharacterID AND PO.GiftRejectFlag = CAST(1 AS bit)))
				AND PO.GiftFlag = CAST(1 AS bit)
				AND PO.OrderStatusCode = 2
				AND PO.GiftReceiveFlag = CAST(0 AS bit)
				AND (PO.GiftExpireDate IS NULL OR PO.GiftExpireDate > GETDATE())
				AND PD.RefundFlag = CAST(0 AS bit)
			GROUP BY PO.PurchaseOrderID, PD.PurchaseOrderDetailID, PD.ProductID
		)
		SELECT PO.PayMethodCode, PO.PurchaseOrderID, CH.CharacterID AS SenderCharacterID, COALESCE(CH.CharacterName, DC.CharacterName, PO.GiftSenderName) AS SenderCharacterName
			, C1.ProductID, CAST(C1.ItemIDs AS nvarchar(4000)) AS ItemIDs, PO.OrderDate, PO.GiftMemo, PO.GiftExpireDate, ISNULL(PO.GiftRejectFlag, CAST(0 AS bit)) AS GiftRejectFlag
			, ISNULL(PO.NewFlag, CAST(0 AS bit)) AS NewFlag
		FROM C1
			INNER JOIN dbo.PurchaseOrders PO WITH (NOLOCK) ON C1.PurchaseOrderID = PO.PurchaseOrderID
			LEFT OUTER JOIN dbo.Characters CH WITH (NOLOCK) ON PO.CharacterID = CH.CharacterID
			LEFT OUTER JOIN dbo.DeletedCharacters DC WITH (NOLOCK) ON CH.CharacterID = DC.CharacterID
		ORDER BY PO.PurchaseOrderID;

	ELSE IF @inyVersion = 9
		WITH C1 AS (
			SELECT PO.PurchaseOrderID
				, PD.ProductID
				, STUFF((
					SELECT ',' + CAST(ItemID AS nvarchar(10))
					FROM dbo.SelectedItems WITH (NOLOCK)
					WHERE PurchaseOrderDetailID = PD.PurchaseOrderDetailID
					FOR XML PATH ('')
				), 1, 1, '') AS ItemIDs
				, STUFF((
					SELECT ',' + CAST(ItemOption AS nvarchar(10))
					FROM dbo.SelectedItems WITH (NOLOCK)
					WHERE PurchaseOrderDetailID = PD.PurchaseOrderDetailID
					FOR XML PATH ('')
				), 1, 1, '') AS ItemOptions
			FROM dbo.PurchaseOrders PO WITH (NOLOCK)
				INNER JOIN dbo.PurchaseOrderDetails PD WITH (NOLOCK) ON PO.PurchaseOrderID = PD.PurchaseOrderID
			WHERE ((PO.ReceiverCharacterID = @inbCharacterID AND PO.GiftRejectFlag = CAST(0 AS bit)) OR (PO.CharacterID = @inbCharacterID AND PO.GiftRejectFlag = CAST(1 AS bit)))
				AND PO.GiftFlag = CAST(1 AS bit)
				AND PO.OrderStatusCode = 2
				AND PO.GiftReceiveFlag = CAST(0 AS bit)
				AND (PO.GiftExpireDate IS NULL OR PO.GiftExpireDate > GETDATE())
				AND PD.RefundFlag = CAST(0 AS bit)
			GROUP BY PO.PurchaseOrderID, PD.PurchaseOrderDetailID, PD.ProductID
		)
		SELECT PO.PayMethodCode, PO.PurchaseOrderID, CH.CharacterID AS SenderCharacterID, COALESCE(CH.CharacterName, DC.CharacterName, PO.GiftSenderName) AS SenderCharacterName
			, C1.ProductID, CAST(C1.ItemIDs AS nvarchar(4000)) AS ItemIDs, CAST(C1.ItemOptions AS nvarchar(4000)) AS ItemOptions, PO.OrderDate, PO.GiftMemo, PO.GiftExpireDate
			, ISNULL(PO.GiftRejectFlag, CAST(0 AS bit)) AS GiftRejectFlag
			, ISNULL(PO.NewFlag, CAST(0 AS bit)) AS NewFlag
		FROM C1
			INNER JOIN dbo.PurchaseOrders PO WITH (NOLOCK) ON C1.PurchaseOrderID = PO.PurchaseOrderID
			LEFT OUTER JOIN dbo.Characters CH WITH (NOLOCK) ON PO.CharacterID = CH.CharacterID
			LEFT OUTER JOIN dbo.DeletedCharacters DC WITH (NOLOCK) ON CH.CharacterID = DC.CharacterID
		ORDER BY PO.PurchaseOrderID;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 101153;
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

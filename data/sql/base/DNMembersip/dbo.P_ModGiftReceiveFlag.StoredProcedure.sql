USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_ModGiftReceiveFlag]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_ModGiftReceiveFlag]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_ModGiftReceiveFlag] AS' 
END
GO
/**
version : 13
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-04-05
description : ???? ??? ?? ????? ?????? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101154 = ?? ??? ???? ????.
history : 
2 = ProductID, ItemID ? ????? ?? at 2010-04-07
4 = ListPrice? ????? ?? at 2010-04-07
6 = output param ?? - @inbReplySenderCharacterID, @nvcReplyReceiverCharacterName at 2010-04-07
7 = output param ?? - @inbGiftSenderCharacterID at 2010-04-08
8 = OrderStatusCode ?? 2?? ????? ?? at 2010-04-09
9 = Result Set? ???? ??? ??. Result Set? ??? SP [P_GetListItemOfGift]? ?? ?? at 2010-04-13
12 = ??? ??? ?? Sender?? ?????? OUTPUT ?? ?? at 2011-02-11
**/

ALTER PROCEDURE [dbo].[P_ModGiftReceiveFlag]
	  @inbPurchaseOrderID bigint --// ??? ?? ?? ID
	, @inbReplySenderCharacterID bigint = NULL OUTPUT --// ??? ?? ?? ??? ??? ??? ID
	, @nvcReplyReceiverCharacterName nvarchar(30) = NULL OUTPUT --// ??? ?? ?? ??? ?? ??? ??
	, @inbGiftSenderCharacterID bigint = NULL OUTPUT --// ?? ?? ??? ID
	, @intPaidCashAmount int = NULL OUTPUT --// ??? ??? ?? ?? ??
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
	/**_# [PurchaseOrders] ???? UPDATE???.*/
	UPDATE dbo.PurchaseOrders
	SET GiftReceiveFlag = 1
		, GiftReceiveDate = GETDATE()
		, @inbReplySenderCharacterID = CASE PO.GiftRejectFlag WHEN 1 THEN PO.CharacterID ELSE PO.ReceiverCharacterID END
		, @nvcReplyReceiverCharacterName = CH.CharacterName
		, @inbGiftSenderCharacterID = PO.CharacterID
		, @intPaidCashAmount = ISNULL(PO.PaidCashAmount, PO.Price)
	FROM dbo.PurchaseOrders PO
		LEFT OUTER JOIN dbo.Characters CH WITH (NOLOCK) ON PO.CharacterID = CH.CharacterID
	WHERE PO.PurchaseOrderID = @inbPurchaseOrderID AND PO.GiftReceiveFlag = CAST(0 AS bit) AND PO.OrderStatusCode = 2;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 101154;
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

USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetCountNotReceivedGift]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetCountNotReceivedGift]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetCountNotReceivedGift] AS' 
END
GO
/**
version : 15
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-04-13
description : ?? ?? ?? ??? ??????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103214 = ?? ??? ??? ???? ????.
history :
4 = ?? ??? ??? ???? ???? ?? ?? at 2010-07-02
6 = ?? ?? ?? ??? ?? ?? at 2010-10-14
7 = RefundFlag, RefundDate ??? ??? ?? ?? at 2010-11-02
8 = ????? ?? ??? ?? (?? ??? ?? ??)     by 20110502.wgkim
9 = ?? ??     by 20110503.wgkim
10 = dbo.P_SendGiftBySchedule ?? ?? @intReturnValue OUTPUT ??     by 20110614.wgkim
11 = ???? ??     by ??? at 2012-01-26
12 = UNION ALL? ????? ?? by ??? at 2012-02-14
13~14 = Index Hint ??     by ??? at 2012-03-16
15 = @inyVersion > 5 ? ?? ?????? ??? ?? ?? ??     by ??? at 2012-06-20
**/

ALTER PROCEDURE [dbo].[P_GetCountNotReceivedGift]
	  @inbCharacterID bigint --// ??? ID
	, @intNotReceivedGiftCount int OUTPUT --// ?? ?? ?? ??
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
	/**_# ????? ?? ??? ?????. */
	EXEC @intReturnValue = dbo.P_SendGiftBySchedule
		 @inbCharacterID = @inbCharacterID;

	/**_# ?? ?? ?? ??? ??????. */
	IF @inyVersion <= 5
		SELECT @intNotReceivedGiftCount = COUNT(*)
		FROM dbo.PurchaseOrders PO WITH (NOLOCK, FORCESEEK, INDEX(IX_NNI_PurchaseOrders_ReceiverCharacterID_GiftFlag))
			INNER JOIN dbo.PurchaseOrderDetails PD WITH (NOLOCK, FORCESEEK, INDEX(IX_NN_PurchaseOrderDetails_PurchaseOrderID)) ON PO.PurchaseOrderID = PD.PurchaseOrderID
		WHERE PO.ReceiverCharacterID = @inbCharacterID
			AND PO.OrderStatusCode = 2
			AND PO.GiftFlag = CAST(1 AS bit)
			AND PO.GiftReceiveFlag = CAST(0 AS bit)
			AND (PO.GiftExpireDate IS NULL OR PO.GiftExpireDate > GETDATE())
			AND PD.RefundFlag = CAST(0 AS bit)
		OPTION (FORCE ORDER, LOOP JOIN, KEEPFIXED PLAN);

	ELSE
		SELECT @intNotReceivedGiftCount = COUNT(*)
		FROM (
			SELECT PurchaseOrderID
			FROM dbo.PurchaseOrders WITH (NOLOCK, INDEX(IX_NNI_PurchaseOrders_ReceiverCharacterID_GiftFlag), FORCESEEK)
			WHERE ReceiverCharacterID = @inbCharacterID
				AND GiftFlag = CAST(1 AS bit)
				AND GiftRejectFlag = CAST(0 AS bit)
				AND OrderStatusCode = 2
				AND GiftReceiveFlag = CAST(0 AS bit)
				AND (GiftExpireDate IS NULL OR GiftExpireDate > GETDATE())
			UNION ALL
			SELECT PurchaseOrderID
			FROM dbo.PurchaseOrders WITH (NOLOCK, INDEX(IX_NN_PurchaseOrders_CharacterID), FORCESEEK)
			WHERE CharacterID = @inbCharacterID
				AND GiftRejectFlag = CAST(1 AS bit)
				AND OrderStatusCode = 2
				AND GiftReceiveFlag = CAST(0 AS bit)
				AND (GiftExpireDate IS NULL OR GiftExpireDate > GETDATE())
		) PO
			INNER JOIN dbo.PurchaseOrderDetails PD WITH (NOLOCK, INDEX(IX_NN_PurchaseOrderDetails_PurchaseOrderID), FORCESEEK) ON PO.PurchaseOrderID = PD.PurchaseOrderID
		WHERE PD.RefundFlag = CAST(0 AS bit)
		OPTION (KEEPFIXED PLAN);

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

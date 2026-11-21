USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_ModGiftRejectFlag]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_ModGiftRejectFlag]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_ModGiftRejectFlag] AS' 
END
GO
/**
version : 3
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-10-14
description : ?? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101179 = ?? ??? ??? ????.
history :
3 = RefundFlag, RefundDate ??? ??? ?? ?? at 2010-11-02
**/

ALTER PROCEDURE [dbo].[P_ModGiftRejectFlag]
	  @inbPurchaseOrderID bigint = NULL OUTPUT --// ??? ?? ?? ID
	, @bitGiftRejectFlag bit --// 0=?? ?? ??, 1=?? ??
	, @intSenderAccountID int OUTPUT --// ?? ?? ?? ID
	, @inbSenderCharacterID bigint OUTPUT --// ?? ?? ??? ID
	, @intNotReceivedGiftCount int OUTPUT --// ?? ?? ? ?? ?? ???? ???? ? ?? ?
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
	SET @inbSenderCharacterID = CharacterID
		, GiftRejectFlag = @bitGiftRejectFlag
		, GiftRejectDate = CASE @bitGiftRejectFlag WHEN 1 THEN GETDATE() ELSE NULL END
	FROM dbo.PurchaseOrders PO
	WHERE PurchaseOrderID = @inbPurchaseOrderID
		AND GiftReceiveFlag <> @bitGiftRejectFlag
		AND GiftFlag = CAST(1 AS bit)
		AND GiftReceiveFlag = CAST(0 AS bit)
		AND NOT EXISTS (SELECT * FROM dbo.PurchaseOrderDetails WHERE PurchaseOrderID = PO.PurchaseOrderID AND RefundFlag = CAST(1 AS bit));

	/**_# UPDATE? ?? ??? 101179? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 101179;
		GOTO ErrorHandler;
	END

	SELECT @intSenderAccountID = AccountID FROM dbo.Characters WITH (NOLOCK) WHERE CharacterID = @inbSenderCharacterID;

	EXEC dbo.P_GetCountNotReceivedGift
		  @inbCharacterID = @inbSenderCharacterID
		, @intNotReceivedGiftCount = @intNotReceivedGiftCount OUTPUT
		, @inyVersion = 6;
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

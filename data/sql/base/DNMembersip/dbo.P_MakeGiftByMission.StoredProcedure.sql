USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_MakeGiftByMission]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_MakeGiftByMission]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_MakeGiftByMission] AS' 
END
GO
/**
version : 12
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-04-22
description : ?? ?? ???? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101150 = ??? ??? ???? ?????.
history :
2 = ??? ??? @intProductID? ????? ?? (input parameter type ?? ?) at 2010-04-26
3 = Transaction ?? ?? at 2010-07-14
4 = ?? ?? ?? ?? at 2010-10-11
5 = GiftRejectFlag ?? ??? ?? ?? at 2010-10-14
6 = ?? ?? ?? ?? at 2010-11-02
7 = intput param. @bitNewFlag ?? at 2011-04-06
9 = input param. @vchItemOptions ?? at 2011-10-04
11 = TF_Split(), TF_NSplit() ??? ????? ?? by ??? at 2011-11-02
12 = ??? ??? ?? ?? ?? by ??? at 2012-03-12
**/

ALTER PROCEDURE [dbo].[P_MakeGiftByMission]
	  @inbCharacterID bigint --// ??? ID
	, @bitPCRoomFlag bit --// 0=??, 1=PC?
	, @intProductID int --// ?? ID
	, @vchSelectedItemIDs varchar(8000) --// |? ?????? ??? ??? ID ???
	, @intMissionID int --// ?? ID
	, @nvcGiftMemo nvarchar(100) --// ?? ??
	, @intLifespan int --// ?????? ????. ?? : ?
	, @vchIP varchar(15) --// IP ??
	, @inbPurchaseOrderID bigint = NULL OUTPUT --// ??? ?? ?? ID
	, @bitNewFlag bit = 0 --// ??? ??? ??? ??
	, @vchItemOptions varchar(8000) = '' --// |? ?????? ??? ??? ?? ???
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @inbPurchaseOrderDetailID bigint;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# [PurchaseOrders] ???? INSERT???.*/
	INSERT dbo.PurchaseOrders (CharacterID, ReceiverCharacterID, GiftFlag, PCRoomFlag, Price, IP, OrderDate, OrderStatusCode, PayMethodCode, OrderKey, GiftReceiveFlag, GiftMemo, CouponKey, GiftExpireDate, GiftRejectFlag, NewFlag)
	VALUES (@inbCharacterID, @inbCharacterID, 1, @bitPCRoomFlag, 0, dbo.FN_IP2IPNumber(@vchIP), GETDATE(), 2, 7, CAST(@intMissionID AS varchar(10)), 0, @nvcGiftMemo, NULL, DATEADD(MINUTE, @intLifespan, GETDATE()), 0, @bitNewFlag);

	SET @inbPurchaseOrderID = SCOPE_IDENTITY();

	/**_# [PurchaseOrderDetails] ???? INSERT???.*/
	INSERT dbo.PurchaseOrderDetails (PurchaseOrderID, ProductID, ListPrice)
	VALUES (@inbPurchaseOrderID, @intProductID, 0);

	SET @inbPurchaseOrderDetailID = SCOPE_IDENTITY();

	/**_# [SelectedItems] ???? INSERT???.*/
	INSERT dbo.SelectedItems (PurchaseOrderDetailID, ItemID, ItemOption)
	SELECT @inbPurchaseOrderDetailID, CAST(A.string AS int), CAST(B.string AS tinyint)
	FROM dbo.TF_Split(@vchSelectedItemIDs, '|') A
		LEFT OUTER JOIN dbo.TF_Split(@vchItemOptions, '|') B ON A.seq = B.seq
	OPTION (KEEPFIXED PLAN);
END TRY
BEGIN CATCH
	DELETE dbo.SelectedItems WHERE PurchaseOrderDetailID = @inbPurchaseOrderDetailID;
	DELETE dbo.PurchaseOrderDetails WHERE PurchaseOrderID = @inbPurchaseOrderID;
	DELETE dbo.PurchaseOrders WHERE PurchaseOrderID = @inbPurchaseOrderID;

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

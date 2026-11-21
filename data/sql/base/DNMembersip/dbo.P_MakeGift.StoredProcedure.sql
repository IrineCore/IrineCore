USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_MakeGift]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_MakeGift]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_MakeGift] AS' 
END
GO
/**
version : 14
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-06-08
description : ??? ?????. (?? dbo.P_MakeGiftByMission, dbo.P_MakeGiftByQuest? ??? ? ????.)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101147 = ?? ?????. ?? ??? ? ?? ??? ???????.
101150 = ??? ??? ???? ?????.
history :
2 = Dead-Lock ??? ?? ??? ????? ???? ??? ?? at 2010-07-13
3 = ?? ?? ?? ?? at 2010-10-11
4 = GiftRejectFlag ?? ??? ?? ?? at 2010-10-14
5 = ?? ?? ?? ?? at 2010-11-02
6 = input param. @bitNewFlag ?? at 2011-04-06
7 = input param. ?? ??     by 20110412.wgkim
8 = input param. @vchItemOptions ?? at 2011-10-04
10 = TF_Split(), TF_NSplit() ??? ????? ?? by ??? at 2011-11-02
11 = (53070) ?? ?? ?? ?? ?? by ??? at 2012-03-12
12 = 11? ?? ?? ?? by ??? at 2012-03-14
13 = (53070) ?? ?? ?? ?? ?? by ??? at 2012-03-15
**/

ALTER PROCEDURE [dbo].[P_MakeGift]
	  @inbCharacterID bigint --// ??? ID
	, @bitPCRoomFlag bit --// 0=??, 1=PC?
	, @intProductID int --// ?? ID
	, @vchSelectedItemIDs varchar(8000) --// |? ?????? ??? ??? ID ???
	, @nvcGiftMemo nvarchar(100) --// ?? ??
	, @intLifespan int --// ?????? ????. ?? : ?
	, @inyPayMethodCode tinyint --// 1=????, 2=??, 3=??, 4=??, 5=?????, 6=??? ??, 7=?? ??, 8=??? ?? ? ??, 9=VIP ??, 10=PVP ?? ???, 11=?? ?? ???, 12=?? ?? ???, 13=?? ?? ???, 14=??2, 15=Time Event ??, 16=??? ?? ???, 17=??? ??, 19=??????, 20=????, 21=???????
	, @vchOrderKey varchar(32) --// @inyPayMethodCode? ?? ?? ?. @inyPayMethodCode ?? 6? ? QuestID, 7? ? MissionID, 8? ? CharacterLevel? ?????.
	, @vchIP varchar(15) --// IP ??
	, @inbPurchaseOrderID bigint = NULL OUTPUT --// ??? ?? ?? ID
	, @bitNewFlag bit = 0 --// ??? ??? ??? ??
	, @vchItemOptions varchar(8000) = '' --// |? ?????? ??? ??? ?? ???
	, @intLimitCountPerPartition int = 0 --// ???? ??? ?? ?? ??. ???? ?? 0
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @inbPurchaseOrderDetailID bigint
	, @intSaleCount int
	, @dt2Now datetime2(0) = GETDATE();

DECLARE @tblA table (
	Seq int NOT NULL PRIMARY KEY,
	ItemID int NOT NULL
);

DECLARE @tblB table (
	Seq int NOT NULL PRIMARY KEY,
	ItemOption tinyint NOT NULL
);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	INSERT @tblA (Seq, ItemID)
	SELECT seq, CAST(string AS int)
	FROM dbo.TF_Split(@vchSelectedItemIDs, '|')
	OPTION (KEEPFIXED PLAN);

	INSERT @tblB (Seq, ItemOption)
	SELECT seq, CAST(string AS tinyint)
	FROM dbo.TF_Split(@vchItemOptions, '|')
	OPTION (KEEPFIXED PLAN);

	/**_# ?? ??? ?? ?? ??? ???? 101147? ?????.*/
	IF @intLimitCountPerPartition > 0
	BEGIN
		BEGIN TRANSACTION;

		UPDATE dbo.LimitedProducts
		SET @intSaleCount = SaleCount += 1, LastOrderDate = @dt2Now
		WHERE ProductID = @intProductID;

		IF @@ROWCOUNT = 0
			INSERT dbo.LimitedProducts (ProductID, SaleCount, LastOrderDate)
			VALUES (@intProductID, 1, @dt2Now);

		ELSE IF @intSaleCount > @intLimitCountPerPartition
		BEGIN
			SET @intReturnValue = 101147;
			GOTO ErrorHandler;
		END

		COMMIT TRANSACTION;
	END

	/**_# [PurchaseOrders] ???? INSERT???.*/
	INSERT dbo.PurchaseOrders (CharacterID, ReceiverCharacterID, GiftFlag, PCRoomFlag, Price, IP, OrderDate, OrderStatusCode, PayMethodCode, OrderKey, GiftReceiveFlag, GiftMemo, CouponKey, GiftExpireDate, GiftRejectFlag, NewFlag)
	VALUES (@inbCharacterID, @inbCharacterID, 1, @bitPCRoomFlag, 0, dbo.FN_IP2IPNumber(@vchIP), @dt2Now, 2, @inyPayMethodCode, @vchOrderKey, 0, @nvcGiftMemo, NULL, DATEADD(MINUTE, @intLifespan, @dt2Now), 0, @bitNewFlag);

	SET @inbPurchaseOrderID = SCOPE_IDENTITY();

	/**_# [PurchaseOrderDetails] ???? INSERT???.*/
	INSERT dbo.PurchaseOrderDetails (PurchaseOrderID, ProductID, ListPrice)
	VALUES (@inbPurchaseOrderID, @intProductID, 0);

	SET @inbPurchaseOrderDetailID = SCOPE_IDENTITY();

	/**_# [SelectedItems] ???? INSERT???.*/
	INSERT dbo.SelectedItems (PurchaseOrderDetailID, ItemID, ItemOption)
	SELECT @inbPurchaseOrderDetailID, ItemID, ISNULL(ItemOption, 0)
	FROM @tblA A
		LEFT OUTER JOIN @tblB B ON A.Seq = B.Seq;

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

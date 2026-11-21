USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_MakeGiftByCoupon]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_MakeGiftByCoupon]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_MakeGiftByCoupon] AS' 
END
GO
/**
version : 13
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-04-09
description : ???? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101150 = ??? ??? ???? ?????.
history :
4 = Transaction ?? ?? at 2010-07-14
5 = ?? ?? ?? ?? at 2010-10-11
6 = GiftRejectFlag ?? ??? ?? ?? at 2010-10-14
7 = ?? ?? ?? ?? at 2010-11-02
8 = intput param. @bitNewFlag ?? at 2011-04-06
10 = input param. @vchItemOptions ?? at 2011-10-04
12 = TF_Split(), TF_NSplit() ??? ????? ?? by ??? at 2011-11-02
13 = ??? ??? ?? ?? ?? by ??? at 2012-03-12
**/

ALTER PROCEDURE [dbo].[P_MakeGiftByCoupon]
	  @inbCharacterID bigint --// ??? ID
	, @bitPCRoomFlag bit --// 0=??, 1=PC?
	, @vchProductIDs varchar(8000) --// ,? ?????? ?? ID ???. ?) 203724298,923902,934203948
	, @vchSelectedItemIDs varchar(8000) --// ,? |? ?????? ??? ??? ID ???. ,? ??? ????? |? ??? ??? ?? ??? ???? ??????.  ?) 48384290,2394802394|3123123132,238201938|49324802|823409238
	, @nvcCouponKey nvarchar(128) --// ???
	, @vchOrderKey varchar(32) --// ????? ??? ?? ?? ?
	, @vchIP varchar(15) --// IP ??
	, @inbPurchaseOrderID bigint = NULL OUTPUT --// ??? ?? ?? ID
	, @bitNewFlag bit = 0 --// ??? ??? ??? ??
	, @vchItemOptions varchar(8000) = '' --// ,? |? ?????? ??? ??? ?? ???. ,? ??? ????? |? ??? ??? ?? ??? ???? ??????.
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @intPurchaseCount int
	, @i int = 1
	, @j int
	, @vchSelectedItemIDs2 varchar(8000)
	, @vchItemOptions2 varchar(8000)
	, @inbPurchaseOrderDetailID bigint;

DECLARE @tblProducts table (
	Seq int IDENTITY(1, 1) NOT NULL PRIMARY KEY,
	ProductID int NOT NULL,
	ListPrice int NOT NULL,
	PurchaseCount int NOT NULL DEFAULT(0),
	SelectedItemIDs varchar(8000) NOT NULL,
	ItemOptions varchar(8000) NULL
);

DECLARE @tblTemp1 table (
	Seq int IDENTITY(1, 1) NOT NULL PRIMARY KEY,
	PurchaseOrderDetailID bigint NOT NULL
);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	INSERT @tblProducts (ProductID, ListPrice, SelectedItemIDs, ItemOptions)
	SELECT CAST(A.string AS int), 0, D.string, E.string
	FROM dbo.TF_Split(@vchProductIDs, ',') A
		INNER JOIN dbo.TF_Split(@vchSelectedItemIDs, ',') D ON A.seq = D.seq
		LEFT OUTER JOIN dbo.TF_Split(@vchItemOptions, ',') E ON A.seq = E.seq
	WHERE A.string > '' AND D.string > ''
	OPTION (KEEPFIXED PLAN);

	SET @j = @@ROWCOUNT;

	IF @j = 0
	BEGIN
		SET @intReturnValue = 101150;
		GOTO ErrorHandler;
	END

	/**_# [PurchaseOrders] ???? INSERT???.*/
	INSERT dbo.PurchaseOrders (CharacterID, ReceiverCharacterID, GiftFlag, PCRoomFlag, Price, IP, OrderDate, OrderStatusCode, PayMethodCode, OrderKey, GiftReceiveFlag, GiftMemo, CouponKey, GiftExpireDate, GiftRejectFlag, NewFlag)
	VALUES (@inbCharacterID, @inbCharacterID, 1, @bitPCRoomFlag, 0, dbo.FN_IP2IPNumber(@vchIP), GETDATE(), 1, 4, NULL, 0, NULL, @nvcCouponKey, DATEADD(DAY, 7, GETDATE()), 0, @bitNewFlag);

	SET @inbPurchaseOrderID = SCOPE_IDENTITY();

	/**_# [PurchaseOrderDetails] ???? INSERT???.*/
	INSERT dbo.PurchaseOrderDetails (PurchaseOrderID, ProductID, ListPrice)
	OUTPUT inserted.PurchaseOrderDetailID INTO @tblTemp1 (PurchaseOrderDetailID)
	SELECT @inbPurchaseOrderID, ProductID, ListPrice
	FROM @tblProducts
	ORDER BY Seq;

	WHILE @i <= @j
	BEGIN
		SELECT @vchSelectedItemIDs2 = SelectedItemIDs, @vchItemOptions2 = ItemOptions FROM @tblProducts WHERE Seq = @i;
		SELECT @inbPurchaseOrderDetailID = PurchaseOrderDetailID FROM @tblTemp1 WHERE Seq = @i;

		INSERT dbo.SelectedItems (PurchaseOrderDetailID, ItemID, ItemOption)
		SELECT @inbPurchaseOrderDetailID, CAST(A.string AS int), CAST(B.string AS tinyint)
		FROM dbo.TF_Split(@vchSelectedItemIDs2, '|') A
			LEFT OUTER JOIN dbo.TF_Split(@vchItemOptions2, '|') B ON A.seq = B.seq
		OPTION (KEEPFIXED PLAN);

		SET @i += 1;
	END

END TRY
BEGIN CATCH
	DELETE dbo.SelectedItems
	FROM dbo.SelectedItems SI
		INNER JOIN dbo.PurchaseOrderDetails POD WITH (NOLOCK) ON SI.PurchaseOrderDetailID = POD.PurchaseOrderDetailID
	WHERE POD.PurchaseOrderID = @inbPurchaseOrderID;

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

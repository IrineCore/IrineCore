USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_AddPurchaseOrderByCash]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_AddPurchaseOrderByCash]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_AddPurchaseOrderByCash] AS' 
END
GO
/**
version : 31
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-02-19
description : ?? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101147 = ?? ?????. ?? ??? ? ?? ??? ???????.
101150 = ??? ??? ???? ?????.
101176 = ?? ?? ???? ???? ????.
history :
7 = ?? ?? ?? input param ?? - @bitGiftFlag, @inbReceiverCharacterID, @nvcGiftMemo at 2010-03-31
8, 9 = input param @inbReceiverCharacterID? @nvcReceiverCharacterName?? ??
10 = ?? ?? ???? ???? ?? ??? ?? ?? ?? at 2010-10-04
11 = ?? ?? ?? ?? at 2010-10-11
12 = GiftRejectFlag ?? ??? ?? ?? at 2010-10-14
13 = output param. @vchPurchaseOrderDetailIDs ?? at 2010-10-27
14 = ?? ?? ?? ?? at 2010-11-02
15 = ?? ?? ?? ?? at 2010-11-04
16 = input param. @vchRefundableFlags ?? at 2010-11-08
17 = ??? PurchaseOrders.NewFlag ??? default ??? 0? ????? ?? at 2011-04-06
18 = input param. @bitCheatFlag ?? at 2011-06-14
20 = input param. @vchItemOptions ?? at 2011-10-04
22 = TF_Split(), TF_NSplit() ??? ????? ?? by ??? at 2011-11-02
23 = (53070) ?? ?? ?? ?? ?? by ??? at 2012-03-12
24 = 23? ?? ?? ?? by ??? at 2012-03-14
27 = (53070) ?? ?? ?? ?? ?? by ??? at 2012-03-15
28 = (61227) ?? ??? ?? ?? ?? ????? ????? ?? by ??? at 2012-06-29
30 = (61435) version 28? ?? ?? by ??? at 2012-07-02
31 = ???? ??? ???? ???? ???? ??? ?? ?? by ??? at 2013-06-07
**/

ALTER PROCEDURE [dbo].[P_AddPurchaseOrderByCash]
	  @inbCharacterID bigint --// ??? ID
	, @bitPCRoomFlag bit --// 0=??, 1=PC?
	, @vchProductIDs varchar(8000) --// ,? ?????? ?? ID ???. ?) 203724298,923902,934203948
	, @vchListPirces varchar(8000) --// ,? ?????? ??? ?? ???. ?) 990,2500,500
	, @vchLimitCountsPerCharacter varchar(8000) --// ,? ?????? ???? ??? ?? ?? ??. ???? ?? 0? ?????. ?)0,0,1
	, @vchSelectedItemIDs varchar(8000) --// ,? |? ?????? ??? ??? ID ???. ,? ??? ????? |? ??? ??? ?? ??? ???? ??????.  ?) 48384290,2394802394|3123123132,238201938|49324802|823409238
	, @intPrice int --// ? ?? ??
	, @vchIP varchar(15) --// IP ??
	, @bitGiftFlag bit = 0 --// 0=?? ??, 1=??
	, @nvcReceiverCharacterName nvarchar(30) = NULL --// ?? ?? ??? ??. ??? ??? NULL ??
	, @nvcGiftMemo nvarchar(100) = NULL --// ?? ?? ????? ??? ??? ??. ??? ??? NULL ??
	, @inbPurchaseOrderID bigint = NULL OUTPUT --// ?? ?? ?? ID
	, @vchPurchaseOrderDetailIDs varchar(max) = NULL OUTPUT --// ??? ?? ?? ?? ?? ID
	, @vchRefundableFlags varchar(max) = NULL --// ,? ?????? "??????" ???. ??? ???? 1, ????? 0?? ?????. ?, ??? ?? ? ?? ?????.
	, @bitCheatFlag bit = 0 --// 0=??? ??, 1=??? ??
	, @vchItemOptions varchar(8000) = '' --// ,? |? ?????? ??? ??? ?? ???. ,? ??? ????? |? ??? ??? ?? ??? ???? ??????.
	, @vchLimitCountsPerPartition varchar(8000) = '' --// ,? ?????? ???? ??? ?? ?? ??. ???? ?? 0? ?????. ?)0,0,1
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
	, @inbPurchaseOrderDetailID bigint
	, @inbReceiverCharacterID bigint
	, @dt2Now datetime2(0) = GETDATE();

DECLARE @tblProducts table (
	Seq int IDENTITY(1, 1) NOT NULL PRIMARY KEY,
	ProductID int NOT NULL,
	ListPrice int NOT NULL,
	LimitCountPerCharacter int NOT NULL,
	LimitCountPerPartition int NOT NULL,
	PurchaseCount int NOT NULL DEFAULT(0),
	SelectedItemIDs varchar(max) NOT NULL,
	ItemOptions varchar(max) NULL,
	RefundableFlag bit NOT NULL
);

DECLARE @tblLimitedProducts table (
	ProductID int NOT NULL,
	SaleCount int NOT NULL
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
	SET @nvcGiftMemo = CASE @bitGiftFlag WHEN 0 THEN NULL ELSE @nvcGiftMemo END;

	/**_# ??? ?? ?? ??? ??? ?????. ???? ???? ??? 101176? ?????.*/
	IF @bitGiftFlag = 1
	BEGIN
		SELECT @inbReceiverCharacterID = CharacterID
		FROM dbo.Characters WITH (NOLOCK)
		WHERE CharacterName = @nvcReceiverCharacterName AND DeleteFlag = CAST(0 AS bit);

		IF @@ROWCOUNT = 0
		BEGIN
			SET @intReturnValue = 101176;
			GOTO ErrorHandler;
		END
	END

	INSERT @tblProducts (ProductID, ListPrice, LimitCountPerCharacter, LimitCountPerPartition, SelectedItemIDs, RefundableFlag, ItemOptions)
	SELECT CAST(A.string AS int), CAST(B.string AS int), CAST(C.string AS int), ISNULL(CAST(G.string AS bit), 0), D.string, ISNULL(CAST(E.string AS bit), 0), F.string
	FROM dbo.TF_Split(@vchProductIDs, ',') A
		INNER JOIN dbo.TF_Split(@vchListPirces, ',') B ON A.seq = B.seq
		INNER JOIN dbo.TF_Split(@vchLimitCountsPerCharacter, ',') C ON A.seq = C.seq
		INNER JOIN dbo.TF_Split(@vchSelectedItemIDs, ',') D ON A.seq = D.seq
		LEFT OUTER JOIN dbo.TF_Split(@vchRefundableFlags, ',') E ON A.seq = E.seq
		LEFT OUTER JOIN dbo.TF_Split(@vchItemOptions, ',') F ON A.seq = F.seq
		LEFT OUTER JOIN dbo.TF_Split(@vchLimitCountsPerPartition, ',') G ON A.seq = G.seq
	WHERE A.string > '' AND B.string > '' AND C.string > ''
	OPTION (KEEPFIXED PLAN);

	SET @j = @@ROWCOUNT;

	IF @j = 0
	BEGIN
		SET @intReturnValue = 101150;
		GOTO ErrorHandler;
	END

	/**_# ?? ??? ?? ?? ??? ???? 101147? ?????.*/
	IF EXISTS(SELECT * FROM @tblProducts WHERE LimitCountPerCharacter > 0)
	BEGIN
		;WITH C1 AS (
			SELECT POD.ProductID, COUNT(*) AS PurchaseCount
			FROM dbo.PurchaseOrders PO WITH (NOLOCK, INDEX(IX_NN_PurchaseOrders_CharacterID), FORCESEEK)
				INNER JOIN dbo.PurchaseOrderDetails POD WITH (NOLOCK, INDEX(IX_NN_PurchaseOrderDetails_PurchaseOrderID), FORCESEEK) ON PO.PurchaseOrderID = POD.PurchaseOrderID
				INNER JOIN @tblProducts T ON POD.ProductID = T.ProductID
			WHERE PO.CharacterID = @inbCharacterID
				AND PO.OrderStatusCode = 2
				AND POD.RefundFlag = CAST(0 AS bit)
				AND T.LimitCountPerCharacter > 0
			GROUP BY POD.ProductID
		)
		UPDATE @tblProducts
		SET PurchaseCount = C1.PurchaseCount
		FROM @tblProducts T
			INNER JOIN C1 ON T.ProductID = C1.ProductID;

		IF EXISTS (SELECT * FROM @tblProducts WHERE PurchaseCount >= LimitCountPerCharacter AND LimitCountPerCharacter > 0)
		BEGIN
			SET @intReturnValue = 101147;
			GOTO ErrorHandler;
		END

		/**_# ?? ??? ?? ?? ??? ???? 101147? ?????.*/
		BEGIN TRANSACTION;

		MERGE dbo.LimitedProducts T
		USING (SELECT ProductID FROM @tblProducts WHERE LimitCountPerPartition > 0) S
		ON T.ProductID = S.ProductID
		WHEN MATCHED THEN
			UPDATE SET SaleCount += 1, LastOrderDate = @dt2Now
		WHEN NOT MATCHED BY TARGET THEN
			INSERT (ProductID, SaleCount, LastOrderDate) VALUES (S.ProductID, 1, @dt2Now)
		OUTPUT inserted.ProductID, inserted.SaleCount INTO @tblLimitedProducts (ProductID, SaleCount);

		IF @@ROWCOUNT > 0 AND EXISTS (
			SELECT * 
			FROM @tblProducts A
				INNER JOIN @tblLimitedProducts B ON A.ProductID = B.ProductID
			WHERE A.LimitCountPerPartition < B.SaleCount AND A.LimitCountPerPartition > 0
		)
		BEGIN
			SET @intReturnValue = 101147;
			GOTO ErrorHandler;
		END

		COMMIT TRANSACTION;
	END

	BEGIN TRANSACTION;

	/**_# [PurchaseOrders] ???? INSERT???.*/
	INSERT dbo.PurchaseOrders (CharacterID, ReceiverCharacterID, GiftFlag, PCRoomFlag, Price, IP, OrderDate, OrderStatusCode, PayMethodCode, OrderKey, GiftReceiveFlag, GiftMemo, GiftRejectFlag, NewFlag)
	VALUES (@inbCharacterID, @inbReceiverCharacterID, @bitGiftFlag, @bitPCRoomFlag, @intPrice, dbo.FN_IP2IPNumber(@vchIP), @dt2Now, 1, CASE @bitCheatFlag WHEN 0 THEN 2 ELSE 18 END, NULL, 0, @nvcGiftMemo, CASE @bitGiftFlag WHEN 1 THEN 0 ELSE NULL END, 0);

	SET @inbPurchaseOrderID = SCOPE_IDENTITY();

	/**_# [PurchaseOrderDetails] ???? INSERT???.*/
	INSERT dbo.PurchaseOrderDetails (PurchaseOrderID, ProductID, ListPrice, RefundableFlag)
	OUTPUT inserted.PurchaseOrderDetailID INTO @tblTemp1 (PurchaseOrderDetailID)
	SELECT @inbPurchaseOrderID, ProductID, ListPrice, CASE @bitGiftFlag WHEN 1 THEN 0 ELSE RefundableFlag END
	FROM @tblProducts
	ORDER BY Seq;
	SET @vchPurchaseOrderDetailIDs = '';

	WHILE @i <= @j
	BEGIN
		SELECT @vchSelectedItemIDs2 = SelectedItemIDs, @vchItemOptions2 = ItemOptions
		FROM @tblProducts
		WHERE Seq = @i;

		SELECT @inbPurchaseOrderDetailID = PurchaseOrderDetailID
		FROM @tblTemp1
		WHERE Seq = @i;

		INSERT dbo.SelectedItems (PurchaseOrderDetailID, ItemID, ItemOption)
		SELECT @inbPurchaseOrderDetailID, CAST(A.string AS int), CAST(B.string AS tinyint)
		FROM dbo.TF_Split(@vchSelectedItemIDs2, '|') A
			LEFT OUTER JOIN dbo.TF_Split(@vchItemOptions2, '|') B ON A.seq = B.seq
		OPTION (KEEPFIXED PLAN);

		SET @vchPurchaseOrderDetailIDs += (',' + CAST(@inbPurchaseOrderDetailID AS varchar(30)));

		SET @i += 1;
	END

	COMMIT TRANSACTION;

	SET @vchPurchaseOrderDetailIDs = STUFF(@vchPurchaseOrderDetailIDs, 1, 1, '');
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

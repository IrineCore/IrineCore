USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_PurchaseItemByPetal]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_PurchaseItemByPetal]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_PurchaseItemByPetal] AS' 
END
GO
/**
version : 27
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-02-19
description : ??? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101136 = ??? ?????.
101147 = ?? ?????. ?? ??? ? ?? ??? ???????.
history :
10 = ?? ?? ?? ?? at 2010-10-11
11 = ?? ?? ?? ?? at 2010-11-02
12 = ??? PurchaseOrders.NewFlag ??? default ??? 0? ????? ?? at 2011-04-06
13 = input param. @vchItemOptions ?? at 2011-10-04
15 = TF_Split(), TF_NSplit() ??? ????? ?? by ??? at 2011-11-02
16 = (53070) ?? ?? ?? ?? ?? by ??? at 2012-03-12
18 = 16? ?? ?? ?? by ??? at 2012-03-14
19 = (53070) ?? ?? ?? ?? ?? by ??? at 2012-03-15
20 = ??? ?? ?? ?? ?? ?? by ??? at 2012-03-23
21 = ??? ?? ?? ?? ?? ?? by ??? at 2012-04-03
22 = (61227) ?? ??? ?? ?? ?? ????? ????? ?? by ??? at 2012-06-29
23 = (61435) version 22? ?? ?? by ??? at 2012-07-02
25 = ?? ?? ?? ?? ?? by ??? at 2013-06-05
27 = ???? ??? ???? ???? ???? ??? ?? ?? by ??? at 2013-06-07
**/

ALTER PROCEDURE [dbo].[P_PurchaseItemByPetal]
	  @inbCharacterID bigint --// ??? ID
	, @bitPCRoomFlag bit --// 0=??, 1=PC?
	, @vchProductIDs varchar(8000) --// ,? ?????? ?? ID ???. ?) 203724298,923902,934203948
	, @vchListPirces varchar(8000) --// ,? ?????? ??? ?? ???. ?) 990,2500,500
	, @vchLimitCountsPerCharacter varchar(8000) --// ,? ?????? ???? ??? ?? ?? ??. ???? ?? 0? ?????. ?)0,0,1
	, @vchSelectedItemIDs varchar(8000) --// ,? |? ?????? ??? ??? ID ???. ,? ??? ????? |? ??? ??? ?? ??? ???? ??????.  ?) 48384290,2394802394|3123123132,238201938|49324802|823409238
	, @intPrice int --// ?? ??
	, @vchIP varchar(15) --// IP ??
	, @inbPurchaseOrderID bigint = NULL OUTPUT --// ??? ?? ?? ID
	, @vchItemOptions varchar(8000) = '' --// ,? |? ?????? ??? ??? ?? ???. ,? ??? ????? |? ??? ??? ?? ??? ???? ??????.
	, @vchLimitCountsPerPartition varchar(8000) = '' --// ,? ?????? ???? ??? ?? ?? ??. ???? ?? 0? ?????. ?)0,0,1
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dt2Now datetime2(0) = GETDATE()
	, @inbPetalIncomeID bigint
	, @intDeductAmount int = 0
	, @intTemp int = 0
	, @intRemainPetal int
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
	LimitCountForCharacter int NOT NULL,
	LimitCountForPartition int NOT NULL,
	PurchaseCount int NOT NULL DEFAULT(0),
	SelectedItemIDs varchar(max) NOT NULL,
	ItemOptions varchar(max) NULL
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
	INSERT @tblProducts (ProductID, ListPrice, LimitCountForCharacter, LimitCountForPartition, SelectedItemIDs, ItemOptions)
	SELECT CAST(A.string AS int), CAST(B.string AS int), CAST(C.string AS int), ISNULL(CAST(F.string AS int), 0), D.string, E.string
	FROM dbo.TF_Split(@vchProductIDs, ',') A
		INNER JOIN dbo.TF_Split(@vchListPirces, ',') B ON A.seq = B.seq
		INNER JOIN dbo.TF_Split(@vchLimitCountsPerCharacter, ',') C ON A.seq = C.seq
		INNER JOIN dbo.TF_Split(@vchSelectedItemIDs, ',') D ON A.seq = D.seq
		LEFT OUTER JOIN dbo.TF_Split(@vchItemOptions, ',') E ON A.seq = E.seq
		LEFT OUTER JOIN dbo.TF_Split(@vchLimitCountsPerPartition, ',') F ON A.seq = F.seq
	OPTION (KEEPFIXED PLAN);

	SET @j = @@ROWCOUNT;

	/**_# ?? ??? ?? ?? ??? ???? 101147? ?????.*/
	IF EXISTS(SELECT * FROM @tblProducts WHERE LimitCountForCharacter > 0)
	BEGIN
		WITH C1 AS (
			SELECT POD.ProductID, COUNT(*) AS PurchaseCount
			FROM dbo.PurchaseOrders PO WITH (NOLOCK, INDEX(IX_NN_PurchaseOrders_CharacterID), FORCESEEK)
				INNER JOIN dbo.PurchaseOrderDetails POD WITH (NOLOCK, INDEX(IX_NN_PurchaseOrderDetails_PurchaseOrderID), FORCESEEK) ON PO.PurchaseOrderID = POD.PurchaseOrderID
				INNER JOIN @tblProducts T ON POD.ProductID = T.ProductID
			WHERE PO.CharacterID = @inbCharacterID
				AND PO.OrderStatusCode = 2
				AND POD.RefundFlag = CAST(0 AS bit)
				AND T.LimitCountForCharacter > 0
			GROUP BY POD.ProductID
		)
		UPDATE @tblProducts
		SET PurchaseCount = C1.PurchaseCount
		FROM @tblProducts T
			INNER JOIN C1 ON T.ProductID = C1.ProductID;

		IF EXISTS (SELECT * FROM @tblProducts WHERE PurchaseCount >= LimitCountForCharacter AND LimitCountForCharacter > 0)
		BEGIN
			SET @intReturnValue = 101147;
			GOTO ErrorHandler;
		END

		BEGIN TRANSACTION;

		MERGE dbo.LimitedProducts T
		USING (SELECT ProductID, COUNT(*) AS ProductCount FROM @tblProducts WHERE LimitCountForPartition > 0 GROUP BY ProductID) S
		ON T.ProductID = S.ProductID
		WHEN MATCHED THEN
			UPDATE SET SaleCount += S.ProductCount, LastOrderDate = @dt2Now
		WHEN NOT MATCHED BY TARGET THEN
			INSERT (ProductID, SaleCount, LastOrderDate) VALUES (S.ProductID, S.ProductCount, @dt2Now)
		OUTPUT inserted.ProductID, inserted.SaleCount INTO @tblLimitedProducts (ProductID, SaleCount);

		IF @@ROWCOUNT > 0 AND EXISTS (
			SELECT * 
			FROM (SELECT ProductID, MAX(LimitCountForPartition) AS LimitCountForPartition FROM @tblProducts GROUP BY ProductID) A
				INNER JOIN @tblLimitedProducts B ON A.ProductID = B.ProductID
			WHERE A.LimitCountForPartition < B.SaleCount AND A.LimitCountForPartition > 0
		)
		BEGIN
			SET @intReturnValue = 101147;
			GOTO ErrorHandler;
		END

		COMMIT TRANSACTION;
	END

	BEGIN TRANSACTION;

	/**_# [PurchaseOrders] ???? INSERT???.*/
	INSERT dbo.PurchaseOrders (CharacterID, ReceiverCharacterID, GiftFlag, PCRoomFlag, Price, IP, OrderDate, OrderStatusCode, PayMethodCode, NewFlag)
	VALUES (@inbCharacterID, NULL, 0, @bitPCRoomFlag, @intPrice, dbo.FN_IP2IPNumber(@vchIP), @dt2Now, 2, 3, 0);

	SET @inbPurchaseOrderID = SCOPE_IDENTITY();

	WHILE @intDeductAmount < @intPrice
	BEGIN
		/**_# ?? ?? ?? ???? ????.*/
		/**_## ?? ??? ???? 101136? ?????.*/
		SELECT TOP (1) @inbPetalIncomeID = PTI.PetalIncomeID, @intRemainPetal = PTI.PetalAmount - ISNULL(V.PetalAmount, 0)
		FROM dbo.Characters CH WITH (NOLOCK, INDEX(PK_Characters), FORCESEEK)
			INNER JOIN dbo.PetalIncome PTI WITH (NOLOCK, INDEX(IX_NNI_PetalIncome_AccountID), FORCESEEK) ON PTI.AccountID = CH.AccountID
			LEFT OUTER JOIN dbo.VIX_PetalOutgoTotal V WITH (NOEXPAND, NOLOCK) ON PTI.PetalIncomeID = V.PetalIncomeID
		WHERE CH.CharacterID = @inbCharacterID
		  AND PTI.PetalAmount > ISNULL(V.PetalAmount, 0)
		ORDER BY PTI.PetalIncomeID
		OPTION (KEEPFIXED PLAN, FORCE ORDER);

		IF @@ROWCOUNT = 0
		BEGIN
			SET @inbPurchaseOrderID = NULL;
			SET @intReturnValue = 101136;
			GOTO ErrorHandler;
		END

		SET @intTemp = 
			CASE
				WHEN (@intPrice - @intDeductAmount) <= @intRemainPetal THEN @intPrice - @intDeductAmount
				ELSE @intRemainPetal
			END;

		SET @intDeductAmount += @intTemp;

		/**_# [PetalOutgo] ???? INSERT???.*/
		INSERT dbo.PetalOutgo (PetalIncomeID, PetalOutgoCode, PurchaseOrderID, PetalAmount, OutgoDate)
		VALUES (@inbPetalIncomeID, 1, @inbPurchaseOrderID, @intTemp, GETDATE());
	END

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

	COMMIT TRANSACTION;
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

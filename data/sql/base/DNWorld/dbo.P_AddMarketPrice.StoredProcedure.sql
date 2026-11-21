USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddMarketPrice]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 13
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-04-15
description : ??? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_AddMarketPrice]
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @datTradeDate date;

DECLARE @tblTempPrices table (
	TradeDate datetime2(0) NULL, 
	ItemID int NOT NULL, 
	ItemLevel tinyint NOT NULL, 
	ItemOption tinyint NULL, 
	PayMethodCode tinyint NULL, 
	TotalPrice bigint NULL, 
	TradeCount int NULL, 
	MinPrice bigint NULL, 
	MaxPrice bigint NULL, 
	RepresentativeItemSerial bigint NULL, 
	RegisterCount int NULL
);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT TOP 1 @datTradeDate = TradeDate
	FROM dbo.MarketPrices WITH (NOLOCK)
	ORDER BY MarketPriceID DESC;

	IF @@ROWCOUNT = 0
	BEGIN		
		WITH T AS (
			SELECT CAST(TradeDate AS date) AS TradeDate, ItemID, ItemLevel, ItemOption, ISNULL(PayMethodCode, 1) AS PayMethodCode
				, SUM(CAST(ListPrice AS bigint)) AS TotalPrice
				, COUNT(*) AS TradeCount
				, MIN(ListPrice) AS MinPrice
				, MAX(ListPrice) AS MaxPrice
				, MAX(ItemSerial) AS RepresentativeItemSerial		
			FROM dbo.Trade WITH (NOLOCK, INDEX(IX_NNI_Trade_RegisterDate), FORCESEEK)
			WHERE RegisterDate < CAST(CAST(GETDATE() AS date) AS datetime2(0)) AND TradeStatusCode IN (1,2)	
			GROUP BY CAST(TradeDate AS date), ItemID, ItemLevel, ItemOption, ISNULL(PayMethodCode, 1)
		), RC AS (
			SELECT ItemID, ItemLevel, ItemOption, COUNT(*) AS RegisterCount
			FROM dbo.Trade WITH (NOLOCK) 
			WHERE RegisterDate < CAST(CAST(GETDATE() AS date) AS datetime2(0))
			GROUP BY ItemID, ItemLevel, ItemOption
		)
		INSERT @tblTempPrices (TradeDate, ItemID, ItemLevel, ItemOption, PayMethodCode, TotalPrice, TradeCount, MinPrice, MaxPrice, RepresentativeItemSerial, RegisterCount)
		SELECT T.TradeDate, T.ItemID, T.ItemLevel, T.ItemOption, T.PayMethodCode, ISNULL(T.TotalPrice, 0), ISNULL(T.TradeCount, 0), ISNULL(T.MinPrice, 0), ISNULL(T.MaxPrice, 0), T.RepresentativeItemSerial, ISNULL(RC.RegisterCount, 0)
		FROM T LEFT OUTER JOIN RC ON T.ItemID = RC.ItemID AND T.ItemOption = RC.ItemOption AND T.ItemLevel = RC.ItemLevel;

		INSERT dbo.MarketPrices (TradeDate, ItemID, ItemLevel, ItemOption, PayMethodCode, TotalPrice, TradeCount, MinPrice, MaxPrice, RepresentativeItemSerial, RegisterCount, RepresentativeItemDurability, RepresentativeItemPotential, RepresentativeItemSealCount, RepresentativeItemSoulBoundFlag)
		SELECT A.TradeDate, A.ItemID, A.ItemLevel, A.ItemOption, A.PayMethodCode, A.TotalPrice, A.TradeCount, A.MinPrice, A.MaxPrice, A.RepresentativeItemSerial, A.RegisterCount, I.ItemDurability, I.ItemPotential, I.SealCount, I.SoulBoundFlag
		FROM @tblTempPrices A LEFT OUTER JOIN dbo.MaterializedItems I WITH (NOLOCK, INDEX(IX_UNI_MaterializedItems_ItemSerial), FORCESEEK) ON A.RepresentativeItemSerial = I.ItemSerial;
	END
	ELSE
	BEGIN
		WITH T AS (
			SELECT CAST(TradeDate AS date) AS TradeDate, ItemID, ItemLevel, ItemOption, ISNULL(PayMethodCode, 1) AS PayMethodCode
				, SUM(CAST(ListPrice AS bigint)) AS TotalPrice
				, COUNT(*) AS TradeCount
				, MIN(ListPrice) AS MinPrice
				, MAX(ListPrice) AS MaxPrice
				, MAX(ItemSerial) AS RepresentativeItemSerial		
			FROM dbo.Trade WITH (NOLOCK, INDEX(IX_NNI_Trade_RegisterDate), FORCESEEK)
			WHERE RegisterDate >= CAST(DATEADD(DAY, 1, @datTradeDate) AS datetime2(0))
				AND RegisterDate < CAST(CAST(GETDATE() AS date) AS datetime2(0))
				AND TradeStatusCode IN (1,2)	
			GROUP BY CAST(TradeDate AS date), ItemID, ItemLevel, ItemOption, ISNULL(PayMethodCode, 1)
		), RC AS (
			SELECT ItemID, ItemLevel, ItemOption, COUNT(*) AS RegisterCount
			FROM dbo.Trade WITH (NOLOCK) 
			WHERE RegisterDate >= CAST(DATEADD(DAY, 1, @datTradeDate) AS datetime2(0))
				AND RegisterDate < CAST(CAST(GETDATE() AS date) AS datetime2(0))
			GROUP BY ItemID, ItemLevel, ItemOption
		)		
		INSERT @tblTempPrices (TradeDate, ItemID, ItemLevel, ItemOption, PayMethodCode, TotalPrice, TradeCount, MinPrice, MaxPrice, RepresentativeItemSerial, RegisterCount)
		SELECT T.TradeDate, T.ItemID, T.ItemLevel, T.ItemOption, T.PayMethodCode, ISNULL(T.TotalPrice, 0), ISNULL(T.TradeCount, 0), ISNULL(T.MinPrice, 0), ISNULL(T.MaxPrice, 0), T.RepresentativeItemSerial, ISNULL(RC.RegisterCount, 0)
		FROM T LEFT OUTER JOIN RC ON T.ItemID = RC.ItemID AND T.ItemOption = RC.ItemOption AND T.ItemLevel = RC.ItemLevel;
		INSERT dbo.MarketPrices (TradeDate, ItemID, ItemLevel, ItemOption, PayMethodCode, TotalPrice, TradeCount, MinPrice, MaxPrice, RepresentativeItemSerial, RegisterCount, RepresentativeItemDurability, RepresentativeItemPotential, RepresentativeItemSealCount, RepresentativeItemSoulBoundFlag)
		SELECT A.TradeDate, A.ItemID, A.ItemLevel, A.ItemOption, A.PayMethodCode, A.TotalPrice, A.TradeCount, A.MinPrice, A.MaxPrice, A.RepresentativeItemSerial, A.RegisterCount, I.ItemDurability, I.ItemPotential, I.SealCount, I.SoulBoundFlag
		FROM @tblTempPrices A LEFT OUTER JOIN dbo.MaterializedItems I WITH (NOLOCK, INDEX(IX_UNI_MaterializedItems_ItemSerial), FORCESEEK) ON A.RepresentativeItemSerial = I.ItemSerial;
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

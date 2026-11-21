USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListFluctuationRateForDoors]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2011-12-14

description :
	?? ??? ??? ?? Top 10
	- ????? ?? ??? ?? ????
result set :
	Ranking	int	-- ??
	ItemID	int	-- ???ID
	CurrentAvgPrice(CurrentTradeCount)	int	-- ??? ????(???)
	LastAvgPrice(LastTradeCount)	int	-- ?? ????(???)
	FluctuationRate	int	-- ???(%)
	ItemName nvarchar(50) -- ????
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
	1~2 = ?? ??
**/

CREATE PROCEDURE [dbo].[P_GetListFluctuationRateForDoors]
	  @dtTradeDate date --// ????
	, @vchItemCategoryID varchar(max) --// ??? ???? (??? ?? ??? ??)
	, @inyRankType tinyint --// 1=?? ??, 2=?? ??, 3=??? ??, 4=??? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;

CREATE TABLE #Temp_ItemIDs (ItemID int);
CREATE TABLE #Temp_MarketPrices (ItemID int, CurrentAvgPrice numeric(15,2), CurrentTradeCount numeric(15,2), LastAvgPrice numeric(15,2), LastTradeCount numeric(15,2));

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	-- ???? ????? ???? ?? ??? ??? ????.
	EXEC ('
		INSERT #Temp_ItemIDs (ItemID)
		SELECT ItemID
		  FROM dbo.TradeItems WITH (NOLOCK)
		 WHERE ItemCategoryID IN (' + @vchItemCategoryID + ');');

	-- ?? ????? ???? ????/???, ?? ????/???? ????.
	WITH T1 AS (
		SELECT P.ItemID, CASE WHEN SUM(P.TradeCount) = 0 THEN 0 ELSE SUM(P.TotalPrice) / SUM(P.TradeCount) END AS AvgPrice, P.TradeCount
		  FROM dbo.MarketPrices P WITH (NOLOCK)
			   INNER JOIN #Temp_ItemIDs T ON (P.ItemID = T.ItemID)
		 WHERE P.TradeDate >= @dtTradeDate
		   AND P.TradeDate < DATEADD(DAY, 1, @dtTradeDate)
		   AND P.PayMethodCode = 1
		 GROUP BY P.ItemID, P.TradeCount
	), T2 AS (
		SELECT P.ItemID, CASE WHEN SUM(P.TradeCount) = 0 THEN 0 ELSE SUM(P.TotalPrice) / SUM(P.TradeCount) END AS AvgPrice, P.TradeCount
		  FROM dbo.MarketPrices P WITH (NOLOCK)
			   INNER JOIN #Temp_ItemIDs T ON (P.ItemID = T.ItemID)
		 WHERE P.TradeDate >= DATEADD(DAY, -1, @dtTradeDate)
		   AND P.TradeDate < @dtTradeDate
		   AND P.PayMethodCode = 1
		 GROUP BY P.ItemID, P.TradeCount
	)
	INSERT #Temp_MarketPrices (ItemID, CurrentAvgPrice, CurrentTradeCount, LastAvgPrice, LastTradeCount)
	SELECT T1.ItemID, T1.AvgPrice, T1.TradeCount, T2.AvgPrice, T2.TradeCount
	  FROM T1
		   INNER JOIN T2 ON (T1.ItemID = T2.ItemID)
	 WHERE T1.AvgPrice > 0
	   AND T2.AvgPrice > 0

	-- 1=?? ??, 2=?? ??, 3=??? ??, 4=??? ??
	IF @inyRankType = 1
	BEGIN
		SELECT TOP 10 RANK() OVER(ORDER BY ROUND(CurrentAvgPrice / LastAvgPrice * 100.0, 2) DESC) AS Ranking, T.ItemID, CurrentAvgPrice, LastAvgPrice, ROUND(CurrentAvgPrice / LastAvgPrice * 100.0, 2) AS FluctuationRate, TI.ItemName 
		FROM #Temp_MarketPrices T LEFT OUTER JOIN dbo.TradeItems TI WITH (NOLOCK) ON T.ItemID = TI.ItemID;
	END
	ELSE IF @inyRankType = 2
	BEGIN
		SELECT TOP 10 RANK() OVER(ORDER BY ROUND(CurrentAvgPrice / LastAvgPrice * 100.0, 2)) AS Ranking, T.ItemID, CurrentAvgPrice, LastAvgPrice, ROUND(CurrentAvgPrice / LastAvgPrice * 100.0, 2) AS FluctuationRate, TI.ItemName 
		FROM #Temp_MarketPrices T LEFT OUTER JOIN dbo.TradeItems TI WITH (NOLOCK) ON T.ItemID = TI.ItemID;
	END
	ELSE IF @inyRankType = 3
	BEGIN
		SELECT TOP 10 RANK() OVER(ORDER BY ROUND(CurrentTradeCount / LastTradeCount * 100.0, 2) DESC) AS Ranking, T.ItemID, CurrentTradeCount, LastTradeCount, ROUND(CurrentTradeCount / LastTradeCount * 100.0, 2) AS FluctuationRate, TI.ItemName 
		FROM #Temp_MarketPrices T LEFT OUTER JOIN dbo.TradeItems TI WITH (NOLOCK) ON T.ItemID = TI.ItemID;
	END
	ELSE IF @inyRankType = 4
	BEGIN
		SELECT TOP 10 RANK() OVER(ORDER BY ROUND(CurrentTradeCount / LastTradeCount * 100.0, 2)) AS Ranking, T.ItemID, CurrentTradeCount, LastTradeCount, ROUND(CurrentTradeCount / LastTradeCount * 100.0, 2) AS FluctuationRate, TI.ItemName 
		FROM #Temp_MarketPrices T LEFT OUTER JOIN dbo.TradeItems TI WITH (NOLOCK) ON T.ItemID = TI.ItemID;
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

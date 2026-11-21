USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_GetListExpensiveTradeItem]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 21
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-05-04
description : ???? ?? ??? ??? ???? ???? ?????.
return value : 
	version 1 : ItemID/int, TradeCount/int, RegistedCount/int, MaxPrice/int, ItemName/nvarchar(50), ItemRankCode/tinyint
	version 2 : version1 + ItemLevel/tinyint, ItemCategoryID/tinyint, RepresentativeItemSerial/bigint
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_DRS_GetListExpensiveTradeItem]
	  @dt2StartDate datetime2 --// ?? ????
	, @dt2EndDate datetime2 --// ?? ????	
	, @intVersion int = 1 --// ??? ??
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
	/**_# [MarketPrices] ???? SELECT ???.*/
	IF @intVersion = 2
		WITH A AS (
			SELECT RANK() OVER (PARTITION BY ItemID ORDER BY MaxPrice DESC) AS Ranking, ItemID, ItemLevel, MaxPrice, RepresentativeItemSerial
			FROM dbo.MarketPrices WITH (NOLOCK, INDEX(IX_NNI_MarketPrices_TradeDate))	
			WHERE TradeDate >= @dt2StartDate AND TradeDate < @dt2EndDate
		), C AS (
			SELECT ItemID, ItemLevel, MaxPrice, RepresentativeItemSerial
			FROM A
			WHERE Ranking = 1
		), D AS (
			SELECT ItemID, SUM(TradeCount) AS TradeCount, SUM(RegisterCount) AS RegistedCount
			FROM dbo.MarketPrices WITH (NOLOCK, INDEX(IX_NNI_MarketPrices_TradeDate))
			WHERE TradeDate >= @dt2StartDate AND TradeDate < @dt2EndDate		
			GROUP BY ItemID
		), E AS (
			SELECT C.ItemID, C.ItemLevel, C.MaxPrice, D.TradeCount, C.RepresentativeItemSerial, D.RegistedCount
			FROM C JOIN D ON C.ItemID = D.ItemID
		)
		SELECT TOP 100 E.ItemID, ISNULL(E.TradeCount, 0) AS TradeCount, ISNULL(E.RegistedCount, 0) AS RegistedCount, ISNULL(E.MaxPrice, 0) AS MaxPrice, TI.ItemName, TI.ItemRankCode, E.ItemLevel, TI.ItemCategoryID, E.RepresentativeItemSerial
		FROM E LEFT OUTER JOIN dbo.TradeItems TI WITH (NOLOCK, INDEX(PK_TradeItems), FORCESEEK) ON E.ItemID = TI.ItemID
		ORDER BY E.MaxPrice DESC
		OPTION (MAXDOP 1);

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

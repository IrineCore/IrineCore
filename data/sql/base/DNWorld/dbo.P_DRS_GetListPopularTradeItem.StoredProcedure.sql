USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_GetListPopularTradeItem]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 6
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-05-04
description : ????? ?? ?? ??? ???? ?????.
result set : 
return value : ItemID/int, TradeCount/int, RegistedCount/int, AveragePrice/int, ItemName/nvarchar(50), ItemRankCode/tinyint
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_DRS_GetListPopularTradeItem]
	  @dt2StartDate datetime2 --// ?? ????
	, @dt2EndDate datetime2 --// ?? ????	
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
	;WITH M AS (
		SELECT ItemID, SUM(TradeCount) AS TradeCount, AVG(TotalPrice/TradeCount) AS AveragePrice, SUM(RegisterCount) AS RegistedCount
		FROM dbo.MarketPrices WITH (NOLOCK, INDEX(IX_NNI_MarketPrices_TradeDate), FORCESEEK)
		WHERE TradeDate >= @dt2StartDate AND TradeDate < @dt2EndDate
		GROUP BY ItemID
	)
	SELECT TOP 20 M.ItemID, M.TradeCount, M.RegistedCount, M.AveragePrice, TI.ItemName, TI.ItemRankCode
	FROM M LEFT OUTER JOIN dbo.TradeItems TI WITH (NOLOCK, INDEX(PK_TradeItems), FORCESEEK) ON M.ItemID = TI.ItemID
	ORDER BY M.TradeCount DESC
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

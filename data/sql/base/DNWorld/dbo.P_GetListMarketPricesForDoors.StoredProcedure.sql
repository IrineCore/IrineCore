USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListMarketPricesForDoors]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 11
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2011-11-22

description :
	??? ??? ?? ??
	- ?? ??? ??? ??? ??
	- ????? ??? ??? ??
result set :
	TradeDate	date	-- ????
	AvgPrice	int	-- ?? ????
	TradeCount	int	-- ???
	MinPrice    int -- ?? ????
    MaxProce    int -- ?? ????
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
	1~4 = ?? ??
	5 = ??? ??? ??? ????? ??? ??     by ??? at 2011-12-06
	6~8 = ??? ??     by ??? at 2011-12-15
	9~10 = ????, ???? ?? by ??? at 2012-04-26
	11 = result set ?? ?? by ??? at 2012-09-04
**/

CREATE PROCEDURE [dbo].[P_GetListMarketPricesForDoors]
	  @intItemID int --// ??? ID
	, @dtTradeStartDate date --// ?? ???
	, @dtTradeEndDate date --// ?? ???
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;

DECLARE @Temp_MarketPrices Table (TradeDate date, AvgPrice int, TradeCount int, MaxPrice int, MinPrice int);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	WHILE (@dtTradeStartDate <= @dtTradeEndDate)
	BEGIN

		INSERT @Temp_MarketPrices (TradeDate, AvgPrice, TradeCount, MinPrice, MaxPrice)
		SELECT @dtTradeStartDate, CASE WHEN SUM(TradeCount) = 0 THEN 0 ELSE SUM(TotalPrice) / SUM(TradeCount) END, SUM(TradeCount) AS TradeCount, MIN(MinPrice), MAX(MaxPrice)
		  FROM dbo.MarketPrices WITH (NOLOCK)
		 WHERE ItemID = @intItemID
		   AND TradeDate >= @dtTradeStartDate
		   AND TradeDate < DATEADD(DAY, 1, @dtTradeStartDate)
		   AND PayMethodCode = 1;

		SET @dtTradeStartDate = DATEADD(DAY, 1, @dtTradeStartDate);

	END

	SELECT TradeDate, AvgPrice, TradeCount, MinPrice, MaxPrice FROM @Temp_MarketPrices ORDER BY TradeDate;

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

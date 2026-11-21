USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListMarketPrice]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-04-15
description : ?? 7? ??? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListMarketPrice]
	  @intItemID int --// ??? ID
	, @inyItemLevel tinyint --// ??? ?? ??
	, @inyItemOption tinyint --// ??? ??
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
	SELECT PayMethodCode
		, CASE WHEN SUM(TradeCount) = 0 THEN 0 ELSE SUM(TotalPrice) / SUM(TradeCount) END AS AvgPrice
		, MIN(MinPrice) AS MinPrice
		, MAX(MaxPrice) AS MaxPrice
	FROM dbo.MarketPrices WITH (NOLOCK)
	WHERE TradeDate >= CAST(GETDATE() - 7 AS date)
		AND ItemID = @intItemID
		AND ItemLevel = @inyItemLevel
		AND ItemOption = @inyItemOption
	GROUP BY PayMethodCode;
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

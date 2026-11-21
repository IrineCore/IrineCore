USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_TMS_DailyMarketPrices]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : otooi@eyedentitygames.com
created date : 2013-02-04

description :
	(TIMOSE) ?? ??? ?? (dbo.DailyMarketPrices)
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
	1 = ????
**/

CREATE PROCEDURE [dbo].[P_TMS_DailyMarketPrices]
	@datBasisDate date
WITH EXECUTE AS 'SPExecutor'
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
	SELECT MarketPriceID, ItemID, ItemLevel, ItemOption, TradeDate, PayMethodCode, TotalPrice, TradeCount, MinPrice, MaxPrice, RegisterCount
		, CAST(NULL AS tinyint) AS PublisherID, CAST(NULL AS tinyint) AS NationalityCode, CAST(NULL AS smallint) AS PartitionID, ISNULL(WorldID, 0) AS WorldID
	FROM dbo.DailyMarketPrices WITH (NOLOCK)
	WHERE (TradeDate >= @datBasisDate) AND (TradeDate < DATEADD(day, 1, @datBasisDate));
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

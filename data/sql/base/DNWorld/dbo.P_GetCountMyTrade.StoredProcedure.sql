USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetCountMyTrade]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-31
description : ?? ?? ??? ??? ???? ???? ??????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetCountMyTrade]
	  @inbCharacterID bigint --// ??? ID
	, @intSellingCount int OUTPUT --// ?? ?? ??? ?
	, @intExpiredCount int OUTPUT --// ?? ??? ??? ??? ?
	, @intTotalCount int = NULL OUTPUT --// ???, ??????, ???? ?? ? ?
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dt2Now datetime2(0) = GETDATE()
	, @intTradeCount0 int
	, @intTradeCount1 int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SET @intSellingCount = 0;
	SET @intExpiredCount = 0;

	WITH C1 AS (
		SELECT CASE TradeStatusCode WHEN 0 THEN TradeCount ELSE NULL END AS [0]
			, CASE TradeStatusCode WHEN 1 THEN TradeCount ELSE NULL END AS [1]
		FROM dbo.VIX_TradeTotals WITH (NOEXPAND, NOLOCK)
		WHERE CharacterID = @inbCharacterID AND TradeStatusCode BETWEEN 0 AND 1
	)
	SELECT @intTradeCount0 = ISNULL(SUM([0]), 0), @intTradeCount1 = ISNULL(SUM([1]), 0)
	FROM C1;

	IF @@ROWCOUNT > 0
	BEGIN
		SELECT @intExpiredCount = COUNT(*)
		FROM dbo.Trade WITH (NOLOCK)
		WHERE CharacterID = @inbCharacterID AND TradeStatusCode = 0 AND TradeExpireDate < @dt2Now;

		SET @intSellingCount = @intTradeCount0 - @intExpiredCount;
	END

	SET @intTotalCount = @intTradeCount0 + @intTradeCount1;
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

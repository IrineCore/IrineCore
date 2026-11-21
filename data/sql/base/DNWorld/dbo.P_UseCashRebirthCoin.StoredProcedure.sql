USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_UseCashRebirthCoin]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 8
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-26
description : ?? ?? ??? ?????. (1?? ?????.)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103158 = ?? ?? ??? ????.
history :
6 = IP? ????? ?? at 2010-06-28
7 = ??? ?? ?? at 2011-03-18
**/

CREATE PROCEDURE [dbo].[P_UseCashRebirthCoin]
	  @inbCharacterID bigint --// ??? ID
	, @intChannelID int --// ?? ID
	, @intMapID int --// ? ID
	, @vchIP varchar(15) = NULL --// ????? ?? IP ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dt2Now datetime2(0) = GETDATE()
	, @inbCashRebirthCoinID bigint
	, @insRemainCoinCountBefore smallint = 0
	, @insRemainCoinCountAfter smallint
	, @insUsedCoinCount smallint
	, @intRemainCoinPriceBefore int
	, @intRemainCoinPriceAfter int
	, @intUsedCoinPrice int
	, @insCoinCountAfter smallint
	, @insCoinCountBefore smallint
	, @intIP int = dbo.FN_IP2IPNumber(@vchIP);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	BEGIN TRANSACTION;

	/**_# [CashRebirthCoins]???? UPDATE???.*/
	UPDATE dbo.CashRebirthCoins
	SET @inbCashRebirthCoinID = CashRebirthCoinID
		, @insRemainCoinCountBefore = RemainCoinCount
		, @intRemainCoinPriceBefore = RemainCoinPrice
		, @insRemainCoinCountAfter = RemainCoinCount = CASE WHEN RemainCoinCount >= 1 THEN RemainCoinCount - 1 ELSE 0 END
		, @intRemainCoinPriceAfter = RemainCoinPrice = CoinPrice * CASE WHEN RemainCoinCount >= 1 THEN RemainCoinCount - 1 ELSE 0 END / CoinCount
	WHERE CharacterID = @inbCharacterID AND consumptionTargetFlag = CAST(1 AS bit);

	/**_## ??? ?? ?? ??? ??? 103158? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103158;
		GOTO ErrorHandler;
	END

	/**_## ?? ? ??? 0? ???? ???? ? ???? ?? consmuptionTargetFlag? 1? UPDATE???.*/
	IF @insRemainCoinCountAfter = 0
	BEGIN
		DELETE dbo.CashRebirthCoins WHERE CashRebirthCoinID = @inbCashRebirthCoinID;

		WITH C1 AS (
			SELECT TOP (1) CashRebirthCoinID
			FROM dbo.CashRebirthCoins WITH (UPDLOCK)
			WHERE CharacterID = @inbCharacterID AND CashRebirthCoinID > @inbCashRebirthCoinID
		)
		UPDATE dbo.CashRebirthCoins
		SET consumptionTargetFlag = 1
		FROM dbo.CashRebirthCoins CR
			INNER JOIN C1 ON CR.CashRebirthCoinID = C1.CashRebirthCoinID;
	END

	COMMIT TRANSACTION;

	SET @insUsedCoinCount = @insRemainCoinCountBefore - @insRemainCoinCountAfter;
	SET @intUsedCoinPrice = @intRemainCoinPriceBefore - @intRemainCoinPriceAfter;

	SELECT @insCoinCountAfter = RemainCoinCount, @insCoinCountBefore = RemainCoinCount + 1
	FROM dbo.VIX_CashRebirthCoinTotals WITH (NOEXPAND, NOLOCK)
	WHERE CharacterID = @inbCharacterID;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @insCoinCountAfter = 0;
		SET @insCoinCountBefore = 1;
	END

	IF @insUsedCoinCount > 0
		EXEC dbo.P_AddCashRebirthCoinLog
			  @dt2LogDate = @dt2Now
			, @inbCharacterID = @inbCharacterID
			, @insCoinCountBefore = @insCoinCountBefore
			, @insCoinCountAfter = @insCoinCountAfter
			, @intMapID = @intMapID
			, @intIP = @intIP;

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

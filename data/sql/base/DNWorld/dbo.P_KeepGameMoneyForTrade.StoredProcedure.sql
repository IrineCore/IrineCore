USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_KeepGameMoneyForTrade]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2012-12-05
description : ?? ?? ??? ?? ?? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103102 = ???? ???? ????.
103123 = ??? ?????.
history :
2 = CoinChangeCode ?? by ??? at 2012-12-06
4 = input param. @intTotalCashAmount ?? by ??? at 2013-05-16
**/

CREATE PROCEDURE [dbo].[P_KeepGameMoneyForTrade]
	  @vchGameMoneyTradeKey varchar(50) --// ?? ?? ?? ? (C2C ???? ??? ??)
	, @inbCharacterID bigint --// ??? ID
	, @inbGameMoneyAmount bigint --// ??? ?? ??
	, @binCheckSumBin binary(4) = NULL --// Checksum
	, @intTotalCashAmount int = NULL --// ??? ?? ??? ??? ??? ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @inbCurrentCoin bigint;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SET @inbGameMoneyAmount = - @inbGameMoneyAmount;

	EXEC @intReturnValue = dbo.P_ModCoin
		  @inbCharacterID = @inbCharacterID
		, @inyCoinChangeCode = 49
		, @inbCoinChangeFKey = 0
		, @inbCoin = @inbGameMoneyAmount
		, @intChannelID = 0
		, @intMapID = 0
		, @inbCurrentCoin = @inbCurrentCoin OUTPUT
		, @binCheckSumBin = @binCheckSumBin ;

	IF @intReturnValue <> 0
		GOTO ErrorHandler;

	SET @inbGameMoneyAmount = - @inbGameMoneyAmount;

	INSERT dbo.GameMoneyTrade (GameMoneyTradeKey, CharacterID, GameMoneyAmount, IncomeDate, GameMoneyTradeStatusCode, ToCharacterID, TradeDate, TotalCashAmount)
	VALUES (@vchGameMoneyTradeKey, @inbCharacterID, @inbGameMoneyAmount, GETDATE(), 1, NULL, NULL, @intTotalCashAmount);
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

USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_TransferGameMoneyForTrade]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 7
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2012-12-05
description : ??? ?? ????? ????? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103355 = ??? ????? ???? ????.
103381 = ?? ??? ???????.
103382 = ???? ?? ??? ???????.
history :
7 = input param. @intPaidCashAmount ?? by ??? at 2013-05-16
**/

CREATE PROCEDURE [dbo].[P_TransferGameMoneyForTrade]
	  @vchGameMoneyTradeKey varchar(50) --// ?? ?? ?? ? (C2C ???? ??? ??)
	, @inbGameMoneyAmount bigint --// ??? ?? ??
	, @inbToCharacterID bigint --// ?? ?? ??? ID (???? ?? ????? ??? ?????)
	, @binCheckSumBin binary(4) = NULL --// checksum
	, @intPaidCashAmount int = NULL --// ?? ??? ??? ?? (C2C ??? ?????)
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @inbCurrentCoin bigint
	, @inbCharacterID bigint
	, @inyGameMoneyTradeStatusCode tinyint
	, @inyCoinChangeCode tinyint
	, @dtmNow datetime = GETDATE();

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	BEGIN TRANSACTION;

	SELECT @inbCharacterID = CharacterID, @inyGameMoneyTradeStatusCode = GameMoneyTradeStatusCode
	FROM dbo.GameMoneyTrade WITH (UPDLOCK)
	WHERE GameMoneyTradeKey = @vchGameMoneyTradeKey
		AND GameMoneyAmount = @inbGameMoneyAmount

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103355;
		GOTO ErrorHandler;
	END

	IF @inyGameMoneyTradeStatusCode <> 1
	BEGIN
		IF @inyGameMoneyTradeStatusCode = 2
			SET @intReturnValue = 103382;

		ELSE IF @inyGameMoneyTradeStatusCode = 3
			SET @intReturnValue = 103381;

		ELSE 
			SET @intReturnValue = 103355;
		GOTO ErrorHandler;
	END

	SET @inyGameMoneyTradeStatusCode = 
		CASE @inbCharacterID
			WHEN @inbToCharacterID THEN 2
			ELSE 3
		END;

	SET @inyCoinChangeCode = 
		CASE @inbCharacterID
			WHEN @inbToCharacterID THEN 50
			ELSE 51
		END;

	UPDATE dbo.GameMoneyTrade
	SET GameMoneyTradeStatusCode = @inyGameMoneyTradeStatusCode
		, ToCharacterID = CASE @inyGameMoneyTradeStatusCode WHEN 2 THEN NULL ELSE @inbToCharacterID END
		, TradeDate = @dtmNow
		, PaidCashAmount = CASE @inyGameMoneyTradeStatusCode WHEN 2 THEN NULL WHEN 3 THEN @intPaidCashAmount END
	WHERE GameMoneyTradeKey = @vchGameMoneyTradeKey;

	COMMIT TRANSACTION;

	EXEC @intReturnValue = dbo.P_ModCoin
		  @inbCharacterID = @inbToCharacterID
		, @inyCoinChangeCode = @inyCoinChangeCode
		, @inbCoinChangeFKey = 0
		, @inbCoin = @inbGameMoneyAmount
		, @intChannelID = 0
		, @intMapID = 0
		, @inbCurrentCoin = @inbCurrentCoin OUTPUT
		, @binCheckSumBin = @binCheckSumBin;

	IF @intReturnValue <> 0
		GOTO ErrorHandler;

END TRY
BEGIN CATCH
	UPDATE dbo.GameMoneyTrade
	SET GameMoneyTradeStatusCode = 1
		, ToCharacterID = NULL
		, TradeDate = NULL
	WHERE GameMoneyTradeKey = @vchGameMoneyTradeKey
		AND GameMoneyTradeStatusCode = @inyGameMoneyTradeStatusCode
		AND TradeDate = @dtmNow;

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

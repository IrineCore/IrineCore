USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_TradeCalculate]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-31
description : ?? ?? ???? ?????. (??)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103176 = ??? ???? ????.
103204 = ??? ?? ??? ?? 42? ??? ??? ? ????.
history :
4 = ?? ? ??? ?? ??? 42? ??? ???? ??? ???? ????? ?? at 2010-07-06
5 = ??? ???? ?? ?? (output param. @inyPayMethodCode ??) at 2011-01-24
**/

CREATE PROCEDURE [dbo].[P_TradeCalculate]
	  @inbCharacterID bigint --// ??? ID
	, @intTradeID int --// ?? ID
	, @intChannelID int --// ?? ID
	, @intMapID int --// ? ID
	, @intCalculateResult int OUTPUT --// ??? ?? (?? - ???)
	, @inyPayMethodCode tinyint = 1 OUTPUT --// 1=????, 3=??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dt2Now datetime2(0) = GETDATE()
	, @intPrice int
	, @intTradeCommission int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	BEGIN TRANSACTION;

	UPDATE dbo.Trade
	SET @intPrice = Price
		, @inyPayMethodCode = ISNULL(PayMethodCode, 1)
		, @intTradeCommission = TradeCommission
		, @intCalculateResult = Price - TradeCommission
		, TradeStatusCode = 2
	WHERE TradeID = @intTradeID AND CharacterID = @inbCharacterID AND TradeStatusCode = 1;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103176;
		GOTO ErrorHandler;
	END

	IF @inyPayMethodCode = 1 AND (SELECT Coin + @intPrice FROM dbo.CharacterStatus WITH (NOLOCK) WHERE CharacterID = @inbCharacterID) > 4200000000
	BEGIN
		SET @intReturnValue = 103204;
		GOTO ErrorHandler;
	END

	COMMIT TRANSACTION;

	/**_# ????? ??? ???? ?? ?? ??? ?? ???? ?????.*/
	IF @inyPayMethodCode = 1
	BEGIN
		EXEC @intReturnValue = dbo.P_ModCoinForSubProc
			  @inbCharacterID = @inbCharacterID
			, @inyCoinChangeCode = 23
			, @inbCoinChangeFKey = @intTradeID
			, @inbCoin = @intPrice
			, @intChannelID = @intChannelID
			, @intMapID = @intMapID
			, @dt2LogDate = @dt2Now;

		EXEC @intReturnValue = dbo.P_ModCoinForSubProc
			  @inbCharacterID = @inbCharacterID
			, @inyCoinChangeCode = 24
			, @inbCoinChangeFKey = @intTradeID
			, @inbCoin = @intTradeCommission
			, @intChannelID = @intChannelID
			, @intMapID = @intMapID
			, @dt2LogDate = @dt2Now;
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

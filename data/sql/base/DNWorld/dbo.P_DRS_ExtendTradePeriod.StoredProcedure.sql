USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_ExtendTradePeriod]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 9
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-01-08
description : ??? ?????? ????? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
1030000026 = ??? ???? ????.
1030000024 = ??? ??? ?????.
1030000027 = ??? ??? ID? ??? ??? ID? ???? ????.
1030000059 = ?? ?? ??? ??? ? ????.
1030000060 = ?? ?? ??? ?? ? ? ????.	
**/

CREATE PROCEDURE [dbo].[P_DRS_ExtendTradePeriod]
	  @intTradeID int --// ?? ID
	, @intPrice int --// ?? ??
	, @intListPrice int --// ?? ??
	, @intRegisterCommission int --// ?? ?? ???
	, @intTradeCommission int --// ?? ?? ???	
	, @inySellPeriod int --// ?? ?? (??)
	, @bitPremiumFlag bit --// 0=?? ??, 1=???? ??	
	, @inbSellerCharacterID bigint = NULL -- // ??? ??? ID	
	, @inyCharacterLevel tinyint = NULL OUTPUT -- // ??? ??
	, @intCharacterExp int = NULL OUTPUT -- // ??? ???
	, @inbCoin bigint = NULL OUTPUT -- // ??? ??
	, @inbWarehouseCoin bigint = NULL OUTPUT -- // ??? ????
	, @insMaxRegisterCount1 smallint = NULL --// ??? ? ?? ?? ?? (?? ?? ?? ?? ?? ? ??? ?? ?)
	, @insMaxRegisterCount2 smallint = NULL --// ??? ? ?? ?? ?? (??? AM 04:00 ?? ???? ??? ? ?? ?? ??)
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
		, @inbCharacterID bigint
		, @inbChangedCoin bigint
		, @inbCurrentCoin bigint
		, @dt2Now datetime2(0) = GETDATE()
		, @intTradeCount int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END
BEGIN TRY
	/**_# ?? ? ??? ??? ??? ?????.*/
	IF @insMaxRegisterCount1 IS NOT NULL
	BEGIN
		SELECT @intTradeCount = TradeCount
		FROM dbo.VIX_TradeTotals WITH (NOEXPAND, NOLOCK)
		WHERE CharacterID = @inbCharacterID AND TradeStatusCode = 0
		OPTION (KEEPFIXED PLAN);

		IF @@ROWCOUNT = 0
			SET @intTradeCount = 0;

		IF @insMaxRegisterCount1 <= @intTradeCount
		BEGIN
			SET @intReturnValue = 1030000059;
			GOTO ErrorHandler;
		END
	END

	IF @bitPremiumFlag = 0 AND @insMaxRegisterCount2 IS NOT NULL
	BEGIN
		SELECT @intTradeCount = COUNT(*)
		FROM dbo.Trade WITH (NOLOCK, INDEX(IX_NNI_Trade_CharacterID_TradeStatusCode), FORCESEEK)
		WHERE CharacterID = @inbCharacterID
			AND PremiumFlag = CAST(0 AS bit)
			AND RegisterDate >= DATEADD(DAY, - DATEPART(WEEKDAY, DATEADD(HOUR, 4, DATEADD(DAY, DATEDIFF(DAY, 0, @dt2Now) + CASE WHEN DATEPART(HOUR, @dt2Now) BETWEEN 0 AND 3 THEN 0 ELSE 1 END, 0))), DATEADD(HOUR, 4, DATEADD(DAY, DATEDIFF(DAY, 0, @dt2Now) + CASE WHEN DATEPART(HOUR, @dt2Now) BETWEEN 0 AND 3 THEN 0 ELSE 1 END, 0)))
		OPTION (KEEPFIXED PLAN);

		IF @insMaxRegisterCount2 <= @intTradeCount
		BEGIN
			SET @intReturnValue = 1030000060;
			GOTO ErrorHandler;
		END
	END

	BEGIN TRANSACTION;
	
	/**_# ??? ?????? ?? ?? UPDATE ???. */
	UPDATE dbo.Trade
	SET @inbCharacterID = CharacterID,
		TradeStatusCode = 0,
		Price = @intPrice, 
		ListPrice = @intListPrice, 
		RegisterCommission = @intRegisterCommission,
		TradeCommission = @intTradeCommission,
		PremiumFlag = @bitPremiumFlag,
		RegisterDate = @dt2Now,
		TradeExpireDate = DATEADD(HOUR, @inySellPeriod, @dt2Now)
	WHERE TradeID = @intTradeID;
	
	/**_# ?? ???? ?? ?? */
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 1030000026;
		GOTO ErrorHandler;
	END

	/**_# ??? ???? ?? ???? ?? ?? */

	IF @inbSellerCharacterID IS NOT NULL
	BEGIN
		IF @inbSellerCharacterID <> @inbCharacterID
		BEGIN
			SET @intReturnValue = 1030000027;
			GOTO ErrorHandler;
		END
	END    

	/**_# ?? ??? ??? ?? */
	IF (
		SELECT Coin 
		FROM dbo.CharacterStatus WITH (NOLOCK) 
		WHERE CharacterID = @inbCharacterID
	) < @intRegisterCommission
	BEGIN
		SET @intReturnValue = 1030000024;
		GOTO ErrorHandler;
	END

	/**_# ?? ???? ?????. */	
	UPDATE dbo.CharacterStatus
	SET @inbCurrentCoin = Coin
		, Coin = Coin - @intRegisterCommission
		, @inyCharacterLevel = CharacterLevel
		, @intCharacterExp = CharacterExp
		, @inbCoin = Coin - @intRegisterCommission
		, @inbWarehouseCoin = WarehouseCoin
	WHERE CharacterID = @inbCharacterID;
		
	COMMIT TRANSACTION;

	/**_# ???? ??? ????. */

	SET @inbChangedCoin = @intRegisterCommission * -1;
	SET @inbCurrentCoin = @inbCurrentCoin - @intRegisterCommission;

	EXEC @intReturnValue = dbo.P_AddCoinLog
		  @dt2LogDate = @dt2Now
		, @inbCharacterID = @inbCharacterID
		, @inyCoinTypeCode = 1
		, @inyCoinChangeCode = 59
		, @inbCurrentCoin = @inbCurrentCoin
		, @inbChangedCoin = @inbChangedCoin
		, @intChannelID = NULL
		, @intMapID = NULL
		, @inbCoinChangeFKey = NULL;

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

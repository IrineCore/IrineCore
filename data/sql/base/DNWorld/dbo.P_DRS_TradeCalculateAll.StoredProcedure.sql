USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_TradeCalculateAll]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-04-22
description : ??? ???? ?? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
1030000058 = 42? ??? ???? ??? ? ????.
**/

CREATE PROCEDURE [dbo].[P_DRS_TradeCalculateAll]
	  @inbCharacterID bigint --// ??? ID	
	, @inbCoin bigint OUTPUT --// ?? ? ????
	, @inbWarehouseCoin bigint OUTPUT --// ?? ? ????
	, @intCharacterExp int OUTPUT --// ?? ? ??? ???
	, @inyCharacterLevel tinyint OUTPUT --// ?? ? ??? ???
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @intCalculateResultTemp int
	, @intTradeID int
	, @i int = 1
	, @j int
	, @inyPayMethodCode tinyint;

DECLARE @tblTrades table (seq int IDENTITY(1, 1) NOT NULL PRIMARY KEY, TradeID int NOT NULL, PayMethodCode tinyint NOT NULL);
DECLARE @inbTmp1 bigint, @inbTmp2 bigint;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT @inbTmp1 = Coin FROM dbo.CharacterStatus WITH (NOLOCK, FORCESEEK) WHERE CharacterID = @inbCharacterID OPTION (KEEPFIXED PLAN);
	SELECT @inbTmp2 = SUM(Price) FROM dbo.Trade WITH (NOLOCK, INDEX(IX_NNI_Trade_CharacterID_TradeStatusCode), FORCESEEK) WHERE CharacterID = @inbCharacterID AND TradeStatusCode = 1 AND (PayMethodCode = 1 OR PayMethodCode IS NULL) OPTION (KEEPFIXED PLAN);

	IF (@inbTmp1 + @inbTmp2) > 4200000000
	BEGIN
		SET @intReturnValue = 1030000058;
		GOTO ErrorHandler;
	END
	INSERT @tblTrades (TradeID, PayMethodCode)
	SELECT TradeID, ISNULL(PayMethodCode, 1) FROM dbo.Trade WITH (NOLOCK, INDEX(IX_NNI_Trade_CharacterID_TradeStatusCode), FORCESEEK) WHERE CharacterID = @inbCharacterID AND TradeStatusCode = 1 OPTION (KEEPFIXED PLAN);

	SET @j = @@ROWCOUNT;	

	WHILE @i <= @j
	BEGIN
		SELECT @intTradeID = TradeID, @inyPayMethodCode = PayMethodCode FROM @tblTrades WHERE seq = @i;

		EXEC @intReturnValue = dbo.P_TradeCalculate
			  @inbCharacterID = @inbCharacterID
			, @intTradeID = @intTradeID
			, @intChannelID = NULL
			, @intMapID = NULL
			, @intCalculateResult = @intCalculateResultTemp OUTPUT
			, @inyPayMethodCode = @inyPayMethodCode OUTPUT;

		SET @i += 1;
	END

	SELECT @inbCoin = Coin, @inbWarehouseCoin = WarehouseCoin, @intCharacterExp = CharacterExp, @inyCharacterLevel = CharacterLevel
	FROM dbo.CharacterStatus WITH (NOLOCK)
	WHERE CharacterID = @inbCharacterID;

END TRY
BEGIN CATCH	
	GOTO ErrorHandler;
END CATCH;

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

IF @intReturnValue IS NULL OR @intReturnValue = 0
	EXEC @intReturnValue = dbo.P_AddErrorLog @intPROCID = @@PROCID;

RETURN @intReturnValue;
GO

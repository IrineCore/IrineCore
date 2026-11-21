USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_TradeCalculateAll]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 10
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
5 = ?? ?? ?? ?? (output param. @intCalculateResultForPetal ??) at 2011-01-24
6 = @intCalculateResultForPetal ?? ?? 0? ?? ?? at 2011-01-26
8 = INDEX Hint, ForceSeek ??. at 2012-03-16 by dailykim
10 = 8? ???? ?? ?? by ??? at 2012-03-22
**/

CREATE PROCEDURE [dbo].[P_TradeCalculateAll]
	  @inbCharacterID bigint --// ??? ID
	, @intChannelID int --// ?? ID
	, @intMapID int --// ? ID
	, @intCalculateResult int OUTPUT --// ??? ???? ?? (?? - ???)
	, @intCalculateResultForPetal int = 0 OUTPUT --// ??? ?? ?? (?? - ???)
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
		SET @intReturnValue = 103204;
		GOTO ErrorHandler;
	END

	INSERT @tblTrades (TradeID, PayMethodCode)
	SELECT TradeID, ISNULL(PayMethodCode, 1) FROM dbo.Trade WITH (NOLOCK, INDEX(IX_NNI_Trade_CharacterID_TradeStatusCode), FORCESEEK) WHERE CharacterID = @inbCharacterID AND TradeStatusCode = 1 OPTION (KEEPFIXED PLAN);

	SET @j = @@ROWCOUNT;

	SET @intCalculateResult = 0;
	SET @intCalculateResultForPetal = 0;

	WHILE @i <= @j
	BEGIN
		SELECT @intTradeID = TradeID, @inyPayMethodCode = PayMethodCode FROM @tblTrades WHERE seq = @i;

		EXEC @intReturnValue = dbo.P_TradeCalculate
			  @inbCharacterID = @inbCharacterID
			, @intTradeID = @intTradeID
			, @intChannelID = @intChannelID
			, @intMapID = @intMapID
			, @intCalculateResult = @intCalculateResultTemp OUTPUT
			, @inyPayMethodCode = @inyPayMethodCode OUTPUT;
		
		IF @inyPayMethodCode = 1
			SET @intCalculateResult += @intCalculateResultTemp;

		ELSE
			SET @intCalculateResultForPetal += @intCalculateResultTemp;

		SET @i += 1;
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

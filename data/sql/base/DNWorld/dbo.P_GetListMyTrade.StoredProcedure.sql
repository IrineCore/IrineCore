USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListMyTrade]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 17
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-31

description :
	?? ?? ??? ??? ??? ??? ?????.
result set : 
	@intVersion < 14 : TradeID/int, ItemID/int, ItemCount/smallint, ItemDurability/smallint, RandomSeed/int, ItemLevel/tinyint, ItemPotential/tinyint, ItemOption/tinyint, Price/int, TradeStatusCode/tinyint, RemainTime/int, SealCount/tinyint, PremiumFlag/bit
	@intVersion = 14 : TradeID/int, ItemID/int, ItemCount/smallint, ItemDurability/smallint, RandomSeed/int, ItemLevel/tinyint, ItemPotential/tinyint, ItemOption/tinyint, Price/int, TradeStatusCode/tinyint, RemainTime/int, SealCount/tinyint, PremiumFlag/bit, PayMethodCode/tinyint
	@intVersion = 15 : TradeID/int, ItemID/int, ItemCount/smallint, ItemDurability/smallint, RandomSeed/int, ItemLevel/tinyint, ItemPotential/tinyint, ItemOption/tinyint, Price/int, TradeStatusCode/tinyint, RemainTime/int, SealCount/tinyint, PremiumFlag/bit, PayMethodCode/tinyint, ItemPotentialMoveCount/tinyint
	@intVersion = 16 : Version 15 + DragonJewelType/tinyint, ItemSerial/bigint
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
	103172 = ?? ??? ??? ???? ????.
history :
	13 = ??? ?? PremiumFlag ? ????? ?? at 2010-07-21
	14 = ??? ?? PayMethodCode ? ????? ?? (input param. @intVersion ??) at 2011-01-24
	15~16 = ?? ?? ?? ?? (@intVersion = 15)     by ??? at 2012-04-10
**/

CREATE PROCEDURE [dbo].[P_GetListMyTrade]
	  @inbCharacterID bigint --// ??? ID
	, @intSellingCount int OUTPUT --// ????????
	, @intExpiredCount int OUTPUT --// ????????????
	, @intWeeklyRegisterCount int OUTPUT --// ??? AM 04:00 ? ???? ???? ??? ?? ??
	, @intTotalTradeCount int = NULL OUTPUT --// ???, ??????, ???? ?? ? ?
	, @intVersion int = 13 --// ??? SP? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;
DECLARE @intReturnValue int
	, @dt2Now datetime2(0) = GETDATE();

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	EXEC @intReturnValue = dbo.P_GetCountMyTrade
		  @inbCharacterID = @inbCharacterID
		, @intSellingCount = @intSellingCount OUTPUT
		, @intExpiredCount = @intExpiredCount OUTPUT
		, @intTotalCount = @intTotalTradeCount OUTPUT;

	SELECT @intWeeklyRegisterCount = COUNT(*)
	FROM dbo.Trade WITH (NOLOCK)
	WHERE CharacterID = @inbCharacterID AND PremiumFlag = CAST(0 AS bit) AND RegisterDate >= DATEADD(DAY, - DATEPART(WEEKDAY, DATEADD(HOUR, 4, DATEADD(DAY, DATEDIFF(DAY, 0, @dt2Now) + CASE WHEN DATEPART(HOUR, @dt2Now) BETWEEN 0 AND 3 THEN 0 ELSE 1 END, 0))), DATEADD(HOUR, 4, DATEADD(DAY, DATEDIFF(DAY, 0, @dt2Now) + CASE WHEN DATEPART(HOUR, @dt2Now) BETWEEN 0 AND 3 THEN 0 ELSE 1 END, 0)))

	IF @intVersion <= 13
		SELECT TradeID, ItemID, ItemCount, ItemDurability, RandomSeed, ItemLevel, ItemPotential, ItemOption, Price, TradeStatusCode, DATEDIFF(HOUR, @dt2Now, TradeExpireDate) AS RemainTime, SealCount, PremiumFlag
		FROM dbo.Trade WITH (NOLOCK, FORCESEEK)
		WHERE CharacterID = @inbCharacterID AND TradeStatusCode BETWEEN 0 AND 1
		ORDER BY TradeID;

	ELSE IF @intVersion = 14
		SELECT TradeID, ItemID, ItemCount, ItemDurability, RandomSeed, ItemLevel, ItemPotential, ItemOption, Price, TradeStatusCode, DATEDIFF(HOUR, @dt2Now, TradeExpireDate) AS RemainTime, SealCount, PremiumFlag, PayMethodCode
		FROM dbo.Trade WITH (NOLOCK, FORCESEEK)
		WHERE CharacterID = @inbCharacterID AND TradeStatusCode BETWEEN 0 AND 1
		ORDER BY TradeID;

	ELSE IF @intVersion = 15
		SELECT TradeID, ItemID, ItemCount, ItemDurability, RandomSeed, ItemLevel, ItemPotential, ItemOption, Price, TradeStatusCode, DATEDIFF(HOUR, @dt2Now, TradeExpireDate) AS RemainTime, SealCount, PremiumFlag, PayMethodCode, ItemPotentialMoveCount
		FROM dbo.Trade WITH (NOLOCK, FORCESEEK)
		WHERE CharacterID = @inbCharacterID AND TradeStatusCode BETWEEN 0 AND 1
		ORDER BY TradeID;

	ELSE IF @intVersion = 16
		SELECT TradeID, ItemID, ItemCount, ItemDurability, RandomSeed, ItemLevel, ItemPotential, ItemOption, Price, TradeStatusCode, DATEDIFF(HOUR, @dt2Now, TradeExpireDate) AS RemainTime, SealCount, PremiumFlag, PayMethodCode, ItemPotentialMoveCount, DragonJewelType, ItemSerial
		FROM dbo.Trade WITH (NOLOCK, FORCESEEK)
		WHERE CharacterID = @inbCharacterID AND TradeStatusCode BETWEEN 0 AND 1
		ORDER BY TradeID;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103172;
		GOTO ErrorHandler;
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

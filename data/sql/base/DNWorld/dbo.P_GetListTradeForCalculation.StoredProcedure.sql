USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListTradeForCalculation]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 12
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-31
description : ?? ?? ?? ??? ?????.
result set :
	@intVersion < 8 : TradeID/int, ItemID/int, ItemCount/smallint, ItemDurability/smallint, RandomSeed/int, ItemLevel/tinyint, ItemPotential/tinyint, ItemOption/tinyint, Price/int, ListPrice/int, TradeDate/datetime2, SealCount/tinyint
	@intVersion = 8 : TradeID/int, ItemID/int, ItemCount/smallint, ItemDurability/smallint, RandomSeed/int, ItemLevel/tinyint, ItemPotential/tinyint, ItemOption/tinyint, Price/int, ListPrice/int, TradeDate/datetime2, SealCount/tinyint, PayMethodCode/tinyint
	@intVersion = 9 : TradeID/int, ItemID/int, ItemCount/smallint, ItemDurability/smallint, RandomSeed/int, ItemLevel/tinyint, ItemPotential/tinyint, ItemOption/tinyint, Price/int, ListPrice/int, TradeDate/datetime2, SealCount/tinyint, PayMethodCode/tinyint, ItemPotentialMoveCount/tinyint
	@intVersion = 10 : Version 9 + DragonJewelType/tinyint, ItemSetial/bigint
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
	9~10 = ?? ?? ?? ?? (@intVersion = 9)     by ??? at 2012-04-10
	11~12 = ?? ?? ?? ?? (@intVersion = 10)     by ??? at 2013-04-04
**/

CREATE PROCEDURE [dbo].[P_GetListTradeForCalculation]
	  @inbCharacterID bigint --// ??? ID
	, @intVersion int = 7 --// ??? SP? ??
--WITH EXECUTE AS 'SPExecutor'
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
	IF @intVersion <= 7
		SELECT TradeID, ItemID, ItemCount, ItemDurability, RandomSeed, ItemLevel, ItemPotential, ItemOption, Price, ListPrice, TradeDate, SealCount
		FROM dbo.Trade WITH (NOLOCK)
		WHERE CharacterID = @inbCharacterID AND TradeStatusCode = 1
		ORDER BY TradeID;

	ELSE IF @intVersion = 8
		SELECT TradeID, ItemID, ItemCount, ItemDurability, RandomSeed, ItemLevel, ItemPotential, ItemOption, Price, ListPrice, TradeDate, SealCount, PayMethodCode
		FROM dbo.Trade WITH (NOLOCK)
		WHERE CharacterID = @inbCharacterID AND TradeStatusCode = 1
		ORDER BY TradeID;

	ELSE IF @intVersion = 9
		SELECT TradeID, ItemID, ItemCount, ItemDurability, RandomSeed, ItemLevel, ItemPotential, ItemOption, Price, ListPrice, TradeDate, SealCount, PayMethodCode, ItemPotentialMoveCount
		FROM dbo.Trade WITH (NOLOCK)
		WHERE CharacterID = @inbCharacterID AND TradeStatusCode = 1
		ORDER BY TradeID;

	ELSE IF @intVersion = 10
		SELECT TradeID, ItemID, ItemCount, ItemDurability, RandomSeed, ItemLevel, ItemPotential, ItemOption, Price, ListPrice, TradeDate, SealCount, PayMethodCode, ItemPotentialMoveCount, DragonJewelType, ItemSerial
		FROM dbo.Trade WITH (NOLOCK)
		WHERE CharacterID = @inbCharacterID AND TradeStatusCode = 1
		ORDER BY TradeID;

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

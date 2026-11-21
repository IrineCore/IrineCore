USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_CancelTrade2]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 7
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-01-18

description :
	????? ?????.
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
	103173 = ?? ??? ?? ??? ???? ???? ????.
	103174 = ?? ?? ??????.
history :
	5~6 = ?? ?? ?? ??     by ??? at 2012-04-10
	7 = ?? ???? ?? by ??? at 2013-04-04
**/

CREATE PROCEDURE [dbo].[P_CancelTrade2]
	  @inbCharacterID bigint --// ??? ID
	, @intTradeID int --// ?? ID
	, @inyItemLocationCode tinyint --// 2=????, 5=?? ????
	, @intChannelID int --// ?? ID
	, @intMapID int --// ? ID
	, @inbItemSerial bigint OUTPUT --// ??? ???
	, @intItemID int OUTPUT --// ??? ID
	, @insItemCount smallint OUTPUT --// ??? ??
	, @insItemDurability smallint OUTPUT --// ??? ???
	, @intRandomSeed int OUTPUT --// ?? ??
	, @inyItemLevel tinyint OUTPUT --// ??? ??
	, @inyItemPotential tinyint OUTPUT --// ??? ???
	, @inyItemOption tinyint OUTPUT --// ??? ??
	, @inySealCount tinyint	 OUTPUT --// ?? ??
	, @intPrice int OUTPUT --// ??
	, @inyItemPotentialMoveCount tinyint = NULL OUTPUT --// ???? ??
	, @inyDragonJewelType tinyint = NULL OUTPUT --// ?? ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @inyTradeStatusCode tinyint
	, @dt2Now datetime2(0) = GETDATE()
	, @inbMaterializedItemID bigint
	, @bitCashItemFlag bit;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	BEGIN TRANSACTION;

	/**_# [Trade] ???? ?? ?? ??? UPDATE???.*/
	UPDATE dbo.Trade
	SET @inbItemSerial = ItemSerial
		, @intItemID = ItemID
		, @inyTradeStatusCode = TradeStatusCode
		, @insItemCount = ItemCount
		, TradeStatusCode = 3
		, TradeDate = @dt2Now
	WHERE TradeID = @intTradeID AND CharacterID = @inbCharacterID AND TradeStatusCode BETWEEN 0 AND 1;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103173;
		GOTO ErrorHandler;
	END

	IF @inyTradeStatusCode = 1
	BEGIN
		SET @intReturnValue = 103174;
		GOTO ErrorHandler;
	END

	UPDATE dbo.MaterializedItems
	SET @inbMaterializedItemID = MaterializedItemID
		, @insItemDurability = ItemDurability
		, @intRandomSeed = RandomSeed
		, @inyItemLevel = ItemLevel
		, @inyItemPotential = ItemPotential
		, @inyItemOption = ItemOption
		, @inyItemPotentialMoveCount = ItemPotentialMoveCount
		, @inyDragonJewelType = DragonJewelType
		, ItemLocationCode = @inyItemLocationCode
		, @inySealCount = SealCount = 
			CASE
				WHEN EXISTS (
					SELECT *
					FROM dbo.TradeItems WITH (NOLOCK, INDEX(PK_TradeItems), FORCESEEK)
					WHERE ItemID = dbo.MaterializedItems.ItemID
						AND CashItemFlag = CAST(1 AS bit)
				) THEN SealCount + 1
				ELSE SealCount
			END
	WHERE ItemSerial = @inbItemSerial AND OwnerCharacterID = @inbCharacterID AND ItemLocationCode = 12;

	IF @@ROWCOUNT = 0
	BEGIN
		COMMIT TRANSACTION;

		SET @intReturnValue = 103173;
		GOTO ErrorHandler;
	END

	COMMIT TRANSACTION;
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

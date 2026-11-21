USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_BuyTradeItem2]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 16
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-07-08

description :
	????? ??? ???? ?????.
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
	103123 = ??? ?????.
	103151 = ???? ???? ????.
	103174 = ?? ?? ??????.
	103175 = ???? ?? ?? ??????.
	103325 = ??? ??? ???? ??? ? ????.
history :
	5 = ?? ? ItemLocationIndex ?? NULL? ??? by ??? at 2011-10-21
	6 = (50500) ???? ???? ???? 103325? ????? ?? at 2012-01-31
	8 = ?? ??? ?? ? ItemLog ??? ItemLogFKey2? RandomSeed? ??? ?? by ??? at 2012-02-13
	9 = version 8 ?? ?? by ??? at 2012-02-14
	10 = ??? ?? ??? ItemLevel(????)? ?? ??? ?? by ??? at 2012-03-08
	11 = ?? ?? ?? ??     by ??? at 2012-04-10
	12 = ??? ??? ??? ?? ?? ?? ????? ?? by ??? at 2012-04-19
	16 = PushQueue ??? AccountID ????? ?? by ??? at 2013-08-16
**/

CREATE PROCEDURE [dbo].[P_BuyTradeItem2]
	  @inbBuyerCharacterID bigint --// ?? ??? ID
	, @intTradeID int --// ?? ID
	, @inyItemLocationCode tinyint --// ??? ???? ??? ??. 2=????, 5=??????
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
	, @inbSellerCharacterID bigint OUTPUT --// ??? ??? ID
	, @vchIP varchar(15) = NULL --// ????? ?? IP ??
	, @inyPayMethodCode tinyint = 1 OUTPUT --// 1=?? ??, 3=??
	, @intSellerAccountID int = 0 OUTPUT --// ??? ?? ID
	, @inyItemPotentialMoveCount tinyint = NULL OUTPUT --// ???? ??
	, @inyDragonJewelType tinyint = NULL OUTPUT --// ?? ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dt2Now datetime2(0) = GETDATE()
	, @inyTradeStatusCode tinyint
	, @dt2OwnershipStartDate datetime2(0)
	, @intIP int = dbo.FN_IP2IPNumber(@vchIP)
	, @inbMaterializedItemID bigint
	, @bitSoulBoundFlag bit
	, @inbCharacterID bigint;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	BEGIN TRANSACTION;

	UPDATE dbo.Trade
	SET @inbItemSerial = ItemSerial
		, @intItemID = ItemID
		, @insItemCount = ItemCount
		, @intPrice = Price
		, @inyTradeStatusCode = TradeStatusCode
		, @inbSellerCharacterID = CharacterID
		, @inyPayMethodCode = ISNULL(PayMethodCode, 1)
		, @inbCharacterID = CharacterID
		, TradeStatusCode = 1
		, BuyerCharacterID = @inbBuyerCharacterID
		, TradeDate = @dt2Now
	WHERE TradeID = @intTradeID AND TradeStatusCode BETWEEN 0 AND 1;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103175;
		GOTO ErrorHandler;
	END

	IF @inbCharacterID = @inbBuyerCharacterID
	BEGIN
		SET @intReturnValue = 103325;
		GOTO ErrorHandler;
	END

	IF @inyTradeStatusCode = 1
	BEGIN
		SET @intReturnValue = 103174;
		GOTO ErrorHandler;
	END

	IF @inyPayMethodCode = 1 AND (
		SELECT Coin 
		FROM dbo.CharacterStatus WITH (NOLOCK) 
		WHERE CharacterID = @inbBuyerCharacterID
	) < @intPrice
	BEGIN
		SET @intReturnValue = 103123;
		GOTO ErrorHandler;
	END

	UPDATE dbo.MaterializedItems
	SET OwnerCharacterID = @inbBuyerCharacterID
		, OwnershipStartDate = @dt2Now
		, TradeItemFlag = 1
		, ItemLocationCode = @inyItemLocationCode
		, ItemLocationIndex = NULL
		, @inbMaterializedItemID = MaterializedItemID
		, @insItemDurability = ItemDurability
		, @intRandomSeed = RandomSeed
		, @inyItemLevel = ItemLevel
		, @inyItemPotential = ItemPotential
		, @inyItemOption = ItemOption
		, @inySealCount = SealCount
		, @dt2OwnershipStartDate = OwnershipStartDate
		, @bitSoulBoundFlag = SoulBoundFlag
		, @inyItemPotentialMoveCount = ItemPotentialMoveCount
		, @inyDragonJewelType = DragonJewelType
	WHERE ItemSerial = @inbItemSerial
		AND OwnerCharacterID = @inbSellerCharacterID
		AND ItemLocationCode = 12;

	IF @@ROWCOUNT = 0
	BEGIN
		ROLLBACK TRANSACTION;
		UPDATE dbo.Trade
		SET TradeStatusCode = 3
			, TradeDate = @dt2Now
		WHERE TradeID = @intTradeID;
		SET @intReturnValue = 103151;
		GOTO ErrorHandler;
	END

	COMMIT TRANSACTION;

	SELECT @intSellerAccountID = AccountID 
	FROM dbo.Characters WITH (NOLOCK) 
	WHERE CharacterID = @inbSellerCharacterID;

	/**_# ??? ?? ???? ?? ???? ??? ?????.*/
	INSERT dbo.PushQueue (WishListID, CharacterID, RegistedDate, PushTypeCode, ItemID, AccountID)
	SELECT NULL, CharacterID, @dt2Now, 1, @intItemID, AccountID
	FROM dbo.Characters WITH (NOLOCK)
	WHERE CharacterID = @inbSellerCharacterID AND MobileAuthenticationFlag = CAST(1 AS bit);

	IF @inyPayMethodCode = 1
		EXEC @intReturnValue = dbo.P_ModCoinForSubProc
			  @inbCharacterID = @inbBuyerCharacterID
			, @inyCoinChangeCode = 22
			, @inbCoinChangeFKey = @intTradeID
			, @inbCoin = @intPrice
			, @intChannelID = @intChannelID
			, @intMapID = @intMapID
			, @dt2LogDate = @dt2Now;

	/**_# ??? ??? ?????.*/
	EXEC @intReturnValue = dbo.P_AddItemLog
		  @dt2LogDate = @dt2Now
		, @inbItemSerial = @inbItemSerial
		, @intItemID = @intItemID
		, @inyItemLogCode = 28
		, @inbOwnerCharacterID = @inbBuyerCharacterID
		, @inbOwnerCharacterIDBefore = @inbSellerCharacterID
		, @insItemCount = @insItemCount
		, @insItemCountBefore = @insItemCount
		, @inbItemLogFKey1 = @intTradeID
		, @inbItemLogFKey2 = @intMapID
		, @dt2OwnershipStartDate = @dt2OwnershipStartDate
		, @intIP = @intIP
		, @inySealCount = @inySealCount
		, @bitSoulBoundFlag = @bitSoulBoundFlag
		, @inyItemOption = @inyItemOption
		, @inyItemPotential = @inyItemPotential
		, @inyItemLevel = @inyItemLevel
		, @inyItemPotentialMoveCount = @inyItemPotentialMoveCount
		, @inyDragonJewelType = @inyDragonJewelType;

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

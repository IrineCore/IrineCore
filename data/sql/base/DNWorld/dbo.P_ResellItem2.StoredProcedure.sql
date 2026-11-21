USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ResellItem2]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 12
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-07-11
description : ???? ????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103151 = ???? ???? ????.
103191 = ??? ???? ??? ?? ????.
history :
2~6 = ??? ??? ?? ??     by ??? at 2011-10-26
7 = ?? ?? ?? ??     by ??? at 2012-04-09
8 = ??? ??? ??? ?? ?? ?? ????? ?? by ??? at 2012-04-19
9 = ??? ??? ??? ??? ?? ??? ??? ???? ????? ??     by ??? at 2012-05-11
10 = ??? ??? ??? ??? ?? ??? ?? ?? ?? ??     by ??? at 2012-05-22
**/

CREATE PROCEDURE [dbo].[P_ResellItem2]
	  @inbCharacterID bigint --// ??? ID
	, @inbItemSerial bigint --// ??? ?? ??
	, @insItemCount smallint --// ???? ??? ??
	, @inbResellPrice bigint --// ??? ??
	, @intNPCID int --// ??? NPC ID
	, @intChannelID int --// ???? ??? ?? ID
	, @intMapID int --// ???? ??? ? ID
	, @vchIP varchar(15) = NULL --// ????? ?? IP ??
	, @bitAllFlag bit = NULL --// 0=?? ?? ??, 1=?? ?? ??
	, @bitRepurchaseFlag bit = NULL --// ??? ??? ???? (0=??? ??, 1=??? ??)
	, @bitSealFlag bit = 0 --// ???? (0=???, 1=??)
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dt2LogDate datetime2(0) = GETDATE()
	, @insRemainItemCount smallint
	, @inbCurrentCoin bigint
	, @intItemID int
	, @dt2OwnershipStartDate datetime2(0)
	, @inyPayMethodCode tinyint
	, @inbOwnerCharacterID bigint
	, @insRemainItemCountBefore smallint
	, @intIP int = dbo.FN_IP2IPNumber(@vchIP)
	, @inbMaterializedItemID bigint
	, @inySealCount tinyint
	, @bitSoulBoundFlag bit
	, @inyItemOption tinyint
	, @inyItemPotential tinyint
	, @insItemDurability smallint
	, @intRandomSeed int
	, @inyItemLevel tinyint
	, @intCoolTime int
	, @dt2ItemMaterializeDate datetime2(0)
	, @bitEternityFlag bit
	, @sdtItemExpireDate smalldatetime
	, @intAdditiveItemID int
	, @inyItemPotentialMoveCount tinyint
	, @inyDragonJewelType tinyint;

DECLARE @tblTemp table (
	  MaterializedItemID bigint NOT NULL
	, ItemRemainCount smallint NOT NULL
	, ItemID int NOT NULL
	, OwnershipStartDate datetime2(0)
	, PayMethodCode tinyint
	, ItemPotential	tinyint
	, SoulBoundFlag	bit
	, SealCount	tinyint
	, ItemOption	tinyint
	, ItemDurability smallint
	, RandomSeed int
	, ItemLevel tinyint
	, CoolTime int
	, ItemMaterializeDate datetime2(0)
	, EternityFlag bit
	, ItemExpireDate smalldatetime
	, AdditiveItemID int
	, ItemPotentialMoveCount tinyint
	, DragonJewelType tinyint
);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	IF @bitAllFlag = 0 OR @bitAllFlag IS NULL
	BEGIN
		/**_# ???? ??? ?????.*/
		UPDATE dbo.MaterializedItems
		SET	@inbMaterializedItemID = MaterializedItemID
			, @insRemainItemCountBefore = ItemRemainCount
			, @intItemID = ItemID
			, @dt2OwnershipStartDate = OwnershipStartDate
			, @inyPayMethodCode = PayMethodCode
			, @insRemainItemCount = ItemRemainCount -= @insItemCount
			, @inbOwnerCharacterID = OwnerCharacterID = CASE WHEN ItemRemainCount > @insItemCount THEN OwnerCharacterID ELSE NULL END
			, @inySealCount = SealCount
			, @bitSoulBoundFlag = SoulBoundFlag
			, @inyItemOption = ItemOption
			, @inyItemPotential = ItemPotential
			, @insItemDurability = ItemDurability
			, @intRandomSeed = RandomSeed
			, @inyItemLevel = ItemLevel
			, @intCoolTime = CoolTime
			, @dt2ItemMaterializeDate = ItemMaterializeDate
			, @bitEternityFlag = EternityFlag
			, @sdtItemExpireDate = ItemExpireDate
			, @intAdditiveItemID = AdditiveItemID
			, @inyItemPotentialMoveCount = ItemPotentialMoveCount
			, @inyDragonJewelType = DragonJewelType
		WHERE ItemSerial = @inbItemSerial AND OwnerCharacterID = @inbCharacterID;

		/**_# ???? ???? ??? 103151? ?????.*/
		IF @@ROWCOUNT = 0
		BEGIN
			SET @intReturnValue = 103151;
			GOTO ErrorHandler;
		END

		/**_# ?? ??? 0?? ???? ?? (??, ???? ?)?? ?????.*/
		IF @insRemainItemCount <= 0
			DELETE dbo.MaterializedItems 
			WHERE MaterializedItemID = @inbMaterializedItemID
			OPTION (KEEPFIXED PLAN);

		/**_# ??? ??? ??? ???? ?? ??? ??? ???? INSERT */
		-- ?? ?? ??? ???? ItemSerial ?? ? ?
		IF @bitRepurchaseFlag = 1
		BEGIN
			INSERT dbo.RepurchaseItem (CharacterID, SellDate, SellPrice
			, ItemID, ItemRemainCount, ItemDurability, RandomSeed, CoolTime
			, ItemLevel, ItemPotential, SoulBoundFlag, ItemOption, SealCount
			, ItemMaterializeDate, EternityFlag, ItemExpireDate, AdditiveItemID, ItemPotentialMoveCount, DragonJewelType)
			SELECT @inbCharacterID, @dt2LogDate, @inbResellPrice
			, @intItemID, @insItemCount, @insItemDurability, @intRandomSeed, @intCoolTime
			, @inyItemLevel, @inyItemPotential, CASE WHEN @bitSealFlag = CONVERT(bit, 0) THEN @bitSoulBoundFlag ELSE CONVERT(bit, 1) END, @inyItemOption, @inySealCount
			, @dt2ItemMaterializeDate, @bitEternityFlag, @sdtItemExpireDate, @intAdditiveItemID, @inyItemPotentialMoveCount, @inyDragonJewelType;
		END

	END
	ELSE
	BEGIN
		DELETE dbo.MaterializedItems 
		OUTPUT deleted.MaterializedItemID, deleted.ItemRemainCount, deleted.ItemID, deleted.OwnershipStartDate, deleted.PayMethodCode, deleted.ItemPotential, deleted.SoulBoundFlag, deleted.SealCount, deleted.ItemOption, deleted.ItemDurability, deleted.RandomSeed, deleted.ItemLevel, deleted.SealCount, deleted.ItemMaterializeDate, deleted.EternityFlag, deleted.ItemExpireDate, deleted.AdditiveItemID, deleted.ItemPotentialMoveCount, deleted.DragonJewelType
		INTO @tblTemp (MaterializedItemID, ItemRemainCount, ItemID, OwnershipStartDate, PayMethodCode, ItemPotential, SoulBoundFlag, SealCount, ItemOption, ItemDurability, RandomSeed, ItemLevel, CoolTime, ItemMaterializeDate, EternityFlag, ItemExpireDate, AdditiveItemID, ItemPotentialMoveCount, DragonJewelType)
		WHERE ItemSerial = @inbItemSerial AND OwnerCharacterID = @inbCharacterID AND ItemRemainCount = @insItemCount
		OPTION (KEEPFIXED PLAN);

		IF @@ROWCOUNT = 0
		BEGIN
			SET @intReturnValue = 103151;
			GOTO ErrorHandler;
		END

		SELECT @inbMaterializedItemID = MaterializedItemID
			, @insRemainItemCountBefore = ItemRemainCount
			, @intItemID = ItemID
			, @dt2OwnershipStartDate = OwnershipStartDate
			, @inyPayMethodCode = PayMethodCode
			, @insRemainItemCount = 0
			, @inbOwnerCharacterID = NULL
			, @inySealCount = SealCount
			, @bitSoulBoundFlag = SoulBoundFlag
			, @inyItemOption = ItemOption
			, @inyItemPotential = ItemPotential
			, @insItemDurability = ItemDurability
			, @intRandomSeed = RandomSeed
			, @inyItemLevel = ItemLevel
			, @intCoolTime = CoolTime
			, @dt2ItemMaterializeDate = ItemMaterializeDate
			, @bitEternityFlag = EternityFlag
			, @sdtItemExpireDate = ItemExpireDate
			, @intAdditiveItemID = AdditiveItemID
			, @inyItemPotentialMoveCount = ItemPotentialMoveCount
			, @inyDragonJewelType = DragonJewelType
		FROM @tblTemp;

		/**_# ??? ??? ??? ???? ?? ??? ??? ???? INSERT */
		-- ?? ?? ??? ???? ItemSerial ?? ?
		IF @bitRepurchaseFlag = 1
		BEGIN
			INSERT dbo.RepurchaseItem (CharacterID, SellDate, SellPrice, ItemSerial
			, ItemID, ItemRemainCount, ItemDurability, RandomSeed, CoolTime
			, ItemLevel, ItemPotential, SoulBoundFlag, ItemOption, SealCount
			, ItemMaterializeDate, EternityFlag, ItemExpireDate, AdditiveItemID, ItemPotentialMoveCount, DragonJewelType)
			SELECT @inbCharacterID, @dt2LogDate, @inbResellPrice, @inbItemSerial
			, @intItemID, @insItemCount, @insItemDurability, @intRandomSeed, @intCoolTime
			, @inyItemLevel, @inyItemPotential, CASE WHEN @bitSealFlag = CONVERT(bit, 0) THEN @bitSoulBoundFlag ELSE CONVERT(bit, 1) END, @inyItemOption, @inySealCount
			, @dt2ItemMaterializeDate, @bitEternityFlag, @sdtItemExpireDate, @intAdditiveItemID, @inyItemPotentialMoveCount, @inyDragonJewelType;
		END

	END

	/**_# [CharacterStatus].[Coin]? UPDATE???.*/
	UPDATE dbo.CharacterStatus
	SET @inbCurrentCoin = Coin += @inbResellPrice
	WHERE CharacterID = @inbCharacterID
	OPTION (KEEPFIXED PLAN);

	/**_# ??? ??? ?????.*/
	SET @dt2OwnershipStartDate = CASE @insRemainItemCount WHEN 0 THEN @dt2OwnershipStartDate ELSE NULL END;

	-- ??? ???? ?? ??? ??? ??? ?? ?? ??
	IF @bitSealFlag = CONVERT(bit, 1)
	BEGIN
		EXEC dbo.P_AddItemLog
			  @dt2LogDate = @dt2LogDate
			, @inbItemSerial = @inbItemSerial
			, @intItemID = @intItemID
			, @inyItemLogCode = 40
			, @inbOwnerCharacterID = @inbCharacterID
			, @inbOwnerCharacterIDBefore = @inbCharacterID
			, @insItemCount = @insRemainItemCountBefore
			, @insItemCountBefore = @insRemainItemCountBefore
			, @inbItemLogFKey1 = @intMapID
			, @inbItemLogFKey2 = NULL
			, @dt2OwnershipStartDate = NULL
			, @intIP = NULL
			, @inySealCount = @inySealCount
			, @bitSoulBoundFlag = 1
			, @inyItemOption = @inyItemOption
			, @inyItemPotential = @inyItemPotential
			, @inyItemPotentialMoveCount = @inyItemPotentialMoveCount
			, @inyDragonJewelType = @inyDragonJewelType;
	END

	EXEC dbo.P_AddItemLog
		  @dt2LogDate = @dt2LogDate
		, @inbItemSerial = @inbItemSerial
		, @intItemID = @intItemID
		, @inyItemLogCode = 22
		, @inbOwnerCharacterID = @inbOwnerCharacterID
		, @inbOwnerCharacterIDBefore = @inbCharacterID
		, @insItemCount = @insRemainItemCount
		, @insItemCountBefore = @insRemainItemCountBefore
		, @inbItemLogFKey1 = @intMapID
		, @inbItemLogFKey2 = @intNPCID
		, @dt2OwnershipStartDate = @dt2OwnershipStartDate
		, @intIP = @intIP
		, @inySealCount = @inySealCount
		, @bitSoulBoundFlag = @bitSoulBoundFlag
		, @inyItemOption = @inyItemOption
		, @inyItemPotential = @inyItemPotential
		, @insItemDurability = @insItemDurability
		, @inyItemLevelBefore = @inyItemLevel
		, @inyItemLevel = @inyItemLevel
		, @inyItemPotentialMoveCount = @inyItemPotentialMoveCount
		, @inyDragonJewelType = @inyDragonJewelType;

	/**_# [P_AddCoinLog]? ???? ?? ??? ?????.*/
	EXEC dbo.P_AddCoinLog
		  @dt2LogDate = @dt2LogDate
		, @inbCharacterID = @inbCharacterID
		, @inyCoinTypeCode = 1
		, @inyCoinChangeCode = 10
		, @inbCurrentCoin = @inbCurrentCoin
		, @inbChangedCoin = @inbResellPrice
		, @intChannelID = @intChannelID
		, @intMapID = @intMapID
		, @inbCoinChangeFKey = @intNPCID;

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

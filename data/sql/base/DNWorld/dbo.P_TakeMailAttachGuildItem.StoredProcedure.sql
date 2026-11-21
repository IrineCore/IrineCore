USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_TakeMailAttachGuildItem]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 8
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-09-03
description : ??? ??? ?????? ????.
result set : 
	Version < 4 : ItemSerial/bigint, ItemID/int, ItemRemainCount/smallint, ItemDurability/smallint, RandomSeed/int, CoolTime/int, ItemLevel/tinyint, ItemPotential/tinyint, SoulBoundFlag/bit, SealCount/tinyint, ItemOption/tinyint, EternityFlag/bit, ItemExpireDate/smalldatetime, ItemPotentialMoveCount/tinyint
	Version = 4 : ItemSerial/bigint, ItemID/int, ItemRemainCount/smallint, ItemDurability/smallint, RandomSeed/int, CoolTime/int, ItemLevel/tinyint, ItemPotential/tinyint, SoulBoundFlag/bit, SealCount/tinyint, ItemOption/tinyint, EternityFlag/bit, ItemExpireDate/smalldatetime, ItemPotentialMoveCount/tinyint, DragonJewelType/tinyint
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103168 = ?? ???? ????.
history :
3 = ???? MaterializedItemID ?? ?? ?? by ??? at 2012-12-27
**/

CREATE PROCEDURE [dbo].[P_TakeMailAttachGuildItem]
	  @intMailID int --// ?? ID
	, @inbItemSerial bigint --// ??? ?? ??
	, @intChannelID int --// ?? ID
	, @intMapID int --// ? ID
	, @intGuildID int --// ??ID
	, @vchIP varchar(15) = NULL --// ????? ?? IP ??
	, @intVersion int = NULL --// ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dt2Now datetime2(0) = GETDATE()
	, @intItemID int
	, @inbReceiverCharacterID bigint
	, @inbCharacterID bigint
	, @insOldItemCount smallint
	, @insOldItemRemainCount smallint
	, @inbOldItemPrice bigint
	, @dt2OldOwnershipStartDate datetime2(0)
	, @insItemCount smallint
	, @intIP int = dbo.FN_IP2IPNumber(@vchIP)
	, @inbMaterializedItemID bigint
	, @insItemRemainCount smallint
	, @insItemDurability smallint
	, @intRandomSeed int
	, @intCoolTime int
	, @inyItemLevel tinyint
	, @inyItemPotential tinyint
	, @bitSoulBoundFlag bit
	, @inySealCount tinyint
	, @inyItemOption tinyint
	, @bitEternityFlag bit
	, @dtsItemExpireDate smalldatetime
	, @inyItemPotentialMoveCount tinyint
	, @inyDragonJewelType tinyint;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	IF @intVersion IS NULL
		SET @intVersion = 1;

	UPDATE dbo.MailAttachItems
	SET @intItemID = ItemID
		, @insItemCount = AttachItemCount
		, ReceiveFlag = 1
	WHERE MailID = @intMailID AND ItemSerial = @inbItemSerial AND ReceiveFlag = CAST(0 AS bit);

	IF @@ROWCOUNT = 0
	BEGIN
		IF @intVersion < 4		
			SELECT @inbItemSerial AS ItemSerial
				, @intItemID AS ItemID
				, @insItemRemainCount AS ItemRemainCount
				, @insItemDurability AS ItemDurability
				, @intRandomSeed AS RandomSeed
				, @intCoolTime AS CoolTime
				, @inyItemLevel AS ItemLevel
				, @inyItemPotential AS ItemPotential
				, @bitSoulBoundFlag AS SoulBoundFlag
				, @inySealCount AS SealCount
				, @inyItemOption AS ItemOption
				, @bitEternityFlag AS EternityFlag
				, @dtsItemExpireDate AS ItemExpireDate
				, @inyItemPotentialMoveCount AS ItemPotentialMoveCount
			WHERE 1 = 0;			
		ELSE IF @intVersion = 4
			SELECT @inbItemSerial AS ItemSerial
				, @intItemID AS ItemID
				, @insItemRemainCount AS ItemRemainCount
				, @insItemDurability AS ItemDurability
				, @intRandomSeed AS RandomSeed
				, @intCoolTime AS CoolTime
				, @inyItemLevel AS ItemLevel
				, @inyItemPotential AS ItemPotential
				, @bitSoulBoundFlag AS SoulBoundFlag
				, @inySealCount AS SealCount
				, @inyItemOption AS ItemOption
				, @bitEternityFlag AS EternityFlag
				, @dtsItemExpireDate AS ItemExpireDate
				, @inyItemPotentialMoveCount AS ItemPotentialMoveCount
				, @inyDragonJewelType AS DragonJewelType
			WHERE 1 = 0;

		SET @intReturnValue = 103168;
		GOTO ErrorHandler;
	END

	SELECT @inbCharacterID = ReceiverCharacterID
	FROM dbo.Mails WITH (NOLOCK)
	WHERE MailID = @intMailID;
	UPDATE dbo.MaterializedItems
	SET @inbMaterializedItemID = MaterializedItemID
		, @intItemID = ItemID
		, @insItemRemainCount = ItemRemainCount
		, @insItemDurability = ItemDurability
		, @intRandomSeed = RandomSeed
		, @intCoolTime = CoolTime
		, @inyItemLevel = ItemLevel
		, @inyItemPotential = ItemPotential
		, @bitSoulBoundFlag = SoulBoundFlag
		, @inySealCount = SealCount
		, @inyItemOption = ItemOption
		, @bitEternityFlag = EternityFlag
		, @dtsItemExpireDate = ItemExpireDate
		, @inyItemPotentialMoveCount = ItemPotentialMoveCount
		, @inyDragonJewelType = DragonJewelType
		, OwnerCharacterID = NULL
		, OwnershipStartDate = NULL
		, ItemExpireDate = CASE EternityFlag WHEN 0 THEN CASE WHEN ItemExpireDate IS NULL AND ItemLifespan > 0 THEN DATEADD(MINUTE, ItemLifespan, @dt2Now) ELSE ItemExpireDate END ELSE ItemExpireDate END
		, ItemLocationCode = 16
		, ItemLocationIndex = NULL
	WHERE ItemSerial = @inbItemSerial AND ItemLocationCode = 15;

	INSERT dbo.GuildDepots (GuildID, ItemSerial, LockFlag, LockDate, MaterializedItemID)
	VALUES (@intGuildID, @inbItemSerial, 0, NULL, @inbMaterializedItemID);

	IF @@ROWCOUNT = 0
	BEGIN		
		IF  @intVersion < 4		
			SELECT @inbItemSerial AS ItemSerial
				, @intItemID AS ItemID
				, @insItemRemainCount AS ItemRemainCount
				, @insItemDurability AS ItemDurability
				, @intRandomSeed AS RandomSeed
				, @intCoolTime AS CoolTime
				, @inyItemLevel AS ItemLevel
				, @inyItemPotential AS ItemPotential
				, @bitSoulBoundFlag AS SoulBoundFlag
				, @inySealCount AS SealCount
				, @inyItemOption AS ItemOption
				, @bitEternityFlag AS EternityFlag
				, @dtsItemExpireDate AS ItemExpireDate
				, @inyItemPotentialMoveCount AS ItemPotentialMoveCount
			WHERE 1 = 0;
		ELSE IF @intVersion = 4	
			SELECT @inbItemSerial AS ItemSerial
				, @intItemID AS ItemID
				, @insItemRemainCount AS ItemRemainCount
				, @insItemDurability AS ItemDurability
				, @intRandomSeed AS RandomSeed
				, @intCoolTime AS CoolTime
				, @inyItemLevel AS ItemLevel
				, @inyItemPotential AS ItemPotential
				, @bitSoulBoundFlag AS SoulBoundFlag
				, @inySealCount AS SealCount
				, @inyItemOption AS ItemOption
				, @bitEternityFlag AS EternityFlag
				, @dtsItemExpireDate AS ItemExpireDate
				, @inyItemPotentialMoveCount AS ItemPotentialMoveCount
				, @inyDragonJewelType AS DragonJewelType
			WHERE 1 = 0;

		SET @intReturnValue = 103168;
		GOTO ErrorHandler;
	END

	/**_# ??? ??? ?????.*/
	EXEC dbo.P_AddItemLog
		  @dt2LogDate = @dt2Now
		, @inbItemSerial = @inbItemSerial
		, @intItemID = @intItemID
		, @inyItemLogCode = 45
		, @inbOwnerCharacterID = NULL
		, @inbOwnerCharacterIDBefore = @inbCharacterID
		, @insItemCount = @insItemCount
		, @insItemCountBefore = @insItemCount
		, @inbItemLogFKey1 = @intMailID
		, @inbItemLogFKey2 = @intGuildID
		, @dt2OwnershipStartDate = NULL
		, @intIP = @intIP
		, @inySealCount = @inySealCount
		, @bitSoulBoundFlag = @bitSoulBoundFlag
		, @inyItemOption = @inyItemOption
		, @inyItemPotential = @inyItemPotential
		, @inyItemPotentialMoveCount = @inyItemPotentialMoveCount;

	/**_# ?? ?? ??? ?????.*/
	INSERT dbo.GuildDepotHistory (GuildID, CharacterID, CharacterName, ItemCoinFlag, InOutFlag, ItemID, ItemLevel, ItemCount, GuildCoin, RegisterDate)
	SELECT @intGuildID, @inbCharacterID, CharacterName, 0, 0, @intItemID, @inyItemLevel, @insItemCount, 0, @dt2Now
	FROM dbo.Characters WITH (NOLOCK)
	WHERE CharacterID = @inbCharacterID;
	
	IF  @intVersion < 4		
		SELECT @inbItemSerial AS ItemSerial
			, @intItemID AS ItemID
			, @insItemRemainCount AS ItemRemainCount
			, @insItemDurability AS ItemDurability
			, @intRandomSeed AS RandomSeed
			, @intCoolTime AS CoolTime
			, @inyItemLevel AS ItemLevel
			, @inyItemPotential AS ItemPotential
			, @bitSoulBoundFlag AS SoulBoundFlag
			, @inySealCount AS SealCount
			, @inyItemOption AS ItemOption
			, @bitEternityFlag AS EternityFlag
			, @dtsItemExpireDate AS ItemExpireDate
			, @inyItemPotentialMoveCount AS ItemPotentialMoveCount;	
	ELSE IF @intVersion = 4	
		SELECT @inbItemSerial AS ItemSerial
			, @intItemID AS ItemID
			, @insItemRemainCount AS ItemRemainCount
			, @insItemDurability AS ItemDurability
			, @intRandomSeed AS RandomSeed
			, @intCoolTime AS CoolTime
			, @inyItemLevel AS ItemLevel
			, @inyItemPotential AS ItemPotential
			, @bitSoulBoundFlag AS SoulBoundFlag
			, @inySealCount AS SealCount
			, @inyItemOption AS ItemOption
			, @bitEternityFlag AS EternityFlag
			, @dtsItemExpireDate AS ItemExpireDate
			, @inyItemPotentialMoveCount AS ItemPotentialMoveCount
			, @inyDragonJewelType AS DragonJewelType;	

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

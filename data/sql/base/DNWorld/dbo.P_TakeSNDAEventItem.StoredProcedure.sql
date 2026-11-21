USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_TakeSNDAEventItem]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : dailykim
e-mail : dailykim@eyedentitygames.com
created date : 2013-08-27
description : ?? ??? ???? ?????.
result set :
	@intVersion < 3 : ItemSerial/bigint, ItemID/int, ItemRemainCount/smallint, ItemDurability/smallint, RandomSeed/int, CoolTime/int, ItemLevel/tinyint, ItemPotential/tinyint, SoulBoundFlag/bit, SealCount/tinyint, ItemOption/tinyint, EternityFlag/bit, ItemExpireDate/smalldatetime
	@intVersion = 3 : ItemSerial/bigint, ItemID/int, ItemRemainCount/smallint, ItemDurability/smallint, RandomSeed/int, CoolTime/int, ItemLevel/tinyint, ItemPotential/tinyint, SoulBoundFlag/bit, SealCount/tinyint, ItemOption/tinyint, EternityFlag/bit, ItemExpireDate/smalldatetime, ItemPotentialMoveCount/tinyint	
	@intVersion = 4 : Version 3 + DragonJewelType/tinyint
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103168 = ?? ???? ????.
103420 = ?? ?? ??? ???? ??? ????.
**/

CREATE PROCEDURE [dbo].[P_TakeSNDAEventItem]
	  @intMailID int --// ?? ID
	, @inbItemSerial bigint --// ??? ?? ??
	, @intChannelID int --// ?? ID
	, @intMapID int --// ? ID
	, @vchIP varchar(15) = NULL --// ????? ?? IP ??
	, @intVersion int = 2 --// ??? SP? ??
	, @intGuildID int = NULL --// ???? ???? ?? ??? ?? ??ID
	, @bitGuildItemMailFlag bit = NULL OUTPUT --//???? ??? ??
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
	, @inyMailTypeCode tinyint
	, @inyDragonJewelType tinyint
	, @inbSenderCharacterID bigint;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	SELECT @inbCharacterID = ReceiverCharacterID, @inbSenderCharacterID = SenderCharacterID
	FROM dbo.Mails WITH (NOLOCK)
	WHERE MailID = @intMailID;

	-- ?? ?? ??? ???? ???? ??
	IF EXISTS (SELECT * FROM dbo.SNDAEventItemHistory WITH (NOLOCK) WHERE ReceiveCharacterID = @inbCharacterID AND ReceiveFlag = CAST (1 AS bit))
	BEGIN
		SET @intReturnValue = 103420;
		GOTO ErrorHandler;
	END

	UPDATE dbo.MailAttachItems
	SET @intItemID = ItemID
		, @insItemCount = AttachItemCount
		, ReceiveFlag = 1
	WHERE MailID = @intMailID AND ItemSerial = @inbItemSerial AND ReceiveFlag = CAST(0 AS bit);

	IF @@ROWCOUNT = 0
	BEGIN
		IF @intVersion < 3
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
			WHERE 1 = 0;

		ELSE IF @intVersion = 3
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
		, @inySealCount = 0
		, @inyItemOption = ItemOption
		, @bitEternityFlag = EternityFlag
		, @dtsItemExpireDate = ItemExpireDate
		, @inyItemPotentialMoveCount = ItemPotentialMoveCount
		, @inyDragonJewelType = DragonJewelType
		, OwnerCharacterID = @inbCharacterID
		, OwnershipStartDate = @dt2Now
		, ItemExpireDate = CASE EternityFlag WHEN 0 THEN CASE WHEN ItemExpireDate IS NULL AND ItemLifespan > 0 THEN DATEADD(MINUTE, ItemLifespan, @dt2Now) ELSE ItemExpireDate END ELSE ItemExpireDate END
		, ItemLocationCode = 2
		, ItemLocationIndex = NULL
		, SealCount = 0
	WHERE ItemSerial = @inbItemSerial AND ItemLocationCode = 15;

	IF @@ROWCOUNT = 0
	BEGIN
		IF @intVersion < 3
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
			WHERE 1 = 0;

		ELSE IF @intVersion = 3
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
	
	/**_# ??? ??? ??? ?????.*/
	UPDATE dbo.SNDAEventItemHistory 
	SET ReceiveFlag = CAST (1 AS bit), ReceiveDate = GETDATE()
	WHERE ReceiveCharacterID = @inbCharacterID AND SenderCharacterID = @inbSenderCharacterID;

	/**_# ??? ??? ?????.*/
	EXEC dbo.P_AddItemLog
		  @dt2LogDate = @dt2Now
		, @inbItemSerial = @inbItemSerial
		, @intItemID = @intItemID
		, @inyItemLogCode = 31
		, @inbOwnerCharacterID = @inbCharacterID
		, @inbOwnerCharacterIDBefore = NULL
		, @insItemCount = @insItemCount
		, @insItemCountBefore = @insItemCount
		, @inbItemLogFKey1 = @intMailID
		, @inbItemLogFKey2 = NULL
		, @dt2OwnershipStartDate = NULL
		, @intIP = @intIP
		, @inySealCount = @inySealCount
		, @bitSoulBoundFlag = @bitSoulBoundFlag
		, @inyItemOption = @inyItemOption
		, @inyItemPotential = @inyItemPotential
		, @inyItemPotentialMoveCount = @inyItemPotentialMoveCount;

	IF @intVersion < 3
	BEGIN
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
			, @dtsItemExpireDate AS ItemExpireDate;
	END
	ELSE IF @intVersion = 3
	BEGIN
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
	END
	ELSE IF @intVersion = 4
	BEGIN
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
	END	
END TRY
BEGIN CATCH
	IF XACT_STATE() <> 0
		ROLLBACK TRANSACTION;

	IF @intReturnValue IS NULL OR @intReturnValue = 0
		EXEC @intReturnValue = dbo.P_AddErrorLog @intPROCID = @@PROCID;

	ELSE
		GOTO ErrorHandler;

	RETURN @intReturnValue;
END CATCH;

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

RETURN @intReturnValue;
GO

USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_TakeMailAttachCashItem]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 6
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-03-11
description : ??? ??? ?????? ????.
result set : ItemSerial/bigint, ItemID/int, ItemRemainCount/smallint, ItemDurability/smallint, RandomSeed/int, CoolTime/int, ItemLevel/tinyint, ItemPotential/tinyint, SoulBoundFlag/bit, SealCount/tinyint, ItemOption/tinyint, EternityFlag/bit, ItemExpireDate/smalldatetime, ItemPotentialMoveCount/tinyint	, ItemLifespan/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103168 = ?? ???? ????.
**/

CREATE PROCEDURE [dbo].[P_TakeMailAttachCashItem]
	  @intMailID int --// ?? ID
	, @inbItemSerial bigint --// ??? ?? ??
	, @intChannelID int --// ?? ID
	, @intMapID int --// ? ID
	, @vchIP varchar(15) = NULL --// ????? ?? IP ??
	, @bitDeleteItem bit --// MaterializedItems ????? ???? ?? (ex. ??? ???..)		
	, @inyItemLocationCode tinyint = 5 --// ItemLocationCode (?? ????? = 5, ?? = 9, ? = 10)
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dt2Now datetime2(0) = GETDATE()
	, @intItemID int
	, @inbReceiverCharacterID bigint
	, @inbCharacterID bigint				
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
	, @intItemLifespan int;

DECLARE @tblDeletedItem table (
	MaterializedItemID bigint NOT NULL,
	ItemID int NOT NULL,
	ItemRemainCount smallint NOT NULL,
	ItemDurability smallint NOT NULL,
	RandomSeed int NOT NULL,
	CoolTime int NOT NULL,
	ItemLevel tinyint NOT NULL,
	ItemPotential tinyint NOT NULL,
	SoulBoundFlag bit NOT NULL,
	SealCount tinyint NOT NULL,
	ItemOption tinyint NOT NULL,
	EternityFlag bit NOT NULL,
	ItemExpireDate smalldatetime NULL,
	ItemPotentialMoveCount tinyint NULL,
	ItemLifespan int NULL
)

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	UPDATE dbo.MailAttachItems
	SET @intItemID = ItemID
		, @insItemCount = AttachItemCount
		, ReceiveFlag = 1
	WHERE MailID = @intMailID AND ItemSerial = @inbItemSerial AND ReceiveFlag = CAST(0 AS bit);

	IF @@ROWCOUNT = 0
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
			, @intItemLifespan AS ItemLifespan
		WHERE 1 = 0;

		SET @intReturnValue = 103168;
		GOTO ErrorHandler;
	END

	SELECT @inbCharacterID = ReceiverCharacterID
	FROM dbo.Mails WITH (NOLOCK)
	WHERE MailID = @intMailID;

	IF @bitDeleteItem = 0
	BEGIN
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
			, @dtsItemExpireDate = CASE EternityFlag WHEN 0 THEN CASE WHEN ItemExpireDate IS NULL AND ItemLifespan > 0 THEN DATEADD(MINUTE, ItemLifespan, @dt2Now) ELSE ItemExpireDate END ELSE ItemExpireDate END
			, @inyItemPotentialMoveCount = ItemPotentialMoveCount
			, @intItemLifespan = ItemLifespan
			, OwnerCharacterID = @inbCharacterID
			, OwnershipStartDate = @dt2Now
			, ItemExpireDate = CASE EternityFlag WHEN 0 THEN CASE WHEN ItemExpireDate IS NULL AND ItemLifespan > 0 THEN DATEADD(MINUTE, ItemLifespan, @dt2Now) ELSE ItemExpireDate END ELSE ItemExpireDate END
			, ItemLocationCode = @inyItemLocationCode
			, ItemLocationIndex = NULL
		WHERE ItemSerial = @inbItemSerial AND ItemLocationCode = 15;
	END
	ELSE
	BEGIN
		DELETE dbo.MaterializedItems 
		OUTPUT deleted.MaterializedItemID, deleted.ItemID, deleted.ItemRemainCount, deleted.ItemDurability, deleted.RandomSeed, deleted.CoolTime
			 , deleted.ItemLevel, deleted.ItemPotential, deleted.SoulBoundFlag, deleted.SealCount, deleted.ItemOption, deleted.EternityFlag
			 , CASE deleted.EternityFlag WHEN 0 THEN CASE WHEN deleted.ItemExpireDate IS NULL AND deleted.ItemLifespan > 0 THEN DATEADD(MINUTE, deleted.ItemLifespan, @dt2Now) ELSE deleted.ItemExpireDate END ELSE deleted.ItemExpireDate END
			 , deleted.ItemPotentialMoveCount, deleted.ItemLifespan
		INTO @tblDeletedItem (MaterializedItemID, ItemID, ItemRemainCount, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, EternityFlag, ItemExpireDate, ItemPotentialMoveCount, ItemLifespan)
		WHERE ItemSerial = @inbItemSerial AND ItemLocationCode = 15;
	
		SELECT @inbMaterializedItemID = MaterializedItemID
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
			, @intItemLifespan = ItemLifespan
		FROM @tblDeletedItem;
	END

	IF @@ROWCOUNT = 0
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
			, @intItemLifespan AS ItemLifespan
		WHERE 1 = 0;

		SET @intReturnValue = 103168;
		GOTO ErrorHandler;
	END
	
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
		, @intItemLifespan AS ItemLifespan;

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

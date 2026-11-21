USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_UseBatchItems]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-03-12
description : ??? ?????? ??? ?????.(10? ??)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
1 = ?? ?? 
**/

CREATE PROCEDURE [dbo].[P_UseBatchItems]
	  @vchItemSerials varchar(1000) --// ??? ??? 
    , @vchUsedItemCount varchar(1000) --// ??? ???
    , @vchAllFlag varchar(1000) --// ??? ???
	, @intChannelID int --// ?? ID. 
	, @intMapID int --// ? ID 
	, @vchIP varchar(15) = NULL --// ????? ?? IP ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @tblTemp table (
	Seq int identity(1,1),
	ItemSerial bigint NOT NULL,
	UsedItemCount smallint NOT NULL,
	AllFlag bit NOT NULL
);
DECLARE @i int = 1
	, @inbItemSerial bigint
	, @insUsedItemCount smallint
	, @bitAllFlag bit
	, @dt2LogDate datetime2(0) = GETDATE()
	, @insRemainItemCount smallint
	, @insItemRemainCountBefore smallint
	, @inbOwnerCharacterID bigint
	, @intItemID int
	, @dt2OwnershipStartDate datetime2(0)
	, @inyPayMethodCode tinyint
	, @inbOwnerCharacterIDAfter bigint
	, @inyItemLogCode tinyint
	, @intIP int = dbo.FN_IP2IPNumber(@vchIP)
	, @inbMaterializedItemID bigint
	, @inySealCount tinyint
	, @bitSoulBoundFlag bit
	, @inyItemOption tinyint
	, @inyItemPotential tinyint
	, @inyItemLevel tinyint
	, @intLogTableSuffix int
	, @insItemDurability smallint;

DECLARE @tblItemLogs TABLE(
	  ItemSerial bigint NOT NULL
	, ItemID int NOT NULL
	, OwnerCharacterID bigint NULL
	, OwnerCharacterIDBefore bigint NOT NULL
	, ItemCount smallint NOT NULL
	, ItemCountBefore smallint NOT NULL
	, OwnershipStartDate datetime2(0) NOT NULL
	, SealCount tinyint NOT NULL
	, ItemOption tinyint NOT NULL
	, SoulBoundFlag bit NOT NULL
	, ItemPotential tinyint NOT NULL
	, ItemDurability smallint NOT NULL
	, ItemLevel tinyint NOT NULL
)

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	INSERT @tblTemp (ItemSerial, UsedItemCount, AllFlag)
	SELECT CAST(A.string as bigint), CAST(B.string as smallint), CAST(C.string as bit) 
	FROM dbo.TF_Split(@vchItemSerials,',') A 
		INNER JOIN dbo.TF_Split(@vchUsedItemCount,',') B ON A.seq = B.seq
		INNER JOIN dbo.TF_Split(@vchAllFlag,',') C ON A.seq = C.seq;

	SELECT A.Seq, A.AllFlag, B.ItemSerial, B.ItemID, B.ItemRemainCount, B.PayMethodCode, B.SenderCharacterID, B.ItemMaterializeCode, B.ItemMaterializeFKey, B.ItemDurability, B.RandomSeed, B.CoolTime, B.ItemLevel, B.ItemPotential, B.SoulBoundFlag, B.SealCount, B.ItemOption, B.ItemMaterializeDate, B.ItemLifespan, B.EternityFlag, B.ItemExpireDate, B.OwnerCharacterID, B.OwnershipStartDate, B.ExpireCompleteFlag, B.TradeItemFlag, B.AdditiveItemID, B.ItemLocationCode, B.ItemLocationIndex, B.ItemPotentialMoveCount, B.ItemCount, B.ItemPrice
	INTO #DeletedItems
	FROM @tblTemp A
		INNER JOIN dbo.MaterializedItems B WITH(NOLOCK) ON A.ItemSerial = B.ItemSerial;

	WHILE 1=1
	BEGIN
		SELECT @inbItemSerial = ItemSerial
		     , @insUsedItemCount = UsedItemCount
		     , @bitAllFlag = AllFlag
		FROM @tblTemp 
		WHERE Seq = @i

		IF @@ROWCOUNT = 0
			BREAK;

		IF @bitAllFlag = 0
		BEGIN
			UPDATE dbo.MaterializedItems
			SET @inbMaterializedItemID = MaterializedItemID
				, @insItemRemainCountBefore = ItemRemainCount
				, @inbOwnerCharacterID = OwnerCharacterID
				, @intItemID = ItemID
				, @inyPayMethodCode = PayMethodCode
				, @dt2OwnershipStartDate = OwnershipStartDate
				, @insRemainItemCount = ItemRemainCount -= @insUsedItemCount
				, @inbOwnerCharacterIDAfter = OwnerCharacterID = CASE WHEN ItemRemainCount > @insUsedItemCount THEN OwnerCharacterID ELSE NULL END
				, @inySealCount = SealCount
				, @bitSoulBoundFlag = SoulBoundFlag
				, @inyItemOption = ItemOption
				, @inyItemPotential = ItemPotential
				, @inyItemLevel = ItemLevel
				, @insItemDurability = ItemDurability
			WHERE ItemSerial = @inbItemSerial;

			/**_# ???? ???? ??? 103151? ?????.*/
			IF @@ROWCOUNT = 0 OR @insRemainItemCount < 0
			BEGIN
				SET @intReturnValue = 103151;
				GOTO ErrorHandler;
			END

			/**_# ?? ??? 0?? ???? ?? (??, ???? ?)?? ?????.*/
			IF @insRemainItemCount = 0
				DELETE dbo.MaterializedItems
				WHERE MaterializedItemID = @inbMaterializedItemID
				OPTION (KEEPFIXED PLAN);
	  
			INSERT @tblItemLogs (ItemSerial, ItemID, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, OwnershipStartDate, SealCount, ItemOption, SoulBoundFlag, ItemPotential, ItemDurability, ItemLevel)
			VALUES (@inbItemSerial, @intItemID, @inbOwnerCharacterIDAfter, @inbOwnerCharacterID, @insRemainItemCount, @insItemRemainCountBefore, @dt2OwnershipStartDate, @inySealCount, @inyItemOption, @bitSoulBoundFlag, @inyItemPotential, @insItemDurability, @inyItemLevel)
		END
		ELSE
		BEGIN
			SELECT @inbMaterializedItemID = MaterializedItemID
				, @insItemRemainCountBefore = ItemRemainCount
				, @inbOwnerCharacterID = OwnerCharacterID
				, @intItemID = ItemID
				, @inyPayMethodCode = PayMethodCode
				, @dt2OwnershipStartDate = OwnershipStartDate
				, @insRemainItemCount = ItemRemainCount - @insUsedItemCount
				, @inbOwnerCharacterIDAfter = NULL
				, @inySealCount = SealCount
				, @bitSoulBoundFlag = SoulBoundFlag
				, @inyItemOption = ItemOption
				, @inyItemPotential = ItemPotential
				, @inyItemLevel = ItemLevel
				, @insItemDurability = ItemDurability
			FROM dbo.MaterializedItems WITH (NOLOCK)
			WHERE ItemSerial = @inbItemSerial;

			IF @@ROWCOUNT = 0 OR @insRemainItemCount < 0
			BEGIN
				SET @intReturnValue = 103151;
				GOTO ErrorHandler;
			END

			DELETE dbo.MaterializedItems
			WHERE MaterializedItemID = @inbMaterializedItemID
			OPTION (KEEPFIXED PLAN);

			INSERT @tblItemLogs (ItemSerial, ItemID, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, OwnershipStartDate, SealCount, ItemOption, SoulBoundFlag, ItemPotential, ItemDurability, ItemLevel)
			VALUES (@inbItemSerial, @intItemID, NULL, @inbOwnerCharacterID, @insRemainItemCount, @insItemRemainCountBefore, @dt2OwnershipStartDate, @inySealCount, @inyItemOption, @bitSoulBoundFlag, @inyItemPotential, @insItemDurability, @inyItemLevel)
		END

		SET @i += 1;
	END	

	SET @intLogTableSuffix = dbo.FN_LogTableSuffix(@dt2LogDate);

	/**_# [ItemLogs] ???? INSERT???.*/
	IF @intLogTableSuffix = 0
		INSERT dbo.ItemLogs0 (LogDate, ItemSerial, ItemID, ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, IP, SealCount, ItemOption, SoulBoundFlag, ItemPotential, ItemDurability, ItemLevel, ItemLevelBefore)
		SELECT @dt2LogDate, ItemSerial, ItemID, 20, NULL, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, @intMapID, NULL, OwnershipStartDate, @intIP, SealCount, ItemOption, SoulBoundFlag, ItemPotential, ItemDurability, ItemLevel, ItemLevel  
		FROM @tblItemLogs
		

	ELSE IF @intLogTableSuffix = 1
		INSERT dbo.ItemLogs1 (LogDate, ItemSerial, ItemID, ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, IP, SealCount, ItemOption, SoulBoundFlag, ItemPotential, ItemDurability, ItemLevel, ItemLevelBefore)
		SELECT @dt2LogDate, ItemSerial, ItemID, 20, NULL, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, @intMapID, NULL, OwnershipStartDate, @intIP, SealCount, ItemOption, SoulBoundFlag, ItemPotential, ItemDurability, ItemLevel, ItemLevel  
		FROM @tblItemLogs

	ELSE
		INSERT dbo.ItemLogs2 (LogDate, ItemSerial, ItemID, ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, IP, SealCount, ItemOption, SoulBoundFlag, ItemPotential, ItemDurability, ItemLevel, ItemLevelBefore)
		SELECT @dt2LogDate, ItemSerial, ItemID, 20, NULL, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, @intMapID, NULL, OwnershipStartDate, @intIP, SealCount, ItemOption, SoulBoundFlag, ItemPotential, ItemDurability, ItemLevel, ItemLevel  
		FROM @tblItemLogs


END TRY
BEGIN CATCH

	GOTO ErrorHandler;
END CATCH;

RETURN 0;

ErrorHandler:
	SET @i = @i-1;

	WHILE 1=1
	BEGIN
		SELECT @bitAllFlag = AllFlag
		FROM #DeletedItems
		WHERE Seq = @i;
		
		IF @@ROWCOUNT = 0
			BREAK;

		IF @bitAllFlag = 0
			UPDATE dbo.MaterializedItems
			SET ItemRemainCount += B.UsedItemCount	
			FROM #DeletedItems A
				INNER JOIN @tblTemp B ON A.Seq = B.Seq
				INNER JOIN MaterializedItems C ON A.ItemSerial = C.ItemSerial
			WHERE A.Seq = @i
		ELSE
			INSERT dbo.MaterializedItems (ItemSerial, ItemID, ItemRemainCount, PayMethodCode, SenderCharacterID, ItemMaterializeCode, ItemMaterializeFKey, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, ItemMaterializeDate, ItemLifespan, EternityFlag, ItemExpireDate, OwnerCharacterID, OwnershipStartDate, ExpireCompleteFlag, TradeItemFlag, AdditiveItemID, ItemLocationCode, ItemLocationIndex, ItemPotentialMoveCount, ItemCount, ItemPrice)
			SELECT ItemSerial, ItemID, ItemRemainCount, PayMethodCode, SenderCharacterID, ItemMaterializeCode, ItemMaterializeFKey, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, ItemMaterializeDate, ItemLifespan, EternityFlag, ItemExpireDate, OwnerCharacterID, OwnershipStartDate, ExpireCompleteFlag, TradeItemFlag, AdditiveItemID, ItemLocationCode, ItemLocationIndex, ItemPotentialMoveCount, ItemCount, ItemPrice 
			FROM #DeletedItems
			WHERE Seq = @i;

		SET @i -= 1;
	END

IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

IF @intReturnValue IS NULL OR @intReturnValue = 0
	EXEC @intReturnValue = dbo.P_AddErrorLog;

RETURN @intReturnValue;
GO

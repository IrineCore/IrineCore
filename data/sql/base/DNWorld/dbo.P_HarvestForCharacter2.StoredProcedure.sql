USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_HarvestForCharacter2]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-07-11

description :
	?? ????? ?????.
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
	2 = ??? ??? ?? ?? ??     by 20110914.wgkim
	3 = ?? ???? ???? ??? ???? ?? ?? ??     by 20110916.wgkim
	4 = ItemLog ???? SkipItem ?? ?? ??     by ??? at 2012-01-15
**/

CREATE PROCEDURE [dbo].[P_HarvestForCharacter2]
	  @inbCharacterID bigint --// ??? ID
	, @insFieldIndex smallint --// ?? ID

	, @inbItemSerial1 bigint --// ??? ?? ??
	, @intItemID1 int --// ??? ID
	, @insItemCount1 smallint --// ??? ??
	, @bitSoulBoundFlag1 bit --// 0=??, 1=??
	, @inySealCount1 tinyint --// ?? ???
	, @inyItemOption1 tinyint --// ??? ??
	, @bitEternityFlag1 bit --// 0=???? ?? ??, 1=?? ???
	, @intItemLifespan1 int --// ??? ?? (?)

	, @inbItemSerial2 bigint --// ??? ?? ??
	, @intItemID2 int --// ??? ID
	, @insItemCount2 smallint --// ??? ??
	, @bitSoulBoundFlag2 bit --// 0=??, 1=??
	, @inySealCount2 tinyint --// ?? ???
	, @inyItemOption2 tinyint --// ??? ??
	, @bitEternityFlag2 bit --// 0=???? ?? ??, 1=?? ???
	, @intItemLifespan2 int --// ??? ?? (?)

	, @inbItemSerial3 bigint --// ??? ?? ??
	, @intItemID3 int --// ??? ID
	, @insItemCount3 smallint --// ??? ??
	, @bitSoulBoundFlag3 bit --// 0=??, 1=??
	, @inySealCount3 tinyint --// ?? ???
	, @inyItemOption3 tinyint --// ??? ??
	, @bitEternityFlag3 bit --// 0=???? ?? ??, 1=?? ???
	, @intItemLifespan3 int --// ??? ?? (?)

	, @intMapID int --// ?? ? ID
	, @vchIP varchar(15) = NULL --// ????? ?? IP ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @dt2Now datetime2(0) = GETDATE(), @intIP int = dbo.FN_IP2IPNumber(@vchIP), @xmlAttachments xml;
DECLARE @intLogTableSuffix int = dbo.FN_LogTableSuffix(@dt2Now);
DECLARE @tblTemp2 table (MaterializedItemID bigint NOT NULL);
DECLARE @tblTemp table (
	ItemSerial bigint NOT NULL,
	ItemID int NOT NULL,
	ItemCount smallint NOT NULL,
	SoulBoundFlag bit NOT NULL,
	SealCount tinyint NOT NULL,
	ItemOption tinyint NOT NULL,
	EternityFlag bit NOT NULL,
	ItemLifespan int NULL
);

DECLARE @tblFieldDeleteLogs TABLE
( FieldIndex	smallint		NOT NULL
, CharacterID	bigint			NOT NULL
, ItemID		int				NOT NULL
, PlantDate		datetime2(0)	NOT NULL
, ElapsedTime	int				NOT NULL
);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	INSERT @tblTemp (ItemSerial, ItemID, ItemCount, SoulBoundFlag, SealCount, ItemOption, EternityFlag, ItemLifespan)
	SELECT @inbItemSerial1, @intItemID1, @insItemCount1, @bitSoulBoundFlag1, @inySealCount1, @inyItemOption1, @bitEternityFlag1, @intItemLifespan1 WHERE @inbItemSerial1 IS NOT NULL
	UNION ALL
	SELECT @inbItemSerial2, @intItemID2, @insItemCount2, @bitSoulBoundFlag2, @inySealCount2, @inyItemOption2, @bitEternityFlag2, @intItemLifespan2 WHERE @inbItemSerial2 IS NOT NULL
	UNION ALL
	SELECT @inbItemSerial3, @intItemID3, @insItemCount3, @bitSoulBoundFlag3, @inySealCount3, @inyItemOption3, @bitEternityFlag3, @intItemLifespan3 WHERE @inbItemSerial3 IS NOT NULL;

	SET @xmlAttachments = (
		SELECT AttachedItemID, AttachedItemCount, AttachedDate
		FROM dbo.FieldForCharacterAttachments WITH (NOLOCK)
		WHERE CharacterID = @inbCharacterID AND FieldIndex = @insFieldIndex
		FOR XML AUTO, ROOT('root')
	);

	/**_# [FieldForCharacterAttachments] ????? DELETE ???.*/
	DELETE dbo.FieldForCharacterAttachments
	WHERE CharacterID = @inbCharacterID AND FieldIndex = @insFieldIndex
	OPTION (KEEPFIXED PLAN);

	/**_# [FieldsForCharacter] ????? DELETE ???.*/
	DELETE dbo.FieldsForCharacter
	OUTPUT deleted.FieldIndex, deleted.CharacterID, deleted.ItemID, deleted.PlantDate, deleted.ElapsedTime
	INTO @tblFieldDeleteLogs (FieldIndex, CharacterID, ItemID, PlantDate, ElapsedTime)
	WHERE CharacterID = @inbCharacterID AND FieldIndex = @insFieldIndex
	OPTION (KEEPFIXED PLAN);

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103283;
		GOTO ErrorHandler;
	END

	/**_# [MaterializedItems] ???? INSERT ???.*/
	INSERT dbo.MaterializedItems (ItemSerial, ItemID, ItemRemainCount, PayMethodCode, SenderCharacterID, ItemMaterializeCode, ItemMaterializeFKey, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, ItemMaterializeDate, ItemLifespan, EternityFlag, ItemExpireDate, OwnerCharacterID, OwnershipStartDate, ExpireCompleteFlag, TradeItemFlag, ItemLocationCode, ItemLocationIndex)
	OUTPUT inserted.MaterializedItemID INTO @tblTemp2 (MaterializedItemID)
	SELECT ItemSerial, ItemID, ItemCount, 1, NULL, 23, 0, 0, 0, 0, 0, 0, SoulBoundFlag, SealCount, ItemOption, @dt2Now, ItemLifespan, EternityFlag, DATEADD(MINUTE, CASE EternityFlag WHEN 0 THEN ItemLifespan ELSE 525600 END, @dt2Now), @inbCharacterID, @dt2Now, 0, 0, 11, 0
	FROM @tblTemp
	OPTION (KEEPFIXED PLAN);

	/**_# [ItemLogs] ???? INSERT???.*/
	IF @intLogTableSuffix = 0
		INSERT dbo.ItemLogs0 (LogDate, ItemSerial, ItemID, ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, IP, SealCount, ItemOption, SoulBoundFlag, ItemPotential, ItemDurability)
		SELECT @dt2Now, T.ItemSerial, T.ItemID, 47, @inbCharacterID, NULL, T.ItemCount, 0, @intMapID, 0, @dt2Now, @intIP, T.SealCount, T.ItemOption, T.SoulBoundFlag, 0, 0
		FROM @tblTemp T
		WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK, INDEX(IX_UC_SkipLoggingItems_ItemID_ItemLogCode), FORCESEEK) WHERE ItemID = T.ItemID AND (ItemLogCode IS NULL OR ItemLogCode = 47));

	ELSE IF @intLogTableSuffix = 1
		INSERT dbo.ItemLogs1 (LogDate, ItemSerial, ItemID, ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, IP, SealCount, ItemOption, SoulBoundFlag, ItemPotential, ItemDurability)
		SELECT @dt2Now, T.ItemSerial, T.ItemID, 47, @inbCharacterID, NULL, T.ItemCount, 0, @intMapID, 0, @dt2Now, @intIP, T.SealCount, T.ItemOption, T.SoulBoundFlag, 0, 0
		FROM @tblTemp T
		WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK, INDEX(IX_UC_SkipLoggingItems_ItemID_ItemLogCode), FORCESEEK) WHERE ItemID = T.ItemID AND (ItemLogCode IS NULL OR ItemLogCode = 47));

	ELSE
		INSERT dbo.ItemLogs2 (LogDate, ItemSerial, ItemID, ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, IP, SealCount, ItemOption, SoulBoundFlag, ItemPotential, ItemDurability)
		SELECT @dt2Now, T.ItemSerial, T.ItemID, 47, @inbCharacterID, NULL, T.ItemCount, 0, @intMapID, 0, @dt2Now, @intIP, T.SealCount, T.ItemOption, T.SoulBoundFlag, 0, 0
		FROM @tblTemp T
		WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK, INDEX(IX_UC_SkipLoggingItems_ItemID_ItemLogCode), FORCESEEK) WHERE ItemID = T.ItemID AND (ItemLogCode IS NULL OR ItemLogCode = 47));

	/**_# [FieldsForCharacter] ??? ?? ??? ?????. */
	IF @intLogTableSuffix = 0
	BEGIN
		INSERT dbo.FieldDeleteLogs0 (LogDate, FarmID, FieldIndex, CharacterID, ItemID, PlantDate, ElapsedTime, HarvestFlag)
		SELECT @dt2Now, -1, FieldIndex, CharacterID, ItemID, PlantDate, ElapsedTime, 1 FROM @tblFieldDeleteLogs OPTION (KEEPFIXED PLAN);

		IF @xmlAttachments IS NOT NULL
		BEGIN
			INSERT dbo.FieldAttachmentDeleteLogs0 (LogDate, FarmID, FieldIndex, CharacterID, Attachments, HarvestFlag)
			SELECT @dt2Now, -1, FieldIndex, CharacterID, @xmlAttachments, 1 FROM @tblFieldDeleteLogs OPTION (KEEPFIXED PLAN);
		END
	END
	ELSE IF @intLogTableSuffix = 1
	BEGIN
		INSERT dbo.FieldDeleteLogs1 (LogDate, FarmID, FieldIndex, CharacterID, ItemID, PlantDate, ElapsedTime, HarvestFlag)
		SELECT @dt2Now, -1, FieldIndex, CharacterID, ItemID, PlantDate, ElapsedTime, 1 FROM @tblFieldDeleteLogs OPTION (KEEPFIXED PLAN);

		IF @xmlAttachments IS NOT NULL
		BEGIN
			INSERT dbo.FieldAttachmentDeleteLogs1 (LogDate, FarmID, FieldIndex, CharacterID, Attachments, HarvestFlag)
			SELECT @dt2Now, -1, FieldIndex, CharacterID, @xmlAttachments, 1 FROM @tblFieldDeleteLogs OPTION (KEEPFIXED PLAN);
		END
	END
	ELSE
	BEGIN
		INSERT dbo.FieldDeleteLogs2 (LogDate, FarmID, FieldIndex, CharacterID, ItemID, PlantDate, ElapsedTime, HarvestFlag)
		SELECT @dt2Now, -1, FieldIndex, CharacterID, ItemID, PlantDate, ElapsedTime, 1 FROM @tblFieldDeleteLogs OPTION (KEEPFIXED PLAN);

		IF @xmlAttachments IS NOT NULL
		BEGIN
			INSERT dbo.FieldAttachmentDeleteLogs2 (LogDate, FarmID, FieldIndex, CharacterID, Attachments, HarvestFlag)
			SELECT @dt2Now, -1, FieldIndex, CharacterID, @xmlAttachments, 1 FROM @tblFieldDeleteLogs OPTION (KEEPFIXED PLAN);
		END
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

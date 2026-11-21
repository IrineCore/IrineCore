USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_MakeItemFromItems]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-12-20
description : ???? ???? ? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103123 = ??? ?????.
103151 = ???? ???? ????.
103156 = ItemSerial ?????.
history :
2 = (50729) ??? ?? ???? ?? ?? by ??? at 2012-01-30 
**/

CREATE PROCEDURE [dbo].[P_MakeItemFromItems]
	  @inbCharacterID bigint --// ??? ID
	, @vchSRCItemSerials varchar(8000) --// ,? ?????? ?? ???? ??? ???
	, @vchSRCItemCounts varchar(8000) --// ,? ?????? ?? ???? ?? ???
	, @vchSRCAllFlags varchar(8000) --// ,? ?????? ? ???? ?? ???? ?? (0 or 1) ???
	, @inbFee bigint --// ???
	, @inyItemMaterializeCode tinyint --// ??? ?? ??. 9=???? ??, 11=??? ???? ??, 13=??, 14=??, 19=Fixed Item, 21=??? ??, 31=?? ??? ??
	, @inbItemSerial bigint --// ??? ???
	, @intItemID int --// ??? ID
	, @insItemCount smallint --// ??? ??
	, @insItemDurability smallint --// ???
	, @intRandomSeed int --// ?? ??
	, @inyItemLevel tinyint --// ??? ??
	, @inyItemPotential tinyint --// ??? ???
	, @bitSoulBoundFlag bit --// 0=??, 1=?????
	, @inySealCount tinyint --// ?? ???
	, @inyItemOption tinyint --// ??? ??
	, @bitEternityFlag bit --// 0=???? ?? ??, 1=?? ???
	, @intItemLifespan int --// ??? ?? (?)
	, @inyItemLocationCode tinyint --// 2=????, 5=?? ????
	, @intChannelID int --// ?? ID
	, @intMapID int --// ? ID
	, @vchIP varchar(15) = NULL --// ????? ?? IP ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dt2Now datetime2(0) = GETDATE()
	, @intIP int = dbo.FN_IP2IPNumber(@vchIP)
	, @inyItemLogCode tinyint = CASE @inyItemMaterializeCode WHEN 21 THEN 43 WHEN 31 THEN 57 ELSE @inyItemMaterializeCode END
	, @inyCoinChangeCode tinyint
	, @inbSRCItemSerial bigint
	, @insSRCItemCount smallint
	, @bitSRCItemAllFlag bit
	, @i int = 1
	, @j int
	, @insSRCItemRemainCountBefore smallint
	, @intSRCItemID int
	, @dt2SRCOwnershipStartDate datetime2(0)
	, @insSRCRemainItemCount smallint
	, @inbSRCOwnerCharacterIDAfter bigint
	, @inySRCSealCount tinyint
	, @bitSRCSoulBoundFlag bit
	, @inySRCItemOption tinyint
	, @inySRCItemPotential tinyint
	, @inySRCItemLevel tinyint
	, @inbSRCMaterializedItemID bigint;

DECLARE @tblSRCItems table (
	Seq int IDENTITY(1, 1) NOT NULL PRIMARY KEY,
	ItemSerial bigint NOT NULL,
	ItemCount smallint NOT NULL,
	AllFlag bit NOT NULL
);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	INSERT @tblSRCItems (ItemSerial, ItemCount, AllFlag)
	SELECT A.string, B.string, C.string
	FROM dbo.TF_Split(@vchSRCItemSerials, ',') A
		INNER JOIN dbo.TF_Split(@vchSRCItemCounts, ',') B ON A.seq = B.seq
		INNER JOIN dbo.TF_Split(@vchSRCAllFlags, ',') C ON A.seq = C.seq
	OPTION (FORCE ORDER, LOOP JOIN, KEEPFIXED PLAN);

	SET @j = @@ROWCOUNT;

	IF @j = 0 OR EXISTS (
		SELECT *
		FROM @tblSRCItems S
		WHERE NOT EXISTS (
			SELECT *
			FROM dbo.MaterializedItems WITH (NOLOCK, INDEX(IX_UNI_MaterializedItems_ItemSerial), FORCESEEK)
			WHERE ItemSerial = S.ItemSerial AND ItemRemainCount >= S.ItemCount AND OwnerCharacterID = @inbCharacterID AND ItemLocationCode IN (2, 5)
		)
	)
	BEGIN
		SET @intReturnValue = 103151;
		GOTO ErrorHandler;
	END

	IF @inbFee > 0
	BEGIN
		SET @inyCoinChangeCode = CASE @inyItemMaterializeCode WHEN 11 THEN 19 ELSE 25 END

		EXEC @intReturnValue = dbo.P_ModCoinForSubProc
			  @inbCharacterID = @inbCharacterID
			, @inyCoinChangeCode = @inyCoinChangeCode
			, @inbCoinChangeFKey = @inbItemSerial
			, @inbCoin = @inbFee
			, @intChannelID = @intChannelID
			, @intMapID = @intMapID
			, @dt2LogDate = @dt2Now;

		IF @intReturnValue <> 0
			GOTO ErrorHandler;
	END

	WHILE @i <= @j
	BEGIN
		SELECT @inbSRCItemSerial = ItemSerial
			, @insSRCItemCount = ItemCount
			, @bitSRCItemAllFlag = AllFlag
		FROM @tblSRCItems
		WHERE Seq = @i;

		IF @bitSRCItemAllFlag = 0
		BEGIN
			UPDATE dbo.MaterializedItems
			SET @inbSRCMaterializedItemID = MaterializedItemID
				, @insSRCItemRemainCountBefore = ItemRemainCount
				, @intSRCItemID = ItemID
				, @dt2SRCOwnershipStartDate = OwnershipStartDate
				, @insSRCRemainItemCount = ItemRemainCount -= @insSRCItemCount
				, @inbSRCOwnerCharacterIDAfter = OwnerCharacterID = CASE WHEN ItemRemainCount > @insSRCItemCount THEN OwnerCharacterID ELSE NULL END
				, @inySRCSealCount = SealCount
				, @bitSRCSoulBoundFlag = SoulBoundFlag
				, @inySRCItemOption = ItemOption
				, @inySRCItemPotential = ItemPotential
				, @inySRCItemLevel = ItemLevel
			WHERE ItemSerial = @inbSRCItemSerial;

			/**_# ???? ???? ??? 103151? ?????.*/
			IF @@ROWCOUNT = 0 OR @insSRCRemainItemCount < 0
			BEGIN
				SET @intReturnValue = 103151;
				GOTO ErrorHandler;
			END

			/**_# ?? ??? 0?? ???? ?? (??, ???? ?)?? ?????.*/
			IF @insSRCRemainItemCount = 0
				DELETE dbo.MaterializedItems
				WHERE MaterializedItemID = @inbSRCMaterializedItemID
				OPTION (KEEPFIXED PLAN);
		END

		ELSE
		BEGIN
			SELECT @inbSRCMaterializedItemID = MaterializedItemID
				, @insSRCItemRemainCountBefore = ItemRemainCount
				, @intSRCItemID = ItemID
				, @dt2SRCOwnershipStartDate = OwnershipStartDate
				, @insSRCRemainItemCount = 0
				, @inbSRCOwnerCharacterIDAfter = NULL
				, @inySRCSealCount = SealCount
				, @bitSRCSoulBoundFlag = SoulBoundFlag
				, @inySRCItemOption = ItemOption
				, @inySRCItemPotential = ItemPotential
			FROM dbo.MaterializedItems WITH (NOLOCK)
			WHERE ItemSerial = @inbSRCItemSerial;

			DELETE dbo.MaterializedItems
			WHERE MaterializedItemID = @inbSRCMaterializedItemID
			OPTION (KEEPFIXED PLAN);

			IF @@ROWCOUNT = 0 OR @insSRCRemainItemCount < 0
			BEGIN
				SET @intReturnValue = 103151;
				GOTO ErrorHandler;
			END
		END

		/**_# ??? ??? ?????.*/
		SET @dt2SRCOwnershipStartDate = CASE @insSRCRemainItemCount WHEN 0 THEN @dt2SRCOwnershipStartDate ELSE NULL END;

		EXEC @intReturnValue = dbo.P_AddItemLog
			  @dt2LogDate = @dt2Now
			, @inbItemSerial = @inbSRCItemSerial
			, @intItemID = @intSRCItemID
			, @inyItemLogCode = 20
			, @inbOwnerCharacterID = @inbSRCOwnerCharacterIDAfter
			, @inbOwnerCharacterIDBefore = @inbCharacterID
			, @insItemCount = @insSRCRemainItemCount
			, @insItemCountBefore = @insSRCItemRemainCountBefore
			, @inbItemLogFKey1 = @intMapID
			, @inbItemLogFKey2 = @intItemID
			, @dt2OwnershipStartDate = @dt2SRCOwnershipStartDate
			, @intIP = @intIP
			, @inySealCount = @inySRCSealCount
			, @bitSoulBoundFlag = @bitSRCSoulBoundFlag
			, @inyItemOption = @inySRCItemOption
			, @inyItemPotential = @inySRCItemPotential
			, @inyItemLevel = @inySRCItemLevel;

		SET @i += 1;
	END

	INSERT dbo.MaterializedItems (ItemSerial, ItemID, ItemRemainCount, SenderCharacterID, ItemMaterializeCode, ItemMaterializeFKey, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, ItemMaterializeDate, ItemLifespan, EternityFlag, ItemExpireDate, OwnerCharacterID, OwnershipStartDate, PayMethodCode, ItemLocationCode, ItemLocationIndex)
	VALUES (@inbItemSerial, @intItemID, @insItemCount, NULL, @inyItemMaterializeCode, NULL, @insItemDurability, @intRandomSeed, 0, @inyItemLevel, @inyItemPotential, @bitSoulBoundFlag, @inySealCount, @inyItemOption, @dt2Now, @intItemLifespan, @bitEternityFlag, DATEADD(MINUTE, CASE @bitEternityFlag WHEN 0 THEN @intItemLifespan ELSE 525600 END, @dt2Now), @inbCharacterID, @dt2Now, NULL, @inyItemLocationCode, NULL);

	EXEC dbo.P_AddItemLog
		  @dt2LogDate = @dt2Now
		, @inbItemSerial = @inbItemSerial
		, @intItemID = @intItemID
		, @inyItemLogCode = @inyItemLogCode
		, @inbOwnerCharacterID = @inbCharacterID
		, @inbOwnerCharacterIDBefore = NULL
		, @insItemCount = @insItemCount
		, @insItemCountBefore = 0
		, @inbItemLogFKey1 = NULL
		, @inbItemLogFKey2 = NULL
		, @dt2OwnershipStartDate = NULL
		, @intIP = @intIP
		, @inySealCount = @inySealCount
		, @bitSoulBoundFlag = @bitSoulBoundFlag
		, @inyItemOption = @inyItemOption
		, @inyItemPotential = @inyItemPotential
		, @inyItemLevel = @inyItemLevel;

END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 2601 AND ERROR_MESSAGE() LIKE N'%IX_UNI_MaterializedItems_ItemSerial%'
		SET @intReturnValue = 103156;

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

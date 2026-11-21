USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddMaterializedItem2]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 36
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-07-08
description : ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103123 = ??? ?????.
103156 = ItemSerial ?????.
103309 = merge ?? ???? ???? ????.
103336 = ?? ?? ??? ????.
103406 = ???? ???? ????.
history :
3 = ???? ?? ?? ?? ??? ??, ???? ?? ? ?? ?? ????? ??	by2011/10/18.ohtack
5 = ItemCopyCheck ???? ???? ???? ????? ?? at 2011-10-19
6~8 = ??? ??? ?? ??     by ??? at 2011-10-26
10 = TF_Split(), TF_NSplit() ??? ????? ?? by ??? at 2011-11-01
11 = ?? ?? ?? ?? ?? by ??? at 2011-12-01
12~13 = ? ?? ?? ?? ?? by ??? at 2011-12-08
14 = ?? ??? ??? ??? ?? ????? ?? by ??? at 2011-12-08
15 = @inyItemMaterializeCode=31,32 ??     by ??? at 2012-01-25
16~17 = (50729) ??? ?? ???? ?? ?? by ??? at 2012-01-30 
18 = ItemMerge? ItemLog ?? ??? ?? by ??? at 2012-02-24
19~20 = ?? ?? ?? ??     by ??? at 2012-04-09
21 = ?? ???? ??? ??? ???? ????? ?? by ??? at 2012-04-19
22 = ? ?? ??? ??? ?? ?? by ??? at 2011-12-08
23 = 18? ?? by ??? at 2012-08-24
24 = ???? ?? ?? ?? by ??? at 2012-08-24
25 = @inyItemMaterializeCode, @inbItemMaterializeFKey ?? ?? (36=?????)     by ??? at 2012-09-11
26~27 = dbo.MaterializedItems ???? ItemCount, ItemPrice ?? ??     by ??? at 2012-10-25
29 = ItemLogCode ?? , 73=??????, 74=??(??)	by ??? at 2013-01-21
34 = ?????? ??? ?? ??	by ??? at 2013-03-25
35 = ??? ? ???? ?? ?? ???? ??	by ??? at 2013-05-20
**/

CREATE PROCEDURE [dbo].[P_AddMaterializedItem2]
	  @inbItemSerial bigint --// ??? ?? ??
	, @inbCharacterID bigint --// ???? ??? ??? ID
	, @inyItemMaterializeCode tinyint --// ??? ?? ??. 1=??, 2=?? ??, 3=?????, 4=??, 5=?????? ??, 6=?? ??, 7=?? ??, 8=DNA??? ??, 9=???? ??, 10=???  ?? ??, 11=??? ???? ??, 13=??, 14=??, 15=?? ??, 16=PVP ??, 17=?? ??, 18=??? ?? ??, 19=Fixed Item, 20=??????, 21=??? ??, 22= ??, 23=??, 24=???, 25=PVP?????? ??, 26=?? ???? ??, 27=??? ??,  28=??? ?? ???? ??, 29=???, 30=?? ?? ??, 31=(DOORS) ?? ??, 32=(DOORS) ?? ??, 33=???? ?? ?????, 34 =????, 35=??????, 36=?????, 37=???? ?? ??? ??, 38=???? ??? ??? ?? ,39=???? ?? ??? ?? ,40=?? ??, 41=??????????? 
	, @inbItemMaterializeFKey bigint --// ItemMaterializeCode ? ?? ? ?. 1:PartyID or NULL, 2:MissionID, 3:QuestID, 4:PurchaseID, 5:NPCID, 6/7/8:PurchaseID, 10:MailID, 11:NPCID, 13:NULL, 14:NULL, 15:NULL, 16:NULL, 17:NULL, 18:MailID, 19:ItemSerial, 22:CharacterID, 29:RepurchaseNo, 30:GuildID, 31=(DOORS) ContractID, 32=(DOORS) AuctionID, 36=EventRewardID
	, @intItemID int --// ??? ID
	, @insItemCount smallint --// ??? ??
	, @insItemDurability smallint --// ???
	, @intRandomSeed int --// ?? ??
	, @inyItemLevel tinyint --// ??? ??
	, @inyItemPotential tinyint --// ??? ???
	, @bitSoulBoundFlag bit --// 0=??, 1=?????
	, @inySealCount tinyint --// ?? ???
	, @inyItemOption tinyint --// ??? ??
	, @inbItemPrice bigint --// ??? ?? ??. ???? ?? ???? ?? 0? ?????. ?, ??? ??? ?? ?? ??? ??, ??? ???? ? ???? ??? ?? ??? 1/n? ?????.
	, @bitEternityFlag bit --// 0=???? ?? ??, 1=?? ???
	, @intItemLifespan int --// ??? ?? (?)
	, @inbSenderCharacterID bigint --// ??? ?? ??? ??? ID
	, @inyItemLocationCode tinyint --// 2=????, 4=??? ????, 5=?? ????, 7=?? ????, 8=??, 9=??, 10=?
	, @inyItemLocationIndex tinyint --// ???? ??? ??? ?. ????, ????, ??? ????, ?? ???? ? ???? ?????. ? ? 0? ?????.
	, @inyPayMethodCode tinyint --// 1=????, 2=??, 3=??, 4=??, 5=?????, 6=??? ??, 7=?? ??, 8=??? ?? ? ??, 9=VIP ??, 10=PVP ?? ???, 11=?? ?? ???, 12=?? ?? ???, 13=?? ?? ???, 14=??2, 15=Time Event ??, 16=??? ?? ???, 17=??? ??, 18=???, 19=??????, 20=?? ??, 25=??, 26=???????????
	, @intChannelID int --// ?? ID
	, @intMapID int --// ? ID
	, @vchIP varchar(15) = NULL --// ????? ?? IP ??
	, @vchPetEquipmentIndexes varchar(100) = NULL --// ?? or ?? ???? ?? default? ???? ?? ???? ??? ???. ,? ???? string ??.
	, @vchPetEquipmentItemIDs varchar(100) = NULL --// ?? or ?? ???? ?? default? ???? ?? ???? ??? ID. ,? ???? string ??.
	, @vchPetEquipmentItemSerials varchar(100) = NULL --// ?? or ?? ???? ?? default? ???? ?? ???? ????. ,? ???? string ??.
	, @intPetSkinColor int = NULL --// ?? or ? ??
	, @bitStackableFlag bit = 1 --// 0=Stack ??, 1=Stack ??
	, @intPetSkinColor2 int = NULL --// ? ?? 2
	, @intPetSkillID1 int = NULL --// ? ?? 1
	, @intPetSkillID2 int = NULL --// ? ?? 2
	, @bitMergeOccurFlag bit --// 0=???? merge?? ??, 1=???? merge?
	, @inbMergeTargetItemSerial bigint --// ??? ???? ????? merge?? ??, target ???? ??? ???
	, @inyDragonJewelType tinyint = NULL --// ??????
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dt2Now datetime2(0) = GETDATE()
	, @intIP int = dbo.FN_IP2IPNumber(@vchIP)
	, @insTargetItemCountBefore smallint
	, @insTargetItemCountAfter smallint
	, @inyItemLogCode tinyint = 
		CASE @inyItemMaterializeCode
			WHEN 20 THEN 39
			WHEN 21 THEN 43
			WHEN 22 THEN 48
			WHEN 23 THEN 47
			WHEN 24 THEN 49
			WHEN 25 THEN 50
			WHEN 26 THEN 51
			WHEN 27 THEN 52
			WHEN 28 THEN 53
			WHEN 29 THEN 54
			WHEN 30 THEN 55
			WHEN 31 THEN 59
			WHEN 32 THEN 60
			WHEN 34 THEN 64
			WHEN 35 THEN 67
			WHEN 36 THEN 73
			WHEN 37 THEN 81
			WHEN 38 THEN 82
			WHEN 39 THEN 83
			WHEN 40 THEN 74
			WHEN 41 THEN 78
		ELSE @inyItemMaterializeCode END
	, @bitPetExpandSkillFlag bit = 0
	, @intCoolTime int
	, @dt2ItemMaterializeDate datetime2(0)
	, @sdtItemExpireDate smalldatetime
	, @intAdditiveItemID int
	, @inyCoinChangeCode tinyint
	, @inyItemPotentialMoveCount tinyint = 0;

DECLARE @tblEquipments table (
	PetEquipmentIndex tinyint NOT NULL PRIMARY KEY,
	ItemID int NOT NULL,
	ItemSerial bigint NOT NULL
);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	IF EXISTS (
		SELECT *
		FROM dbo.ItemCopyCheck WITH (NOLOCK, INDEX(PK_ItemCopyCheck), FORCESEEK)
		WHERE CharacterID = @inbCharacterID
			AND ItemMaterializeDate >= DATEADD(SECOND, -3, GETDATE())
	)
	BEGIN
		INSERT dbo.TableAuditLogs (tableName, operationCode, columnNames, beforeImages, modifier, occurredDate)
		VALUES (N'P_UseItem2', 2, '', CAST(@intItemID AS varchar(50)) + ' / ' + CAST(@insItemCount AS varchar(50)), 'SYS', GETDATE());

		SET @intReturnValue = 100;
		GOTO ErrorHandler;
	END

	/**_# ???? ??? ?? ??? ?????.*/
	/**_## ??? ???? 103123? ?????.*/
	IF @inyItemMaterializeCode IN (5, 29, 30) AND @inyPayMethodCode = 1 AND @inbItemPrice > 0
	BEGIN
		IF @inyItemMaterializeCode IN (5, 29)
			SET @inyCoinChangeCode = 18;
		ELSE IF @inyItemMaterializeCode = 30
			SET @inyCoinChangeCode = 33;

		EXEC @intReturnValue = dbo.P_ModCoinForSubProc
			  @inbCharacterID = @inbCharacterID
			, @inyCoinChangeCode = @inyCoinChangeCode
			, @inbCoinChangeFKey = @inbItemSerial
			, @inbCoin = @inbItemPrice
			, @intChannelID = @intChannelID
			, @intMapID = @intMapID
			, @dt2LogDate = @dt2Now;

		IF @intReturnValue <> 0
			GOTO ErrorHandler;
	END

	IF @bitMergeOccurFlag = 0 --0=???? merge?? ??, 1=???? merge?
	BEGIN
		-- ???? ?? parameter ???
		IF @inyItemMaterializeCode = 29
		BEGIN
			SELECT @inbCharacterID = CharacterID
				 , @inbItemSerial = ISNULL(ItemSerial, @inbItemSerial)
				 , @intItemID = ItemID
				 , @insItemCount = ItemRemainCount
				 , @insItemDurability = ItemDurability
				 , @intRandomSeed = RandomSeed
				 , @intCoolTime  = CoolTime
				 , @inyItemLevel = ItemLevel
				 , @inyItemPotential = ItemPotential
				 , @bitSoulBoundFlag = SoulBoundFlag
				 , @inyItemOption = ItemOption
				 , @inySealCount = SealCount
				 , @dt2ItemMaterializeDate = ItemMaterializeDate
				 , @bitEternityFlag = EternityFlag
				 , @sdtItemExpireDate = ItemExpireDate
				 , @intAdditiveItemID = AdditiveItemID
			     , @inyItemPotentialMoveCount = ItemPotentialMoveCount
				 , @inyDragonJewelType = DragonJewelType
			  FROM dbo.RepurchaseItem WITH (NOLOCK)
			 WHERE RepurchaseID = @inbItemMaterializeFKey;

			IF @@ROWCOUNT < 0
			BEGIN
				SET @intReturnValue = 103406;
				GOTO ErrorHandler;
			END

			INSERT dbo.MaterializedItems (ItemSerial, ItemID
			, ItemRemainCount, SenderCharacterID, ItemMaterializeCode, ItemMaterializeFKey, ItemDurability
			, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag
			, SealCount, ItemOption, ItemMaterializeDate, ItemLifespan, EternityFlag
			, ItemExpireDate, OwnerCharacterID, OwnershipStartDate, PayMethodCode, ItemLocationCode, ItemLocationIndex, ItemPotentialMoveCount, DragonJewelType)
			VALUES (@inbItemSerial, @intItemID
			, @insItemCount, @inbSenderCharacterID, @inyItemMaterializeCode, @inbItemMaterializeFKey, @insItemDurability
			, @intRandomSeed, @intCoolTime, @inyItemLevel, @inyItemPotential, @bitSoulBoundFlag
			, @inySealCount, @inyItemOption, @dt2ItemMaterializeDate, @intItemLifespan, @bitEternityFlag
			, @sdtItemExpireDate, @inbCharacterID, @dt2Now, @inyPayMethodCode, @inyItemLocationCode, @inyItemLocationIndex, @inyItemPotentialMoveCount, @inyDragonJewelType);
		END
		ELSE
		BEGIN
			INSERT dbo.MaterializedItems (ItemSerial, ItemID, ItemRemainCount, SenderCharacterID, ItemMaterializeCode, ItemMaterializeFKey, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, ItemMaterializeDate, ItemLifespan, EternityFlag, ItemExpireDate, OwnerCharacterID, OwnershipStartDate, PayMethodCode, ItemLocationCode, ItemLocationIndex, ItemPotentialMoveCount, ItemCount, ItemPrice, DragonJewelType)
			VALUES (@inbItemSerial, @intItemID, @insItemCount, @inbSenderCharacterID, @inyItemMaterializeCode, @inbItemMaterializeFKey, @insItemDurability, @intRandomSeed, 0, @inyItemLevel, @inyItemPotential, @bitSoulBoundFlag, @inySealCount, @inyItemOption, @dt2Now, @intItemLifespan, @bitEternityFlag, DATEADD(MINUTE, CASE @bitEternityFlag WHEN 0 THEN @intItemLifespan ELSE 525600 END, @dt2Now), @inbCharacterID, @dt2Now, @inyPayMethodCode, @inyItemLocationCode, @inyItemLocationIndex, @inyItemPotentialMoveCount, @insItemCount, @inbItemPrice, @inyDragonJewelType);
		END
	END
	ELSE
	BEGIN
		UPDATE dbo.MaterializedItems
		SET @insTargetItemCountBefore = ItemRemainCount
			, @insTargetItemCountAfter = ItemRemainCount += @insItemCount
		WHERE ItemSerial = @inbMergeTargetItemSerial
			AND OwnerCharacterID = @inbCharacterID
			AND ItemLocationCode = @inyItemLocationCode;

		IF @@ROWCOUNT = 0
		BEGIN
			SET @intReturnValue = 103309;
			GOTO ErrorHandler;
		END
		
		EXEC dbo.P_AddItemLog
			  @dt2LogDate = @dt2Now
			, @inbItemSerial = @inbItemSerial
			, @intItemID = @intItemID
			, @inyItemLogCode = 33
			, @inbOwnerCharacterID = NULL
			, @inbOwnerCharacterIDBefore = @inbCharacterID
			, @insItemCount = 0
			, @insItemCountBefore = @insItemCount
			, @inbItemLogFKey1 = @inbMergeTargetItemSerial
			, @inbItemLogFKey2 = NULL
			, @dt2OwnershipStartDate = @dt2Now
			, @intIP = NULL;

		EXEC dbo.P_AddItemLog
			  @dt2LogDate = @dt2Now
			, @inbItemSerial = @inbMergeTargetItemSerial
			, @intItemID = @intItemID
			, @inyItemLogCode = 34
			, @inbOwnerCharacterID = @inbCharacterID
			, @inbOwnerCharacterIDBefore = @inbCharacterID
			, @insItemCount = @insTargetItemCountAfter
			, @insItemCountBefore = @insTargetItemCountBefore
			, @inbItemLogFKey1 = @inbItemSerial
			, @inbItemLogFKey2 = NULL
			, @dt2OwnershipStartDate = NULL
			, @intIP = NULL;
		
	END

	SET @inbItemMaterializeFKey = 
		CASE
			WHEN @inyItemMaterializeCode IN (1, 11, 15, 22, 24, 27) THEN @intMapID
			ELSE @inbItemMaterializeFKey
		END;

	EXEC dbo.P_AddItemLog
		  @dt2LogDate = @dt2Now
		, @inbItemSerial = @inbItemSerial
		, @intItemID = @intItemID
		, @inyItemLogCode = @inyItemLogCode
		, @inbOwnerCharacterID = @inbCharacterID
		, @inbOwnerCharacterIDBefore = NULL
		, @insItemCount = @insItemCount
		, @insItemCountBefore = 0
		, @inbItemLogFKey1 = @inbItemMaterializeFKey
		, @inbItemLogFKey2 = @inbSenderCharacterID
		, @dt2OwnershipStartDate = NULL
		, @intIP = @intIP
		, @inySealCount = @inySealCount
		, @bitSoulBoundFlag = @bitSoulBoundFlag
		, @inyItemOption = @inyItemOption
		, @inyItemPotential = @inyItemPotential
		, @inyItemLevel = @inyItemLevel;

	/**_# ??? ???? ?? ??? ???? ?? ???? ?????. */
	IF @inyItemMaterializeCode = 29
	BEGIN
		DELETE dbo.RepurchaseItem WHERE RepurchaseID = @inbItemMaterializeFKey
	END

	/**_# ?? or ?? ?? ?? ?? ??? ?????.*/
	IF @inyItemLocationCode IN (9, 10)
	BEGIN
		INSERT @tblEquipments (PetEquipmentIndex, ItemID, ItemSerial)
		SELECT A.string, B.string, C.string
		FROM dbo.TF_Split(@vchPetEquipmentIndexes, ',') A
			INNER JOIN dbo.TF_Split(@vchPetEquipmentItemIDs, ',') B ON A.seq = B.seq
			INNER JOIN dbo.TF_Split(@vchPetEquipmentItemSerials, ',') C ON A.seq = C.seq
		WHERE A.string > '' AND B.string > '' AND C.string > ''
		OPTION (KEEPFIXED PLAN, MAXDOP 1);

		INSERT dbo.MaterializedItems (ItemSerial, ItemID, ItemRemainCount, SenderCharacterID, ItemMaterializeCode, ItemMaterializeFKey, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, ItemMaterializeDate, ItemLifespan, EternityFlag, ItemExpireDate, OwnerCharacterID, OwnershipStartDate, PayMethodCode, ItemLocationCode, ItemLocationIndex)
		SELECT ItemSerial, ItemID, 1, NULL, 17, @inbItemSerial, 0, 0, 0, 0, 0, 1, 0, 0, @dt2Now, 0, 1, NULL, @inbCharacterID, @dt2Now, NULL, 13, 0
		FROM @tblEquipments
		OPTION (KEEPFIXED PLAN);

		INSERT dbo.PetEquipments (PetItemSerial, PetEquipmentIndex, PetEquipmentItemSerial, SkinColor)
		SELECT @inbItemSerial, PetEquipmentIndex, ItemSerial, NULL
		FROM @tblEquipments
		OPTION (KEEPFIXED PLAN);

		IF @intPetSkillID1 IS NOT NULL
		BEGIN
			/**_# ? ?? ??? INSERT ???.*/
			EXEC dbo.P_AddPetSkillLog
					@inbItemSerial,
					@intPetSkillID1;
		END
		IF @intPetSkillID2 IS NOT NULL
		BEGIN
			/**_# ? ?? ??? ?????.*/
			SET @bitPetExpandSkillFlag = 1;
			/**_# ? ?? ??? INSERT ???.*/
			EXEC dbo.P_AddPetSkillLog
					@inbItemSerial,
					@intPetSkillID2;
		END

		INSERT dbo.PetEquipments (PetItemSerial, PetEquipmentIndex, PetEquipmentItemSerial, SkinColor, SkinColor2, SkillID1, SkillID2, PetName, ExpandSkillFlag)
		VALUES (@inbItemSerial, 255, NULL, @intPetSkinColor, @intPetSkinColor2, @intPetSkillID1, @intPetSkillID2, NULL, @bitPetExpandSkillFlag)
		OPTION (KEEPFIXED PLAN);
	END

	/**_# ?? ??? ???? ?? ??? UPDATE???.*/
	IF @inyItemMaterializeCode = 34
	BEGIN
		UPDATE dbo.BestFriends
		SET ItemSerial = @inbItemSerial
		WHERE CharacterID = @inbCharacterID;

		IF @@ROWCOUNT = 0
		BEGIN
			SET @intReturnValue = 103336;
			GOTO ErrorHandler;
		END
	END

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

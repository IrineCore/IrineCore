USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_SendSystemMailForPet]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-04-11
description : ? & ??? ??? ??? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103163 = ?? ?? ???? ???? ????.
103149 = ?? ???? ItemSerial???.
103380 = DWC ????? ??? ??? ? ????.
**/

CREATE PROCEDURE [dbo].[P_SendSystemMailForPet]
	  @nvcSenderName nvarchar(50) --// ??? ??
	, @inbReceiverCharacterID bigint --// ?? ??? ID
	, @inyMailTypeCode tinyint --// 2=?? ??, 4=??? ??
	, @inbSystemMailFKey bigint --// MailTypeCode ? ?? ? ?. 2:MissionID, 4:0
	, @nvcSubject nvarchar(50) --// ??
	, @nvcContent nvarchar(300) --// ??
	, @inbAttachCoin bigint --// ?? ??

	, @inbItemSerial bigint = NULL --// (optional) ??? ?? ??
	, @intItemID int = NULL --// (optional)??? ID
	, @insItemCount smallint = NULL --// (optional)??? ??
	, @insItemDurability smallint = NULL --// (optional)???
	, @intRandomSeed int = NULL --// (optional)?? ??
	, @inyItemLevel tinyint = NULL --// (optional)??? ??
	, @inyItemPotential tinyint = NULL --// (optional)??? ???
	, @bitSoulBoundFlag bit = NULL --// (optional)0=??, 1=?????
	, @inySealCount tinyint = NULL --// (optional)?? ???
	, @inyItemOption tinyint = NULL --// (optional)??? ??
	, @bitEternityFlag bit = NULL --// (optional)0=???? ?? ??, 1=?? ???
	, @intItemLifespan int = NULL --// (optional)??? ?? (?)

	, @vchPetEquipmentIndexes varchar(100) = NULL --// (optional)?? or ?? ???? ?? default? ???? ?? ???? ??? ???. ,? ???? string ??.
	, @vchPetEquipmentItemIDs varchar(100) = NULL --// (optional)?? or ?? ???? ?? default? ???? ?? ???? ??? ID. ,? ???? string ??.
	, @vchPetEquipmentItemSerials varchar(100) = NULL --// (optional)?? or ?? ???? ?? default? ???? ?? ???? ????. ,? ???? string ??.
	, @intPetSkinColor int = NULL --// (optional)?? or ? ??	
	, @intPetSkinColor2 int = NULL --// (optional)? ?? 2
	, @intPetSkillID1 int = NULL --// (optional)? ?? 1
	, @intPetSkillID2 int = NULL --// (optional)? ?? 2

	, @intChannelID int --// ?? ID
	, @intMapID int --// ? ID

	, @intTotalMailCount int OUTPUT --// ? ?? ?? ?
	, @intNotReadMailCount int OUTPUT --// ?? ?? ?? ?
	, @int7DaysLeftMailCount int OUTPUT --// ?? ??? 7? ?? ?? ?

	, @intMailID int = NULL OUTPUT --// ?? ID

	, @bitNewFlag bit = NULL --// ???? ??? ?? ??. 0=false, 1=true

	, @intMailLifespan int = 43200 --// ?? ???? (?, ?? 30?)
	, @bitAttachCashItem bit = 0 --// ????? ?? ?? 
	, @inyMailTabCode tinyint = NULL --// ?? ? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dt2Now datetime2(0) = GETDATE()
	, @intAttachItemCount int = 0
	, @intAttachEquipCount int = 0
	, @intLogTableSuffix int
	, @inyAccountLevelCode tinyint
	, @bitPetExpandSkillFlag bit = 0;

DECLARE @tblItemSerials table (seq tinyint IDENTITY(1, 1) NOT NULL, ItemSerial bigint NOT NULL PRIMARY KEY, ItemID int NOT NULL, ItemCount smallint NOT NULL);

DECLARE @tblAttachItems table (
	seq tinyint IDENTITY(1, 1) NOT NULL PRIMARY KEY,
	ItemSerial bigint NOT NULL,
	ItemID int NOT NULL,
	ItemCount smallint NOT NULL,
	ItemDurability smallint NOT NULL,
	RandomSeed int NOT NULL,
	ItemLevel tinyint NOT NULL,
	ItemPotential tinyint NOT NULL,
	SoulBoundFlag bit NOT NULL,
	SealCount tinyint NOT NULL,
	ItemOption tinyint NOT NULL,
	ItemLifespan int NULL,
	EternityFlag bit NOT NULL
);

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
	/**_# ?? ?? ???? ???? ??? 103163? ?????.*/
	SELECT @inyAccountLevelCode = AccountLevelCode
	FROM dbo.Characters WITH (NOLOCK)
	WHERE CharacterID = @inbReceiverCharacterID AND DeleteFlag = CAST(0 AS bit)

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103163;
		GOTO ErrorHandler;
	END

	/**_# ?? ?? ???? DWC ????? 103380? ?????.*/
	IF @inyAccountLevelCode = 110
	BEGIN
		SET @intReturnValue = 103380;
		GOTO ErrorHandler;
	END

	INSERT @tblAttachItems (ItemSerial, ItemID, ItemCount, ItemDurability, RandomSeed, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, ItemLifespan, EternityFlag)
	SELECT @inbItemSerial, @intItemID, @insItemCount, @insItemDurability, @intRandomSeed, @inyItemLevel, @inyItemPotential, @bitSoulBoundFlag, @inySealCount, @inyItemOption, @intItemLifespan, @bitEternityFlag WHERE @inbItemSerial > 0;

	SET @intAttachItemCount = @@ROWCOUNT;

	/**_# [Mails] ???? INSERT???.*/
	INSERT dbo.Mails (SenderCharacterID, ReceiverCharacterID, MailTypeCode, DeliveryTypeCode, Subject, Content, SendDate, ReadFlag, DeleteFlag, AttachCoin, AttachCoinReceiveFlag, SenderName, SystemMailFKey, NewFlag, MailExpireDate, MailTabCode)
	VALUES (NULL, @inbReceiverCharacterID, @inyMailTypeCode, CASE @intAttachItemCount WHEN 1 THEN 1 ELSE 2 END, @nvcSubject, @nvcContent, @dt2Now, 0, 0, @inbAttachCoin, 0, @nvcSenderName, @inbSystemMailFKey, @bitNewFlag, DATEADD(MINUTE, @intMailLifespan, @dt2Now), @inyMailTabCode);
	SET @intMailID = SCOPE_IDENTITY();

	IF @intAttachItemCount > 0
	BEGIN
		INSERT dbo.MaterializedItems (ItemSerial, ItemID, ItemRemainCount, SenderCharacterID, ItemMaterializeCode, ItemMaterializeFKey, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, ItemMaterializeDate, ItemLifespan, EternityFlag, ItemExpireDate, OwnerCharacterID, OwnershipStartDate, ItemLocationCode, ItemLocationIndex)
		OUTPUT inserted.ItemSerial, inserted.ItemID, inserted.ItemRemainCount INTO @tblItemSerials (ItemSerial, ItemID, ItemCount)
		SELECT ItemSerial, ItemID, ItemCount, NULL, 18, @intMailID, ItemDurability, RandomSeed, 0, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, @dt2Now, ItemLifespan, EternityFlag
			 , CASE WHEN @bitAttachCashItem = 1 AND EternityFlag = 0 THEN NULL 
			   ELSE DATEADD(MINUTE, CASE EternityFlag WHEN 0 THEN ItemLifespan ELSE 525600 END, @dt2Now) END
			 , NULL, NULL, 15, 0
		FROM @tblAttachItems;

		INSERT @tblEquipments (PetEquipmentIndex, ItemID, ItemSerial)
		SELECT A.string, B.string, C.string
		FROM dbo.TF_Split(@vchPetEquipmentIndexes, ',') A
			INNER JOIN dbo.TF_Split(@vchPetEquipmentItemIDs, ',') B ON A.seq = B.seq
			INNER JOIN dbo.TF_Split(@vchPetEquipmentItemSerials, ',') C ON A.seq = C.seq
		WHERE A.string > '' AND B.string > '' AND C.string > ''
		OPTION (KEEPFIXED PLAN, MAXDOP 1);

		SET @intAttachEquipCount = @@ROWCOUNT;
		
		IF @intAttachEquipCount > 0
		BEGIN

			INSERT dbo.MaterializedItems (ItemSerial, ItemID, ItemRemainCount, SenderCharacterID, ItemMaterializeCode, ItemMaterializeFKey, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, ItemMaterializeDate, ItemLifespan, EternityFlag, ItemExpireDate, OwnerCharacterID, OwnershipStartDate, PayMethodCode, ItemLocationCode, ItemLocationIndex)
			SELECT ItemSerial, ItemID, 1, NULL, 17, @inbItemSerial, 0, 0, 0, 0, 0, 1, 0, 0, @dt2Now, 0, 1, NULL, @inbReceiverCharacterID, @dt2Now, NULL, 13, 0
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

		/**_# [MailAttachItems] ???? INSERT???.*/
		INSERT dbo.MailAttachItems (MailID, ItemSerial, ItemID, FirstAttachItemFlag, ReceiveFlag, AttachItemCount)
		SELECT @intMailID, ItemSerial, ItemID, CASE seq WHEN 1 THEN 1 ELSE 0 END, 0, ItemCount FROM @tblItemSerials;
		SET @intLogTableSuffix = dbo.FN_LogTableSuffix(@dt2Now);

		/**_# [ItemLogs] ???? INSERT???.*/
		IF @intLogTableSuffix = 0
			INSERT dbo.ItemLogs0 (LogDate, ItemSerial, ItemID, ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, IP)
			SELECT @dt2Now, ItemSerial, ItemID, 10, NULL, NULL, ItemCount, 0, @intMailID, NULL, NULL, NULL
			FROM @tblItemSerials A
			WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK) WHERE ItemID = A.ItemID AND (ItemLogCode IS NULL OR ItemLogCode = 18));

		ELSE IF @intLogTableSuffix = 1
			INSERT dbo.ItemLogs1 (LogDate, ItemSerial, ItemID, ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, IP)
			SELECT @dt2Now, ItemSerial, ItemID, 18, NULL, NULL, ItemCount, 0, @intMailID, NULL, NULL, NULL
			FROM @tblItemSerials A
			WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK) WHERE ItemID = A.ItemID AND (ItemLogCode IS NULL OR ItemLogCode = 18));

		ELSE
			INSERT dbo.ItemLogs2 (LogDate, ItemSerial, ItemID, ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, IP)
			SELECT @dt2Now, ItemSerial, ItemID, 18, NULL, NULL, ItemCount, 0, @intMailID, NULL, NULL, NULL
			FROM @tblItemSerials A
			WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK) WHERE ItemID = A.ItemID AND (ItemLogCode IS NULL OR ItemLogCode = 18));
	END

	EXEC @intReturnValue = dbo.P_GetCountReceiveMail
		  @inbReceiverCharacterID = @inbReceiverCharacterID
		, @intTotalMailCount = @intTotalMailCount OUTPUT
		, @intNotReadMailCount = @intNotReadMailCount OUTPUT
		, @int7DaysLeftMailCount = @int7DaysLeftMailCount OUTPUT;
END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 547 AND ERROR_MESSAGE() LIKE N'%FK_Characters_Mails_ReceiverCharacterID%'
		SET @intReturnValue = 103163;
	ELSE IF ERROR_NUMBER() = 2601 AND ERROR_MESSAGE() LIKE N'%IX_UNI_MaterializedItems_ItemSerial%'
		SET @intReturnValue = 103149;

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

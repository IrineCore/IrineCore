USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_SendSystemMail]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 36
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-27
description : ??? ??? ????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103163 = ?? ?? ???? ???? ????.
103149 = ?? ???? ItemSerial???.
103380 = DWC ????? ??? ??? ? ????.
history : 
20 = @inyMailTypeCode, @inbSystemMailFKey ?? ?? at 2010-06-08
21 = SkipLoggingItems ???? ???? ?? ??? ????? ?? at 2010-08-09
22 = ?? ??? ??? 2? ???? ???? ???? ???? ?? at 2010-08-09
23 = ??? ??? ??, ??? ??? at 2010-11-25
24 = ???? at 2011-03-29
27 = (50729) ??? ?? ???? ?? ?? by ??? at 2012-01-30 
29~30 = DWC ???? ?? ?? ? ??? ?? by ??? at 2013-02-19
31~32 = ??? ???? ??? ?? ?? by ??? at 2013-04-03
33 = ??? ??? OwnerCharacterID ??? ?? by ??? at 2013-04-09
34 = 33? ?? by ??? at 2013-04-09
35 = ?????? ?? by ??? at 2013-07-29
**/

CREATE PROCEDURE [dbo].[P_SendSystemMail]
	  @nvcSenderName nvarchar(50) --// ??? ??
	, @inbReceiverCharacterID bigint --// ?? ??? ID
	, @inyMailTypeCode tinyint --// 2=?? ??, 4=??? ??
	, @inbSystemMailFKey bigint --// MailTypeCode ? ?? ? ?. 2:MissionID, 4:0
	, @nvcSubject nvarchar(50) --// ??
	, @nvcContent nvarchar(300) --// ??
	, @inbAttachCoin bigint --// ?? ??

	, @inbItemSerial1 bigint = NULL --// (optional) ??? ?? ??
	, @intItemID1 int = NULL --// (optional)??? ID
	, @insItemCount1 smallint = NULL --// (optional)??? ??
	, @insItemDurability1 smallint = NULL --// (optional)???
	, @intRandomSeed1 int = NULL --// (optional)?? ??
	, @inyItemLevel1 tinyint = NULL --// (optional)??? ??
	, @inyItemPotential1 tinyint = NULL --// (optional)??? ???
	, @bitSoulBoundFlag1 bit = NULL --// (optional)0=??, 1=?????
	, @inySealCount1 tinyint = NULL --// (optional)?? ???
	, @inyItemOption1 tinyint = NULL --// (optional)??? ??
	, @bitEternityFlag1 bit = NULL --// (optional)0=???? ?? ??, 1=?? ???
	, @intItemLifespan1 int = NULL --// (optional)??? ?? (?)	

	, @inbItemSerial2 bigint = NULL --// (optional) ??? ?? ??
	, @intItemID2 int = NULL --// (optional)??? ID
	, @insItemCount2 smallint = NULL --// (optional)??? ??
	, @insItemDurability2 smallint = NULL --// (optional)???
	, @intRandomSeed2 int = NULL --// (optional)?? ??
	, @inyItemLevel2 tinyint = NULL --// (optional)??? ??
	, @inyItemPotential2 tinyint = NULL --// (optional)??? ???
	, @bitSoulBoundFlag2 bit = NULL --// (optional)0=??, 1=?????
	, @inySealCount2 tinyint = NULL --// (optional)?? ???
	, @inyItemOption2 tinyint = NULL --// (optional)??? ??
	, @bitEternityFlag2 bit = NULL --// (optional)0=???? ?? ??, 1=?? ???
	, @intItemLifespan2 int = NULL --// (optional)??? ?? (?)	

	, @inbItemSerial3 bigint = NULL --// (optional) ??? ?? ??
	, @intItemID3 int = NULL --// (optional)??? ID
	, @insItemCount3 smallint = NULL --// (optional)??? ??
	, @insItemDurability3 smallint = NULL --// (optional)???
	, @intRandomSeed3 int = NULL --// (optional)?? ??
	, @inyItemLevel3 tinyint = NULL --// (optional)??? ??
	, @inyItemPotential3 tinyint = NULL --// (optional)??? ???
	, @bitSoulBoundFlag3 bit = NULL --// (optional)0=??, 1=?????
	, @inySealCount3 tinyint = NULL --// (optional)?? ???
	, @inyItemOption3 tinyint = NULL --// (optional)??? ??
	, @bitEternityFlag3 bit = NULL --// (optional)0=???? ?? ??, 1=?? ???
	, @intItemLifespan3 int = NULL --// (optional)??? ?? (?)	

	, @inbItemSerial4 bigint = NULL --// (optional) ??? ?? ??
	, @intItemID4 int = NULL --// (optional)??? ID
	, @insItemCount4 smallint = NULL --// (optional)??? ??
	, @insItemDurability4 smallint = NULL --// (optional)???
	, @intRandomSeed4 int = NULL --// (optional)?? ??
	, @inyItemLevel4 tinyint = NULL --// (optional)??? ??
	, @inyItemPotential4 tinyint = NULL --// (optional)??? ???
	, @bitSoulBoundFlag4 bit = NULL --// (optional)0=??, 1=?????
	, @inySealCount4 tinyint = NULL --// (optional)?? ???
	, @inyItemOption4 tinyint = NULL --// (optional)??? ??
	, @bitEternityFlag4 bit = NULL --// (optional)0=???? ?? ??, 1=?? ???
	, @intItemLifespan4 int = NULL --// (optional)??? ?? (?)
	, @inbItemSerial5 bigint = NULL --// (optional) ??? ?? ??	
	
	, @intItemID5 int = NULL --// (optional)??? ID
	, @insItemCount5 smallint = NULL --// (optional)??? ??
	, @insItemDurability5 smallint = NULL --// (optional)???
	, @intRandomSeed5 int = NULL --// (optional)?? ??
	, @inyItemLevel5 tinyint = NULL --// (optional)??? ??
	, @inyItemPotential5 tinyint = NULL --// (optional)??? ???
	, @bitSoulBoundFlag5 bit = NULL --// (optional)0=??, 1=?????
	, @inySealCount5 tinyint = NULL --// (optional)?? ???
	, @inyItemOption5 tinyint = NULL --// (optional)??? ??
	, @bitEternityFlag5 bit = NULL --// (optional)0=???? ?? ??, 1=?? ???
	, @intItemLifespan5 int = NULL --// (optional)??? ?? (?)	

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
	, @inyDragonJewelType1 tinyint = NULL --// (optional)??????1
	, @inyDragonJewelType2 tinyint = NULL --// (optional)??????2
	, @inyDragonJewelType3 tinyint = NULL --// (optional)??????3
	, @inyDragonJewelType4 tinyint = NULL --// (optional)??????4
	, @inyDragonJewelType5 tinyint = NULL --// (optional)??????5
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dt2Now datetime2(0) = GETDATE()
	, @intAttachItemCount int = 0
	, @intLogTableSuffix int
	, @inyAccountLevelCode tinyint;

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
	EternityFlag bit NOT NULL,
	DragonJewelType tinyint NULL
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

	INSERT @tblAttachItems (ItemSerial, ItemID, ItemCount, ItemDurability, RandomSeed, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, ItemLifespan, EternityFlag, DragonJewelType)
	SELECT @inbItemSerial1, @intItemID1, @insItemCount1, @insItemDurability1, @intRandomSeed1, @inyItemLevel1, @inyItemPotential1, @bitSoulBoundFlag1, @inySealCount1, @inyItemOption1, @intItemLifespan1, @bitEternityFlag1, @inyDragonJewelType1 WHERE @inbItemSerial1 > 0 UNION ALL
	SELECT @inbItemSerial2, @intItemID2, @insItemCount2, @insItemDurability2, @intRandomSeed2, @inyItemLevel2, @inyItemPotential2, @bitSoulBoundFlag2, @inySealCount2, @inyItemOption2, @intItemLifespan2, @bitEternityFlag2, @inyDragonJewelType2 WHERE @inbItemSerial2 > 0 UNION ALL
	SELECT @inbItemSerial3, @intItemID3, @insItemCount3, @insItemDurability3, @intRandomSeed3, @inyItemLevel3, @inyItemPotential3, @bitSoulBoundFlag3, @inySealCount3, @inyItemOption3, @intItemLifespan3, @bitEternityFlag3, @inyDragonJewelType3 WHERE @inbItemSerial3 > 0 UNION ALL
	SELECT @inbItemSerial4, @intItemID4, @insItemCount4, @insItemDurability4, @intRandomSeed4, @inyItemLevel4, @inyItemPotential4, @bitSoulBoundFlag4, @inySealCount4, @inyItemOption4, @intItemLifespan4, @bitEternityFlag4, @inyDragonJewelType4 WHERE @inbItemSerial4 > 0 UNION ALL
	SELECT @inbItemSerial5, @intItemID5, @insItemCount5, @insItemDurability5, @intRandomSeed5, @inyItemLevel5, @inyItemPotential5, @bitSoulBoundFlag5, @inySealCount5, @inyItemOption5, @intItemLifespan5, @bitEternityFlag5, @inyDragonJewelType5 WHERE @inbItemSerial5 > 0;

	SET @intAttachItemCount = @@ROWCOUNT;

	/**_# [Mails] ???? INSERT???.*/
	INSERT dbo.Mails (SenderCharacterID, ReceiverCharacterID, MailTypeCode, DeliveryTypeCode, Subject, Content, SendDate, ReadFlag, DeleteFlag, AttachCoin, AttachCoinReceiveFlag, SenderName, SystemMailFKey, NewFlag, MailExpireDate, MailTabCode)
	VALUES (NULL, @inbReceiverCharacterID, @inyMailTypeCode, CASE @intAttachItemCount WHEN 1 THEN 1 ELSE 2 END, @nvcSubject, @nvcContent, @dt2Now, 0, 0, @inbAttachCoin, 0, @nvcSenderName, @inbSystemMailFKey, @bitNewFlag, DATEADD(MINUTE, @intMailLifespan, @dt2Now), @inyMailTabCode);
	SET @intMailID = SCOPE_IDENTITY();

	IF @intAttachItemCount > 0
	BEGIN
		INSERT dbo.MaterializedItems (ItemSerial, ItemID, ItemRemainCount, SenderCharacterID, ItemMaterializeCode, ItemMaterializeFKey, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, ItemMaterializeDate, ItemLifespan, EternityFlag, ItemExpireDate, OwnerCharacterID, OwnershipStartDate, ItemLocationCode, ItemLocationIndex, DragonJewelType)
		OUTPUT inserted.ItemSerial, inserted.ItemID, inserted.ItemRemainCount INTO @tblItemSerials (ItemSerial, ItemID, ItemCount)
		SELECT ItemSerial, ItemID, ItemCount, NULL, 18, @intMailID, ItemDurability, RandomSeed, 0, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, @dt2Now, ItemLifespan, EternityFlag
			 , CASE WHEN @bitAttachCashItem = 1 AND EternityFlag = 0 THEN NULL 
			   ELSE DATEADD(MINUTE, CASE EternityFlag WHEN 0 THEN ItemLifespan ELSE 525600 END, @dt2Now) END
			 , NULL, NULL, 15, 0, DragonJewelType
		FROM @tblAttachItems;

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
	EXEC @intReturnValue = dbo.P_AddErrorLog;

RETURN @intReturnValue;
GO

USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DLS_SendDOLISMailWithXML]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-05-21
description : DOLIS?? ??? ?????. (With XML)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103102 = ???? ???? ????.
103193 = @vchCertKey? ?? ????.
103214 = ?? ??? ??? ???? ????.
history :
2 = ????? ??? ???? ?? ?? by ??? at 2013-07-23
**/

CREATE PROCEDURE [dbo].[P_DLS_SendDOLISMailWithXML]
	  @intAuditLogID int --// GSM? ?? ?? ID
	, @nvcSenderName nvarchar(50) --// ??? ??
	, @inyReceiverTarget tinyint --// 1=P_StoreTargetCharacter? ???? ?? ??? ??, 2=??? ?? ??, 3=??
	, @vchCertKey char(32) = NULL --// P_StoreTargetCharacter ??? ??? @vchCertKey ?. ???? ?? ???? ???? ?? ? ?? NULL? ?????.
	, @xmlCharacterIDs xml = NULL --// ?? ?? ??? ID XML??? (<root><CH CharacterID="123" /><CH CharacterID="456" /></root>). ???? ???? ???? NULL ?????.
	, @xmlCharacterNames xml = NULL --// ?? ?? ??? ?? XML??? (<root><CH CharacterName="aaa" /><CH CharacterName="bbb" /></root>). ???? ???? ???? NULL ?????.

	, @nvcSubject nvarchar(50) --// ??
	, @nvcContent nvarchar(300) --// ??
	, @inbAttachCoin bigint --// ?? ??

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
	, @inyDragonJewelType1 tinyint = NULL --// ??????
	
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
	, @inyDragonJewelType2 tinyint = NULL --// ??????
	
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
	, @inyDragonJewelType3 tinyint = NULL --// ??????
	
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
	, @inyDragonJewelType4 tinyint = NULL --// ??????
	
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
	, @inyDragonJewelType5 tinyint = NULL --// ??????
	
	, @dtsSendDate smalldatetime = NULL --// (optional) ?? ?? ?? ??. ?, @inyReceiverTarget ?? 3 ?? 4? ???? ?? ??? ?????.
	, @dtsMailScheduleExpireDate smalldatetime = NULL --// (optional) ?? ?? ????. ?? ?? ???? ???? ??? ???? ????.

	, @bitGambleReward bit = NULL --//(optional) ????? ?? ?? 0=??, 1=????
	, @intMailLifespan int = 43200 --// ?? ???? (?, ??? 30?)
	, @inyMailTabCode tinyint = NULL --// ?? ? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @dt2Now datetime2(0), @inbCharacterID bigint = 0, @i int = 1, @intJobCodeCount int, @intLogTableSuffix int, @intCharacterCount int, @intMailScheduleID int, @intDoc int;
DECLARE @tblCharacterIDs table (CharacterID bigint NOT NULL PRIMARY KEY);
DECLARE @tblCharacterNames table (CharacterName nvarchar(30) NOT NULL);
DECLARE @tblTemp table (MailID int NOT NULL, CharacterID bigint NOT NULL PRIMARY KEY);
DECLARE @tblItemSerials table (Seq int IDENTITY(1, 1) NOT NULL PRIMARY KEY, MailID int NOT NULL, ItemSerial bigint NOT NULL, ItemID int NOT NULL, ItemCount smallint NOT NULL, FirstAttachItemFlag bit NOT NULL);
DECLARE @tblAttachItems table (
	Seq tinyint IDENTITY(1, 1) NOT NULL PRIMARY KEY,
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
	SET @dtsSendDate = CASE WHEN @dtsSendDate IS NULL THEN GETDATE() ELSE @dtsSendDate END;

	INSERT @tblAttachItems (ItemID, ItemCount, ItemDurability, RandomSeed, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, ItemLifespan, EternityFlag, DragonJewelType)
	SELECT @intItemID1, @insItemCount1, @insItemDurability1, @intRandomSeed1, @inyItemLevel1, @inyItemPotential1, @bitSoulBoundFlag1, @inySealCount1, @inyItemOption1, @intItemLifespan1, @bitEternityFlag1, @inyDragonJewelType1 WHERE @intItemID1 > 0 UNION ALL
	SELECT @intItemID2, @insItemCount2, @insItemDurability2, @intRandomSeed2, @inyItemLevel2, @inyItemPotential2, @bitSoulBoundFlag2, @inySealCount2, @inyItemOption2, @intItemLifespan2, @bitEternityFlag2, @inyDragonJewelType2 WHERE @intItemID2 > 0 UNION ALL
	SELECT @intItemID3, @insItemCount3, @insItemDurability3, @intRandomSeed3, @inyItemLevel3, @inyItemPotential3, @bitSoulBoundFlag3, @inySealCount3, @inyItemOption3, @intItemLifespan3, @bitEternityFlag3, @inyDragonJewelType3 WHERE @intItemID3 > 0 UNION ALL
	SELECT @intItemID4, @insItemCount4, @insItemDurability4, @intRandomSeed4, @inyItemLevel4, @inyItemPotential4, @bitSoulBoundFlag4, @inySealCount4, @inyItemOption4, @intItemLifespan4, @bitEternityFlag4, @inyDragonJewelType4 WHERE @intItemID4 > 0 UNION ALL
	SELECT @intItemID5, @insItemCount5, @insItemDurability5, @intRandomSeed5, @inyItemLevel5, @inyItemPotential5, @bitSoulBoundFlag5, @inySealCount5, @inyItemOption5, @intItemLifespan5, @bitEternityFlag5, @inyDragonJewelType5 WHERE @intItemID5 > 0;

	/**_# P_StoreTargetCharacter? ??? ?? ???? ?? ??? ??*/
	IF @inyReceiverTarget = 1
	BEGIN
		IF OBJECT_ID(N'TargetCharacters', N'U') IS NULL OR OBJECT_ID(N'TargetCharacterDetails', N'U') IS NULL
		BEGIN
			SET @intReturnValue = 103214;
			GOTO ErrorHandler;
		END		
	END

	/**_# ?? ???*/
	ELSE IF @inyReceiverTarget = 2
	BEGIN
		IF @xmlCharacterIDs IS NOT NULL
		BEGIN
			EXEC sys.sp_xml_preparedocument @intDoc OUTPUT, @xmlCharacterIDs;

			WITH C AS (
				SELECT CharacterID
				FROM OPENXML (@intDoc, 'root/CH')
				WITH (CharacterID nvarchar(30) '@CharacterID')
			)
			INSERT @tblCharacterIDs (C.CharacterID)
			SELECT CH.CharacterID
			FROM C
				INNER JOIN dbo.Characters CH WITH (NOLOCK) ON CAST(C.CharacterID AS bigint) = CH.CharacterID
			WHERE C.CharacterID > 0 AND CH.AccountLevelCode <> 110
			ORDER BY CH.CharacterID;

			EXEC sys.sp_xml_removedocument @intDoc;
		END
		ELSE IF @xmlCharacterNames IS NOT NULL
		BEGIN
			EXEC sys.sp_xml_preparedocument @intDoc OUTPUT, @xmlCharacterNames;

			WITH C AS (
				SELECT CharacterName
				FROM OPENXML (@intDoc, 'root/CH')
				WITH (CharacterName nvarchar(30) '@CharacterName')
			)
			INSERT @tblCharacterIDs (CH.CharacterID)
			SELECT CH.CharacterID
			FROM C
				INNER JOIN dbo.Characters CH WITH (NOLOCK) ON C.CharacterName = CH.CharacterName
			WHERE CH.DeleteFlag = CAST(0 AS bit) AND CH.AccountLevelCode <> 110
			ORDER BY CH.CharacterID;
		
			EXEC sys.sp_xml_removedocument @intDoc;
		END

		WHILE 1 = 1
		BEGIN
			SET @dt2Now = GETDATE();

			DELETE @tblTemp;
			DELETE @tblItemSerials;

			INSERT dbo.Mails (SenderCharacterID, ReceiverCharacterID, MailTypeCode, DeliveryTypeCode, [Subject], Content, SendDate, ReadFlag, DeleteFlag, AttachCoin, AttachCoinReceiveFlag, SenderName, SystemMailFKey, MailExpireDate, MailTabCode)
			OUTPUT inserted.MailID, inserted.ReceiverCharacterID INTO @tblTemp (MailID, CharacterID)
			SELECT TOP (50) NULL, CH.CharacterID, CASE @bitGambleReward WHEN 1 THEN 20 ELSE 3 END , 2, @nvcSubject, @nvcContent, @dt2Now, 0, 0, @inbAttachCoin, 0, @nvcSenderName, @intAuditLogID, DATEADD(MINUTE, @intMailLifespan, @dt2Now), @inyMailTabCode
			FROM @tblCharacterIDs CH
			WHERE CH.CharacterID > @inbCharacterID
			ORDER BY CH.CharacterID;

			IF @@ROWCOUNT = 0
				BREAK;

			INSERT dbo.MaterializedItems (ItemSerial, ItemID, ItemRemainCount, SenderCharacterID, ItemMaterializeCode, ItemMaterializeFKey, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, ItemMaterializeDate, ItemLifespan, EternityFlag, ItemExpireDate, OwnerCharacterID, OwnershipStartDate, ItemLocationCode, ItemLocationIndex, DragonJewelType)
			OUTPUT inserted.ItemMaterializeFKey, inserted.ItemSerial, inserted.ItemID, inserted.ItemRemainCount, 0 INTO @tblItemSerials (MailID, ItemSerial, ItemID, ItemCount, FirstAttachItemFlag)
			SELECT ABS(CAST(CAST(NEWID() AS binary(8)) AS bigint)), T2.ItemID, T2.ItemCount, NULL, 10, T1.MailID, T2.ItemDurability, T2.RandomSeed, 0, T2.ItemLevel, T2.ItemPotential, T2.SoulBoundFlag, T2.SealCount, T2.ItemOption, @dt2Now, T2.ItemLifespan, T2.EternityFlag, NULL, NULL, NULL, 15, 0, T2.DragonJewelType
			FROM @tblTemp T1
				CROSS JOIN @tblAttachItems T2;

			WITH C1 AS (
				SELECT ROW_NUMBER() OVER(PARTITION BY MailID ORDER BY Seq) AS RowNumber, ItemSerial
				FROM @tblItemSerials
			)
			UPDATE @tblItemSerials
			SET FirstAttachItemFlag = 1
			FROM @tblItemSerials T
				INNER JOIN C1 ON T.ItemSerial = C1.ItemSerial
			WHERE C1.RowNumber = 1;

			/**_# [MailAttachItems] ???? INSERT???.*/
			INSERT dbo.MailAttachItems (MailID, ItemSerial, ItemID, FirstAttachItemFlag, ReceiveFlag, AttachItemCount)
			SELECT MailID, ItemSerial, ItemID, FirstAttachItemFlag, 0, ItemCount
			FROM @tblItemSerials;

			SELECT TOP (1) @inbCharacterID = CharacterID FROM @tblTemp ORDER BY CharacterID DESC;

			SET @intLogTableSuffix = dbo.FN_LogTableSuffix(@dt2Now);

			/**_# [ItemLogs] ???? INSERT???.*/
			IF @intLogTableSuffix = 0
				INSERT dbo.ItemLogs0 (LogDate, ItemSerial, ItemID, ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, IP)
				SELECT @dt2Now, ItemSerial, ItemID, 10, NULL, NULL, ItemCount, 0, MailID, NULL, NULL, NULL
				FROM @tblItemSerials A
				WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK) WHERE ItemID = A.ItemID AND (ItemLogCode IS NULL OR ItemLogCode = 10));

			ELSE IF @intLogTableSuffix = 1
				INSERT dbo.ItemLogs1 (LogDate, ItemSerial, ItemID, ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, IP)
				SELECT @dt2Now, ItemSerial, ItemID, 10, NULL, NULL, ItemCount, 0, MailID, NULL, NULL, NULL
				FROM @tblItemSerials A
				WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK) WHERE ItemID = A.ItemID AND (ItemLogCode IS NULL OR ItemLogCode = 10));

			ELSE
				INSERT dbo.ItemLogs2 (LogDate, ItemSerial, ItemID, ItemLogCode, OwnerCharacterID, OwnerCharacterIDBefore, ItemCount, ItemCountBefore, ItemLogFKey1, ItemLogFKey2, OwnershipStartDate, IP)
				SELECT @dt2Now, ItemSerial, ItemID, 10, NULL, NULL, ItemCount, 0, MailID, NULL, NULL, NULL
				FROM @tblItemSerials A
				WHERE NOT EXISTS (SELECT * FROM dbo.SkipLoggingItems WITH (NOLOCK) WHERE ItemID = A.ItemID AND (ItemLogCode IS NULL OR ItemLogCode = 10));
		END
	END

	ELSE IF @inyReceiverTarget = 3
	BEGIN
		INSERT dbo.MailSchedules (SendDate, SenderName, [Subject], Content, AttachCoin, MailScheduleExpireDate, MailTypeCode, DeleteFlag, MailLifespan, MailTabCode)
		VALUES (@dtsSendDate, @nvcSenderName, @nvcSubject, @nvcContent, @inbAttachCoin, @dtsMailScheduleExpireDate, 0, 0, @intMailLifespan, @inyMailTabCode);

		SET @intMailScheduleID = SCOPE_IDENTITY();

		INSERT dbo.AttachItemsForMailSchedule (MailScheduleID, ItemID, ItemCount, ItemDurability, RandomSeed, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, ItemLifespan, EternityFlag, DragonJewelType)
		SELECT @intMailScheduleID, @intItemID1, @insItemCount1, @insItemDurability1, @intRandomSeed1, @inyItemLevel1, @inyItemPotential1, @bitSoulBoundFlag1, @inySealCount1, @inyItemOption1, @intItemLifespan1, @bitEternityFlag1, @inyDragonJewelType1 WHERE @intItemID1 > 0 UNION ALL
		SELECT @intMailScheduleID, @intItemID2, @insItemCount2, @insItemDurability2, @intRandomSeed2, @inyItemLevel2, @inyItemPotential2, @bitSoulBoundFlag2, @inySealCount2, @inyItemOption2, @intItemLifespan2, @bitEternityFlag2, @inyDragonJewelType2 WHERE @intItemID2 > 0 UNION ALL
		SELECT @intMailScheduleID, @intItemID3, @insItemCount3, @insItemDurability3, @intRandomSeed3, @inyItemLevel3, @inyItemPotential3, @bitSoulBoundFlag3, @inySealCount3, @inyItemOption3, @intItemLifespan3, @bitEternityFlag3, @inyDragonJewelType3 WHERE @intItemID3 > 0 UNION ALL
		SELECT @intMailScheduleID, @intItemID4, @insItemCount4, @insItemDurability4, @intRandomSeed4, @inyItemLevel4, @inyItemPotential4, @bitSoulBoundFlag4, @inySealCount4, @inyItemOption4, @intItemLifespan4, @bitEternityFlag4, @inyDragonJewelType4 WHERE @intItemID4 > 0 UNION ALL
		SELECT @intMailScheduleID, @intItemID5, @insItemCount5, @insItemDurability5, @intRandomSeed5, @inyItemLevel5, @inyItemPotential5, @bitSoulBoundFlag5, @inySealCount5, @inyItemOption5, @intItemLifespan5, @bitEternityFlag5, @inyDragonJewelType5 WHERE @intItemID5 > 0;
	END
END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 547 AND ERROR_MESSAGE() LIKE N'%FK_Characters_Mails_ReceiverCharacterID%'
		SET @intReturnValue = 103102;

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

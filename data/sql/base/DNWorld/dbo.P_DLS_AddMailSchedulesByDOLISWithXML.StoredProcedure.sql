USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DLS_AddMailSchedulesByDOLISWithXML]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-05-21
description : XML ????? ?? ??? ?????. (???)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
2 = ????? ??? ???? ?? ?? by ??? at 2013-07-23

**/

CREATE PROCEDURE [dbo].[P_DLS_AddMailSchedulesByDOLISWithXML]
	  @inyMailTypeCode tinyint --// ???? (0=??, 1=??? ??, 2=??? ??, 3=?? ??)
	, @xmlCharacterIDs xml = null --// ?? ?? ???ID XML ??? (<root><CH CharacterID="123" /><CH CharacterID="456" /></root>) (@inyMailTypeCode=1 ? ???? ??)
	, @xmlCharacterNames xml = null --// ?? ?? ???? XML ??? (<root><CH CharacterName="aaa" /><CH CharacterName="bbb" /></root>) (@inyMailTypeCode=1 ? ???? ??)
	, @inyCharacterClass tinyint = null --// ?? ?? ??? ??? (@inyMailTypeCode=2 ? ???? ??)
	, @intGuildID int = null --// ??ID (@inyMailTypeCode=3 ? ???? ??)
	, @dtsSendDate smalldatetime = null --// ?? ?? ?? ??
	, @nvcSenderName nvarchar(50) --// ?? ??? ??
	, @nvcSubject nvarchar(50) --// ??
	, @nvcContent nvarchar(300) --// ??
	, @inbAttachCoin bigint --// ?? ?? ??
	, @dtsMailScheduleExpireDate smalldatetime = null --// (optional) ?? ?? ????. ?? ?? ???? ???? ??? ???? ????.

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

	, @bitNewFlag bit = NULL --// 0=?? ??? ?? ?, 1=?? ??? ?? ?

	, @inyCharacterLevelMin tinyint = NULL --// ?? ?? (@inyMailTypeCode=2 ? ???? ??)
	, @inyCharacterLevelMax tinyint = NULL --// ?? ?? (@inyMailTypeCode=2 ? ???? ??)

	, @intMailLifespan int = 43200 --// ?????? (??? 30?)
	, @inyMailTabCode tinyint = NULL --// ?? ? ?? 
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	  , @dt2Now datetime2(0) = GETDATE()
	  , @intMailScheduleID int
	  , @intDoc int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	IF @dtsSendDate IS NULL SET @dtsSendDate = GETDATE();

	-- ?? ?? ??
	INSERT dbo.MailSchedules (SendDate, SenderName, [Subject], Content, AttachCoin, MailScheduleExpireDate, MailTypeCode, NewFlag, DeleteFlag, MailLifespan, MailTabCode)
	VALUES (@dtsSendDate, @nvcSenderName, @nvcSubject, @nvcContent, @inbAttachCoin, @dtsMailScheduleExpireDate, @inyMailTypeCode, @bitNewFlag, 0, @intMailLifespan, @inyMailTabCode);

	SET @intMailScheduleID = SCOPE_IDENTITY();

	INSERT dbo.AttachItemsForMailSchedule (MailScheduleID, ItemID, ItemCount, ItemDurability, RandomSeed, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, ItemLifespan, EternityFlag, DragonJewelType)
	SELECT @intMailScheduleID, @intItemID1, @insItemCount1, @insItemDurability1, @intRandomSeed1, @inyItemLevel1, @inyItemPotential1, @bitSoulBoundFlag1, @inySealCount1, @inyItemOption1, @intItemLifespan1, @bitEternityFlag1, @inyDragonJewelType1 WHERE @intItemID1 > 0 UNION ALL
	SELECT @intMailScheduleID, @intItemID2, @insItemCount2, @insItemDurability2, @intRandomSeed2, @inyItemLevel2, @inyItemPotential2, @bitSoulBoundFlag2, @inySealCount2, @inyItemOption2, @intItemLifespan2, @bitEternityFlag2, @inyDragonJewelType2 WHERE @intItemID2 > 0 UNION ALL
	SELECT @intMailScheduleID, @intItemID3, @insItemCount3, @insItemDurability3, @intRandomSeed3, @inyItemLevel3, @inyItemPotential3, @bitSoulBoundFlag3, @inySealCount3, @inyItemOption3, @intItemLifespan3, @bitEternityFlag3, @inyDragonJewelType3 WHERE @intItemID3 > 0 UNION ALL
	SELECT @intMailScheduleID, @intItemID4, @insItemCount4, @insItemDurability4, @intRandomSeed4, @inyItemLevel4, @inyItemPotential4, @bitSoulBoundFlag4, @inySealCount4, @inyItemOption4, @intItemLifespan4, @bitEternityFlag4, @inyDragonJewelType4 WHERE @intItemID4 > 0 UNION ALL
	SELECT @intMailScheduleID, @intItemID5, @insItemCount5, @insItemDurability5, @intRandomSeed5, @inyItemLevel5, @inyItemPotential5, @bitSoulBoundFlag5, @inySealCount5, @inyItemOption5, @intItemLifespan5, @bitEternityFlag5, @inyDragonJewelType5 WHERE @intItemID5 > 0;

	-- ?? ?? ?? ?? : ??? ??
	IF @inyMailTypeCode = 1 AND @intMailScheduleID IS NOT NULL
	BEGIN
		IF @xmlCharacterIDs IS NOT NULL
		BEGIN
			-- ???ID ??
			EXEC sys.sp_xml_preparedocument @intDoc OUTPUT, @xmlCharacterIDs;

			WITH C AS (
				SELECT CharacterID
				FROM OPENXML (@intDoc, 'root/CH')
				WITH (CharacterID nvarchar(30) '@CharacterID')
			)			
			INSERT dbo.MailSchedulesTarget (MailScheduleID, CharacterID, DeleteFlag)
			SELECT DISTINCT @intMailScheduleID, CharacterID, 0
			FROM C
			OPTION (KEEPFIXED PLAN);

			EXEC sys.sp_xml_removedocument @intDoc;

			IF @@ROWCOUNT = 0
			BEGIN
				SET @intReturnValue = 101172;
				GOTO ErrorHandler;
			END			
		END
		ELSE IF @xmlCharacterIDs IS NULL AND @xmlCharacterNames IS NOT NULL
		BEGIN
			-- ???? ??
			EXEC sys.sp_xml_preparedocument @intDoc OUTPUT, @xmlCharacterNames;

			WITH C AS (
				SELECT CharacterName
				FROM OPENXML (@intDoc, 'root/CH')
				WITH (CharacterName nvarchar(30) '@CharacterName')
			)
			INSERT dbo.MailSchedulesTarget (MailScheduleID, CharacterID, DeleteFlag)
			SELECT DISTINCT @intMailScheduleID, CH.CharacterID, 0
			FROM C
				INNER JOIN dbo.Characters CH WITH (NOLOCK, INDEX(IX_UNF_Characters_CharacterName), FORCESEEK) ON (C.CharacterName = CH.CharacterName)
			WHERE CH.DeleteFlag = 0
			OPTION (FORCE ORDER, LOOP JOIN, KEEPFIXED PLAN);

			EXEC sys.sp_xml_removedocument @intDoc;

			IF @@ROWCOUNT = 0
			BEGIN
				SET @intReturnValue = 101172;
				GOTO ErrorHandler;
			END			
		END
		ELSE IF @xmlCharacterIDs IS NULL AND @xmlCharacterNames IS NULL
		BEGIN
			SET @intReturnValue = 101172;
			GOTO ErrorHandler;
		END
	END
	-- ?? ?? ?? ?? : ??? ??
	ELSE IF @inyMailTypeCode = 2 AND @intMailScheduleID IS NOT NULL
	BEGIN
		INSERT dbo.MailSchedulesTarget (MailScheduleID, CharacterID, DeleteFlag)
		SELECT @intMailScheduleID, C.CharacterID, 0
		  FROM dbo.Characters C WITH (NOLOCK)
		       INNER JOIN dbo.CharacterStatus CS WITH (NOLOCK) ON (C.CharacterID = CS.CharacterID)
		 WHERE (C.DeleteFlag = 0)
		   AND (C.CharacterClassCode = @inyCharacterClass)
		   AND (@inyCharacterLevelMin IS NULL OR CS.CharacterLevel >= @inyCharacterLevelMin)
		   AND (@inyCharacterLevelMax IS NULL OR CS.CharacterLevel <= @inyCharacterLevelMax);

		IF @@ROWCOUNT = 0
		BEGIN
			SET @intReturnValue = 101172;
			GOTO ErrorHandler;
		END
	END
	-- ?? ?? ?? ?? : ?? ??
	ELSE IF @inyMailTypeCode = 3 AND @intMailScheduleID IS NOT NULL
	BEGIN
		INSERT dbo.MailSchedulesTarget (MailScheduleID, CharacterID, DeleteFlag)
		SELECT @intMailScheduleID, CharacterID, 0
		  FROM dbo.GuildMembers WITH (NOLOCK)
		 WHERE GuildID = @intGuildID;

		IF @@ROWCOUNT = 0
		BEGIN
			SET @intReturnValue = 101172;
			GOTO ErrorHandler;
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
	EXEC @intReturnValue = dbo.P_AddErrorLog @intPROCID = @@PROCID;

RETURN @intReturnValue;
GO

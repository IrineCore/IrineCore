USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_SendGuildWarRewardMail]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 8
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2011-05-06

description :
	??? ?? ?? ??? ?????.
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
	101172 = ?? ??? ??? ???? ????.
history :
	3 = ?? ??
	5 = input parameter (@insGuildWarScheduleID) ??, ?? ??? ?? ?? ??     by 20110919.wgkim
	6 = DeleteFlag ??     by 20110929.wgkim
	7 = MailTabCode  ?? by 20130403.dailykim
**/

CREATE PROCEDURE [dbo].[P_SendGuildWarRewardMail]
      @intGuildID int --// ?? ?? ?? ??ID
	, @insGuildWarScheduleID smallint --// ??? ??

	, @dtsSendDate smalldatetime = NULL --// ?? ?? ?? ??
	, @nvcSenderName nvarchar(50) --// ?? ??? ??
	, @nvcSubject nvarchar(50) --// ??
	, @nvcContent nvarchar(300) --// ??
	, @inbAttachCoin bigint --// ?? ?? ??
	, @dtsMailScheduleExpireDate smalldatetime = NULL --// (optional) ?? ?? ????. ?? ?? ???? ???? ??? ???? ????.

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

	, @bitNewFlag bit = NULL --// 0=?? ??? ?? ?, 1=?? ??? ?? ?
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
      , @dtBasisDate smalldatetime
	  , @inyMailTypeCode tinyint --// ???? (0=??, 1=?????)
	  , @dt2Now datetime2(0) = GETDATE()
	  , @intMailScheduleID int;

SET @inyMailTypeCode = 1;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	IF @dtsSendDate IS NULL SET @dtsSendDate = GETDATE();

	INSERT dbo.MailSchedules (SendDate, SenderName, [Subject], Content, AttachCoin, MailScheduleExpireDate, MailTypeCode, NewFlag, DeleteFlag, MailTabCode)
	VALUES (@dtsSendDate, @nvcSenderName, @nvcSubject, @nvcContent, @inbAttachCoin, @dtsMailScheduleExpireDate, @inyMailTypeCode, @bitNewFlag, 0, @inyMailTabCode);

	SET @intMailScheduleID = SCOPE_IDENTITY();

	INSERT dbo.AttachItemsForMailSchedule (MailScheduleID, ItemID, ItemCount, ItemDurability, RandomSeed, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, ItemLifespan, EternityFlag, DragonJewelType)
	SELECT @intMailScheduleID, @intItemID1, @insItemCount1, @insItemDurability1, @intRandomSeed1, @inyItemLevel1, @inyItemPotential1, @bitSoulBoundFlag1, @inySealCount1, @inyItemOption1, @intItemLifespan1, @bitEternityFlag1, @inyDragonJewelType1 WHERE @intItemID1 > 0 UNION ALL
	SELECT @intMailScheduleID, @intItemID2, @insItemCount2, @insItemDurability2, @intRandomSeed2, @inyItemLevel2, @inyItemPotential2, @bitSoulBoundFlag2, @inySealCount2, @inyItemOption2, @intItemLifespan2, @bitEternityFlag2, @inyDragonJewelType2 WHERE @intItemID2 > 0 UNION ALL
	SELECT @intMailScheduleID, @intItemID3, @insItemCount3, @insItemDurability3, @intRandomSeed3, @inyItemLevel3, @inyItemPotential3, @bitSoulBoundFlag3, @inySealCount3, @inyItemOption3, @intItemLifespan3, @bitEternityFlag3, @inyDragonJewelType3 WHERE @intItemID3 > 0 UNION ALL
	SELECT @intMailScheduleID, @intItemID4, @insItemCount4, @insItemDurability4, @intRandomSeed4, @inyItemLevel4, @inyItemPotential4, @bitSoulBoundFlag4, @inySealCount4, @inyItemOption4, @intItemLifespan4, @bitEternityFlag4, @inyDragonJewelType4 WHERE @intItemID4 > 0 UNION ALL
	SELECT @intMailScheduleID, @intItemID5, @insItemCount5, @insItemDurability5, @intRandomSeed5, @inyItemLevel5, @inyItemPotential5, @bitSoulBoundFlag5, @inySealCount5, @inyItemOption5, @intItemLifespan5, @bitEternityFlag5, @inyDragonJewelType5 WHERE @intItemID5 > 0;

	IF @inyMailTypeCode = 1 AND @intMailScheduleID IS NOT NULL
	BEGIN

		SELECT @dtBasisDate = RewardEndDate
		  FROM dbo.GuildWarSchedule WITH (NOLOCK)
		 WHERE GuildWarScheduleID = @insGuildWarScheduleID;

		INSERT dbo.MailSchedulesTarget (MailScheduleID, CharacterID)
		SELECT @intMailScheduleID, CharacterID
		  FROM dbo.GuildMembers WITH (NOLOCK)
		 WHERE GuildID = @intGuildID
		   AND RegisterDate < @dtBasisDate;

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
	EXEC @intReturnValue = dbo.P_AddErrorLog;

RETURN @intReturnValue;
GO

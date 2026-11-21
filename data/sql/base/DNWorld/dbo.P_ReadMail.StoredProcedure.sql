USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ReadMail]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 20
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-27

description :
	??? ????. (?? ??? Output Parameter?, ?? ???? ?????? ?????.)
result set :
	@intVersion < 16 : ReceiveFlag/bit, ItemSerial/bigint, ItemID/int, AttachItemCount/smallint, ItemDurability/smallint, RandomSeed/int, CoolTime/int, ItemLevel/tinyint, ItemPotential/tinyint, SoulBoundFlag/bit, SealCount/tinyint, ItemOption/tinyint, EternityFlag/bit, ItemExpireDate/smalldatetime
	@intVersion = 16 : ReceiveFlag/bit, ItemSerial/bigint, ItemID/int, AttachItemCount/smallint, ItemDurability/smallint, RandomSeed/int, CoolTime/int, ItemLevel/tinyint, ItemPotential/tinyint, SoulBoundFlag/bit, SealCount/tinyint, ItemOption/tinyint, EternityFlag/bit, ItemExpireDate/smalldatetime, ItemPotentialMoveCount/tinyint	
	@intVersion = 17 : Version 16 + DragonJewelType/tinyint, ItemLifespan/int
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
	103166 = ???? ?? ?????.
history :
	14 = 103166? ???? ???? ????? ????? ??
	15 = output param. @bitNewFlag ?? at 2010-11-25
	16 = ?? ?? ?? ?? (@intVersion = 16)     by ??? at 2012-04-12
	17 = MailTypeCode OUTPUT Parameter ?? by ??? at 2012-09-12
**/

CREATE PROCEDURE [dbo].[P_ReadMail]
	  @intMailID int --// ?? ID
	, @nvcContent nvarchar(300) OUTPUT --// ??
	, @dt2SendDate datetime2(0) OUTPUT --// ?? ??
	, @inbAttachCoin bigint OUTPUT --// ?? ??
	, @bitAttachCoinReceiveFlag bit OUTPUT --// 0=????????, 1=??????
	, @intNotReadMailCount int OUTPUT --// ?? ?? ?? ?
	, @inyDeliveryTypeCode tinyint OUTPUT --// 1=??, 2=????
	, @bitNewFlag bit = NULL OUTPUT --// ???? ??? ?? ??. 0=false, 1=true
	, @intVersion int = 15 --// ??? SP? ??
	, @inyMailTypeCode tinyint = NULL OUTPUT --// ?? ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @inbReceiverCharacterID bigint;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SET @intNotReadMailCount = 0;

	/**_# ??? ?? ??? ???? ??? ?????.*/
	UPDATE dbo.Mails
	SET @inbReceiverCharacterID = ReceiverCharacterID, @nvcContent = Content, @dt2SendDate = SendDate, @inbAttachCoin = AttachCoin, @bitAttachCoinReceiveFlag = AttachCoinReceiveFlag, @inyDeliveryTypeCode = DeliveryTypeCode, @bitNewFlag = ISNULL(NewFlag, 0)
		, ReadFlag = 1
		, @inyMailTypeCode = MailTypeCode
	WHERE MailID = @intMailID AND DeleteFlag = CAST(0 AS bit);

	/**_# ??? ???? ??? 103166? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		IF @intVersion < 16
		BEGIN
			SELECT MAI.ReceiveFlag, MAI.ItemSerial, MAI.ItemID, MAI.AttachItemCount, MI.ItemDurability, MI.RandomSeed, MI.CoolTime, MI.ItemLevel, MI.ItemPotential, MI.SoulBoundFlag, MI.SealCount, MI.ItemOption, MI.EternityFlag, ISNULL(MI.ItemExpireDate, DATEADD(YEAR, 3, ItemMaterializeDate)) AS ItemExpireDate
			FROM dbo.MailAttachItems MAI WITH (NOLOCK)
				INNER JOIN dbo.MaterializedItems MI WITH (NOLOCK) ON MAI.ItemSerial = MI.ItemSerial
			WHERE 1 = 0;
		END
		ELSE IF @intVersion = 16
		BEGIN
			SELECT MAI.ReceiveFlag, MAI.ItemSerial, MAI.ItemID, MAI.AttachItemCount, MI.ItemDurability, MI.RandomSeed, MI.CoolTime, MI.ItemLevel, MI.ItemPotential, MI.SoulBoundFlag, MI.SealCount, MI.ItemOption, MI.EternityFlag, ISNULL(MI.ItemExpireDate, DATEADD(YEAR, 3, ItemMaterializeDate)) AS ItemExpireDate, MI.ItemPotentialMoveCount
			FROM dbo.MailAttachItems MAI WITH (NOLOCK)
				INNER JOIN dbo.MaterializedItems MI WITH (NOLOCK) ON MAI.ItemSerial = MI.ItemSerial
			WHERE 1 = 0;
		END		
		ELSE IF @intVersion = 17
		BEGIN
			SELECT MAI.ReceiveFlag, MAI.ItemSerial, MAI.ItemID, MAI.AttachItemCount, MI.ItemDurability, MI.RandomSeed, MI.CoolTime, MI.ItemLevel, MI.ItemPotential, MI.SoulBoundFlag, MI.SealCount, MI.ItemOption, MI.EternityFlag, ISNULL(MI.ItemExpireDate, DATEADD(YEAR, 3, ItemMaterializeDate)) AS ItemExpireDate, MI.ItemPotentialMoveCount, MI.DragonJewelType, MI.ItemLifespan
			FROM dbo.MailAttachItems MAI WITH (NOLOCK)
				INNER JOIN dbo.MaterializedItems MI WITH (NOLOCK) ON MAI.ItemSerial = MI.ItemSerial
			WHERE 1 = 0;
		END	

		SET @intReturnValue = 103166;
		GOTO ErrorHandler;
	END

	/**_# ?? ?? ??? ?? ?????.*/
	SELECT @intNotReadMailCount = COUNT(*)
	FROM dbo.Mails WITH (NOLOCK)
	WHERE ReceiverCharacterID = @inbReceiverCharacterID AND SendDate >= CAST(CAST(DATEADD(DAY, -29, GETDATE()) AS date) AS datetime2(0)) AND DeleteFlag = CAST(0 AS bit) AND ReadFlag = CAST(0 AS bit);

	/**_# ?? ???? ?????.*/
	IF @intVersion < 16
	BEGIN
		SELECT MAI.ReceiveFlag, MAI.ItemSerial, MAI.ItemID, MAI.AttachItemCount, MI.ItemDurability, MI.RandomSeed, MI.CoolTime, MI.ItemLevel, MI.ItemPotential, MI.SoulBoundFlag, MI.SealCount, MI.ItemOption, MI.EternityFlag, ISNULL(MI.ItemExpireDate, DATEADD(YEAR, 3, ItemMaterializeDate)) AS ItemExpireDate
		FROM dbo.MailAttachItems MAI WITH (NOLOCK)
			INNER JOIN dbo.MaterializedItems MI WITH (NOLOCK) ON MAI.ItemSerial = MI.ItemSerial
		WHERE MAI.MailID = @intMailID
		ORDER BY MAI.MailAttachItemID;
	END
	ELSE IF @intVersion = 16
	BEGIN
		SELECT MAI.ReceiveFlag, MAI.ItemSerial, MAI.ItemID, MAI.AttachItemCount, MI.ItemDurability, MI.RandomSeed, MI.CoolTime, MI.ItemLevel, MI.ItemPotential, MI.SoulBoundFlag, MI.SealCount, MI.ItemOption, MI.EternityFlag, ISNULL(MI.ItemExpireDate, DATEADD(YEAR, 3, ItemMaterializeDate)) AS ItemExpireDate, MI.ItemPotentialMoveCount
		FROM dbo.MailAttachItems MAI WITH (NOLOCK)
			INNER JOIN dbo.MaterializedItems MI WITH (NOLOCK) ON MAI.ItemSerial = MI.ItemSerial
		WHERE MAI.MailID = @intMailID
		ORDER BY MAI.MailAttachItemID;
	END		
	ELSE IF @intVersion = 17
	BEGIN
		SELECT MAI.ReceiveFlag, MAI.ItemSerial, MAI.ItemID, MAI.AttachItemCount, MI.ItemDurability, MI.RandomSeed, MI.CoolTime, MI.ItemLevel, MI.ItemPotential, MI.SoulBoundFlag, MI.SealCount, MI.ItemOption, MI.EternityFlag, ISNULL(MI.ItemExpireDate, DATEADD(YEAR, 3, ItemMaterializeDate)) AS ItemExpireDate, MI.ItemPotentialMoveCount, MI.DragonJewelType, MI.ItemLifespan
		FROM dbo.MailAttachItems MAI WITH (NOLOCK)
			INNER JOIN dbo.MaterializedItems MI WITH (NOLOCK) ON MAI.ItemSerial = MI.ItemSerial
		WHERE MAI.MailID = @intMailID
		ORDER BY MAI.MailAttachItemID;
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

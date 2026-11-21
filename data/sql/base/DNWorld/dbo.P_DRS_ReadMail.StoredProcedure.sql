USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_ReadMail]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 6
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-12-14
description : ?? ??? ?????.
result set :
	ReceiveFlag/bit, ItemSerial/bigint, ItemID/int, AttachItemCount/smallint, ItemDurability/smallint, RandomSeed/int, CoolTime/int, ItemLevel/tinyint, ItemPotential/tinyint, SoulBoundFlag/bit, SealCount/tinyint, ItemOption/tinyint, EternityFlag/bit, ItemExpireDate/smalldatetime, ItemPotentialMoveCount/tinyint	, AdditiveItemID/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
1030000018 = ??? ???? ????.
**/

CREATE PROCEDURE [dbo].[P_DRS_ReadMail]
	  @intMailID int --// ?? ID
	, @nvcContent nvarchar(300) OUTPUT --// ??
	, @dt2SendDate datetime2(0) OUTPUT --// ?? ??
	, @inbAttachCoin bigint OUTPUT --// ?? ??
	, @bitAttachCoinReceiveFlag bit OUTPUT --// 0=????????, 1=??????
	, @intNotReadMailCount int OUTPUT --// ?? ?? ?? ?
	, @inyDeliveryTypeCode tinyint OUTPUT --// 1=??, 2=????
	, @bitNewFlag bit = NULL OUTPUT --// ???? ??? ?? ??. 0=false, 1=true		
	, @inyMailTypeCode tinyint OUTPUT --// 1=?? ??, 2=?? ??, 3=??? ??, 4=??? ??? ??, 5=VIP????, 6=?? ??, 7=NPC ??? ??, 8=?????, 9=?????, 10=????????, 11=VIP ??, 12=PVP ?????, 13=???????, 14=???????, 15=???????, 16=???, 17=????????, 18=???, 19=??????
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

	SELECT @inbReceiverCharacterID = ReceiverCharacterID, @nvcContent = Content, @dt2SendDate = SendDate, @inbAttachCoin = AttachCoin, @bitAttachCoinReceiveFlag = AttachCoinReceiveFlag, @inyDeliveryTypeCode = DeliveryTypeCode, @bitNewFlag = ISNULL(NewFlag, 0), @inyMailTypeCode = MailTypeCode
	FROM dbo.Mails
	WHERE MailID = @intMailID AND DeleteFlag = CAST(0 AS bit);

	/**_# ??? ???? ??? 103166? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN		
		SELECT MAI.ReceiveFlag, MAI.ItemSerial, MAI.ItemID, MAI.AttachItemCount, MI.ItemDurability, MI.RandomSeed, MI.CoolTime, MI.ItemLevel, MI.ItemPotential, MI.SoulBoundFlag, MI.SealCount, MI.ItemOption, MI.EternityFlag, ISNULL(MI.ItemExpireDate, DATEADD(YEAR, 3, ItemMaterializeDate)) AS ItemExpireDate, MI.ItemPotentialMoveCount, MI.AdditiveItemID
		FROM dbo.MailAttachItems MAI WITH (NOLOCK)
			INNER JOIN dbo.MaterializedItems MI WITH (NOLOCK) ON MAI.ItemSerial = MI.ItemSerial
		WHERE 1 = 0;		

		SET @intReturnValue = 1030000018;
		GOTO ErrorHandler;
	END

	/**_# ?? ?? ??? ?? ?????.*/
	SELECT @intNotReadMailCount = COUNT(*)
	FROM dbo.Mails WITH (NOLOCK)
	WHERE ReceiverCharacterID = @inbReceiverCharacterID AND SendDate >= CAST(CAST(DATEADD(DAY, -29, GETDATE()) AS date) AS datetime2(0)) AND DeleteFlag = CAST(0 AS bit) AND ReadFlag = CAST(0 AS bit);

	/**_# ?? ???? ?????.*/	
	SELECT MAI.ReceiveFlag, MAI.ItemSerial, MAI.ItemID, MAI.AttachItemCount, MI.ItemDurability, MI.RandomSeed, MI.CoolTime, MI.ItemLevel, MI.ItemPotential, MI.SoulBoundFlag, MI.SealCount, MI.ItemOption, MI.EternityFlag, ISNULL(MI.ItemExpireDate, DATEADD(YEAR, 3, ItemMaterializeDate)) AS ItemExpireDate, MI.ItemPotentialMoveCount, MI.AdditiveItemID
	FROM dbo.MailAttachItems MAI WITH (NOLOCK)
		INNER JOIN dbo.MaterializedItems MI WITH (NOLOCK) ON MAI.ItemSerial = MI.ItemSerial
	WHERE MAI.MailID = @intMailID
	ORDER BY MAI.MailAttachItemID;
	
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

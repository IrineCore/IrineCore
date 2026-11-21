USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListMailAttachmentByMail]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 6
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-02-04

description :
	?? ??? Output Parameter?, ?? ???? ?????? ?????.
result set :
	@intVersion < 4 : ItemSerial/bigint, ItemID/int, AttachItemCount/smallint, ItemDurability/smallint, RandomSeed/int, CoolTime/int, ItemLevel/tinyint, ItemPotential/tinyint, SoulBoundFlag/bit, SealCount/tinyint, ItemOption/tinyint, EternityFlag/bit, ItemExpireDate/smalldatetime
	@intVersion = 4 : ItemSerial/bigint, ItemID/int, AttachItemCount/smallint, ItemDurability/smallint, RandomSeed/int, CoolTime/int, ItemLevel/tinyint, ItemPotential/tinyint, SoulBoundFlag/bit, SealCount/tinyint, ItemOption/tinyint, EternityFlag/bit, ItemExpireDate/smalldatetime, ItemPotentialMoveCount/tinyint
	@intVersion = 5 : Version 5 + DragonJewelType/tinyint
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
	103166 = ???? ?? ?????.
history :
	4~5 = ?? ?? ?? ?? (@intVersion = 4)     by ??? at 2012-04-12
**/

CREATE PROCEDURE [dbo].[P_GetListMailAttachmentByMail]
	  @intMailID int --// ?? ID
	, @inbAttachCoin bigint OUTPUT --// ?? ??
	, @intVersion int = 3 --// ??? SP? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# ?? ?? ?? ??? ?????.*/
	SELECT @inbAttachCoin = CASE AttachCoinReceiveFlag WHEN 0 THEN AttachCoin ELSE 0 END
	FROM dbo.Mails WITH (NOLOCK)
	WHERE MailID = @intMailID;

	/**_# ?? ?? ?? ???? ?????.*/
	IF @intVersion < 4
	BEGIN
		SELECT MAI.ItemSerial, MAI.ItemID, MAI.AttachItemCount, MI.ItemDurability, MI.RandomSeed, MI.CoolTime, MI.ItemLevel, MI.ItemPotential, MI.SoulBoundFlag, MI.SealCount, MI.ItemOption, MI.EternityFlag, MI.ItemExpireDate
		FROM dbo.MailAttachItems MAI WITH (NOLOCK)
			INNER JOIN dbo.MaterializedItems MI WITH (NOLOCK) ON MAI.ItemSerial = MI.ItemSerial
		WHERE MAI.MailID = @intMailID AND MAI.ReceiveFlag = CAST(0 AS bit)
		ORDER BY MAI.MailAttachItemID;
	END
	ELSE IF @intVersion = 4
	BEGIN
		SELECT MAI.ItemSerial, MAI.ItemID, MAI.AttachItemCount, MI.ItemDurability, MI.RandomSeed, MI.CoolTime, MI.ItemLevel, MI.ItemPotential, MI.SoulBoundFlag, MI.SealCount, MI.ItemOption, MI.EternityFlag, MI.ItemExpireDate, MI.ItemPotentialMoveCount
		FROM dbo.MailAttachItems MAI WITH (NOLOCK)
			INNER JOIN dbo.MaterializedItems MI WITH (NOLOCK) ON MAI.ItemSerial = MI.ItemSerial
		WHERE MAI.MailID = @intMailID AND MAI.ReceiveFlag = CAST(0 AS bit)
		ORDER BY MAI.MailAttachItemID;
	END
	ELSE IF @intVersion = 5
	BEGIN
		SELECT MAI.ItemSerial, MAI.ItemID, MAI.AttachItemCount, MI.ItemDurability, MI.RandomSeed, MI.CoolTime, MI.ItemLevel, MI.ItemPotential, MI.SoulBoundFlag, MI.SealCount, MI.ItemOption, MI.EternityFlag, MI.ItemExpireDate, MI.ItemPotentialMoveCount, MI.DragonJewelType
		FROM dbo.MailAttachItems MAI WITH (NOLOCK)
			INNER JOIN dbo.MaterializedItems MI WITH (NOLOCK) ON MAI.ItemSerial = MI.ItemSerial
		WHERE MAI.MailID = @intMailID AND MAI.ReceiveFlag = CAST(0 AS bit)
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

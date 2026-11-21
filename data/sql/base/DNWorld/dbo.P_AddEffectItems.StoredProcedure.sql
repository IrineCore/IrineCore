USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddEffectItems]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 10
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-02-11
description : ?? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103102 = ???? ???? ????.
history :
5 = input param. @intToBeDeletedItemID ?? at 2011-03-04
6 = 2nd input param.? data type? bit => tinyint? ?? at 2011-10-10
7 = ?? ?? ??? ?? by ??? at 2011-12-23
8 = ???? ?? ??? ?? (@tblExpiredItems) ?? by ??? at 2013-02-01
9 = @inyEffectItemGetCode ?? ?? (8=????, 9=??? ??) by ??? at 2013-02-13
**/

CREATE PROCEDURE [dbo].[P_AddEffectItems]
	  @inbCharacterID bigint --// ??? ID
	, @inyItemLifespanRenewal tinyint--// 0=??? ???? ?? ?? ItemID? ?????. 1=?? ItemID? ?? ?? ??? ?????. 2=?? ItemID? ?? ?? ??? ?????.
	, @intItemID int --// ??? ID
	, @inyEffectItemGetCode tinyint --// 1=??? ??, 2=??? ??, 3=??? ??, 4=??? ??, 5=NPC ??, 6=????? ??, 7=????(??,???), 8=????, 9=??? ??
	, @inbEffectItemGetKey bigint --// EffectItemGetCode ? ?? ? ?. 1:PurchaseID, 2:PurchaseID, 3:NULL, 4:AuditLogID, 5:NULL, 6:NULL, 7:GuildID
	, @intPrice int --// ??
	, @bitEternityFlag bit --// 0=??? ???, 1=?? ???
	, @intItemLifespan int --// ??? ???? ??, ??? ?? (?)
	, @intProperty1 int --// ?? 1
	, @intProperty2 int --// ?? 2
	, @intProperty3 int --// ?? 3
	, @intProperty4 int --// ?? 4
	, @intProperty5 int --// ?? 5
	, @inbItemSerial bigint --// ??? ???
	, @intToBeDeletedItemID int = NULL --// ???? ?? ???? ?? ??????? ???? Item ID (???? ItemID?? ??????.)
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @dt2Now datetime2(0) = GETDATE();

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	IF @inyItemLifespanRenewal = 0
		INSERT dbo.EffectItems (CharacterID, ItemID, EffectItemGetCode, EffectItemGetKey, Price, EternityFlag, EffectItemExpireDate, Property1, Property2, Property3, Property4, Property5, RegisterDate, ItemSerial)
		VALUES (@inbCharacterID, @intItemID, @inyEffectItemGetCode, @inbEffectItemGetKey, @intPrice, @bitEternityFlag, CASE @bitEternityFlag WHEN 1 THEN DATEADD(YEAR, 1, @dt2Now) ELSE DATEADD(MINUTE, @intItemLifespan, @dt2Now) END, @intProperty1, @intProperty2, @intProperty3, @intProperty4, @intProperty5, @dt2Now, @inbItemSerial);

	ELSE IF @inyItemLifespanRenewal = 1 AND @intItemLifespan > 0
	BEGIN
		UPDATE dbo.EffectItems
		SET EffectItemExpireDate = DATEADD(MINUTE, @intItemLifespan, EffectItemExpireDate)
		WHERE CharacterID = @inbCharacterID AND ItemID = @intItemID AND EffectItemExpireDate >= @dt2Now;

		IF @@ROWCOUNT = 0
			INSERT dbo.EffectItems (CharacterID, ItemID, EffectItemGetCode, EffectItemGetKey, Price, EternityFlag, EffectItemExpireDate, Property1, Property2, Property3, Property4, Property5, RegisterDate, ItemSerial)
			VALUES (@inbCharacterID, @intItemID, @inyEffectItemGetCode, @inbEffectItemGetKey, @intPrice, @bitEternityFlag, CASE @bitEternityFlag WHEN 1 THEN DATEADD(YEAR, 1, @dt2Now) ELSE DATEADD(MINUTE, @intItemLifespan, @dt2Now) END, @intProperty1, @intProperty2, @intProperty3, @intProperty4, @intProperty5, @dt2Now, @inbItemSerial);
	END

	ELSE IF @inyItemLifespanRenewal = 2 AND @intItemLifespan > 0
	BEGIN
		UPDATE dbo.EffectItems
		SET EffectItemExpireDate = DATEADD(MINUTE, @intItemLifespan, GETDATE())
		WHERE CharacterID = @inbCharacterID AND ItemID = @intItemID AND EffectItemExpireDate >= @dt2Now;

		IF @@ROWCOUNT = 0
			INSERT dbo.EffectItems (CharacterID, ItemID, EffectItemGetCode, EffectItemGetKey, Price, EternityFlag, EffectItemExpireDate, Property1, Property2, Property3, Property4, Property5, RegisterDate, ItemSerial)
			VALUES (@inbCharacterID, @intItemID, @inyEffectItemGetCode, @inbEffectItemGetKey, @intPrice, @bitEternityFlag, CASE @bitEternityFlag WHEN 1 THEN DATEADD(YEAR, 1, @dt2Now) ELSE DATEADD(MINUTE, @intItemLifespan, @dt2Now) END, @intProperty1, @intProperty2, @intProperty3, @intProperty4, @intProperty5, @dt2Now, @inbItemSerial);
	END
	IF @intToBeDeletedItemID IS NOT NULL AND @intToBeDeletedItemID <> @intItemID
	BEGIN
		DELETE dbo.EffectItems
		WHERE CharacterID = @inbCharacterID AND ItemID = @intToBeDeletedItemID;

	END
END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 547 AND ERROR_MESSAGE() LIKE N'%FK_Characters_EffectItems_CharacterID%'
		SET @intReturnValue = 103102;

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

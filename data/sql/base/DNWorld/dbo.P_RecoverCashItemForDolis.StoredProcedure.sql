USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_RecoverCashItemForDolis]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 7
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-02-07
description : ?? ???? ???? ??????.(!!?? - ?? ??? ?? ???? ?????)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103407 = ??? ??? ?? 1? ?????.
history :
7 = ?????? 0?? ??? ?? ?? ?? by ??? at 2013-05-27
**/

CREATE PROCEDURE [dbo].[P_RecoverCashItemForDolis]
	  @inbOwnerCharacterID bigint --// ???? ??? ??? ID
	, @intItemID int --// ??? ID
	, @insItemRemainCount smallint --// ?? ??? ??
	, @intRandomSeed int --// ?? ??
	, @inyItemLevel tinyint --// ??? ??
	, @inySealCount tinyint --// ?? ???
	, @inyItemOption tinyint = NULL --// ??? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @dt2Now datetime = GETDATE()
      , @inbItemSerial bigint = ABS(CAST(CAST(NEWID() AS binary(8)) AS bigint))
      , @bitSoulBoundFlag bit = CASE WHEN @intRandomSeed = 0 THEN 0 ELSE 1 END; 

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	IF @insItemRemainCount <= 0
	BEGIN
		SET @intReturnValue = 103407;
		GOTO ErrorHandler;
	END

	IF @inyItemOption IS NULL
		SET @inyItemOption = 0;

	/**_# [MaterializedItems] ???? INSERT???.*/
	INSERT dbo.MaterializedItems (ItemSerial, ItemID, ItemRemainCount, PayMethodCode, SenderCharacterID, ItemMaterializeCode, ItemMaterializeFKey, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, ItemMaterializeDate, ItemLifespan, EternityFlag, ItemExpireDate, OwnerCharacterID, OwnershipStartDate, ExpireCompleteFlag, TradeItemFlag, AdditiveItemID, ItemLocationCode, ItemLocationIndex)
	VALUES (@inbItemSerial, @intItemID, @insItemRemainCount, 5, NULL, 33, NULL, 0, @intRandomSeed, 0, @inyItemLevel, 0, @bitSoulBoundFlag, @inySealCount, @inyItemOption, @dt2Now, 0, 1, DATEADD(YEAR, 1, @dt2Now), @inbOwnerCharacterID, @dt2Now, 0, NULL, NULL, 5, 0);

	EXEC dbo.P_AddItemLog
			  @dt2LogDate = @dt2Now
			, @inbItemSerial = @inbItemSerial
			, @intItemID = @intItemID
			, @inyItemLogCode = 61
			, @inbOwnerCharacterID = @inbOwnerCharacterID
			, @inbOwnerCharacterIDBefore = NULL
			, @insItemCount = 0
			, @insItemCountBefore = @insItemRemainCount
			, @inbItemLogFKey1 = NULL
			, @inbItemLogFKey2 = NULL
			, @dt2OwnershipStartDate = @dt2Now
			, @intIP = NULL;
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

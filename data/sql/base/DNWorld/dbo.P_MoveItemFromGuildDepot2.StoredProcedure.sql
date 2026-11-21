USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_MoveItemFromGuildDepot2]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 6
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-07-11
description : ???? ?? ???? ?? ????? ??????
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103170 = ???? ???? ??? ??? ??? ?????. (Split ??)
103260 = ????? ?? ???? ???? ????.
103261 = ????? ?? ???? ?? ??? ???? ?? ??? Lock?? ????.
103262 = ????? ?? ???? ??? ???? ????.
103263 = ????? Merge? ?? ???? ???? ????.
103264 = ????? Merge? ?? ???? ??? ???? ????.
103265 = ????? Merge? ?? ???? ???? ?? ????.
103266 = ???? ???? ?? ???? ?????.
103267 = ?? ???? ???? ?? ? ?? ?? ??? ???????.
history :
2 = ItemLocationIndex ?? NULL? ??? by ??? at 2011-10-21
3 = ItemMerge? ItemLog ?? ??? ?? by ??? at 2012-02-24
4 = ItemMerge? ItemLog ??? ?? by ??? at 2012-08-24
5 = ???? ???? ?? ??? ?????? ????? ?? by ??? at 2012-11-14
6 = ???? MaterializedItemID ?? ?? ?? by ??? at 2012-12-27
**/

CREATE PROCEDURE [dbo].[P_MoveItemFromGuildDepot2]
	  @inbCharacterID bigint --// ??? ID
	, @inyItemLocationCode tinyint --// 2=????
	, @intGuildID int --// ?? ID
	, @intItemID int --// ??? ??? ID
	, @inbItemSerial bigint --// ??? ???? ?? ??
	, @inbNewItemSerial bigint --// ???? ?? ??? ??? ??, Split?? ?? ???? ? ?? ??. Split ?? ???? NULL? ?????.
	, @inbMergeTargetItemSerial bigint --// ???? ?? ???? Merge?? ??, ?? ???? ?? ??. Merge ?? ???? NULL? ?????.
	, @insItemCount smallint --// @inbItemSerial ?? ???? ???? ?? ? ? ??
	, @insMovingItemCount smallint --// ??? ???? ??
	, @insMergeItemCount smallint --// @inbMergeItemSerial ?? ???? ???? ?? ? ? ??
	, @intMaxDailyTakeItemCount int --// ?? ?? ???? ???? ?? ? ?? ?? ??
	, @intMapID int --// ? ID
	, @intDailyTakeItemCount int OUTPUT --// ?? ???? ???? ?? ?? (?? ?? 4? ???)
	, @dtsLastGuildDepotUseDate smalldatetime OUTPUT --//  ?? ?? ??? ??? ?? (??? ???, ?? ?? ????)
	, @insGuildDepotItemCount smallint = NULL OUTPUT --// 103262? ???? ?? DB? ??? ??? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dt2Now datetime2(0) = GETDATE()
	, @insItemRemainCount smallint
	, @inyItemLevel tinyint
	, @inbMaterializedItemID bigint
	, @inbNewMaterializedItemID bigint
	, @inbMergeTargetMaterializedItemID bigint
	, @insTargetItemCountBefore smallint
	, @insTargetItemCountAfter smallint;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# ?? ???? ???? ??? 103260? ?????.*/
	SELECT @insItemRemainCount = ItemRemainCount
		, @inyItemLevel = ItemLevel
		, @inbMaterializedItemID = MaterializedItemID
	FROM dbo.MaterializedItems WITH (NOLOCK)
	WHERE ItemSerial = @inbItemSerial AND ItemID = @intItemID AND ItemLocationCode = 16

	IF @@ROWCOUNT = 0
	BEGIN
		UPDATE dbo.GuildDepots
		SET LockFlag = 0, LockDate = NULL
		WHERE GuildID = @intGuildID AND ItemSerial = @inbItemSerial;

		SET @intReturnValue = 103260;
		GOTO ErrorHandler;
	END

	/**_# ?? ???? ?? ? ??? ???? ??? 103262? ?????.*/
	IF @insItemRemainCount <> @insItemCount
	BEGIN
		SET @insGuildDepotItemCount = @insItemRemainCount;
		SET @intReturnValue = 103262;
		GOTO ErrorHandler;
	END

	/**_## ???? Lock? ?????.*/
	UPDATE dbo.GuildDepots
	SET LockFlag = 1
		, LockDate = @dt2Now
	WHERE GuildID = @intGuildID
		AND MaterializedItemID = @inbMaterializedItemID
		AND (LockFlag = CAST(0 AS bit) OR LockDate < DATEADD(SECOND, -90, @dt2Now));

	/**_### ?? ??? ?? ???? ???? ??? Lock?? ??? 103261? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103261;
		GOTO ErrorHandler;
	END

	BEGIN TRANSACTION;

	/**_# ?? ?? ??? ?? ??? ?????.*/
	UPDATE dbo.GuildMembers
	SET @dtsLastGuildDepotUseDate = LastGuildDepotUseDate = @dt2Now
		, @intDailyTakeItemCount = DailyTakeItemCount = 
			CASE
				WHEN LastGuildDepotUseDate < DATEADD(HOUR, 4, CAST(CAST(CASE WHEN DATEPART(HOUR, @dt2Now) < 4 THEN DATEADD(DAY, -1, @dt2Now) ELSE @dt2Now END AS date) AS smalldatetime)) THEN 1
				ELSE DailyTakeItemCount + 1
			END
		, DailyTakeCoinAmount = 
			CASE
				WHEN LastGuildDepotUseDate < DATEADD(HOUR, 4, CAST(CAST(CASE WHEN DATEPART(HOUR, @dt2Now) < 4 THEN DATEADD(DAY, -1, @dt2Now) ELSE @dt2Now END AS date) AS smalldatetime)) THEN 0
				ELSE DailyTakeCoinAmount
			END
	WHERE GuildID = @intGuildID AND CharacterID = @inbCharacterID;

	/**_## ??? ?? ??? ???? ???? 103267? ?????.*/
	IF @intDailyTakeItemCount > @intMaxDailyTakeItemCount
	BEGIN
		ROLLBACK TRANSACTION;
		UPDATE dbo.GuildDepots
		SET LockFlag = 0, LockDate = NULL
		WHERE GuildID = @intGuildID AND MaterializedItemID = @inbMaterializedItemID;
		SET @intReturnValue = 103267;
		GOTO ErrorHandler;
	END

	COMMIT TRANSACTION;


	/**_# Merge ?? ???? ?? ??..*/
	IF @inbMergeTargetItemSerial IS NOT NULL
	BEGIN
		/**_## Merge ?? ???? ???? ??? 103263? ?????.*/
		SELECT @insItemRemainCount = ItemRemainCount
			, @inbMergeTargetMaterializedItemID = MaterializedItemID
		FROM dbo.MaterializedItems WITH (NOLOCK)
		WHERE ItemSerial = @inbMergeTargetItemSerial AND ItemID = @intItemID AND OwnerCharacterID = @inbCharacterID AND ItemLocationCode = 2;

		IF @@ROWCOUNT = 0
		BEGIN
			UPDATE dbo.GuildDepots
			SET LockFlag = 0, LockDate = NULL
			WHERE GuildID = @intGuildID AND MaterializedItemID = @inbMaterializedItemID;

			SET @intReturnValue = 103263;
			GOTO ErrorHandler;
		END
		/**_## Merge ?? ???? ?? ? ??? ???? ??? 103264? ?????.*/
		IF @insItemRemainCount <> @insMergeItemCount
		BEGIN
			UPDATE dbo.GuildDepots
			SET LockFlag = 0, LockDate = NULL
			WHERE GuildID = @intGuildID AND MaterializedItemID = @inbMaterializedItemID;

			SET @intReturnValue = 103264;
			GOTO ErrorHandler;
		END
	END

	/**_# ???? ?? ??? ??? ??..*/
	IF @inbNewItemSerial IS NOT NULL
	BEGIN
		/**_## ???? Split ???.*/
		EXEC @intReturnValue = dbo.P_SplitMaterializedItem
			  @dt2SplitDate = @dt2Now
			, @inbMaterializedItemID = @inbMaterializedItemID
			, @inbNewItemSerial = @inbNewItemSerial
			, @insItemCount = @insMovingItemCount
			, @bitInitOwnerCharacter = 0
			, @intChannelID = NULL
			, @intMapID = @intMapID
			, @inyCallProc = 5
			, @intGuildID = @intGuildID
			, @inbNewOwnerCharacterID2 = @inbCharacterID
			, @inbNewMaterializedItemID = @inbNewMaterializedItemID OUTPUT;

		IF @intReturnValue <> 0 GOTO ErrorHandler;

		/**_## Lock? ?????.*/
		UPDATE dbo.GuildDepots
		SET LockFlag = 0, LockDate = NULL
		WHERE GuildID = @intGuildID AND MaterializedItemID = @inbMaterializedItemID;

		SET @inbMaterializedItemID = @inbNewMaterializedItemID;
		SET @inbItemSerial = @inbNewItemSerial;
	END

	/**_# ???? ?? ???? ??? ??..*/
	IF @inbMergeTargetMaterializedItemID IS NOT NULL
	BEGIN
		DELETE dbo.MaterializedItems
		WHERE MaterializedItemID = @inbMaterializedItemID;

		UPDATE dbo.MaterializedItems
		SET @insTargetItemCountBefore = ItemRemainCount
			, @insTargetItemCountAfter = ItemRemainCount += @insMovingItemCount
		WHERE MaterializedItemID = @inbMergeTargetMaterializedItemID;
		
		EXEC dbo.P_AddItemLog
			  @dt2LogDate = @dt2Now
			, @inbItemSerial = @inbItemSerial
			, @intItemID = @intItemID
			, @inyItemLogCode = 33
			, @inbOwnerCharacterID = NULL
			, @inbOwnerCharacterIDBefore = @inbCharacterID
			, @insItemCount = 0
			, @insItemCountBefore = @insMovingItemCount
			, @inbItemLogFKey1 = @inbMergeTargetItemSerial
			, @inbItemLogFKey2 = NULL
			, @dt2OwnershipStartDate = @dt2Now
			, @intIP = NULL;

		EXEC dbo.P_AddItemLog
			  @dt2LogDate = @dt2Now
			, @inbItemSerial = @inbMergeTargetItemSerial
			, @intItemID = @intItemID
			, @inyItemLogCode = 34
			, @inbOwnerCharacterID = @inbCharacterID
			, @inbOwnerCharacterIDBefore = @inbCharacterID
			, @insItemCount = @insTargetItemCountAfter
			, @insItemCountBefore = @insTargetItemCountBefore
			, @inbItemLogFKey1 = @inbItemSerial
			, @inbItemLogFKey2 = NULL
			, @dt2OwnershipStartDate = NULL
			, @intIP = NULL;
	END

	/**_# ???? ?? ??? ??? ?? ???? ?? ???? ?????.*/
	IF @inbNewItemSerial IS NULL
	BEGIN
		UPDATE dbo.MaterializedItems
		SET OwnerCharacterID = @inbCharacterID
			, OwnershipStartDate = @dt2Now
			, ItemLocationCode = 2
			, ItemLocationIndex = NULL
		WHERE MaterializedItemID = @inbMaterializedItemID;

		DELETE dbo.GuildDepots
		WHERE GuildID = @intGuildID AND MaterializedItemID = @inbMaterializedItemID;
	END

	/**_## ??? ??? ?????.*/
	EXEC dbo.P_AddItemLog
		  @dt2LogDate = @dt2Now
		, @inbItemSerial = @inbItemSerial
		, @intItemID = @intItemID
		, @inyItemLogCode = 46
		, @inbOwnerCharacterID = @inbCharacterID
		, @inbOwnerCharacterIDBefore = NULL
		, @insItemCount = @insItemCount
		, @insItemCountBefore = @insItemCount
		, @inbItemLogFKey1 = @inbItemSerial
		, @inbItemLogFKey2 = @intGuildID
		, @dt2OwnershipStartDate = @dt2Now
		, @intIP = NULL;

	/**_# ?? ?? ??? ?????.*/
	INSERT dbo.GuildDepotHistory (GuildID, CharacterID, CharacterName, ItemCoinFlag, InOutFlag, ItemID, ItemLevel, ItemCount, GuildCoin, RegisterDate)
	SELECT @intGuildID, @inbCharacterID, CharacterName, 0, 1, @intItemID, @inyItemLevel, @insMovingItemCount, 0, @dt2Now
	FROM dbo.Characters WITH (NOLOCK)
	WHERE CharacterID = @inbCharacterID;
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

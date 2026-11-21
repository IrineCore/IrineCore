USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_MoveItemIntoGuildDepot2]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 12
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-07-11
description : ???? ?? ?????? ?? ??? ??????
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103170 = ???? ???? ??? ??? ??? ?????. (Split ??)
103253 = ?? ??? ?? ???? ???? ????.
103254 = ?? ??? ?? ???? ??? ???? ????.
103255 = ?? ??? ? ????.
103256 = ?? ??? Merge? ?? ???? ???? ????.
103257 = ?? ??? Merge? ?? ???? ??? ???? ????.
103258 = ?? ??? Merge? ?? ???? ???? ??? ?? ???? ?? Lock?? ????.
103259 = ?? ??? ?? ???? ????? ???? ?? ????.
103292 = ???? ???? ?? ??????.
history :
5 = ItemMerge? ItemLog ?? ??? ?? by ??? at 2012-02-24 
6 = ItemMerge? ItemLog ??? ?? by ??? at 2012-08-24 
11 = ?? ?? by ??? at 2012-12-20
12 = ???? MaterializedItemID ?? ?? ?? by ??? at 2012-12-27
**/

CREATE PROCEDURE [dbo].[P_MoveItemIntoGuildDepot2]
	  @inbCharacterID bigint --// ??? ID
	, @intGuildID int --// ?? ID
	, @intItemID int --// ??? ??? ID
	, @inbItemSerial bigint --// ??? ???? ?? ??
	, @inbNewItemSerial bigint --// ???? ?? ??? ??? ??, Split?? ?? ???? ? ?? ??. Split ?? ???? NULL? ?????.
	, @inbMergeTargetItemSerial bigint --// ???? ?? ???? Merge?? ??, ?? ???? ?? ??. Merge ?? ???? NULL? ?????.
	, @insItemCount smallint --// @inbItemSerial ?? ???? ???? ?? ? ? ??
	, @insMovingItemCount smallint --// ??? ???? ??
	, @insMergeItemCount smallint --// @inbMergeItemSerial ?? ???? ???? ?? ? ? ??
	, @intMapID int --// ? ID
	, @insGuildDepotItemCount smallint = NULL OUTPUT --// 103257? ???? ?? DB? ??? ??? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dt2Now datetime2(0) = GETDATE()
	, @insItemRemainCount smallint
	, @inbOwnerCharacterID bigint
	, @dt2OwnershipStartDate datetime2(0)
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
	/**_# Merge ?? ???? ?? ??..*/
	IF @inbMergeTargetItemSerial IS NOT NULL
	BEGIN
		/**_## Merge ?? ???? ???? ??? 103256? ?????.*/
		SELECT @insItemRemainCount = ItemRemainCount
			, @inbMergeTargetMaterializedItemID = MaterializedItemID
		FROM dbo.MaterializedItems WITH (NOLOCK, INDEX(IX_UNI_MaterializedItems_ItemSerial), FORCESEEK)
		WHERE ItemSerial = @inbMergeTargetItemSerial
			AND ItemID = @intItemID
			AND ItemLocationCode = 16
		OPTION (KEEPFIXED PLAN);

		IF @@ROWCOUNT = 0
		BEGIN
			SET @intReturnValue = 103256;
			GOTO ErrorHandler;
		END

		/**_## Merge ?? ???? ?? ? ??? ???? ??? 103257? ?????.*/
		IF @insItemRemainCount <> @insMergeItemCount
		BEGIN
			SET @insGuildDepotItemCount = @insItemRemainCount;
			SET @intReturnValue = 103257;
			GOTO ErrorHandler;
		END
		/**_## Merge ?? ???? Lock???. (?? LockDate? 90? ???? ?? Lock? ???? ?? Lock? ???.)*/
		UPDATE dbo.GuildDepots
		SET LockFlag = 1, LockDate = @dt2Now
		FROM dbo.GuildDepots WITH (INDEX(PK_GuildDepots), FORCESEEK)
		WHERE GuildID = @intGuildID
			AND MaterializedItemID = @inbMergeTargetMaterializedItemID
			AND (
				LockFlag = CAST(0 AS bit) OR LockDate < DATEADD(SECOND, -90, @dt2Now)
			)
		OPTION (KEEPFIXED PLAN);

		/**_## Merge ?? ???? ?? ??? ???? ??? ?? Lock?? ??? 103258? ?????.*/
		IF @@ROWCOUNT = 0
		BEGIN
			SET @intReturnValue = 103258;
			GOTO ErrorHandler;
		END
	END

	/**_# ?? ???? ???? ??? 103253? ?????.*/
	SELECT @insItemRemainCount = ItemRemainCount
		, @inyItemLevel = ItemLevel
		, @inbMaterializedItemID = MaterializedItemID
		, @inbOwnerCharacterID = OwnerCharacterID
		, @dt2OwnershipStartDate = OwnershipStartDate
	FROM dbo.MaterializedItems WITH (NOLOCK, INDEX(IX_UNI_MaterializedItems_ItemSerial), FORCESEEK)
	WHERE ItemSerial = @inbItemSerial
		AND ItemID = @intItemID
		AND OwnerCharacterID = @inbCharacterID
		AND ItemLocationCode = 2;

	IF @@ROWCOUNT = 0
	BEGIN
		UPDATE dbo.GuildDepots
		SET LockFlag = 0
			, LockDate = NULL
		FROM dbo.GuildDepots WITH (INDEX(PK_GuildDepots), FORCESEEK)
		WHERE GuildID = @intGuildID
			AND MaterializedItemID = @inbMergeTargetMaterializedItemID
		OPTION (KEEPFIXED PLAN);

		SET @intReturnValue = 103253;
		GOTO ErrorHandler;
	END

	/**_# ?? ???? ?? ? ??? ???? ??? 103254? ?????.*/
	IF @insItemRemainCount <> @insItemCount
	BEGIN
		UPDATE dbo.GuildDepots
		SET LockFlag = 0, LockDate = NULL
		FROM dbo.GuildDepots WITH (INDEX(PK_GuildDepots), FORCESEEK)
		WHERE GuildID = @intGuildID
			AND MaterializedItemID = @inbMergeTargetMaterializedItemID
		OPTION (KEEPFIXED PLAN);

		SET @intReturnValue = 103254;
		GOTO ErrorHandler;
	END

	/**_# ?? ??? FULL ???? 103255? ?????. ?, ??? ??? ?? ?? Max ?? ???? ???? ??? ? ????.*/
	IF @inbMergeTargetItemSerial IS NULL
		AND (
			SELECT CAST(GuildDepotSize AS smallint)
			FROM dbo.VIX_GuildDepotSize WITH (NOLOCK, NOEXPAND, FORCESEEK)
			WHERE GuildID = @intGuildID
		) >= (
			SELECT GuildDepotSize
			FROM dbo.Guilds WITH (NOLOCK, INDEX(PK_Guilds))
			WHERE GuildID = @intGuildID
		)
	BEGIN
		UPDATE dbo.GuildDepots
		SET LockFlag = 0, LockDate = NULL
		FROM dbo.GuildDepots WITH (INDEX(PK_GuildDepots), FORCESEEK)
		WHERE GuildID = @intGuildID
		  AND MaterializedItemID = @inbMergeTargetMaterializedItemID
		OPTION (KEEPFIXED PLAN);

		SET @intReturnValue = 103255;
		GOTO ErrorHandler;
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
			, @bitInitOwnerCharacter = 1
			, @intChannelID = NULL
			, @intMapID = @intMapID
			, @inyCallProc = 4
			, @intGuildID = @intGuildID
			, @inbNewMaterializedItemID = @inbNewMaterializedItemID OUTPUT;

		IF @intReturnValue <> 0
		BEGIN
			UPDATE dbo.GuildDepots
			SET LockFlag = 0, LockDate = NULL
			FROM dbo.GuildDepots WITH (INDEX(PK_GuildDepots), FORCESEEK)
			WHERE GuildID = @intGuildID
			  AND MaterializedItemID = @inbMergeTargetMaterializedItemID
			OPTION (KEEPFIXED PLAN);

			GOTO ErrorHandler;
		END

		SET @inbMaterializedItemID = @inbNewMaterializedItemID;
		SET @inbItemSerial = @inbNewItemSerial;
	END

	/**_# ?? ??? ???? ????.*/
	IF @inbMergeTargetItemSerial IS NULL
	BEGIN
		INSERT dbo.GuildDepots (GuildID, ItemSerial, LockFlag, LockDate, MaterializedItemID)
		VALUES (@intGuildID, @inbItemSerial, 0, NULL, @inbMaterializedItemID)
		OPTION (KEEPFIXED PLAN);

		UPDATE dbo.MaterializedItems
		SET OwnerCharacterID = NULL
			, OwnershipStartDate = NULL
			, ItemLocationCode = 16
			, ItemLocationIndex = NULL
		FROM dbo.MaterializedItems WITH (INDEX(PK_MaterializedItems), FORCESEEK)
		WHERE MaterializedItemID = @inbMaterializedItemID
		OPTION (KEEPFIXED PLAN);
	END

	/**_# Merge ?? ???? ??? ?? ??? ???? Merge???.*/
	ELSE
	BEGIN
		DELETE dbo.MaterializedItems
		FROM dbo.MaterializedItems WITH (INDEX(PK_MaterializedItems), FORCESEEK)
		WHERE MaterializedItemID = @inbMaterializedItemID
		OPTION (KEEPFIXED PLAN);

		UPDATE dbo.MaterializedItems
		SET @insTargetItemCountBefore = ItemRemainCount
			, @insTargetItemCountAfter = ItemRemainCount += @insMovingItemCount
		FROM dbo.MaterializedItems WITH (INDEX(PK_MaterializedItems), FORCESEEK)
		WHERE MaterializedItemID = @inbMergeTargetMaterializedItemID
		OPTION (KEEPFIXED PLAN);

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

		/**_## Lock? ?????.*/
		UPDATE dbo.GuildDepots
		SET LockFlag = 0, LockDate = NULL
		FROM dbo.GuildDepots WITH (INDEX(PK_GuildDepots), FORCESEEK)
		WHERE GuildID = @intGuildID
		  AND MaterializedItemID = @inbMergeTargetMaterializedItemID
		OPTION (KEEPFIXED PLAN);
	END

	EXEC dbo.P_AddItemLog
		  @dt2LogDate = @dt2Now
		, @inbItemSerial = @inbItemSerial
		, @intItemID = @intItemID
		, @inyItemLogCode = 45
		, @inbOwnerCharacterID = NULL
		, @inbOwnerCharacterIDBefore = @inbOwnerCharacterID
		, @insItemCount = @insItemCount
		, @insItemCountBefore = @insItemCount
		, @inbItemLogFKey1 = @inbItemSerial
		, @inbItemLogFKey2 = @intGuildID
		, @dt2OwnershipStartDate = @dt2OwnershipStartDate
		, @intIP = NULL;

	/**_# ?? ?? ??? ?????.*/
	INSERT dbo.GuildDepotHistory (GuildID, CharacterID, CharacterName, ItemCoinFlag, InOutFlag, ItemID, ItemLevel, ItemCount, GuildCoin, RegisterDate)
	SELECT @intGuildID, @inbCharacterID, CharacterName, 0, 0, @intItemID, @inyItemLevel, @insMovingItemCount, 0, @dt2Now
	FROM dbo.Characters WITH (NOLOCK, INDEX(PK_Characters), FORCESEEK)
	WHERE CharacterID = @inbCharacterID
	OPTION (KEEPFIXED PLAN);
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

USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ChangeItemLocation2]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 20
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-07-08
description : ???? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103102 = ???? ???? ????.
103123 = ??? ?????.
103151 = ???? ???? ????.
103160 = ???? ?? ?/? ??? ?????.
103170 = ???? ???? ??? ??? ??? ?????.
103184 = @inbItemSerial ?????.
103292 = ???? ???? ?? ??????.
103309 = merge ?? ???? ???? ????.
103391 = ?? ???? ?? ?? ???? ???? ????.
history :
4 = ItemMerge? ItemLog ?? ??? ?? by ??? at 2012-02-24
8 = 4?? ??     by ??? at 2012-04-09
9 = @bitMergeAllFlag? ???? ??? ?? by ??? at 2012-07-11
11 = 4? ??  by ??? at 2012-08-24
14 = ?? ?? ?/?? ?? by ??? at 2012-10-11
15 = ?? ?? ?/?? ?? by ??? at 2012-10-16
16 = input param. @inyItemLocationBefore ?? by ??? at 2012-10-16
17 = ?? ??? ItemCount? NULL? ???? ?? ?? by ??? at 2012-10-24
18 = ??? ??, ?? ?? by ??? at 2013-01-28
**/

CREATE PROCEDURE [dbo].[P_ChangeItemLocation2]
	  @inbCharacterID bigint --// ??? ID
	, @inbItemSerial bigint --// ??? ??? ?? ??
	, @inbNewItemSerial bigint = NULL --// (optional) ???? ?? ??? ??? ?? ?? ??? ??? ?? ??.
	, @intItemID int --// ??? ??? ID
	, @inyItemLocationCode tinyint --// ?? ? ItemLocationCode 1=????, 2=????, 3=??, 4=??? ????, 5=?? ????, 7=?? ????, 8=??, 9=??, 10=?, 11=?? ??, 17=?? ??, 18=?? ?? ??, 20=??? ????, 21=??? ??
	, @inyItemLocationIndex tinyint --// ?? ? ??? ?? ???
	, @insItemCount smallint --// ??? ??? ??
	, @intChannelID int --// ?? ID
	, @intMapID int --// ? ID
	, @bitPetEquipmentFlag bit = 0 --// ???? ???? "?? or ?? ???? ???"?? ??. 0=false, 1=true
	, @inbPetItemSerial bigint = NULL --// ?? or ?? ???? ?? -> ?? or ?? ??? ?? ??
	, @inyPetEquipmentIndex tinyint = NULL --// ?? or ?? ???? ?? -> ?? ??
	, @bitMergeOccurFlag bit --// 0=???? merge?? ??, 1=???? merge?
	, @inbMergeTargetItemSerial bigint --// ??? ???? ????? merge?? ??, target ???? ??? ???
	, @bitMergeAllFlag bit --// ???? ?? param. ???.
	, @intFee int = 0 --// ??? ?? ???
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dt2Now datetime2(0) = GETDATE()
	, @inyItemLocationCodeBefore tinyint
	, @inyItemLocationIndexBefore tinyint
	, @bitSplit bit = 0
	, @inbMaterializedItemID bigint
	, @inbNewMaterializedItemID bigint
	, @inbOldMaterializedItemID bigint
	, @inbMaterializedItemIDTemp bigint
	, @insTargetItemCountBefore smallint
	, @insTargetItemCountAfter smallint
	, @inbOwnerCharacterID bigint
	, @intAccountID int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SET @inyItemLocationCode = 
		CASE @inyItemLocationCode
			WHEN 0 THEN NULL
			ELSE @inyItemLocationCode
		END;

	/**_# ??? ???? ???? ??? 103184? ?????.*/
	SELECT @inbMaterializedItemID = MaterializedItemID
		, @inyItemLocationCodeBefore = ItemLocationCode
		, @inyItemLocationIndexBefore = ItemLocationIndex
		, @inbOwnerCharacterID = OwnerCharacterID
	FROM dbo.MaterializedItems WITH (NOLOCK, INDEX(IX_UNI_MaterializedItems_ItemSerial), FORCESEEK)
	WHERE ItemSerial = @inbItemSerial
	OPTION (KEEPFIXED PLAN);

	IF @@ROWCOUNT = 0 OR @inyItemLocationCodeBefore IN (12, 14, 15)
	BEGIN
		SET @intReturnValue = 103184;
		GOTO ErrorHandler;
	END

	IF (@inyItemLocationCode BETWEEN 17 AND 18 OR @inyItemLocationCodeBefore BETWEEN 17 AND 18)
	BEGIN
		SELECT @intAccountID = AccountID
		FROM dbo.Characters WITH (NOLOCK, INDEX(PK_Characters), FORCESEEK)
		WHERE CharacterID = @inbCharacterID
		OPTION (KEEPFIXED PLAN);

		IF @inyItemLocationCode BETWEEN 17 AND 18 AND @inbOwnerCharacterID <> @inbCharacterID
		BEGIN
			SET @intReturnValue = 103184;
			GOTO ErrorHandler;
		END
		IF @inyItemLocationCodeBefore BETWEEN 17 AND 18 AND NOT EXISTS (
			SELECT *
			FROM dbo.WorldDepots WITH (NOLOCK, INDEX(PK_WorldDepots), FORCESEEK)
			WHERE AccountID = @intAccountID
				AND MaterializedItemID = @inbMaterializedItemID
		)
		BEGIN
			SET @intReturnValue = 103184;
			GOTO ErrorHandler;
		END
	END
	ELSE IF @inbOwnerCharacterID <> @inbCharacterID
	BEGIN
		SET @intReturnValue = 103184;
		GOTO ErrorHandler;
	END

	/**_# ???? ?? ??? ??? ?? ???? Split???.*/
	IF @inbNewItemSerial IS NOT NULL AND @inbNewItemSerial <> @inbItemSerial
	BEGIN
		SET @bitSplit = 1;
		EXEC @intReturnValue = dbo.P_SplitMaterializedItem
			  @dt2SplitDate = @dt2Now
			, @inbMaterializedItemID = @inbMaterializedItemID
			, @inbNewItemSerial = @inbNewItemSerial
			, @insItemCount = @insItemCount
			, @bitInitOwnerCharacter = 0
			, @intChannelID = @intChannelID
			, @intMapID = @intMapID
			, @inyCallProc = 3
			, @inbNewMaterializedItemID = @inbNewMaterializedItemID OUTPUT;

		IF @intReturnValue <> 0
			GOTO ErrorHandler;

		SET @inbMaterializedItemID = @inbNewMaterializedItemID;
		SET @inbItemSerial = @inbNewItemSerial;
	END

	/**_# ?? or ?? ???? ???? ?? ????..*/
	IF @bitPetEquipmentFlag = 1
	BEGIN
		/**_## ?? ???? ????? ??? ???? [PetEquipments]????? DELETE???.*/
		IF @inyItemLocationCode IS NOT NULL
		BEGIN
			DELETE dbo.PetEquipments
			WHERE PetItemSerial = @inbPetItemSerial AND PetEquipmentIndex = @inyPetEquipmentIndex AND PetEquipmentItemSerial = @inbItemSerial
			OPTION (KEEPFIXED PLAN);

			IF @@ROWCOUNT = 0
			BEGIN
				SET @intReturnValue = 103151;
				GOTO ErrorHandler;
			END
		END

		/**_## ?? ???? ?????? ?? ???? ???? [PetEquipments] ???? INSERT???.*/
		ELSE
		BEGIN
			SET @inyItemLocationCode = 13;

			INSERT dbo.PetEquipments (PetItemSerial, PetEquipmentIndex, PetEquipmentItemSerial)
			VALUES (@inbPetItemSerial, @inyPetEquipmentIndex, @inbItemSerial)
			OPTION (KEEPFIXED PLAN);
		END
	END

	IF @intFee > 0 AND @inyItemLocationCode BETWEEN 17 AND 18
	BEGIN
		EXEC @intReturnValue = dbo.P_ModCoinForSubProc
			  @inbCharacterID = @inbCharacterID
			, @inyCoinChangeCode = 48
			, @inbCoinChangeFKey = @intAccountID
			, @inbCoin = @intFee
			, @intChannelID = @intChannelID
			, @intMapID = @intMapID
			, @dt2LogDate = @dt2Now;

		IF @intReturnValue <> 0
			GOTO ErrorHandler;
	END

	IF @bitMergeOccurFlag = 0
	BEGIN
		BEGIN TRANSACTION;

		UPDATE dbo.MaterializedItems 
		SET ItemLocationCode = @inyItemLocationCode
			, ItemLocationIndex = @inyItemLocationIndex
			, OwnerCharacterID = CASE WHEN @inyItemLocationCode BETWEEN 17 AND 18 THEN NULL ELSE ISNULL(OwnerCharacterID, @inbCharacterID) END
			, OwnershipStartDate = CASE WHEN @inyItemLocationCodeBefore BETWEEN 17 AND 18 THEN @dt2Now ELSE CASE WHEN @inyItemLocationCode BETWEEN 17 AND 18 THEN NULL ELSE OwnershipStartDate END END
		FROM dbo.MaterializedItems WITH (INDEX(PK_MaterializedItems), FORCESEEK)
		WHERE MaterializedItemID = @inbMaterializedItemID
		OPTION (KEEPFIXED PLAN);

		IF @inyItemLocationCode BETWEEN 17 AND 18
			INSERT dbo.WorldDepots (AccountID, MaterializedItemID)
			VALUES (@intAccountID, @inbMaterializedItemID);

		ELSE IF @inyItemLocationCodeBefore BETWEEN 17 AND 18
			DELETE dbo.WorldDepots
			FROM dbo.WorldDepots WITH (INDEX(PK_WorldDepots), FORCESEEK)
			WHERE AccountID = @intAccountID
				AND MaterializedItemID = @inbMaterializedItemID
			OPTION (KEEPFIXED PLAN);

		COMMIT TRANSACTION;
	END

	ELSE
	BEGIN
		BEGIN TRANSACTION;

		DELETE dbo.MaterializedItems
		FROM dbo.MaterializedItems WITH (INDEX(PK_MaterializedItems), FORCESEEK)
		WHERE MaterializedItemID = @inbMaterializedItemID
			AND ItemRemainCount = @insItemCount
		OPTION (KEEPFIXED PLAN);

		IF @@ROWCOUNT = 0
		BEGIN
			SET @intReturnValue = 103151;
			GOTO ErrorHandler;
		END

		IF @inyItemLocationCodeBefore BETWEEN 17 AND 18
			DELETE dbo.WorldDepots
			FROM dbo.WorldDepots WITH (INDEX(PK_WorldDepots), FORCESEEK)
			WHERE AccountID = @intAccountID
				AND MaterializedItemID = @inbMaterializedItemID
			OPTION (KEEPFIXED PLAN);

		UPDATE dbo.MaterializedItems
		SET @insTargetItemCountBefore = ItemRemainCount
			, @insTargetItemCountAfter = ItemRemainCount += @insItemCount
		FROM dbo.MaterializedItems WITH (INDEX(IX_UNI_MaterializedItems_ItemSerial), FORCESEEK)
		WHERE ItemSerial = @inbMergeTargetItemSerial
			AND OwnerCharacterID = @inbCharacterID
			AND ItemLocationCode = @inyItemLocationCode
		OPTION (KEEPFIXED PLAN);

		IF @@ROWCOUNT = 0
		BEGIN
			SET @intReturnValue = 103309;
			GOTO ErrorHandler;
		END

		COMMIT TRANSACTION;

		EXEC dbo.P_AddItemLog
			  @dt2LogDate = @dt2Now
			, @inbItemSerial = @inbItemSerial
			, @intItemID = @intItemID
			, @inyItemLogCode = 33
			, @inbOwnerCharacterID = NULL
			, @inbOwnerCharacterIDBefore = @inbCharacterID
			, @insItemCount = 0
			, @insItemCountBefore = @insItemCount
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

	/**_## ?? ?? ??? ??? ?????.*/
	SET @inbItemSerial = ISNULL(@inbNewItemSerial, @inbItemSerial);

	IF @inyItemLocationCode BETWEEN 17 AND 18
	BEGIN
		EXEC dbo.P_AddItemLog
			  @dt2LogDate = @dt2Now
			, @inbItemSerial = @inbItemSerial
			, @intItemID = @intItemID
			, @inyItemLogCode = 68
			, @inbOwnerCharacterID = NULL
			, @inbOwnerCharacterIDBefore = @inbCharacterID
			, @insItemCount = @insItemCount
			, @insItemCountBefore = @insItemCount
			, @inbItemLogFKey1 = @intAccountID
			, @inbItemLogFKey2 = NULL
			, @dt2OwnershipStartDate = NULL
			, @intIP = NULL;
	END

	IF @inyItemLocationCodeBefore BETWEEN 17 AND 18
	BEGIN
		EXEC dbo.P_AddItemLog
			  @dt2LogDate = @dt2Now
			, @inbItemSerial = @inbItemSerial
			, @intItemID = @intItemID
			, @inyItemLogCode = 69
			, @inbOwnerCharacterID = @inbCharacterID
			, @inbOwnerCharacterIDBefore = NULL
			, @insItemCount = @insItemCount
			, @insItemCountBefore = @insItemCount
			, @inbItemLogFKey1 = @intAccountID
			, @inbItemLogFKey2 = NULL
			, @dt2OwnershipStartDate = @dt2Now
			, @intIP = NULL;
	END
END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 2601 AND ERROR_MESSAGE() LIKE N'%IX_UNF_MaterializedItems_OwnerCharacterID_ItemLocationCode_ItemLocationIndex%'
		SET @intReturnValue = 103391;
	
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

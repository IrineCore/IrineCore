USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetCountPet2]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-07-12
description : ?? or ? ? ?? ??? ?? ??????.
result set : RecordCount/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
2 = (50729) ??? ?? ???? ?? ?? by ??? at 2012-01-30 
**/

CREATE PROCEDURE [dbo].[P_GetCountPet2]
	  @inbCharacterID bigint --// ??? ID
	, @inyPetType tinyint --// 1=??, 2=?&??
	, @bitSummonFlag bit --// ?? or ?? ?? ??. 0=????? ?? ?? or ?, 1=??? ?? or ?
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @inyItemLocationIndex tinyint
	, @inyItemLocationCode tinyint
	, @intRecordCount int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SET @inyItemLocationIndex =
		CASE @inyPetType
			WHEN 1 THEN 255 -- ??
			WHEN 2 THEN 254 -- ?
		END;

	SET @inyItemLocationCode = 
		CASE @inyPetType
			WHEN 1 THEN 9 -- ?? ????
			WHEN 2 THEN 10 -- ? ????
		END;

	IF @inyPetType = 1
	BEGIN
		IF @bitSummonFlag = 1
		BEGIN
			SELECT @intRecordCount = COUNT(*)
			FROM dbo.MaterializedItems WITH (NOLOCK, INDEX(IX_NNI_MaterializedItems_OwnerCharacterID_ItemLocationCode), FORCESEEK)
			WHERE OwnerCharacterID = @inbCharacterID
				AND ItemLocationCode = 1
				AND ItemLocationIndex = 255
			OPTION (KEEPFIXED PLAN);

			SELECT @intRecordCount += COUNT(*)
			FROM dbo.MaterializedItems MI WITH (NOLOCK, INDEX(IX_NNI_MaterializedItems_OwnerCharacterID_ItemLocationCode), FORCESEEK)
				INNER JOIN dbo.PetEquipments PE WITH (NOLOCK, INDEX(PK_PetEquipments), FORCESEEK) ON MI.ItemSerial = PE.PetItemSerial
			WHERE MI.OwnerCharacterID = @inbCharacterID
				AND MI.ItemLocationCode = 1
				AND MI.ItemLocationIndex = 255
			OPTION (FORCE ORDER, LOOP JOIN, KEEPFIXED PLAN);
		END

		ELSE
		BEGIN
			SELECT @intRecordCount = COUNT(*)
			FROM dbo.MaterializedItems MI WITH (NOLOCK, INDEX(IX_NNI_MaterializedItems_OwnerCharacterID_ItemLocationCode), FORCESEEK)
			WHERE OwnerCharacterID = @inbCharacterID
				AND ItemLocationCode = 9
			OPTION (KEEPFIXED PLAN);

			SELECT @intRecordCount += COUNT(*)
			FROM dbo.MaterializedItems MI WITH (NOLOCK, INDEX(IX_NNI_MaterializedItems_OwnerCharacterID_ItemLocationCode), FORCESEEK)
				INNER JOIN dbo.PetEquipments PE WITH (NOLOCK, INDEX(PK_PetEquipments), FORCESEEK) ON MI.ItemSerial = PE.PetItemSerial
			WHERE MI.OwnerCharacterID = @inbCharacterID
				AND MI.ItemLocationCode = 9
			OPTION (FORCE ORDER, LOOP JOIN, KEEPFIXED PLAN);
		END
	END

	ELSE
	BEGIN
		IF @bitSummonFlag = 1
		BEGIN
			SELECT @intRecordCount = COUNT(*)
			FROM dbo.MaterializedItems WITH (NOLOCK, FORCESEEK)
			WHERE OwnerCharacterID = @inbCharacterID
				AND ItemLocationCode = 1
				AND ItemLocationIndex >= 254
			OPTION (KEEPFIXED PLAN);

			SELECT @intRecordCount += COUNT(*)
			FROM dbo.MaterializedItems MI WITH (NOLOCK, INDEX(IX_NNI_MaterializedItems_OwnerCharacterID_ItemLocationCode), FORCESEEK)
				INNER JOIN dbo.PetEquipments PE WITH (NOLOCK, INDEX(PK_PetEquipments), FORCESEEK) ON MI.ItemSerial = PE.PetItemSerial
			WHERE MI.OwnerCharacterID = @inbCharacterID
				AND MI.ItemLocationCode = 1
				AND MI.ItemLocationIndex >= 254
			OPTION (FORCE ORDER, LOOP JOIN, KEEPFIXED PLAN);
		END

		ELSE
		BEGIN
			SELECT @intRecordCount = COUNT(*)
			FROM dbo.MaterializedItems MI WITH (NOLOCK, INDEX(IX_NNI_MaterializedItems_OwnerCharacterID_ItemLocationCode), FORCESEEK)
			WHERE OwnerCharacterID = @inbCharacterID
				AND ItemLocationCode BETWEEN 9 AND 10
			OPTION (KEEPFIXED PLAN);

			SELECT @intRecordCount += COUNT(*)
			FROM dbo.MaterializedItems MI WITH (NOLOCK, INDEX(IX_NNI_MaterializedItems_OwnerCharacterID_ItemLocationCode), FORCESEEK)
				INNER JOIN dbo.PetEquipments PE WITH (NOLOCK, INDEX(PK_PetEquipments), FORCESEEK) ON MI.ItemSerial = PE.PetItemSerial
			WHERE MI.OwnerCharacterID = @inbCharacterID
				AND MI.ItemLocationCode BETWEEN 9 AND 10
			OPTION (FORCE ORDER, LOOP JOIN, KEEPFIXED PLAN);
		END
	END

	SELECT @intRecordCount AS RecordCount;

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

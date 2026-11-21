USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetPagePet2]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 8
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-08-31
description : ?? or ? ? ?? ??? ??? ??????.
result set : 
	@inbVersion = 2
		@inyPetType = 1 : PetIndex/tinyint, ItemSerial/bigint, ItemID/int, ItemRemainCount/smallint, ItemDurability/smallint, RandomSeed/int, CoolTime/int, ItemLevel/tinyint, ItemPotential/tinyint, SoulBoundFlag/bit, ItemOption/tinyint, SealCount/tinyint, ItemMaterializeDate/datetime2(0), EternityFlag/bit, ItemExpireDate/smalldatetime, SkinColor/int
		@inyPetType = 2 : PetIndex/tinyint, ItemSerial/bigint, ItemID/int, ItemRemainCount/smallint, ItemDurability/smallint, RandomSeed/int, CoolTime/int, ItemLevel/tinyint, ItemPotential/tinyint, SoulBoundFlag/bit, ItemOption/tinyint, SealCount/tinyint, ItemMaterializeDate/datetime2(0), EternityFlag/bit, ItemExpireDate/smalldatetime, SkinColor/int, SkinColor2/int, SkillID1/int, SkillID2/int, PetName/nvarchar, PetExp/int, ExpandSkillFlag/bit
	
	@inbVersion = 3
		@inyPetType = 1 : PetIndex/tinyint, ItemSerial/bigint, ItemID/int, ItemRemainCount/smallint, ItemDurability/smallint, RandomSeed/int, CoolTime/int, ItemLevel/tinyint, ItemPotential/tinyint, SoulBoundFlag/bit, ItemOption/tinyint, SealCount/tinyint, ItemMaterializeDate/datetime2(0), EternityFlag/bit, ItemExpireDate/smalldatetime, SkinColor/int
		@inyPetType = 2 : PetIndex/tinyint, ItemSerial/bigint, ItemID/int, ItemRemainCount/smallint, ItemDurability/smallint, RandomSeed/int, CoolTime/int, ItemLevel/tinyint, ItemPotential/tinyint, SoulBoundFlag/bit, ItemOption/tinyint, SealCount/tinyint, ItemMaterializeDate/datetime2(0), EternityFlag/bit, ItemExpireDate/smalldatetime, SkinColor/int, SkinColor2/int, SkillID1/int, SkillID2/int, PetName/nvarchar, PetExp/int, ExpandSkillFlag/bit

	@inbVersion = 7
		@inyPetType = 1 : PetIndex/tinyint, ItemSerial/bigint, ItemID/int, ItemRemainCount/smallint, ItemDurability/smallint, RandomSeed/int, CoolTime/int, ItemLevel/tinyint, ItemPotential/tinyint, SoulBoundFlag/bit, ItemOption/tinyint, SealCount/tinyint, ItemMaterializeDate/datetime2(0), EternityFlag/bit, ItemExpireDate/smalldatetime, SkinColor/int
		@inyPetType = 2 : PetIndex/tinyint, ItemSerial/bigint, ItemID/int, ItemRemainCount/smallint, ItemDurability/smallint, RandomSeed/int, CoolTime/int, ItemLevel/tinyint, ItemPotential/tinyint, SoulBoundFlag/bit, ItemOption/tinyint, SealCount/tinyint, ItemMaterializeDate/datetime2(0), EternityFlag/bit, ItemExpireDate/smalldatetime, SkinColor/int, SkinColor2/int, SkillID1/int, SkillID2/int, PetName/nvarchar, PetExp/int, ExpandSkillFlag/bit, DegreeOfHunger/smallint, LastHungerModifyDate/datetime2(0)

return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history
2 = ?? ?? ?? ???? ????? ??	by 2011/10/18.ohtack
3 = input param. @intVersion ?? by 2011/10/19.ohtack
4 = NumberOfSkill Bit??? ?? by ??? at 2011-12-08
5 = (50729) ??? ?? ???? ?? ?? by ??? at 2012-01-30 
7 = (51462) ?? ??? ?? by ??? at 2012-03-12
**/

CREATE PROCEDURE [dbo].[P_GetPagePet2]
	  @intPageNumber int --// ??? ??.
	, @intPageSize int --// ??? ???.
	, @inbCharacterID bigint --// ??? ID
	, @inyPetType tinyint --// 1=??, 2=(??)?&?? / (??)? only
	, @bitSummonFlag bit --// ?? or ?? ?? ??. 0=????? ?? ?? or ?, 1=??? ?? or ?
	, @intVersion int = 2 --// ??? SP? ??
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
	IF @inyPetType = 1
	BEGIN
		IF @bitSummonFlag = 1
			WITH C1 AS (
				SELECT MI.ItemSerial AS PetItemSerial, CAST(0 AS tinyint) AS PetIndex, MI.ItemSerial, MI.ItemID, MI.ItemRemainCount, MI.ItemDurability, MI.RandomSeed, MI.CoolTime, MI.ItemLevel
					, MI.ItemPotential, MI.SoulBoundFlag, MI.ItemOption, MI.SealCount, MI.ItemMaterializeDate, MI.EternityFlag, MI.ItemExpireDate, NULL AS SkinColor
				FROM dbo.MaterializedItems MI WITH (NOLOCK, INDEX(IX_NNI_MaterializedItems_OwnerCharacterID_ItemLocationCode), FORCESEEK)
				WHERE MI.OwnerCharacterID = @inbCharacterID AND MI.ItemLocationCode = 1 AND MI.ItemLocationIndex = 255
			), C2 AS (
				SELECT PetItemSerial, PetIndex, ItemSerial, ItemID, ItemRemainCount, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, ItemOption, SealCount
					, ItemMaterializeDate, EternityFlag, ItemExpireDate, SkinColor
				FROM C1
				UNION ALL
				SELECT T.PetItemSerial, PE.PetEquipmentIndex, MI.ItemSerial, MI.ItemID, MI.ItemRemainCount, MI.ItemDurability, MI.RandomSeed, MI.CoolTime, MI.ItemLevel
					, MI.ItemPotential, MI.SoulBoundFlag, MI.ItemOption, MI.SealCount, MI.ItemMaterializeDate, MI.EternityFlag, MI.ItemExpireDate, PE.SkinColor
				FROM C1 T
					INNER JOIN dbo.PetEquipments PE WITH (NOLOCK, INDEX(PK_PetEquipments), FORCESEEK) ON T.PetItemSerial = PE.PetItemSerial
					LEFT OUTER JOIN dbo.MaterializedItems MI WITH (NOLOCK, INDEX(IX_UNI_MaterializedItems_ItemSerial), FORCESEEK) ON PE.PetEquipmentItemSerial = MI.ItemSerial
			), C3 AS(
				SELECT ROW_NUMBER() OVER(ORDER BY PetItemSerial, PetIndex) AS Seq
					, PetIndex, ItemSerial, ItemID, ItemRemainCount, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, ItemOption
					, SealCount, ItemMaterializeDate, EternityFlag, ItemExpireDate, SkinColor
				FROM C2
			)
			SELECT PetIndex, ItemSerial, ItemID, ItemRemainCount, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, ItemOption
				, SealCount, ItemMaterializeDate, EternityFlag, ItemExpireDate, SkinColor
			FROM C3
			WHERE Seq BETWEEN (((@intPageNumber - 1) * @intPageSize) + 1) AND (@intPageNumber * @intPageSize)
			ORDER BY Seq
			OPTION (KEEPFIXED PLAN);

		ELSE
			WITH C1 AS (
				SELECT MI.ItemSerial AS PetItemSerial, CAST(0 AS tinyint) AS PetIndex, MI.ItemSerial, MI.ItemID, MI.ItemRemainCount, MI.ItemDurability, MI.RandomSeed, MI.CoolTime, MI.ItemLevel
					, MI.ItemPotential, MI.SoulBoundFlag, MI.ItemOption, MI.SealCount, MI.ItemMaterializeDate, MI.EternityFlag, MI.ItemExpireDate, NULL AS SkinColor
				FROM dbo.MaterializedItems MI WITH (NOLOCK, INDEX(IX_NNI_MaterializedItems_OwnerCharacterID_ItemLocationCode), FORCESEEK)
				WHERE MI.OwnerCharacterID = @inbCharacterID AND MI.ItemLocationCode = 9
			), C2 AS (
				SELECT PetItemSerial, PetIndex, ItemSerial, ItemID, ItemRemainCount, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, ItemOption, SealCount
					, ItemMaterializeDate, EternityFlag, ItemExpireDate, SkinColor
				FROM C1
				UNION ALL
				SELECT T.PetItemSerial, PE.PetEquipmentIndex, MI.ItemSerial, MI.ItemID, MI.ItemRemainCount, MI.ItemDurability, MI.RandomSeed, MI.CoolTime, MI.ItemLevel
					, MI.ItemPotential, MI.SoulBoundFlag, MI.ItemOption, MI.SealCount, MI.ItemMaterializeDate, MI.EternityFlag, MI.ItemExpireDate, PE.SkinColor
				FROM C1 T
					INNER JOIN dbo.PetEquipments PE WITH (NOLOCK, INDEX(PK_PetEquipments), FORCESEEK) ON T.PetItemSerial = PE.PetItemSerial
					LEFT OUTER JOIN dbo.MaterializedItems MI WITH (NOLOCK, INDEX(IX_UNI_MaterializedItems_ItemSerial), FORCESEEK) ON PE.PetEquipmentItemSerial = MI.ItemSerial
			), C3 AS (
				SELECT ROW_NUMBER() OVER(ORDER BY PetItemSerial, PetIndex) AS Seq
					, PetIndex, ItemSerial, ItemID, ItemRemainCount, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, ItemOption
					, SealCount, ItemMaterializeDate, EternityFlag, ItemExpireDate, SkinColor
				FROM C2
			)
			SELECT PetIndex, ItemSerial, ItemID, ItemRemainCount, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, ItemOption
				, SealCount, ItemMaterializeDate, EternityFlag, ItemExpireDate, SkinColor
			FROM C3
			WHERE Seq BETWEEN (((@intPageNumber - 1) * @intPageSize) + 1) AND (@intPageNumber * @intPageSize)
			ORDER BY Seq
			OPTION (KEEPFIXED PLAN);
	END

	ELSE
	BEGIN
		IF @intVersion = 2
		BEGIN
			IF @bitSummonFlag = 1
				WITH C1 AS (
					SELECT MI.ItemSerial AS PetItemSerial, CAST(0 AS tinyint) AS PetIndex, MI.ItemSerial, MI.ItemID, MI.ItemRemainCount, MI.ItemDurability, MI.RandomSeed, MI.CoolTime, MI.ItemLevel
						, MI.ItemPotential, MI.SoulBoundFlag, MI.ItemOption, MI.SealCount, MI.ItemMaterializeDate, MI.EternityFlag, MI.ItemExpireDate
						, NULL AS SkinColor
						, NULL AS SkinColor2
						, NULL AS SkillID1
						, NULL AS SkillID2
						, NULL AS PetName
						, NULL AS PetExp
					FROM dbo.MaterializedItems MI WITH (NOLOCK, INDEX(IX_NNI_MaterializedItems_OwnerCharacterID_ItemLocationCode), FORCESEEK)
					WHERE MI.OwnerCharacterID = @inbCharacterID AND MI.ItemLocationCode = 1 AND MI.ItemLocationIndex = CASE @inyPetType WHEN 1 THEN 255 WHEN 2 THEN 254 END
				), C2 AS (
					SELECT PetItemSerial, PetIndex, ItemSerial, ItemID, ItemRemainCount, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, ItemOption, SealCount
						, ItemMaterializeDate, EternityFlag, ItemExpireDate
						, SkinColor
						, SkinColor2
						, SkillID1
						, SkillID2
						, PetName
						, PetExp
					FROM C1
					UNION ALL
					SELECT T.PetItemSerial, PE.PetEquipmentIndex, MI.ItemSerial, MI.ItemID, MI.ItemRemainCount, MI.ItemDurability, MI.RandomSeed, MI.CoolTime, MI.ItemLevel
						, MI.ItemPotential, MI.SoulBoundFlag, MI.ItemOption, MI.SealCount, MI.ItemMaterializeDate, MI.EternityFlag, MI.ItemExpireDate
						, PE.SkinColor
						, PE.SkinColor2
						, PE.SkillID1
						, PE.SkillID2
						, PE.PetName
						, ISNULL(PE.PetExp, 0) AS PetExp
					FROM C1 T
						INNER JOIN dbo.PetEquipments PE WITH (NOLOCK, INDEX(PK_PetEquipments), FORCESEEK) ON T.PetItemSerial = PE.PetItemSerial
						LEFT OUTER JOIN dbo.MaterializedItems MI WITH (NOLOCK, INDEX(IX_UNI_MaterializedItems_ItemSerial), FORCESEEK) ON PE.PetEquipmentItemSerial = MI.ItemSerial
				), C3 AS(
					SELECT ROW_NUMBER() OVER(ORDER BY PetItemSerial, PetIndex) AS Seq
						, PetIndex, ItemSerial, ItemID, ItemRemainCount, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, ItemOption
						, SealCount, ItemMaterializeDate, EternityFlag, ItemExpireDate
						, SkinColor
						, SkinColor2
						, SkillID1
						, SkillID2
						, PetName
						, PetExp
					FROM C2
				)
				SELECT PetIndex, ItemSerial, ItemID, ItemRemainCount, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, ItemOption
					, SealCount, ItemMaterializeDate, EternityFlag, ItemExpireDate
					, SkinColor
					, SkinColor2
					, SkillID1
					, SkillID2
					, PetName
					, PetExp
				FROM C3
				WHERE Seq BETWEEN (((@intPageNumber - 1) * @intPageSize) + 1) AND (@intPageNumber * @intPageSize)
				ORDER BY Seq
				OPTION (KEEPFIXED PLAN);

				ELSE
					WITH C1 AS (
						SELECT MI.ItemSerial AS PetItemSerial, CAST(0 AS tinyint) AS PetIndex, MI.ItemSerial, MI.ItemID, MI.ItemRemainCount, MI.ItemDurability, MI.RandomSeed, MI.CoolTime, MI.ItemLevel
							, MI.ItemPotential, MI.SoulBoundFlag, MI.ItemOption, MI.SealCount, MI.ItemMaterializeDate, MI.EternityFlag, MI.ItemExpireDate
							, NULL AS SkinColor
							, NULL AS SkinColor2
							, NULL AS SkillID1
							, NULL AS SkillID2
							, NULL AS PetName
							, NULL AS PetExp
						FROM dbo.MaterializedItems MI WITH (NOLOCK, INDEX(IX_NNI_MaterializedItems_OwnerCharacterID_ItemLocationCode), FORCESEEK)
						WHERE MI.OwnerCharacterID = @inbCharacterID AND MI.ItemLocationCode BETWEEN 9 AND 10
					), C2 AS (
						SELECT PetItemSerial, PetIndex, ItemSerial, ItemID, ItemRemainCount, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, ItemOption, SealCount
							, ItemMaterializeDate, EternityFlag, ItemExpireDate
							, SkinColor
							, SkinColor2
							, SkillID1
							, SkillID2
							, PetName
							, PetExp
						FROM C1
						UNION ALL
						SELECT T.PetItemSerial, PE.PetEquipmentIndex, MI.ItemSerial, MI.ItemID, MI.ItemRemainCount, MI.ItemDurability, MI.RandomSeed, MI.CoolTime, MI.ItemLevel
							, MI.ItemPotential, MI.SoulBoundFlag, MI.ItemOption, MI.SealCount, MI.ItemMaterializeDate, MI.EternityFlag, MI.ItemExpireDate
							, PE.SkinColor
							, PE.SkinColor2
							, PE.SkillID1
							, PE.SkillID2
							, PE.PetName
							, ISNULL(PE.PetExp, 0) AS PetExp
						FROM C1 T
							INNER JOIN dbo.PetEquipments PE WITH (NOLOCK, INDEX(PK_PetEquipments), FORCESEEK) ON T.PetItemSerial = PE.PetItemSerial
							LEFT OUTER JOIN dbo.MaterializedItems MI WITH (NOLOCK, INDEX(IX_UNI_MaterializedItems_ItemSerial), FORCESEEK) ON PE.PetEquipmentItemSerial = MI.ItemSerial
					), C3 AS (
						SELECT ROW_NUMBER() OVER(ORDER BY PetItemSerial, PetIndex) AS Seq
							, PetIndex, ItemSerial, ItemID, ItemRemainCount, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, ItemOption
							, SealCount, ItemMaterializeDate, EternityFlag, ItemExpireDate
							, SkinColor
							, SkinColor2
							, SkillID1
							, SkillID2
							, PetName
							, PetExp
						FROM C2
					)
					SELECT PetIndex, ItemSerial, ItemID, ItemRemainCount, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, ItemOption
						, SealCount, ItemMaterializeDate, EternityFlag, ItemExpireDate
						, SkinColor
						, SkinColor2
						, SkillID1
						, SkillID2
						, PetName
						, PetExp
					FROM C3
					WHERE Seq BETWEEN (((@intPageNumber - 1) * @intPageSize) + 1) AND (@intPageNumber * @intPageSize)
					ORDER BY Seq
					OPTION (KEEPFIXED PLAN);
		END
		ELSE IF @intVersion = 3
		BEGIN
			IF @bitSummonFlag = 1
				WITH C1 AS (
					SELECT MI.ItemSerial AS PetItemSerial, CAST(0 AS tinyint) AS PetIndex, MI.ItemSerial, MI.ItemID, MI.ItemRemainCount, MI.ItemDurability, MI.RandomSeed, MI.CoolTime, MI.ItemLevel
						, MI.ItemPotential, MI.SoulBoundFlag, MI.ItemOption, MI.SealCount, MI.ItemMaterializeDate, MI.EternityFlag, MI.ItemExpireDate
						, NULL AS SkinColor
						, NULL AS SkinColor2
						, NULL AS SkillID1
						, NULL AS SkillID2
						, NULL AS PetName
						, NULL AS PetExp
						, NULL AS ExpandSkillFlag
					FROM dbo.MaterializedItems MI WITH (NOLOCK, INDEX(IX_NNI_MaterializedItems_OwnerCharacterID_ItemLocationCode), FORCESEEK)
					WHERE MI.OwnerCharacterID = @inbCharacterID AND MI.ItemLocationCode = 1 AND MI.ItemLocationIndex = CASE @inyPetType WHEN 1 THEN 255 WHEN 2 THEN 254 END
				), C2 AS (
					SELECT PetItemSerial, PetIndex, ItemSerial, ItemID, ItemRemainCount, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, ItemOption, SealCount
						, ItemMaterializeDate, EternityFlag, ItemExpireDate
						, SkinColor
						, SkinColor2
						, SkillID1
						, SkillID2
						, PetName
						, PetExp
						, ExpandSkillFlag
					FROM C1
					UNION ALL
					SELECT T.PetItemSerial, PE.PetEquipmentIndex, MI.ItemSerial, MI.ItemID, MI.ItemRemainCount, MI.ItemDurability, MI.RandomSeed, MI.CoolTime, MI.ItemLevel
						, MI.ItemPotential, MI.SoulBoundFlag, MI.ItemOption, MI.SealCount, MI.ItemMaterializeDate, MI.EternityFlag, MI.ItemExpireDate
						, PE.SkinColor
						, PE.SkinColor2
						, PE.SkillID1
						, PE.SkillID2
						, PE.PetName
						, ISNULL(PE.PetExp, 0) AS PetExp
						, ISNULL(PE.ExpandSkillFlag, 0) AS ExpandSkillFlag
					FROM C1 T
						INNER JOIN dbo.PetEquipments PE WITH (NOLOCK, INDEX(PK_PetEquipments), FORCESEEK) ON T.PetItemSerial = PE.PetItemSerial
						LEFT OUTER JOIN dbo.MaterializedItems MI WITH (NOLOCK, INDEX(IX_UNI_MaterializedItems_ItemSerial), FORCESEEK) ON PE.PetEquipmentItemSerial = MI.ItemSerial
				), C3 AS(
					SELECT ROW_NUMBER() OVER(ORDER BY PetItemSerial, PetIndex) AS Seq
						, PetIndex, ItemSerial, ItemID, ItemRemainCount, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, ItemOption
						, SealCount, ItemMaterializeDate, EternityFlag, ItemExpireDate
						, SkinColor
						, SkinColor2
						, SkillID1
						, SkillID2
						, PetName
						, PetExp
						, ExpandSkillFlag
					FROM C2
				)
				SELECT PetIndex, ItemSerial, ItemID, ItemRemainCount, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, ItemOption
					, SealCount, ItemMaterializeDate, EternityFlag, ItemExpireDate
					, SkinColor
					, SkinColor2
					, SkillID1
					, SkillID2
					, PetName
					, PetExp
					, ExpandSkillFlag
				FROM C3
				WHERE Seq BETWEEN (((@intPageNumber - 1) * @intPageSize) + 1) AND (@intPageNumber * @intPageSize)
				ORDER BY Seq
				OPTION (KEEPFIXED PLAN);

			ELSE
				WITH C1 AS (
					SELECT MI.ItemSerial AS PetItemSerial, CAST(0 AS tinyint) AS PetIndex, MI.ItemSerial, MI.ItemID, MI.ItemRemainCount, MI.ItemDurability, MI.RandomSeed, MI.CoolTime, MI.ItemLevel
						, MI.ItemPotential, MI.SoulBoundFlag, MI.ItemOption, MI.SealCount, MI.ItemMaterializeDate, MI.EternityFlag, MI.ItemExpireDate
						, NULL AS SkinColor
						, NULL AS SkinColor2
						, NULL AS SkillID1
						, NULL AS SkillID2
						, NULL AS PetName
						, NULL AS PetExp
						, NULL AS ExpandSkillFlag
					FROM dbo.MaterializedItems MI WITH (NOLOCK, INDEX(IX_NNI_MaterializedItems_OwnerCharacterID_ItemLocationCode), FORCESEEK)
					WHERE MI.OwnerCharacterID = @inbCharacterID AND MI.ItemLocationCode BETWEEN 9 AND 10
				), C2 AS (
					SELECT PetItemSerial, PetIndex, ItemSerial, ItemID, ItemRemainCount, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, ItemOption, SealCount
						, ItemMaterializeDate, EternityFlag, ItemExpireDate
						, SkinColor
						, SkinColor2
						, SkillID1
						, SkillID2
						, PetName
						, PetExp
						, ExpandSkillFlag
					FROM C1
					UNION ALL
					SELECT T.PetItemSerial, PE.PetEquipmentIndex, MI.ItemSerial, MI.ItemID, MI.ItemRemainCount, MI.ItemDurability, MI.RandomSeed, MI.CoolTime, MI.ItemLevel
						, MI.ItemPotential, MI.SoulBoundFlag, MI.ItemOption, MI.SealCount, MI.ItemMaterializeDate, MI.EternityFlag, MI.ItemExpireDate
						, PE.SkinColor
						, PE.SkinColor2
						, PE.SkillID1
						, PE.SkillID2
						, PE.PetName
						, ISNULL(PE.PetExp, 0) AS PetExp
						, ISNULL(PE.ExpandSkillFlag, 0) AS ExpandSkillFlag
					FROM C1 T
						INNER JOIN dbo.PetEquipments PE WITH (NOLOCK, INDEX(PK_PetEquipments), FORCESEEK) ON T.PetItemSerial = PE.PetItemSerial
						LEFT OUTER JOIN dbo.MaterializedItems MI WITH (NOLOCK, INDEX(IX_UNI_MaterializedItems_ItemSerial), FORCESEEK) ON PE.PetEquipmentItemSerial = MI.ItemSerial
				), C3 AS (
					SELECT ROW_NUMBER() OVER(ORDER BY PetItemSerial, PetIndex) AS Seq
						, PetIndex, ItemSerial, ItemID, ItemRemainCount, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, ItemOption
						, SealCount, ItemMaterializeDate, EternityFlag, ItemExpireDate
						, SkinColor
						, SkinColor2
						, SkillID1
						, SkillID2
						, PetName
						, PetExp
						, ExpandSkillFlag
					FROM C2
				)
				SELECT PetIndex, ItemSerial, ItemID, ItemRemainCount, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, ItemOption
					, SealCount, ItemMaterializeDate, EternityFlag, ItemExpireDate
					, SkinColor
					, SkinColor2
					, SkillID1
					, SkillID2
					, PetName
					, PetExp
					, ExpandSkillFlag
				FROM C3
				WHERE Seq BETWEEN (((@intPageNumber - 1) * @intPageSize) + 1) AND (@intPageNumber * @intPageSize)
				ORDER BY Seq
				OPTION (KEEPFIXED PLAN);
		END

		ELSE IF @intVersion = 7
		BEGIN
			IF @bitSummonFlag = 1
				WITH C1 AS (
					SELECT MI.ItemSerial AS PetItemSerial, CAST(0 AS tinyint) AS PetIndex, MI.ItemSerial, MI.ItemID, MI.ItemRemainCount, MI.ItemDurability, MI.RandomSeed, MI.CoolTime, MI.ItemLevel
						, MI.ItemPotential, MI.SoulBoundFlag, MI.ItemOption, MI.SealCount, MI.ItemMaterializeDate, MI.EternityFlag, MI.ItemExpireDate
						, NULL AS SkinColor
						, NULL AS SkinColor2
						, NULL AS SkillID1
						, NULL AS SkillID2
						, NULL AS PetName
						, NULL AS PetExp
						, NULL AS ExpandSkillFlag
						, NULL AS DegreeOfHunger
						, NULL AS LastHungerModifyDate
					FROM dbo.MaterializedItems MI WITH (NOLOCK, INDEX(IX_NNI_MaterializedItems_OwnerCharacterID_ItemLocationCode), FORCESEEK)
					WHERE MI.OwnerCharacterID = @inbCharacterID AND MI.ItemLocationCode = 1 AND MI.ItemLocationIndex = CASE @inyPetType WHEN 1 THEN 255 WHEN 2 THEN 254 END
				), C2 AS (
					SELECT PetItemSerial, PetIndex, ItemSerial, ItemID, ItemRemainCount, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, ItemOption, SealCount
						, ItemMaterializeDate, EternityFlag, ItemExpireDate
						, SkinColor
						, SkinColor2
						, SkillID1
						, SkillID2
						, PetName
						, PetExp
						, ExpandSkillFlag
						, DegreeOfHunger
						, LastHungerModifyDate
					FROM C1
					UNION ALL
					SELECT T.PetItemSerial, PE.PetEquipmentIndex, MI.ItemSerial, MI.ItemID, MI.ItemRemainCount, MI.ItemDurability, MI.RandomSeed, MI.CoolTime, MI.ItemLevel
						, MI.ItemPotential, MI.SoulBoundFlag, MI.ItemOption, MI.SealCount, MI.ItemMaterializeDate, MI.EternityFlag, MI.ItemExpireDate
						, PE.SkinColor
						, PE.SkinColor2
						, PE.SkillID1
						, PE.SkillID2
						, PE.PetName
						, ISNULL(PE.PetExp, 0) AS PetExp
						, ISNULL(PE.ExpandSkillFlag, 0) AS ExpandSkillFlag
						, PE.DegreeOfHunger
						, PE.LastHungerModifyDate
					FROM C1 T
						INNER JOIN dbo.PetEquipments PE WITH (NOLOCK, INDEX(PK_PetEquipments), FORCESEEK) ON T.PetItemSerial = PE.PetItemSerial
						LEFT OUTER JOIN dbo.MaterializedItems MI WITH (NOLOCK, INDEX(IX_UNI_MaterializedItems_ItemSerial), FORCESEEK) ON PE.PetEquipmentItemSerial = MI.ItemSerial
				), C3 AS(
					SELECT ROW_NUMBER() OVER(ORDER BY PetItemSerial, PetIndex) AS Seq
						, PetIndex, ItemSerial, ItemID, ItemRemainCount, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, ItemOption
						, SealCount, ItemMaterializeDate, EternityFlag, ItemExpireDate
						, SkinColor
						, SkinColor2
						, SkillID1
						, SkillID2
						, PetName
						, PetExp
						, ExpandSkillFlag
						, DegreeOfHunger
						, LastHungerModifyDate
					FROM C2
				)
				SELECT PetIndex, ItemSerial, ItemID, ItemRemainCount, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, ItemOption
					, SealCount, ItemMaterializeDate, EternityFlag, ItemExpireDate
					, SkinColor
					, SkinColor2
					, SkillID1
					, SkillID2
					, PetName
					, PetExp
					, ExpandSkillFlag
					, DegreeOfHunger
					, LastHungerModifyDate
				FROM C3
				WHERE Seq BETWEEN (((@intPageNumber - 1) * @intPageSize) + 1) AND (@intPageNumber * @intPageSize)
				ORDER BY Seq
				OPTION (KEEPFIXED PLAN);

			ELSE
				WITH C1 AS (
					SELECT MI.ItemSerial AS PetItemSerial, CAST(0 AS tinyint) AS PetIndex, MI.ItemSerial, MI.ItemID, MI.ItemRemainCount, MI.ItemDurability, MI.RandomSeed, MI.CoolTime, MI.ItemLevel
						, MI.ItemPotential, MI.SoulBoundFlag, MI.ItemOption, MI.SealCount, MI.ItemMaterializeDate, MI.EternityFlag, MI.ItemExpireDate
						, NULL AS SkinColor
						, NULL AS SkinColor2
						, NULL AS SkillID1
						, NULL AS SkillID2
						, NULL AS PetName
						, NULL AS PetExp
						, NULL AS ExpandSkillFlag
						, NULL AS DegreeOfHunger
						, NULL AS LastHungerModifyDate
					FROM dbo.MaterializedItems MI WITH (NOLOCK, INDEX(IX_NNI_MaterializedItems_OwnerCharacterID_ItemLocationCode), FORCESEEK)
					WHERE MI.OwnerCharacterID = @inbCharacterID AND MI.ItemLocationCode BETWEEN 9 AND 10
				), C2 AS (
					SELECT PetItemSerial, PetIndex, ItemSerial, ItemID, ItemRemainCount, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, ItemOption, SealCount
						, ItemMaterializeDate, EternityFlag, ItemExpireDate
						, SkinColor
						, SkinColor2
						, SkillID1
						, SkillID2
						, PetName
						, PetExp
						, ExpandSkillFlag
						, DegreeOfHunger
						, LastHungerModifyDate
					FROM C1
					UNION ALL
					SELECT T.PetItemSerial, PE.PetEquipmentIndex, MI.ItemSerial, MI.ItemID, MI.ItemRemainCount, MI.ItemDurability, MI.RandomSeed, MI.CoolTime, MI.ItemLevel
						, MI.ItemPotential, MI.SoulBoundFlag, MI.ItemOption, MI.SealCount, MI.ItemMaterializeDate, MI.EternityFlag, MI.ItemExpireDate
						, PE.SkinColor
						, PE.SkinColor2
						, PE.SkillID1
						, PE.SkillID2
						, PE.PetName
						, ISNULL(PE.PetExp, 0) AS PetExp
						, ISNULL(PE.ExpandSkillFlag, 0) AS ExpandSkillFlag
						, PE.DegreeOfHunger
						, PE.LastHungerModifyDate
					FROM C1 T
						INNER JOIN dbo.PetEquipments PE WITH (NOLOCK, INDEX(PK_PetEquipments), FORCESEEK) ON T.PetItemSerial = PE.PetItemSerial
						LEFT OUTER JOIN dbo.MaterializedItems MI WITH (NOLOCK, INDEX(IX_UNI_MaterializedItems_ItemSerial), FORCESEEK) ON PE.PetEquipmentItemSerial = MI.ItemSerial
				), C3 AS (
					SELECT ROW_NUMBER() OVER(ORDER BY PetItemSerial, PetIndex) AS Seq
						, PetIndex, ItemSerial, ItemID, ItemRemainCount, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, ItemOption
						, SealCount, ItemMaterializeDate, EternityFlag, ItemExpireDate
						, SkinColor
						, SkinColor2
						, SkillID1
						, SkillID2
						, PetName
						, PetExp
						, ExpandSkillFlag
						, DegreeOfHunger
						, LastHungerModifyDate
					FROM C2
				)
				SELECT PetIndex, ItemSerial, ItemID, ItemRemainCount, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, ItemOption
					, SealCount, ItemMaterializeDate, EternityFlag, ItemExpireDate
					, SkinColor
					, SkinColor2
					, SkillID1
					, SkillID2
					, PetName
					, PetExp
					, ExpandSkillFlag
					, DegreeOfHunger
					, LastHungerModifyDate
				FROM C3
				WHERE Seq BETWEEN (((@intPageNumber - 1) * @intPageSize) + 1) AND (@intPageNumber * @intPageSize)
				ORDER BY Seq
				OPTION (KEEPFIXED PLAN);
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

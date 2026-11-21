USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DLS_GetListMaterializedItem]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-02-08
description : ?? ??? ??? ?????.
result set : ItemLocationCode/tinyint, ItemLocationIndex/tinyint, ItemSerial/bigint, ItemID/int, ItemRemainCount/smallint, ItemDurability/smallint, RandomSeed/int, CoolTime/int, ItemLevel/tinyint, ItemPotential/tinyint, SoulBoundFlag/bit, ItemOption/tinyint, SealCount/tinyint, ItemMaterializeDate/datetime2(0), EternityFlag/bit, ItemExpireDate/smalldatetime, AdditiveItemID/int, ItemLifespan/int, ItemPotentialMoveCount/tinyint, LockedDate/datetime, UnlockDate/datetime
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103151 = ???? ???? ????.
**/

CREATE PROCEDURE [dbo].[P_DLS_GetListMaterializedItem]
	  @inbCharacterID bigint --// ??? ID
	, @inyItemLocationCode tinyint --// ItemLocationCode
	, @intAccountID int --// ?? ID
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
	/**_# ??? ??? ??? ?????.*/
	IF @inyItemLocationCode IS NOT NULL
	BEGIN
		IF @inyItemLocationCode BETWEEN 17 AND 18
			SELECT MI.ItemLocationCode, MI.ItemLocationIndex, MI.ItemSerial, MI.ItemID, MI.ItemRemainCount, MI.ItemDurability, MI.RandomSeed, MI.CoolTime, MI.ItemLevel, MI.ItemPotential, MI.SoulBoundFlag
				, MI.ItemOption, MI.SealCount, MI.ItemMaterializeDate, MI.EternityFlag, MI.ItemExpireDate, MI.AdditiveItemID, MI.ItemLifespan, ISNULL(MI.ItemPotentialMoveCount, 0) AS ItemPotentialMoveCount
			FROM dbo.WorldDepots WD WITH (NOLOCK, INDEX(PK_WorldDepots), FORCESEEK)
				INNER JOIN dbo.MaterializedItems MI WITH (NOLOCK, INDEX (PK_MaterializedItems), FORCESEEK) ON WD.MaterializedItemID = MI.MaterializedItemID			    
			WHERE WD.AccountID = @intAccountID
				AND MI.ItemRemainCount > 0
				AND MI.ItemLocationCode = @inyItemLocationCode
			OPTION (KEEPFIXED PLAN, FORCE ORDER, LOOP JOIN);

		ELSE
			SELECT MI.ItemLocationCode, MI.ItemLocationIndex, MI.ItemSerial, MI.ItemID, MI.ItemRemainCount, MI.ItemDurability, MI.RandomSeed, MI.CoolTime, MI.ItemLevel, MI.ItemPotential, MI.SoulBoundFlag
				, MI.ItemOption, MI.SealCount, MI.ItemMaterializeDate, MI.EternityFlag, MI.ItemExpireDate, MI.AdditiveItemID, MI.ItemLifespan, ISNULL(MI.ItemPotentialMoveCount, 0) AS ItemPotentialMoveCount
			    , LI.LockedDate, LI.UnlockDate
			FROM dbo.MaterializedItems MI WITH (NOLOCK, INDEX (IX_NNI_MaterializedItems_OwnerCharacterID_ItemLocationCode), FORCESEEK)
				 LEFT OUTER JOIN dbo.LockedItems LI WITH (NOLOCK, INDEX(PK_LockedItems), FORCESEEK) ON MI.OwnerCharacterID = LI.OwnerCharacterID AND MI.ItemLocationCode = LI.ItemLocationCode AND MI.ItemLocationIndex = LI.ItemLocationIndex
			WHERE MI.OwnerCharacterID = @inbCharacterID
				AND MI.ItemLocationCode = @inyItemLocationCode
				AND (@inyItemLocationCode > 1 OR (@inyItemLocationCode = 1 AND MI.ItemLocationIndex < 200))
				AND MI.ItemRemainCount > 0
			OPTION (KEEPFIXED PLAN);
	END

	ELSE
		SELECT MI.ItemLocationCode, MI.ItemLocationIndex, MI.ItemSerial, MI.ItemID, MI.ItemRemainCount, MI.ItemDurability, MI.RandomSeed, MI.CoolTime, MI.ItemLevel, MI.ItemPotential, MI.SoulBoundFlag
			, MI.ItemOption, MI.SealCount, MI.ItemMaterializeDate, MI.EternityFlag, MI.ItemExpireDate, MI.AdditiveItemID, MI.ItemLifespan, ISNULL(MI.ItemPotentialMoveCount, 0) AS ItemPotentialMoveCount
			, LI.LockedDate, LI.UnlockDate
		FROM dbo.MaterializedItems MI WITH (NOLOCK, INDEX (IX_NNI_MaterializedItems_OwnerCharacterID_ItemLocationCode), FORCESEEK)
			 LEFT OUTER JOIN dbo.LockedItems LI WITH (NOLOCK, INDEX(PK_LockedItems), FORCESEEK) ON MI.OwnerCharacterID = LI.OwnerCharacterID AND MI.ItemLocationCode = LI.ItemLocationCode AND MI.ItemLocationIndex = LI.ItemLocationIndex
		WHERE MI.OwnerCharacterID = @inbCharacterID
			AND (MI.ItemLocationCode <= 4 OR MI.ItemLocationCode BETWEEN 7 AND 8 OR MI.ItemLocationCode BETWEEN 20 AND 21)
			AND (MI.ItemLocationCode > 1 OR (MI.ItemLocationCode = 1 AND MI.ItemLocationIndex < 200))
			AND ItemRemainCount > 0
		UNION ALL
		SELECT MI.ItemLocationCode, MI.ItemLocationIndex, MI.ItemSerial, MI.ItemID, MI.ItemRemainCount, MI.ItemDurability, MI.RandomSeed, MI.CoolTime, MI.ItemLevel, MI.ItemPotential, MI.SoulBoundFlag
			, MI.ItemOption, MI.SealCount, MI.ItemMaterializeDate, MI.EternityFlag, MI.ItemExpireDate, MI.AdditiveItemID, MI.ItemLifespan, ISNULL(MI.ItemPotentialMoveCount, 0) AS ItemPotentialMoveCount
			, NULL, NULL
		FROM dbo.WorldDepots WD WITH (NOLOCK, INDEX(PK_WorldDepots), FORCESEEK)
			INNER JOIN dbo.MaterializedItems MI WITH (NOLOCK, INDEX (PK_MaterializedItems), FORCESEEK) ON WD.MaterializedItemID = MI.MaterializedItemID
		WHERE WD.AccountID = @intAccountID
			AND MI.ItemRemainCount > 0
			AND MI.ItemLocationCode BETWEEN 17 AND 18
		OPTION (KEEPFIXED PLAN, FORCE ORDER, LOOP JOIN);

	/**_# ??? ???? ??? 103151? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103151;
		GOTO ErrorHandler;
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
	EXEC @intReturnValue = dbo.P_AddErrorLog @intPROCID = @@PROCID;

RETURN @intReturnValue;
GO

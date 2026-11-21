USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListMissingItem2]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-07-21

description :
	??? ??? ??? ?????.
result set :
	@intVersion < 4 : ItemSerial/bigint, ItemID/int, ItemRemainCount/smallint, ItemDurability/smallint, RandomSeed/int, CoolTime/int, ItemLevel/tinyint, ItemPotential/tinyint, SoulBoundFlag/bit, SealCount/tinyint, ItemOption/tinyint, ItemLifespan/int, EternityFlag/bit, ItemExpireDate/smalldatetime
	@intVersion = 4 : ItemSerial/bigint, ItemID/int, ItemRemainCount/smallint, ItemDurability/smallint, RandomSeed/int, CoolTime/int, ItemLevel/tinyint, ItemPotential/tinyint, SoulBoundFlag/bit, SealCount/tinyint, ItemOption/tinyint, ItemLifespan/int, EternityFlag/bit, ItemExpireDate/smalldatetime, ItemPotentialMoveCount/tinyint
	@intVersion = 5 : Version 4 + DragonJewelType/tinyint
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
	103216 = ??? ???? ???? ????.
history :
	3 = (50729) ??? ?? ???? ?? ?? by ??? at 2012-01-30 
	4 = ?? ?? ?? ?? (@intVersion = 4)     by ??? at 2012-04-12
	5 = ?? ?? ?? ?? (@intVersion = 5)     by ??? at 2013-04-04
**/

CREATE PROCEDURE [dbo].[P_GetListMissingItem2]
	  @inbCharacterID bigint --// ??? ID
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

	IF @intVersion < 4
	BEGIN
		SELECT ItemSerial, ItemID, ItemRemainCount, ItemDurability, RandomSeed, CoolTime, ItemLevel
				, ItemPotential, SoulBoundFlag, SealCount, ItemOption, ItemLifespan, EternityFlag, ItemExpireDate
		FROM dbo.MaterializedItems WITH (NOLOCK, INDEX(IX_NNI_MaterializedItems_OwnerCharacterID_ItemLocationCode), FORCESEEK)
		WHERE OwnerCharacterID = @inbCharacterID
			AND ItemLocationCode IS NULL
		UNION ALL
		SELECT ItemSerial, ItemID, ItemRemainCount, ItemDurability, RandomSeed, CoolTime, ItemLevel
				, ItemPotential, SoulBoundFlag, SealCount, ItemOption, ItemLifespan, EternityFlag, ItemExpireDate
		FROM dbo.MaterializedItems MI WITH (NOLOCK, INDEX(IX_NNI_MaterializedItems_OwnerCharacterID_ItemLocationCode), FORCESEEK)
		WHERE OwnerCharacterID = @inbCharacterID
			AND ItemLocationCode = 12
			AND NOT EXISTS (
				SELECT *
				FROM dbo.Trade WITH (NOLOCK, INDEX(IX_NNI_Trade_ItemSerial), FORCESEEK)
				WHERE ItemSerial = MI.ItemSerial AND TradeStatusCode = 0
			)
		OPTION (KEEPFIXED PLAN);
	END
	ELSE IF @intVersion = 4
	BEGIN
		SELECT ItemSerial, ItemID, ItemRemainCount, ItemDurability, RandomSeed, CoolTime, ItemLevel
				, ItemPotential, SoulBoundFlag, SealCount, ItemOption, ItemLifespan, EternityFlag, ItemExpireDate, ItemPotentialMoveCount
		FROM dbo.MaterializedItems WITH (NOLOCK, INDEX(IX_NNI_MaterializedItems_OwnerCharacterID_ItemLocationCode), FORCESEEK)
		WHERE OwnerCharacterID = @inbCharacterID
			AND ItemLocationCode IS NULL
		UNION ALL
		SELECT ItemSerial, ItemID, ItemRemainCount, ItemDurability, RandomSeed, CoolTime, ItemLevel
				, ItemPotential, SoulBoundFlag, SealCount, ItemOption, ItemLifespan, EternityFlag, ItemExpireDate, ItemPotentialMoveCount
		FROM dbo.MaterializedItems MI WITH (NOLOCK, INDEX(IX_NNI_MaterializedItems_OwnerCharacterID_ItemLocationCode), FORCESEEK)
		WHERE OwnerCharacterID = @inbCharacterID
			AND ItemLocationCode = 12
			AND NOT EXISTS (
				SELECT *
				FROM dbo.Trade WITH (NOLOCK, INDEX(IX_NNI_Trade_ItemSerial), FORCESEEK)
				WHERE ItemSerial = MI.ItemSerial AND TradeStatusCode = 0
			)
		OPTION (KEEPFIXED PLAN);
	END
	ELSE IF @intVersion = 5
	BEGIN
		SELECT ItemSerial, ItemID, ItemRemainCount, ItemDurability, RandomSeed, CoolTime, ItemLevel
				, ItemPotential, SoulBoundFlag, SealCount, ItemOption, ItemLifespan, EternityFlag, ItemExpireDate, ItemPotentialMoveCount, DragonJewelType
		FROM dbo.MaterializedItems WITH (NOLOCK, INDEX(IX_NNI_MaterializedItems_OwnerCharacterID_ItemLocationCode), FORCESEEK)
		WHERE OwnerCharacterID = @inbCharacterID
			AND ItemLocationCode IS NULL
		UNION ALL
		SELECT ItemSerial, ItemID, ItemRemainCount, ItemDurability, RandomSeed, CoolTime, ItemLevel
				, ItemPotential, SoulBoundFlag, SealCount, ItemOption, ItemLifespan, EternityFlag, ItemExpireDate, ItemPotentialMoveCount, DragonJewelType
		FROM dbo.MaterializedItems MI WITH (NOLOCK, INDEX(IX_NNI_MaterializedItems_OwnerCharacterID_ItemLocationCode), FORCESEEK)
		WHERE OwnerCharacterID = @inbCharacterID
			AND ItemLocationCode = 12
			AND NOT EXISTS (
				SELECT *
				FROM dbo.Trade WITH (NOLOCK, INDEX(IX_NNI_Trade_ItemSerial), FORCESEEK)
				WHERE ItemSerial = MI.ItemSerial AND TradeStatusCode = 0
			)
		OPTION (KEEPFIXED PLAN);
	END

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103216;
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
	EXEC @intReturnValue = dbo.P_AddErrorLog;

RETURN @intReturnValue;
GO

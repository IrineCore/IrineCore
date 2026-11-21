USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_GetListMaterializedItemsForMobile]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 9
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-03-22
description : ??????? ?????. (????)
result set: ItemLocationCode/tinyint, ItemLocationIndex/tinyint, ItemSerial/bigint, ItemID/int, ItemRemainCount/smallint, ItemDurability/smallint, ItemLevel/tinyint, ItemPotential/tinyint, SoulBoundFlag/bit, ItemOption/tinyint, SealCount/tinyint, 	EternityFlag/bit, ItemExpireDate/smalldatetime, AdditiveItemID/int, DragonJewelType/tinyint
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
	103102 = ???? ???? ????.
history :
	2 = ?????? ?? ??     by ??? at 2012-04-12
	4 = ??? ?? ?? ?? ??     by ??? at 2013-01-19
	8 = ?? ?? ?? ?? ??? ?? ?? ?? by ??? at 2013-06-11
	9 = ?? ?? ?? by ??? at 2013-06-11
**/

CREATE PROCEDURE [dbo].[P_DRS_GetListMaterializedItemsForMobile]	  
	  @inbCharacterID bigint --// ???? ??? ??? ID		
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
	IF EXISTS (SELECT * FROM dbo.Characters WITH (NOLOCK, INDEX(PK_Characters), FORCESEEK) WHERE CharacterID = @inbCharacterID AND DeleteFlag = CAST(0 AS bit))
	BEGIN		
		/**_# [MaterializedItems] ???? SELECT???.*/		
		SELECT ItemLocationCode, ItemLocationIndex, ItemSerial, ItemID, ItemRemainCount, ItemDurability, ItemLevel, ItemPotential
			, SoulBoundFlag, ItemOption, SealCount, EternityFlag, ItemExpireDate, ItemPotentialMoveCount, AdditiveItemID, DragonJewelType
		FROM dbo.MaterializedItems WITH (NOLOCK, INDEX (IX_NNI_MaterializedItems_OwnerCharacterID_ItemLocationCode), FORCESEEK)
		WHERE OwnerCharacterID = @inbCharacterID
			AND ItemLocationCode IN (1,7,8,19)
			AND NOT (ItemLocationCode = 1 AND (ItemLocationIndex = 1 OR ItemLocationIndex = 0))
			AND ItemRemainCount > 0 
		OPTION (KEEPFIXED PLAN);		
	END
	ELSE
	BEGIN
		SET @intReturnValue = 103102;
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

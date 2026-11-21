USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListPetSubV3]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2013-01-24
description : ?? ????? ??? ??? ??? ??????. (version 3)
result set : Seq/int, ItemLocationCode/tinyint, ItemSerial/bigint, ItemID/int, ItemRemainCount/smallint, ItemDurability/smallint, RandomSeed/int, CoolTime/int, ItemLevel/tinyint, ItemPotential/tinyint, SoulBoundFlag/bit, ItemOption/tinyint, SealCount/tinyint, ItemMaterializeDate/datetime2(0), EternityFlag/bit, ItemExpireDate/smalldatetime, AdditiveItemID/int, ItemLifespan/int, ExpireCompleteFlag/bit, ItemPotentialMoveCount/tinyint
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103151 = ???? ???? ????.
**/

CREATE PROCEDURE [dbo].[P_GetListPetSubV3]
	   @inbCharacterID bigint --// ??? ID
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
	SELECT ROW_NUMBER() OVER(ORDER BY OwnerCharacterID, ItemLocationCode) AS Seq
		, ItemLocationCode, ItemSerial, ItemID, ItemRemainCount, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential
		, SoulBoundFlag, ItemOption, SealCount, ItemMaterializeDate, EternityFlag, ItemExpireDate, AdditiveItemID, ItemLifespan, ExpireCompleteFlag, ItemPotentialMoveCount
	FROM dbo.MaterializedItems WITH (NOLOCK, INDEX(IX_NNI_MaterializedItems_OwnerCharacterID_ItemLocationCode), FORCESEEK)
	WHERE OwnerCharacterID = @inbCharacterID
		AND ItemLocationCode BETWEEN 9 AND 10
	ORDER BY OwnerCharacterID, ItemLocationCode
	OPTION (KEEPFIXED PLAN);

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
	EXEC @intReturnValue = dbo.P_AddErrorLog;
RETURN @intReturnValue;
GO

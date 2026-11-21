USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetPageMaterializedItemSubV2]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2013-01-24
description : ?? ????? ??? ??? ??? ??????. (version 2)
result set : Seq/int, ItemLocationCode/tinyint, ItemSerial/bigint, ItemID/int, ItemRemainCount/smallint, ItemDurability/smallint, RandomSeed/int, CoolTime/int, ItemLevel/tinyint, ItemPotential/tinyint, SoulBoundFlag/bit, ItemOption/tinyint, SealCount/tinyint, ItemMaterializeDate/datetime2(0), EternityFlag/bit, ItemExpireDate/smalldatetime, AdditiveItemID/int, ItemLifespan/int, ExpireCompleteFlag/bit
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103151 = ???? ???? ????.
**/

CREATE PROCEDURE [dbo].[P_GetPageMaterializedItemSubV2]
	  @intPageNumber int --// ??? ??.
	, @intPageSize int --// ??? ???.
	, @inbCharacterID bigint --// ??? ID
	, @inyItemLocationCode tinyint --// 5=?? ????
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
	;WITH IL AS (
		SELECT ROW_NUMBER() OVER(ORDER BY OwnerCharacterID, ItemLocationCode) AS Seq, MaterializedItemID
		FROM dbo.MaterializedItems WITH (NOLOCK, INDEX(IX_NNI_MaterializedItems_OwnerCharacterID_ItemLocationCode), FORCESEEK)
		WHERE OwnerCharacterID = @inbCharacterID
			AND ItemLocationCode = @inyItemLocationCode
	)
	SELECT IL.Seq, MI.ItemLocationCode, MI.ItemSerial, MI.ItemID, MI.ItemRemainCount, MI.ItemDurability, MI.RandomSeed, MI.CoolTime, MI.ItemLevel, MI.ItemPotential, MI.SoulBoundFlag
		, MI.ItemOption, MI.SealCount, MI.ItemMaterializeDate, MI.EternityFlag, MI.ItemExpireDate, MI.AdditiveItemID, MI.ItemLifespan, MI.ExpireCompleteFlag
	FROM IL
		INNER JOIN dbo.MaterializedItems MI WITH (NOLOCK, INDEX (PK_MaterializedItems), FORCESEEK) ON IL.MaterializedItemID = MI.MaterializedItemID
	WHERE IL.Seq BETWEEN (((@intPageNumber - 1) * @intPageSize) + 1) AND (@intPageNumber * @intPageSize)
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
	EXEC @intReturnValue = dbo.P_AddErrorLog;
RETURN @intReturnValue;
GO

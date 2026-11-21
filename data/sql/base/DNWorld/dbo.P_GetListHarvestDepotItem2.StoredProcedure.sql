USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListHarvestDepotItem2]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-07-12
description : ?? ??? ??? ??? ?????.
result set : MaterializedItemID/bigint, ItemSerial/bigint, ItemID/int, ItemRemainCount/smallint, ItemDurability/smallint, RandomSeed/int, CoolTime/int, ItemLevel/tinyint, ItemPotential/tinyint, SoulBoundFlag/bit, ItemOption/tinyint, SealCount/tinyint, ItemMaterializeDate/datetime2(0), EternityFlag/bit, ItemExpireDate/smalldatetime
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103151 = ???? ???? ????.
history :
2 = ????
3 = (50729) ??? ?? ???? ?? ?? by ??? at 2012-01-30 
**/

CREATE PROCEDURE [dbo].[P_GetListHarvestDepotItem2]
	  @inbCharacterID bigint --// ??? ID
	, @inbMaterializedItemID bigint = 0 --// ? ??? ? ?? ?? ???? ?????. (?? 0? ?????.)
	, @inbRowCount int = 101 --// ??? ??? ?
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
	/**_# ?? ??? ??? ??? ?????.*/
	SELECT TOP (@inbRowCount) MaterializedItemID, ItemSerial, ItemID, ItemRemainCount
		, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag
		, ItemOption, SealCount, ItemMaterializeDate, EternityFlag, ItemExpireDate
	FROM dbo.MaterializedItems WITH (NOLOCK, INDEX(IX_NNI_MaterializedItems_OwnerCharacterID_ItemLocationCode), FORCESEEK)
	WHERE OwnerCharacterID = @inbCharacterID
		AND ItemLocationCode = 11
		AND MaterializedItemID > @inbMaterializedItemID
	ORDER BY MaterializedItemID
	OPTION (KEEPFIXED PLAN);

	/**_# ???? ??? 103151? ?????.*/
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

USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetMaterializedItem2]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-07-12

description :
	?? ??? ???? ?? ???? ?????.
result set :
	@intVersion < 3 : ItemSerial/bigint, ItemID/int, ItemRemainCount/smallint, ItemDurability/smallint, RandomSeed/int, CoolTime/int, ItemLevel/tinyint, ItemPotential/tinyint, SoulBoundFlag/bit, ItemOption/tinyint, SealCount/tinyint
	@intVersion = 3 : ItemSerial/bigint, ItemID/int, ItemRemainCount/smallint, ItemDurability/smallint, RandomSeed/int, CoolTime/int, ItemLevel/tinyint, ItemPotential/tinyint, SoulBoundFlag/bit, ItemOption/tinyint, SealCount/tinyint, ItemPotentialMoveCount/tinyint
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
	103151 = ???? ???? ????.
history :
	2 = (50729) ??? ?? ???? ?? ?? by ??? at 2012-01-30 
	3 = ?? ?? ?? ?? (@intVersion = 3)     by ??? at 2012-04-12
**/

CREATE PROCEDURE [dbo].[P_GetMaterializedItem2]
	  @inbCharacterID bigint --// ??? ID
	, @inyItemLocationCode tinyint --// 1=????, 2=????, 3=??, 4=??? ????, 5=?? ????, 7=?? ????
	, @inyItemLocationIndex tinyint --// ??? ?? ??? ??
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

	IF @intVersion < 3
	BEGIN
		SELECT MI.ItemSerial, MI.ItemID, MI.ItemRemainCount, MI.ItemDurability, MI.RandomSeed, MI.CoolTime, MI.ItemLevel, MI.ItemPotential, MI.SoulBoundFlag, MI.ItemOption, MI.SealCount
		FROM dbo.MaterializedItems MI WITH (NOLOCK, INDEX(IX_NNI_MaterializedItems_OwnerCharacterID_ItemLocationCode), FORCESEEK)
		WHERE MI.OwnerCharacterID = @inbCharacterID AND MI.ItemLocationCode = @inyItemLocationCode AND MI.ItemLocationIndex = @inyItemLocationIndex
		OPTION (KEEPFIXED PLAN);
	END
	ELSE IF @intVersion = 3
	BEGIN
		SELECT MI.ItemSerial, MI.ItemID, MI.ItemRemainCount, MI.ItemDurability, MI.RandomSeed, MI.CoolTime, MI.ItemLevel, MI.ItemPotential, MI.SoulBoundFlag, MI.ItemOption, MI.SealCount, MI.ItemPotentialMoveCount
		FROM dbo.MaterializedItems MI WITH (NOLOCK, INDEX(IX_NNI_MaterializedItems_OwnerCharacterID_ItemLocationCode), FORCESEEK)
		WHERE MI.OwnerCharacterID = @inbCharacterID AND MI.ItemLocationCode = @inyItemLocationCode AND MI.ItemLocationIndex = @inyItemLocationIndex
		OPTION (KEEPFIXED PLAN);
	END

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

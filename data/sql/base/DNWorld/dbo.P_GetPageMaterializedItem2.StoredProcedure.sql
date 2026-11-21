USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetPageMaterializedItem2]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 9
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-07-12
description : ?? ????? ??? ??? ??? ??????.
result set : 
	@intVersion < 2 : Seq/int, ItemLocationCode/tinyint, ItemSerial/bigint, ItemID/int, ItemRemainCount/smallint, ItemDurability/smallint, RandomSeed/int, CoolTime/int, ItemLevel/tinyint, ItemPotential/tinyint, SoulBoundFlag/bit, ItemOption/tinyint, SealCount/tinyint, ItemMaterializeDate/datetime2(0), EternityFlag/bit, ItemExpireDate/smalldatetime, AdditiveItemID/int
	@intVersion = 2 : Seq/int, ItemLocationCode/tinyint, ItemSerial/bigint, ItemID/int, ItemRemainCount/smallint, ItemDurability/smallint, RandomSeed/int, CoolTime/int, ItemLevel/tinyint, ItemPotential/tinyint, SoulBoundFlag/bit, ItemOption/tinyint, SealCount/tinyint, ItemMaterializeDate/datetime2(0), EternityFlag/bit, ItemExpireDate/smalldatetime, AdditiveItemID/int, ItemLifespan/int, ExpireCompleteFlag/bit
	@intVersion = 3 : Seq/int, ItemLocationCode/tinyint, ItemSerial/bigint, ItemID/int, ItemRemainCount/smallint, ItemDurability/smallint, RandomSeed/int, CoolTime/int, ItemLevel/tinyint, ItemPotential/tinyint, SoulBoundFlag/bit, ItemOption/tinyint, SealCount/tinyint, ItemMaterializeDate/datetime2(0), EternityFlag/bit, ItemExpireDate/smalldatetime, AdditiveItemID/int, ItemLifespan/int, ExpireCompleteFlag/bit, ItemPotentialMoveCount/tinyint
	@intVersion = 4 : Version 3 + DragonJewelType/tinyint
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
	103151 = ???? ???? ????.
history :
	3 = (50729) ??? ?? ???? ?? ?? by ??? at 2012-01-30 
	3 = ?? ?? ?? ?? (@intVersion = 3)     by ??? at 2012-04-12
	6 = ??? ???? ?????? ?? by ??? at 2013-01-05
	7 = 6 ?? ?? ?? by ??? at 2013-01-05
**/

CREATE PROCEDURE [dbo].[P_GetPageMaterializedItem2]
	  @intPageNumber int --// ??? ??.
	, @intPageSize int --// ??? ???.
	, @inbCharacterID bigint --// ??? ID
	, @inyItemLocationCode tinyint --// 5=?? ????
	, @intVersion int = 1 --// ??? SP? ??
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
	IF @intVersion = 4
	BEGIN
		IF @inyItemLocationCode = 10
		BEGIN
			EXEC @intReturnValue = dbo.P_GetListPetSubV4
				  @inbCharacterID = @inbCharacterID;

			IF @intReturnValue <> 0
				GOTO ErrorHandler;
		END

		ELSE
		BEGIN
			EXEC @intReturnValue = dbo.P_GetPageMaterializedItemSubV4
				  @intPageNumber = @intPageNumber
				, @intPageSize = @intPageSize
				, @inbCharacterID = @inbCharacterID
				, @inyItemLocationCode = @inyItemLocationCode;

			IF @intReturnValue <> 0
				GOTO ErrorHandler;
		END
	END

	ELSE IF @intVersion = 3
	BEGIN
		IF @inyItemLocationCode = 10
		BEGIN
			EXEC @intReturnValue = dbo.P_GetListPetSubV3
				  @inbCharacterID = @inbCharacterID;

			IF @intReturnValue <> 0
				GOTO ErrorHandler;
		END

		ELSE
		BEGIN
			EXEC @intReturnValue = dbo.P_GetPageMaterializedItemSubV3
				  @intPageNumber = @intPageNumber
				, @intPageSize = @intPageSize
				, @inbCharacterID = @inbCharacterID
				, @inyItemLocationCode = @inyItemLocationCode;

			IF @intReturnValue <> 0
				GOTO ErrorHandler;
		END
	END

	ELSE IF @intVersion = 2
	BEGIN
		IF @inyItemLocationCode = 10
		BEGIN
			EXEC @intReturnValue = dbo.P_GetListPetSubV2
				  @inbCharacterID = @inbCharacterID;

			IF @intReturnValue <> 0
				GOTO ErrorHandler;
		END

		ELSE
		BEGIN
			EXEC @intReturnValue = dbo.P_GetPageMaterializedItemSubV2
				  @intPageNumber = @intPageNumber
				, @intPageSize = @intPageSize
				, @inbCharacterID = @inbCharacterID
				, @inyItemLocationCode = @inyItemLocationCode;

			IF @intReturnValue <> 0
				GOTO ErrorHandler;
		END
	END

	ELSE IF @intVersion < 2
	BEGIN
		IF @inyItemLocationCode = 10
		BEGIN
			EXEC @intReturnValue = dbo.P_GetListPetSubV1
				  @inbCharacterID = @inbCharacterID;

			IF @intReturnValue <> 0
				GOTO ErrorHandler;
		END

		ELSE
		BEGIN
			EXEC @intReturnValue = dbo.P_GetPageMaterializedItemSubV1
				  @intPageNumber = @intPageNumber
				, @intPageSize = @intPageSize
				, @inbCharacterID = @inbCharacterID
				, @inyItemLocationCode = @inyItemLocationCode;

			IF @intReturnValue <> 0
				GOTO ErrorHandler;
		END
	END

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

USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListMaterializedItem2]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 20
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-07-12
description : ??? ??? ??? ?????.
result set : 
	@intVersion = 2 : ItemLocationCode/tinyint, ItemLocationIndex/tinyint, ItemSerial/bigint, ItemID/int, ItemRemainCount/smallint, ItemDurability/smallint, RandomSeed/int, CoolTime/int, ItemLevel/tinyint, ItemPotential/tinyint, SoulBoundFlag/bit, ItemOption/tinyint, SealCount/tinyint, ItemMaterializeDate/datetime2(0), EternityFlag/bit, ItemExpireDate/smalldatetime, AdditiveItemID/int
	@intVersion = 3 : ItemLocationCode/tinyint, ItemLocationIndex/tinyint, ItemSerial/bigint, ItemID/int, ItemRemainCount/smallint, ItemDurability/smallint, RandomSeed/int, CoolTime/int, ItemLevel/tinyint, ItemPotential/tinyint, SoulBoundFlag/bit, ItemOption/tinyint, SealCount/tinyint, ItemMaterializeDate/datetime2(0), EternityFlag/bit, ItemExpireDate/smalldatetime, AdditiveItemID/int, ItemLifespan/int
	@intVersion = 4 : ItemLocationCode/tinyint, ItemLocationIndex/tinyint, ItemSerial/bigint, ItemID/int, ItemRemainCount/smallint, ItemDurability/smallint, RandomSeed/int, CoolTime/int, ItemLevel/tinyint, ItemPotential/tinyint, SoulBoundFlag/bit, ItemOption/tinyint, SealCount/tinyint, ItemMaterializeDate/datetime2(0), EternityFlag/bit, ItemExpireDate/smalldatetime, AdditiveItemID/int, ItemLifespan/int, ItemPotentialMoveCount/tinyint	
	@intVersion = 5 : Version 4 + DragonJewelType/tinyint
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103151 = ???? ???? ????.
history :
2 = (50729) ??? ?? ???? ?? ?? by ??? at 2012-01-30
3 = (52680) input param. @intVersion ?? by ??? at 2012-02-28
4 = (52315) ?? ?? ??? ?? ????? ?? (@intVersion = 4)     by ??? at 2012-02-29
5~6 = [dbo.MaterializedItems]? ItemPotentialMoveCount(?? ?? ??) ?? ?? (@intVersion = 4)     by ??? at 2012-04-09
11 = ItemLocationCode = 19 ??? ????? ??(@intVersion = 4)     by ??? at 2012-12-20
14 = ??? ???? ?????? ?? by ??? at 2013-01-05
15 = 14 ?? ?? ?? by ??? at 2013-01-05
16 = Sub SP ????? ?? by ??? at 2013-01-24
17 = ??? ??, ??? ?? ?? by ??? at 2013-01-28
**/

CREATE PROCEDURE [dbo].[P_GetListMaterializedItem2]
	  @inbCharacterID bigint --// ??? ID
	, @vchItemLocationCodes varchar(8000) --// ,? ?????? ItemLocationCode ??? (1=????, 2=????, 3=??, 4=??? ????, 5=?? ????, 7=?? ????, 17=?? ??, 18=?? ?? ??, 20=??? ????, 21=??? ??)
	, @intVersion int = 2 --// ??? SP? ??
	, @intAccountID int = NULL --// ?? ID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @inyItemLocationCode tinyint;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SET @inyItemLocationCode = 
		CASE
			WHEN CHARINDEX(',', @vchItemLocationCodes) = 0 THEN CAST(@vchItemLocationCodes AS tinyint)
			ELSE NULL
		END;
	IF @intVersion = 5
	BEGIN
		EXEC @intReturnValue = dbo.P_GetListMaterializedItemSubV5
			  @inbCharacterID = @inbCharacterID
			, @inyItemLocationCode = @inyItemLocationCode
			, @intAccountID = @intAccountID;

		IF @intReturnValue <> 0
			GOTO ErrorHandler;
	END

	ELSE IF @intVersion = 4
	BEGIN
		EXEC @intReturnValue = dbo.P_GetListMaterializedItemSubV4
			  @inbCharacterID = @inbCharacterID
			, @inyItemLocationCode = @inyItemLocationCode
			, @intAccountID = @intAccountID;

		IF @intReturnValue <> 0
			GOTO ErrorHandler;
	END

	ELSE IF @intVersion = 3
	BEGIN
		EXEC @intReturnValue = dbo.P_GetListMaterializedItemSubV3
			  @inbCharacterID = @inbCharacterID
			, @inyItemLocationCode = @inyItemLocationCode
			, @intAccountID = @intAccountID;

		IF @intReturnValue <> 0
			GOTO ErrorHandler;
	END

	ELSE IF @intVersion = 2
	BEGIN
		EXEC @intReturnValue = dbo.P_GetListMaterializedItemSubV2
			  @inbCharacterID = @inbCharacterID
			, @inyItemLocationCode = @inyItemLocationCode
			, @intAccountID = @intAccountID;

		IF @intReturnValue <> 0
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

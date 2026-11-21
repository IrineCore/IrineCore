USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListRepurchaseItem]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 11
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2011-10-19

description :
	??? ??? ??? ??
result set : 
	-- ??? ??
	ItemLocationCode (null)
	ItemLocationIndex (null)
	ItemSerial / bigint
	ItemID / int
	ItemRemainCount / smallint
	ItemDurability / smallint
	RandomSeed / int
	CoolTime / int
	ItemLevel / tinyint
	ItemPotential / tinyint
	SoulBoundFlag / bit
	ItemOption / tinyint
	SealCount / tinyint
	ItemMaterializeDate / datetime2(0)
	EternityFlag / bit
	ItemExpireDate / smalldatetime
	AdditiveItemID / int
	-- ??? ??
	RepurchaseID / int / ?????
	SellDate / datetime / ????
	SellPrice / int / ????
	-- @intVersion = 8 ??
	ItemPotentialMoveCount / tinyint
	-- @intVersion = 9 ??
	DragonJewelType / tinyint
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
	7 = ?? ??
	8~10 = ?? ?? ?? ?? (@intVersion = 8 ? ??)     by ??? at 2012-04-09
**/

CREATE PROCEDURE [dbo].[P_GetListRepurchaseItem]
	  @inbCharacterID bigint --// ???ID
	, @intVersion int = 7 --// ??? SP? ??
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

	IF @intVersion <= 7
	BEGIN
		SELECT TOP 30 null as ItemLocationCode, null as ItemLocationIndex, ItemSerial, ItemID, ItemRemainCount, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, ItemOption, SealCount, ItemMaterializeDate, EternityFlag, ItemExpireDate, AdditiveItemID, RepurchaseID, SellDate, SellPrice
		  FROM dbo.RepurchaseItem WITH (NOLOCK)
		 WHERE CharacterID = @inbCharacterID
		   AND SellDate >= DATEADD(HOUR, -4, GETDATE())
		 ORDER BY SellDate DESC;
	END
	ELSE IF @intVersion = 8
	BEGIN
		SELECT TOP 30 null as ItemLocationCode, null as ItemLocationIndex, ItemSerial, ItemID, ItemRemainCount, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, ItemOption, SealCount, ItemMaterializeDate, EternityFlag, ItemExpireDate, AdditiveItemID, RepurchaseID, SellDate, SellPrice, ItemPotentialMoveCount
		  FROM dbo.RepurchaseItem WITH (NOLOCK)
		 WHERE CharacterID = @inbCharacterID
		   AND SellDate >= DATEADD(HOUR, -4, GETDATE())
		 ORDER BY SellDate DESC;
	END
	ELSE IF @intVersion = 9
	BEGIN
		SELECT TOP 30 null as ItemLocationCode, null as ItemLocationIndex, ItemSerial, ItemID, ItemRemainCount, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, ItemOption, SealCount, ItemMaterializeDate, EternityFlag, ItemExpireDate, AdditiveItemID, RepurchaseID, SellDate, SellPrice, ItemPotentialMoveCount, DragonJewelType
		  FROM dbo.RepurchaseItem WITH (NOLOCK)
		 WHERE CharacterID = @inbCharacterID
		   AND SellDate >= DATEADD(HOUR, -4, GETDATE())
		 ORDER BY SellDate DESC;
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

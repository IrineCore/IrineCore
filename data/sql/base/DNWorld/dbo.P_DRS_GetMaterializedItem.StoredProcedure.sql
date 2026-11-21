USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_GetMaterializedItem]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 7
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2012-01-25

description :
	???? ??? ??? ?? ??
result set :
	ItemID / int
	ItemRemainCount / smallint
	PayMethodCode / tinyint
	SenderCharacterID / bigint
	ItemMaterializeCode / tinyint
	ItemMaterializeFKey / bigint
	ItemDurability / smallint
	RandomSeed / int
	CoolTime / int
	ItemLevel / tinyint
	ItemPotential / tinyint
	SoulBoundFlag / bit
	SealCount / tinyint
	ItemOption / tinyint
	ItemMaterializeDate / datetime2(0)
	ItemLifespan / int
	EternityFlag / bit
	ItemExpireDate / smalldatetime
	OwnerCharacterID / bigint
	OwnershipStartDate / datetime2(0)
	ExpireCompleteFlag / bit
	TradeItemFlag / bit
	AdditiveItemID / int
	ItemLocationCode / tinyint
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
	103151 = ???? ???? ????.
history :
	1~4 = ?? ??
	5 = ?????? ?? ??     by ??? at 2012-04-12
**/

CREATE PROCEDURE [dbo].[P_DRS_GetMaterializedItem]
	  @inbCharacterID bigint --// ??? ID
	, @inbItemSerial bigint --// ??? ?? ??	
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

	SELECT ItemID, ItemRemainCount, PayMethodCode, SenderCharacterID, ItemMaterializeCode, ItemMaterializeFKey, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, ItemMaterializeDate, ItemLifespan, EternityFlag, ItemExpireDate, OwnerCharacterID, OwnershipStartDate, ExpireCompleteFlag, TradeItemFlag, AdditiveItemID, ItemLocationCode, ItemPotentialMoveCount
	  FROM dbo.MaterializedItems WITH (NOLOCK, INDEX(IX_UNI_MaterializedItems_ItemSerial), FORCESEEK)
	 WHERE OwnerCharacterID = @inbCharacterID
	   AND ItemSerial = @inbItemSerial
	   AND ItemRemainCount > 0;

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

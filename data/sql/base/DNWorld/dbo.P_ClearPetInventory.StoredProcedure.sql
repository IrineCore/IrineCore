USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ClearPetInventory]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-07-21
description : ??/? ????? ???? ?????. (??? ?)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_ClearPetInventory]
	  @inbCharacterID bigint --// ??? ID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @tblTemp table (
	ItemSerial bigint NOT NULL
);
DECLARE @tblTemp2 table (
	ItemSerial bigint NULL
);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	DELETE dbo.MaterializedItems
	OUTPUT deleted.ItemSerial INTO @tblTemp (ItemSerial)
	WHERE OwnerCharacterID = @inbCharacterID AND ItemLocationCode BETWEEN 9 AND 10;

	DELETE dbo.PetEquipments
	OUTPUT deleted.PetEquipmentItemSerial INTO @tblTemp2 (ItemSerial)
	FROM dbo.PetEquipments PE
		INNER JOIN @tblTemp T ON PE.PetItemSerial = T.ItemSerial;

	DELETE dbo.MaterializedItems
	FROM @tblTemp2 T
		INNER JOIN dbo.MaterializedItems MI WITH(INDEX(IX_UNI_MaterializedItems_ItemSerial), FORCESEEK) ON MI.ItemSerial = T.ItemSerial
	WHERE T.ItemSerial IS NOT NULL
	OPTION(KEEPFIXED PLAN);
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

USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListEquipment2]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-07-12
description : ??? ??? ??? ?????.
result set : ItemLocationCode/tinyint, ItemLocationIndex/tinyint, ItemID/int, AdditiveItemID/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103103 = Equip? ????.
**/

CREATE PROCEDURE [dbo].[P_GetListEquipment2]
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
	/**_# ??? ??? ??? ?????.*/
		SELECT ItemLocationCode, ItemLocationIndex, ItemID, AdditiveItemID
		FROM dbo.MaterializedItems WITH (NOLOCK, INDEX(IX_UNF_MaterializedItems_OwnerCharacterID_ItemLocationCode_ItemLocationIndex), FORCESEEK)
		WHERE OwnerCharacterID = @inbCharacterID
			AND ItemLocationCode IN (1, 7)
			AND ItemLocationIndex < 200
		OPTION (KEEPFIXED PLAN);

	/**_# Equipment? ????? 103103? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103103;
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

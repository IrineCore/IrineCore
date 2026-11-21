USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_TakeCashEquipmentOff2]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-07-11
description : ?? ???? ???? ???? ?????. (?? ???? -> ?? ????)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
2 = TF_Split(), TF_NSplit() ??? ????? ?? by ??? at 2011-11-02
**/

CREATE PROCEDURE [dbo].[P_TakeCashEquipmentOff2]
	  @inbCharacterID bigint --// ??? ID
	, @vchCashEquipmentIndexes varchar(50) --// ??? ?? ???? ???? ???. ???? ,???.
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @tblCashEquipmentIndexes table (ItemLocationIndex tinyint NOT NULL);
DECLARE @tblMaterializedItemIDs table (MaterializedItemID bigint NOT NULL);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	INSERT @tblCashEquipmentIndexes (ItemLocationIndex)
	SELECT CAST(string AS tinyint)
	FROM dbo.TF_Split(@vchCashEquipmentIndexes, ',')
	WHERE string > ''
	OPTION (KEEPFIXED PLAN);

	UPDATE dbo.MaterializedItems
	SET ItemLocationCode = 5
	FROM dbo.MaterializedItems MI
		INNER JOIN @tblCashEquipmentIndexes T ON MI.ItemLocationIndex = T.ItemLocationIndex
	WHERE MI.OwnerCharacterID = @inbCharacterID AND MI.ItemLocationCode = 7;
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

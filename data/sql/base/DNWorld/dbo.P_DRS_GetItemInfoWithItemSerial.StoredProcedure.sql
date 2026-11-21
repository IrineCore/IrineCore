USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_GetItemInfoWithItemSerial]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-01-07
description : ??? ???? ??? ??? ?????.
result set : ItemID/int, ItemDurability/smallint, ItemLevel/tinyint, ItemPotential/tinyint, ItemOption/tinyint, SealCount/tinyint, ItemPotentialMoveCount/tinyint, AdditiveItemID/int, DragonJewelType/tinyint
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_DRS_GetItemInfoWithItemSerial]
	  @inbItemSerial bigint --// ??? ?? ??	
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
	/**_# [MaterializedItems] ???? SELECT???.*/	
	;WITH A AS (
		SELECT MaterializedItemID
		  FROM dbo.MaterializedItems WITH (NOLOCK, INDEX(IX_UNI_MaterializedItems_ItemSerial), FORCESEEK)
		 WHERE ItemSerial = @inbItemSerial
	)
	SELECT MI.ItemID, MI.ItemDurability, MI.ItemLevel, MI.ItemPotential, MI.ItemOption, MI.SealCount, MI.ItemPotentialMoveCount, MI.AdditiveItemID, MI.DragonJewelType
	  FROM A INNER JOIN dbo.MaterializedItems MI WITH (NOLOCK) ON A.MaterializedItemID = MI.MaterializedItemID;

END TRY
BEGIN CATCH	GOTO ErrorHandler;
END CATCH;

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

IF @intReturnValue IS NULL OR @intReturnValue = 0
	EXEC @intReturnValue = dbo.P_AddErrorLog;

RETURN @intReturnValue;
GO

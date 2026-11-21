USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DLS_MergeAuctionEnableItem]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-03-15
description : ?? ???? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_DLS_MergeAuctionEnableItem]	  
WITH EXECUTE AS 'SPExecutor'
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

	/**_# [AuctionEnableItemsFromDNT] ???? Merge???.*/
	MERGE dbo.AuctionEnableItemsFromDNT AS A
	USING dbo.ImportedAuctionEnableItems AS B
	ON A.ItemID = B.ItemID	
	WHEN NOT MATCHED BY TARGET THEN
		INSERT (ItemID, ItemCategoryID, ItemName, ActivateFlag, MaxCount)
		VALUES (B.ItemID, B.ItemCategoryID, B.ItemName, 1, B.MaxCount)
	WHEN NOT MATCHED BY SOURCE THEN
		DELETE
	OPTION (KEEPFIXED PLAN, LOOP JOIN);

END TRY
BEGIN CATCH	
	GOTO ErrorHandler;
END CATCH;

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

IF @intReturnValue IS NULL OR @intReturnValue = 0
	EXEC @intReturnValue = dbo.P_AddErrorLog @intPROCID = @@PROCID;

RETURN @intReturnValue;
GO

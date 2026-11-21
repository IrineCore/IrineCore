USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_GetTradeItemInfo]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 7
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-04-25
description : ???? ??? ???? ????? ?????.
result set : 
	TradeID/int, 
	CharacterID/bigint, 
	ItemID/int, 
	ItemLevel/tinyint, 
	ItemCount/smallint, 
	Price/int, 
	ListPrice/int, 
	ItemDurability/smallint, 
	ItemPotential/tinyint, 
	ItemOption/tinyint, 
	SealCount/tinyint, 
	ItemPotentialMoveCount/tinyint,
	CharacterName/nvarchar(30),
	AdditiveItemID/int,
	DragonJewelType/tinyint, 
	ItemSerial/bigint
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_DRS_GetTradeItemInfo]	  	
	 @intTradeID int --// ?? ID
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
	/**_# [Trade] ???? Select ???.*/
	SELECT	T.TradeID, T.CharacterID, T.ItemID, T.ItemLevel, T.ItemCount, T.Price, T.ListPrice, T.ItemDurability, T.ItemPotential, T.ItemOption, T.SealCount, T.ItemPotentialMoveCount, CH.CharacterName, MI.AdditiveItemID, MI.DragonJewelType, T.ItemSerial
	FROM dbo.Trade T WITH (NOLOCK, INDEX(PK_Trade), FORCESEEK) INNER JOIN dbo.Characters CH WITH (NOLOCK) ON T.CharacterID = CH.CharacterID
	     INNER JOIN dbo.MaterializedItems MI WITH (NOLOCK, INDEX(IX_UNI_MaterializedItems_ItemSerial), FORCESEEK) ON T.ItemSerial = MI.ItemSerial
	WHERE T.TradeID = @intTradeID;
	
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

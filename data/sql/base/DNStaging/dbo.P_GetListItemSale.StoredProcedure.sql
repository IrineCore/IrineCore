USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListItemSale]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-07-12
description : ?? ???? ?? ?? ??? ?????.
result set : BasisDate, WorldName, ItemID, ItemName, ItemRankCode, LevelLimit, CharacterCount, SaleCount, ItemCount
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListItemSale]
	  @datStartDate date --// (??) ?? ?? ??
	, @datEndDate date --// (??) ?? ? ??
	, @intWorldID int = NULL --// (optional) ?? ID. NULL ??? ??
	, @intItemID int = NULL --// (optional) ??? ID. NULL ??? ??
	, @inySaleDirectionCode tinyint --// (??) 1=NPC??? ??, 2=NPC?? ??
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
	SELECT ST.BasisDate, DW.WorldName, DI.ItemID, DI.ItemName, DI.ItemRankCode, DI.LevelLimit, ST.CharacterCount, ST.SaleCount, ST.ItemCount
	FROM dbo.ST_ItemSales ST WITH (NOLOCK)
		INNER JOIN dbo.DimWorlds DW WITH (NOLOCK) ON ST.WorldID = DW.WorldID
		INNER JOIN dbo.DimItems DI WITH (NOLOCK) ON ST.ItemID = DI.ItemID
	WHERE ST.SaleDirectionCode = @inySaleDirectionCode
		AND ST.BasisDate BETWEEN @datStartDate AND @datEndDate
		AND ST.WorldID = COALESCE(@intWorldID, ST.WorldID)
		AND ST.ItemID = COALESCE(@intItemID, ST.ItemID)
	ORDER BY ST.BasisDate, ST.WorldID, ST.ItemID;
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

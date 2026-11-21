USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_GetTradeCountNMinPrice]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-03-26
description : ???? ??? ????? ?? ???? ?????.
result set : 
	TradeCount / int / ????,
	MinPrice / int / ?? ???(??)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_DRS_GetTradeCountNMinPrice]	  
	  @intItemID int --// ??? ID	
	, @inyItemLevel tinyint = NULL --// ??? ??	(Optional)		
	, @inyItemPotential tinyint = NULL --// ??? (???? ??)
	, @inyItemOption tinyint = NULL --// ??? ?? (???? ??)
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
	/**_# [Trade] ????? ??? ???? ?? ??? ?? ??(??)? ?????.*/
	SELECT COUNT(*) AS TradeCount, ISNULL(MIN(ListPrice),0) AS MinPrice
	FROM dbo.Trade WITH (NOLOCK, INDEX(IX_NNIF_Trade_ItemID), FORCESEEK)
	WHERE TradeStatusCode = 0
		AND TradeExpireDate >= GETDATE()
		AND ItemID = @intItemID
		AND ItemLevel = ISNULL(@inyItemLevel, ItemLevel)
	OPTION (KEEPFIXED PLAN);
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

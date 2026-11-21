USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListTradeSuccessLogForETL]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 7
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-04-21
description : ETL? ??? ?? ?? ??? ?????.
result set : TradeDate, TradeID, BuyerCharacterID, ItemID, ItemSerial, ItemLevel, SealCount, ItemDurability, ItemCount, Price, WorldID, CharacterID, PayMethodCode, TradeStatusCode, RandomSeed
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
5 = result set ?? : PayMethodCode     by 20110405.wgkim
6 = result set ?? : TradeStatusCode     by 20110405.ohtack
7 = result set ?? : RandomSeed by ??? at 2012-02-15
**/
CREATE PROCEDURE [dbo].[P_GetListTradeSuccessLogForETL]
	  @dtmBasisDate datetime
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @intWorldID int;;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT TOP (1) @intWorldID = WorldID FROM dbo.WorldCoverage WITH (NOLOCK) ORDER BY WorldID;

	SELECT TradeDate, TradeID, BuyerCharacterID, ItemID, ItemSerial, ItemLevel, SealCount, ItemDurability, ItemCount, Price, @intWorldID AS WorldID, CharacterID, PayMethodCode, TradeStatusCode, RandomSeed
	FROM dbo.Trade WITH (NOLOCK)
	WHERE TradeStatusCode BETWEEN 1 AND 2 AND TradeDate >= CAST(@dtmBasisDate AS datetime2(0)) AND TradeDate < CAST(@dtmBasisDate + 1 AS datetime2(0))
	OPTION (MAXDOP 1);
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

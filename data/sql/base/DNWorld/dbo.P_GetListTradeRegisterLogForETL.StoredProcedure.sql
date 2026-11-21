USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListTradeRegisterLogForETL]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-04-21
description : ETL? ??? ?? ??? ?????.
result set : RegisterDate, TradeID, CharacterID, ItemID, ItemSerial, ItemLevel, SealCount, ItemDurability, ItemCount, Price, WorldID, PayMethodCode
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
4 = result set ?? : PayMethodCode     by 20110405.wgkim
**/

CREATE PROCEDURE [dbo].[P_GetListTradeRegisterLogForETL]
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

	SELECT RegisterDate, TradeID, CharacterID, ItemID, ItemSerial, ItemLevel, SealCount, ItemDurability, ItemCount, Price, @intWorldID AS WorldID, PayMethodCode
	FROM dbo.Trade WITH (NOLOCK)
	WHERE RegisterDate >= CAST(@dtmBasisDate AS datetime2(0)) AND RegisterDate < CAST(@dtmBasisDate + 1 AS datetime2(0))
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

USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListGameMoneyTradeForETL]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-05-21
description : C2C ???? ????? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListGameMoneyTradeForETL]
	  @dtmBasisDate datetime --// ?? ??
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
	SELECT GameMoneyTradeKey, CharacterID, GameMoneyAmount, IncomeDate, ToCharacterID, TradeDate, PaidCashAmount, TotalCashAmount
	FROM dbo.GameMoneyTrade WITH(NOLOCK)
	WHERE TradeDate >= @dtmBasisDate AND TradeDate < DATEADD(DAY, 1, @dtmBasisDate)
      AND GameMoneyTradeStatusCode = 2
	OPTION(MAXDOP 1);

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

USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListAutoTradeResult]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-08-19
description : ?? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListAutoTradeResult]
	  @dt2SDate datetime2(0) --// ?? ?? ??
	, @dt2EDate datetime2(0) --// ?? ? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @intSTradeID int, @intETradeID int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT @intSTradeID = MIN(TradeID), @intETradeID = MAX(TradeID)
	FROM dbo.AutoTradeResults WITH (NOLOCK, INDEX(IX_NN_AutoTradeResults_TradeDate), FORCESEEK)
	WHERE TradeDate BETWEEN @dt2SDate AND @dt2EDate;

	SELECT ATR.ItemID, CH.CharacterID, CH.CharacterName, ATR.UnitPrice, ATR.UsedBudget, ATR.TradeDate
	FROM dbo.AutoTradeResults ATR WITH (NOLOCK, INDEX(PK_AutoTradeResults), FORCESEEK)
		INNER JOIN dbo.Characters CH WITH (NOLOCK, INDEX(PK_Characters), FORCESEEK) ON ATR.CharacterID = CH.CharacterID
	WHERE ATR.TradeID BETWEEN @intSTradeID AND @intETradeID
	OPTION (FORCE ORDER, LOOP JOIN, KEEPFIXED PLAN);
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

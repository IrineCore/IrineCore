USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DLS_ModTradeStatusCode]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-04-12
description : ???? ?? ????? ?????. (???)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103403 = ?? ??? ?????.
103404 = ?? ???? ?? ????? ?????.
**/

CREATE PROCEDURE [dbo].[P_DLS_ModTradeStatusCode]
	  @intTradeID int --// ?? ID
	, @inyTradeStatusCode tinyint --// 0=???, 1=????(??? ??), 2=????(??? ??), 3=????, 4=??? ??? ??	    
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @inyCurrentTradeStatusCode tinyint;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	SELECT @inyCurrentTradeStatusCode = TradeStatusCode
	FROM dbo.Trade WITH (NOLOCK)
	WHERE TradeID = @intTradeID;

	IF @inyCurrentTradeStatusCode = 2
	BEGIN
		SET @intReturnValue = 103403;
		GOTO ErrorHandler;
	END

	IF @inyCurrentTradeStatusCode = 4
	BEGIN
		SET @intReturnValue = 103404;
		GOTO ErrorHandler;
	END
	
	/**_# [Trade] ???? UPDATE???.*/
	UPDATE dbo.Trade
	SET TradeStatusCode = @inyTradeStatusCode
	WHERE TradeID = @intTradeID;

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

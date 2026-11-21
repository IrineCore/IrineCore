USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetTradeItemID]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-06-22
description : ?? ??? ID? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
2 = output param. @inyPayMethodCode, @intPrice ?? at 2011-01-24
**/

CREATE PROCEDURE [dbo].[P_GetTradeItemID]
	  @intTradeID int --// ?? ID
	, @intItemID int OUTPUT --// ??? ID
	, @inyPayMethodCode tinyint = 1 OUTPUT --// 1=????, 3=??
	, @intPrice int = 0 OUTPUT --// ??
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
	SELECT @intItemID = ItemID, @inyPayMethodCode = PayMethodCode, @intPrice = Price
	FROM dbo.Trade WITH (NOLOCK)
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
	EXEC @intReturnValue = dbo.P_AddErrorLog;

RETURN @intReturnValue;
GO

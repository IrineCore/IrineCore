USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddHiddenCashShopLog]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-07-23
description : ?? ??? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_AddHiddenCashShopLog]
	  @inbCharacterID bigint --// ???ID
	, @intHiddenShopID int --// ?? ? ID
	, @bitPurchaseFlag bit --// 0=??????, 1=???
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
      , @dt2Now datetime2(0) = GETDATE()
      , @intLogTableSuffix int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SET @intLogTableSuffix = dbo.FN_LogTableSuffix(@dt2Now);
	
	/**_# [HiddenCachShopLogs] ???? INSERT???.*/
	IF @intLogTableSuffix = 0
		INSERT dbo.HiddenCashShopLogs0 (LogDate, CharacterID, HiddenShopID, PurchaseFlag)
		VALUES (@dt2Now, @inbCharacterID, @intHiddenShopID, @bitPurchaseFlag);
		
	IF @intLogTableSuffix = 1	
		INSERT dbo.HiddenCashShopLogs1 (LogDate, CharacterID, HiddenShopID, PurchaseFlag)
		VALUES (@dt2Now, @inbCharacterID, @intHiddenShopID, @bitPurchaseFlag);
		
	ELSE	
		INSERT dbo.HiddenCashShopLogs2 (LogDate, CharacterID, HiddenShopID, PurchaseFlag)
		VALUES (@dt2Now, @inbCharacterID, @intHiddenShopID, @bitPurchaseFlag);
END TRY
BEGIN CATCH

	IF XACT_STATE() <> 0
		ROLLBACK TRANSACTION;

	IF @intReturnValue IS NULL OR @intReturnValue = 0
		EXEC @intReturnValue = dbo.P_AddErrorLog @intPROCID = @@PROCID;

	ELSE
		GOTO ErrorHandler;

	RETURN @intReturnValue;
END CATCH;

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

RETURN @intReturnValue;
GO

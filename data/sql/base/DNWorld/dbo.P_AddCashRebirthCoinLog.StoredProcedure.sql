USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddCashRebirthCoinLog]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-03-01
description : ?? ?? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_AddCashRebirthCoinLog]
	  @dt2LogDate datetime2(0) --// ?? ??
	, @inbCharacterID bigint --// ??? ID
	, @insCoinCountBefore smallint --// ?? ? ?? ?
	, @insCoinCountAfter smallint --// ?? ? ?? ?
	, @intMapID int --// ? ID
	, @intIP int --// IP ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @intLogTableSuffix int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SET @intLogTableSuffix = dbo.FN_LogTableSuffix(@dt2LogDate);

	/**_# [CashRebirthCoinUseLogs] ???? INSERT???.*/
	IF @intLogTableSuffix = 0
		INSERT dbo.CashRebirthCoinLogs0 (LogDate, CharacterID, CoinCountBefore, CoinCountAfter, MapID, IP)
		VALUES (@dt2LogDate, @inbCharacterID, @insCoinCountBefore, @insCoinCountAfter, @intMapID, @intIP);

	ELSE IF @intLogTableSuffix = 1
		INSERT dbo.CashRebirthCoinLogs1 (LogDate, CharacterID, CoinCountBefore, CoinCountAfter, MapID, IP)
		VALUES (@dt2LogDate, @inbCharacterID, @insCoinCountBefore, @insCoinCountAfter, @intMapID, @intIP);

	ELSE
		INSERT dbo.CashRebirthCoinLogs2 (LogDate, CharacterID, CoinCountBefore, CoinCountAfter, MapID, IP)
		VALUES (@dt2LogDate, @inbCharacterID, @insCoinCountBefore, @insCoinCountAfter, @intMapID, @intIP);

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

USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddCashRebirthCoin]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 12
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-25
description : ?? ?? ??? ?????. (??!! Cash Server? ?? ?????.)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103102 = ???? ???? ????.
103367 = ???? ??? 0?? ? ?? ????? ???.
history :
5 = IP? ????? ?? at 2010-06-28
6 = @inyCashRebirthCoinGetCode, @inbCashRebirthCoinGetKey ?? ?? at 2010-08-10
8 = ??? ?? ?? at 2011-03-18
9 = ??? ??? ?? ?? at 2011-03-29
10 = CoinCount? 0??? ?? ??? ?? ?? ?? ?? by ??? at 2012-08-17
12 = @inyCashRebirthCoinGetCode ?? ?? (5=??) by ??? at 2013-02-13
**/

CREATE PROCEDURE [dbo].[P_AddCashRebirthCoin]
	  @inbCharacterID bigint --// ??? ID
	, @insCoinCount smallint --// ?? ?? ?? ?
	, @intCoinPrice int --// ?? ?? ?? ?? ??
	, @inyCashRebirthCoinGetCode tinyint --// 1=??? ??, 2=??? ??, 3=??? ??, 4=??? ??, 5=??
	, @inbCashRebirthCoinGetKey bigint --// CashRebirthCoinGetCode ? ?? ? ?. 1:PurchaseOrderID, 2:PurchaseOrderID, 3:AuditLogID, 4:QuestID
	, @vchIP varchar(15) = NULL --// ????? ?? IP ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dt2Now datetime2(0) = GETDATE()
	, @insCoinCountBefore smallint
	, @insCoinCountAfter smallint
	, @intIP int = dbo.FN_IP2IPNumber(@vchIP);;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# [CashRebirthCoins] ???? INSERT???.*/
	INSERT dbo.CashRebirthCoins (CharacterID, CoinCount, RemainCoinCount, CoinPrice, RemainCoinPrice, CashRebirthCoinGetCode, CashRebirthCoinGetKey, RegisterDate, consumptionTargetFlag)
	VALUES (@inbCharacterID, @insCoinCount, @insCoinCount, @intCoinPrice, @intCoinPrice, @inyCashRebirthCoinGetCode, @inbCashRebirthCoinGetKey, @dt2Now, CASE WHEN EXISTS (SELECT * FROM dbo.CashRebirthCoins WITH (UPDLOCK) WHERE CharacterID = @inbCharacterID) THEN 0 ELSE 1 END);

	SELECT @insCoinCountAfter = RemainCoinCount, @insCoinCountBefore = RemainCoinCount - @insCoinCount
	FROM dbo.VIX_CashRebirthCoinTotals WITH (NOEXPAND, NOLOCK)
	WHERE CharacterID = @inbCharacterID;

	EXEC @intReturnValue = dbo.P_AddCashRebirthCoinLog
		  @dt2LogDate = @dt2Now
		, @inbCharacterID = @inbCharacterID
		, @insCoinCountBefore = @insCoinCountBefore
		, @insCoinCountAfter = @insCoinCountAfter
		, @intMapID = NULL
		, @intIP = @intIP;

END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 547 AND ERROR_MESSAGE() LIKE N'%FK_Characters_CashRebirthCoins_CharacterID%'
		SET @intReturnValue = 103102;

	ELSE IF ERROR_NUMBER() = 547 AND ERROR_MESSAGE() LIKE N'%CK_CashRebirthCoins_CoinCount%'
		SET @intReturnValue = 103367;


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

USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListCashRebirthCoinForGSM]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-03-16
description : ?? ??? ???(=??????) ??? ?????. (GSM ???)
result set : CoinCount/smallint, RemainCoinCount/smallint, CashRebirthCoinGetCode/tinyint, CashRebirthCoinGetKey/bigint, RegisterDate/datetime2
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103158 = ?? ?? ??? ????.
**/

CREATE PROCEDURE [dbo].[P_GetListCashRebirthCoinForGSM]
	  @inbCharacterID bigint --// ??? ID
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
	SELECT CoinCount, RemainCoinCount, CashRebirthCoinGetCode, CashRebirthCoinGetKey, RegisterDate
	FROM dbo.CashRebirthCoins WITH (NOLOCK)
	WHERE CharacterID = @inbCharacterID
	ORDER BY CashRebirthCoinID;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103158;
		GOTO ErrorHandler;
	END
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

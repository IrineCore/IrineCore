USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListGameMoneyTrade]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-05-21
description : C2C ?? ??? ?????.
result set : TradeDate/datetime, GameMoneyTradeKey/varchar(50), CharacterID/bigint, CharacterName/nvarchar(50), GameMoneyAmount/bigint, IncomeDate/datetime, ToCharacterID/bigint, ToCharacterName/nvarchar(50), PaidCashAmount/int, TotalCashAmount/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListGameMoneyTrade]
	  @dtmTradeStartDate datetime --// ?? ?? ??
    , @dtmTradeEndDate datetime --// ?? ?? ??
WITH EXECUTE AS 'SPExecutor'
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
	/**_# [GameMoneyTrade] ???? INSERT???.*/
	SELECT A.TradeDate, A.GameMoneyTradeKey, A.CharacterID, B.CharacterName, A.GameMoneyAmount, A.IncomeDate, A.ToCharacterID, C.CharacterName AS ToCharacterName, A.PaidCashAmount, A.TotalCashAmount
	FROM dbo.GameMoneyTrade A WITH(NOLOCK)
		INNER JOIN dbo.DimCharacters B WITH(NOLOCK) ON A.CharacterID = B.CharacterID
		INNER JOIN dbo.DimCharacters C WITH(NOLOCK) ON A.ToCharacterID = C.CharacterID
	WHERE TradeDate BETWEEN @dtmTradeStartDate AND @dtmTradeEndDate;

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

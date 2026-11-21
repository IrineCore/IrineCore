USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListTradeSuccessLog]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 9
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-07-09
description : ??? ?? ?? ??? ?????.
result set : TradeDate, BuyerAccountName, BuyerCharacterID, BuyerCharacterName, AccountName, CharacterID, CharacterName, ItemID, ItemName, ItemRankCode, ItemCount, LevelLimit, ItemLevel, SealCount, Price
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
2 = ?? ?? ?? ?? at 2010-07-12
3 = ???? ???? ??? ???? ?? ?? at 2010-07-14
4 = ?? ?? ??? ? ???? ? ??? ?? at 2010-12-02
6 = input parameter ?? (@inyTradeStatusCode)     by 20111017.wgkim
7 = WITH EXECUTE AS 'SPExecutor' ???? ??, input parameter @intWorldID ?? ??, result set ?? WorldName ?? by ??? at 2011-11-01
8 = ????? VIEW ???? ???? ?? by ??? at 2012-02-07

**/

CREATE PROCEDURE [dbo].[P_GetListTradeSuccessLog]
	  @dt2StartDate datetime2(0) --// ?? ?? ??
	, @dt2EndDate datetime2(0) --// ?? ? ??
	, @inbCharacterID bigint = NULL --// ??? ??? ID
	, @inbBuyerCharacterID bigint = NULL --// ??? ??? ID
	, @intWorldID int --// ?? ID
	, @inyTradeStatusCode tinyint = NULL --// ???? (1=???, 2=??)
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @nvcStmt nvarchar(max)
	, @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName nvarchar(50) = N'TradeSuccessLogs'
	, @nvcStartDate nvarchar(8) = CONVERT(nvarchar(8), @dt2StartDate, 112)
	, @nvcEndDate nvarchar(8) = CONVERT(nvarchar(8), @dt2EndDate, 112)	
	, @nvcHint nvarchar(50) = N'NOLOCK'
	, @nvcStmtOUT nvarchar(max) = N'';

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/** ????? ??? ?? by ??? at 2012-02-07 **/
	EXEC dbo.P_MakeTableString
		  @nvcDBName = @nvcDBName
		, @nvcTblName = @nvcTblName
		, @nvcStartDate = @nvcStartDate
		, @nvcEndDate = @nvcEndDate
		, @nvcHint = @nvcHint
		, @nvcStmtOUT = @nvcStmtOUT OUTPUT;	

	SET @nvcStmt = N'
SELECT TS.TradeDate, AC1.AccountName AS BuyerAccountName, DC1.CharacterID AS BuyerCharacterID, DC1.CharacterName AS BuyerCharacterName
	, AC2.AccountName, DC2.CharacterID, DC2.CharacterName, DI.ItemID, DI.ItemName, DI.ItemRankCode, TS.ItemCount, DI.LevelLimit, TS.ItemLevel, TS.SealCount, TS.Price, TS.TradeStatusCode
FROM (' + @nvcStmtOUT + N') TS
	INNER JOIN dbo.DimCharacters DC1 WITH (NOLOCK) ON TS.BuyerCharacterID = DC1.CharacterID
	INNER JOIN dbo.DimAccounts AC1 WITH (NOLOCK) ON DC1.AccountID = AC1.AccountID
	INNER JOIN dbo.DimCharacters DC2 WITH (NOLOCK) ON TS.CharacterID = DC2.CharacterID
	INNER JOIN dbo.DimAccounts AC2 WITH (NOLOCK) ON DC2.AccountID = AC2.AccountID
	INNER JOIN dbo.DimItems DI WITH (NOLOCK) ON TS.ItemID = DI.ItemID
WHERE TS.TradeDate BETWEEN ''' + CONVERT(nvarchar(19), @dt2StartDate, 121) + N''' AND ''' + CONVERT(nvarchar(19), @dt2EndDate, 121) + N'''' +
	CASE WHEN @inbCharacterID IS NOT NULL THEN N' AND TS.CharacterID = @inbCharacterID' ELSE N'' END +
	CASE WHEN @inbBuyerCharacterID IS NOT NULL THEN N' AND TS.BuyerCharacterID = @inbBuyerCharacterID' ELSE N'' END +
	CASE WHEN @inyTradeStatusCode IS NOT NULL THEN N' AND TS.TradeStatusCode = @inyTradeStatusCode' ELSE N'' END +
	CASE WHEN @intWorldID IS NOT NULL THEN N' AND TS.WorldID = @intWorldID' ELSE N'' END + N'
ORDER BY TS.TradeDate'

	EXEC sp_executesql @nvcStmt, N'@inbCharacterID bigint, @inbBuyerCharacterID bigint, @intWorldID int, @inyTradeStatusCode tinyint', @inbCharacterID = @inbCharacterID, @inbBuyerCharacterID = @inbBuyerCharacterID, @intWorldID = @intWorldID, @inyTradeStatusCode = @inyTradeStatusCode;
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

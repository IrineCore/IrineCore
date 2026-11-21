USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_AddGameMoneyForETL]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 8
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2011-10-25
description : GameMoney ?? ???? ?????
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history
2 = ????.
3~5 = ????? VIEW ???? ???? ?? by ??? at 2012-02-07
7 = DWC ???? ?? ????? ?? by ??? at 2013-02-19
8 = ?? ??? ???? ????? ?? by ??? at 2013-03-19
**/

CREATE PROCEDURE [dbo].[P_AddGameMoneyForETL]
	  @dtmBasisDate datetime --// ?? ??
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @nvcStmt nvarchar(max) = N''
	, @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName1 nvarchar(50) = N'CoinLogs'
	, @nvcTblName2 nvarchar(50) = N'ItemLogs'
	, @nvcStartDate nvarchar(8) = CONVERT(nvarchar(8), @dtmBasisDate, 112)
	, @nvcEndDate nvarchar(8) = CONVERT(nvarchar(8), @dtmBasisDate, 112)	
	, @nvcHint nvarchar(50) = N'NOLOCK'
	, @nvcStmtOUT1 nvarchar(max) = N''
	, @nvcStmtOUT2 nvarchar(max) = N'';
	
/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	DELETE dbo.Mon_GameMoney1 WHERE LogDate < DATEADD(DAY, -90, @dtmBasisDate);

	DELETE dbo.Mon_GameMoney2 WHERE BasisDate < DATEADD(DAY, -90, @dtmBasisDate);

	DELETE dbo.Mon_GameMoney3 WHERE BasisDate < DATEADD(DAY, -90, @dtmBasisDate);

	DELETE dbo.ST_ItemSales WHERE BasisDate < DATEADD(DAY, -90, @dtmBasisDate);

	/** ????? ??? ?? by ??? at 2012-02-07 **/
	EXEC dbo.P_MakeTableString
		  @nvcDBName = @nvcDBName
		, @nvcTblName = @nvcTblName1
		, @nvcStartDate = @nvcStartDate
		, @nvcEndDate = @nvcEndDate
		, @nvcHint = @nvcHint
		, @nvcStmtOUT = @nvcStmtOUT1 OUTPUT;
	EXEC dbo.P_MakeTableString
		  @nvcDBName = @nvcDBName
		, @nvcTblName = @nvcTblName2
		, @nvcStartDate = @nvcStartDate
		, @nvcEndDate = @nvcEndDate
		, @nvcHint = @nvcHint
		, @nvcStmtOUT = @nvcStmtOUT2 OUTPUT;
		
	SET @nvcStmt = N'
	INSERT dbo.Mon_GameMoney1 (LogDate, CharacterID, CoinChangeCode, GameMoneyAmount, MailID, CounterpartCharacterID, IP)
	SELECT LogDate, CharacterID, CoinChangeCode, ChangedCoin
		, CASE WHEN CoinChangeCode IN (6, 7) THEN CoinChangeFKey ELSE NULL END AS MailID
		, CASE WHEN CoinChangeCode IN (11, 12) THEN CoinChangeFKey ELSE NULL END AS CounterpartCharacterID
		, IP
	FROM (' + @nvcStmtOUT1 + N') V
	WHERE LogDate >= @dtmBasisDate AND LogDate < (@dtmBasisDate + 1)
		AND CoinTypeCode = 1
		AND CoinChangeCode IN (3, 4, 6, 7, 11, 12)
		AND (ChangedCoin >= 100000 OR ChangedCoin <= -100000)
	ORDER BY LogDate;
	
	INSERT dbo.Mon_GameMoney2 (BasisDate, CharacterID, GameMoneyAmount)
	SELECT @dtmBasisDate, CharacterID, SUM(ABS(ChangedCoin))
	FROM (' + @nvcStmtOUT1 + N') V
	WHERE LogDate >= @dtmBasisDate AND LogDate < (@dtmBasisDate + 1)
		AND CoinTypeCode = 1
		AND CoinChangeCode IN (3, 4, 6, 7, 11, 12)
	GROUP BY CharacterID
	HAVING SUM(ABS(ChangedCoin)) >= 500000
	ORDER BY CharacterID;

	INSERT dbo.Mon_GameMoney3 (BasisDate, CoinChangeCode, CharacterID, GameMoneyAmount)
	SELECT CAST(@dtmBasisDate AS date), CoinChangeCode, CharacterID, CAST(ABS(CoinVariance) / 10000.0 AS DECIMAL(15,4))
	FROM ( 
		SELECT 1 AS Code UNION ALL -- ??? ??
		SELECT 2 UNION ALL -- ?? ??
		SELECT 3 UNION ALL -- ??
		SELECT 4 UNION ALL -- ??? ??
		SELECT 7 UNION ALL -- ??????
		SELECT 10 UNION ALL -- ??? ???
		SELECT 11 UNION ALL -- ?? ??
		SELECT 23 UNION ALL -- ???? ??? ???
		SELECT 27 -- ?? ??? ??
	) A
		CROSS APPLY (
			SELECT TOP 500 CharacterID, CoinChangeCode, SUM(ChangedCoin) AS CoinVariance
			FROM (' + @nvcStmtOUT1 + N') V
			WHERE LogDate >= @dtmBasisDate AND LogDate < @dtmBasisDate + 1
				AND A.Code = CoinChangeCode
			GROUP BY CharacterID, CoinChangeCode
			ORDER BY CoinVariance DESC
		) B
	ORDER BY CoinChangeCode, CoinVariance DESC;
	
	INSERT dbo.ST_ItemSales (BasisDate, WorldID, ItemID, SaleDirectionCode, CharacterCount, SaleCount, ItemCount)
	SELECT @dtmBasisDate, WorldID, ItemID, CASE ItemLogCode WHEN 5 THEN 1 ELSE 2 END, COUNT(DISTINCT CASE ItemLogCode WHEN 5 THEN OwnerCharacterID ELSE OwnerCharacterIDBefore END), COUNT(*), SUM(ABS(ItemCount - ItemCountBefore))
	FROM (' + @nvcStmtOUT2 + N') V
	WHERE LogDate >= @dtmBasisDate AND LogDate < (@dtmBasisDate + 1) AND ItemLogCode IN (5, 22)
	GROUP BY WorldID, ItemID, ItemLogCode
	ORDER BY WorldID, ItemID, ItemLogCode;
	';
	EXEC sp_executesql @nvcStmt, N'@dtmBasisDate datetime', @dtmBasisDate = @dtmBasisDate;
	
	INSERT dbo.ST_ActiveGameMoney (BasisDate, WorldID, ActiveGameMoneyCode, GameMoneyAmount, CharacterCount)
	SELECT BasisDate, WorldID, ActiveGameMoneyCode, GameMoneyAmount, CharacterCount
	FROM (
		SELECT @dtmBasisDate AS BasisDate, DC.WorldID, 1 AS ActiveGameMoneyCode, SUM(DC.Coin + DC.WarehouseCoin) AS GameMoneyAmount, COUNT(DISTINCT DC.CharacterID) AS CharacterCount
		FROM dbo.DimCharacters DC WITH (NOLOCK)
		WHERE DC.DeleteFlag = CAST(0 AS bit) AND DC.AccountLevelCode = 0
		GROUP BY DC.WorldID
		UNION ALL
		SELECT @dtmBasisDate, DC.WorldID, 2, SUM(DC.Coin + DC.WarehouseCoin), COUNT(DISTINCT DC.CharacterID)
		FROM (SELECT DISTINCT AccountID FROM dbo.DailyUVAccounts WITH (NOLOCK) WHERE BasisDate >= DATEADD(DAY, -6, @dtmBasisDate)) UV
			INNER JOIN dbo.DimCharacters DC WITH (NOLOCK) ON UV.AccountID = DC.AccountID
		WHERE DC.DeleteFlag = CAST(0 AS bit) AND DC.AccountLevelCode = 0
		GROUP BY DC.WorldID
		UNION ALL
		SELECT @dtmBasisDate, DC.WorldID, 3, SUM(DC.Coin + DC.WarehouseCoin), COUNT(DISTINCT DC.CharacterID)
		FROM (SELECT DISTINCT AccountID FROM dbo.DailyUVAccounts WITH (NOLOCK) WHERE BasisDate >= DATEADD(DAY, -13, @dtmBasisDate)) UV
			INNER JOIN dbo.DimCharacters DC WITH (NOLOCK) ON UV.AccountID = DC.AccountID
		WHERE DC.DeleteFlag = CAST(0 AS bit) AND DC.AccountLevelCode = 0
		GROUP BY DC.WorldID
	) S
	ORDER BY WorldID, ActiveGameMoneyCode;

	INSERT dbo.ST_ActiveGameMoneyByCharacterLevel (BasisDate, WorldID, CharacterLevel, GameMoneyAmount, CharacterCount)
	SELECT BasisDate, WorldID, CharacterLevel, GameMoneyAmount, CharacterCount
	FROM (
		SELECT @dtmBasisDate AS BasisDate, DC.WorldID, DC.CharacterLevel, SUM(DC.Coin + DC.WarehouseCoin) AS GameMoneyAmount, COUNT(DISTINCT DC.CharacterID) AS CharacterCount
		FROM (SELECT DISTINCT AccountID FROM dbo.DailyUVAccounts WITH (NOLOCK) WHERE BasisDate >= DATEADD(DAY, -6, @dtmBasisDate)) UV
			INNER JOIN dbo.DimCharacters DC WITH (NOLOCK) ON UV.AccountID = DC.AccountID
		WHERE DC.DeleteFlag = CAST(0 AS bit) AND DC.AccountLevelCode = 0
		GROUP BY DC.WorldID, DC.CharacterLevel
	) S
	ORDER BY BasisDate, WorldID, CharacterLevel;
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

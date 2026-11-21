USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_AddST_DailyCoinUse]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 6
author : ???
e-mail : otooi@eyedentitygames.com
created date : 2013-04-08
description : ?? ?? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
1~2 = ???? by ??? at 2013-04-08
**/

CREATE PROCEDURE [dbo].[P_AddST_DailyCoinUse]
	  @datBasisDate datetime = NULL --// ?? ??
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @nvcStmt nvarchar(max) = N''
	, @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName nvarchar(50) = N'CoinLogs'
	, @nvcStartDate nvarchar(8) = CONVERT(nvarchar(8), @datBasisDate, 112)
	, @nvcEndDate nvarchar(8) = CONVERT(nvarchar(8), @datBasisDate, 112)	
	, @nvcHint nvarchar(50) = N'NOLOCK'
	, @nvcStmtOUT nvarchar(max) = N'';

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SET @datBasisDate = CAST(ISNULL(@datBasisDate, GETDATE()) AS date);

	/** ????? ??? ?? by ??? at 2012-02-07 **/
	EXEC dbo.P_MakeTableString
		  @nvcDBName = @nvcDBName
		, @nvcTblName = @nvcTblName
		, @nvcStartDate = @nvcStartDate
		, @nvcEndDate = @nvcEndDate
		, @nvcHint = @nvcHint
		, @nvcStmtOUT = @nvcStmtOUT OUTPUT;	
	
	SET @nvcStmt = N'
	/**_# [ST_DailyCoinUse] ???? INSERT???.*/
	INSERT dbo.ST_DailyCoinUse (BasisDate, WorldID, CharacterLevel, JobCode, CoinChangeCode, CoinTypeCode, CoinGroupCode, ChangedCoin)
	SELECT CAST(V.LogDate AS date) AS BasisDate, b.WorldID, b.CharacterLevel, b.JobCode, V.CoinChangeCode, V.CoinTypeCode
		, CASE WHEN b.Coin >= 0 AND b.Coin < 10000 THEN 1
			WHEN b.Coin >= 10000 AND b.Coin < 100000 THEN 2
			WHEN b.Coin >= 100000 AND b.Coin < 1000000 THEN 3
			WHEN b.Coin >= 1000000 AND b.Coin < 10000000 THEN 4
			WHEN b.Coin >= 10000000 AND b.Coin < 100000000 THEN 5
			WHEN b.Coin >= 100000000 AND b.Coin < 1000000000 THEN 6
			WHEN b.Coin >= 1000000000 AND b.Coin < 10000000000 THEN 7
			WHEN b.Coin >= 10000000000 THEN 8 
			ELSE 0 END AS CoinGroupCode
		, SUM(ABS(V.ChangedCoin)) AS ChangedCoin
	FROM (' + @nvcStmtOUT + N') V INNER JOIN dbo.DimCharacters b WITH (NOLOCK) ON b.CharacterID = V.CharacterID
	WHERE V.LogDate >= @datBasisDate AND V.LogDate < DATEADD(DAY, 1, @datBasisDate)
	GROUP BY CAST(V.LogDate AS date), b.WorldID, b.CharacterLevel, b.JobCode, V.CoinChangeCode, V.CoinTypeCode
		, CASE WHEN b.Coin >= 0 AND b.Coin < 10000 THEN 1
			WHEN b.Coin >= 10000 AND b.Coin < 100000 THEN 2
			WHEN b.Coin >= 100000 AND b.Coin < 1000000 THEN 3
			WHEN b.Coin >= 1000000 AND b.Coin < 10000000 THEN 4
			WHEN b.Coin >= 10000000 AND b.Coin < 100000000 THEN 5
			WHEN b.Coin >= 100000000 AND b.Coin < 1000000000 THEN 6
			WHEN b.Coin >= 1000000000 AND b.Coin < 10000000000 THEN 7
			WHEN b.Coin >= 10000000000 THEN 8
			ELSE 0 END
	OPTION (MAXDOP 1);
	';
	EXEC sp_executesql @nvcStmt, N'@datBasisDate date', @datBasisDate = @datBasisDate;
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

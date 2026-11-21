USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetDonateRank]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 8
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-01-13
description : ?? ??? ????? ?????
result set : DonateRank/int, CharacterID/bigint, CharacterName/nvarchar, DonateAmount/bigint, DonateDate/datetime
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
3~5 = ????? VIEW ???? ???? ?? by ??? at 2012-02-07
6 = WorldID Input ???? ?? by ??? at 2012-03-12
7 = RowCount Input ???? ?? by ??? at 2012-03-12
**/

CREATE PROCEDURE [dbo].[P_GetDonateRank]
	  @dtmStartDate datetime --// ??? ?? ??
	, @dtmEndDate datetime --// ??? ? ??
    , @intWorldID int --// ??ID
	, @intRankCount int --// ??? ?? ?
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @nvcStmt nvarchar(max) = N''
	, @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName nvarchar(50) = N'CoinLogs'
	, @nvcStartDate nvarchar(8) = CONVERT(nvarchar(8), @dtmStartDate, 112)
	, @nvcEndDate nvarchar(8) = CONVERT(nvarchar(8), @dtmEndDate, 112)		
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
	;WITH CTE AS(
		SELECT CharacterID, ABS(SUM(ChangedCoin)) AS DonateAmount, MAX(LogDate) AS DonateDate
		FROM (' + @nvcStmtOUT + N') V
		WHERE CoinChangeCode = 34
		  AND LogDate >= @dtmStartDate AND LogDate <= @dtmEndDate
		GROUP BY CharacterID
	)
	,CTE2 (DonateRank, CharacterID, CharacterName, DonateAmount, DonateDate) AS(
		SELECT ROW_NUMBER() OVER(ORDER BY A.DonateAmount DESC, A.DonateDate) AS DonateRank, A.CharacterID, DC.CharacterName, A.DonateAmount, A.DonateDate 
		FROM CTE A INNER JOIN dbo.DimCharacters DC WITH(NOLOCK) ON A.CharacterID = DC.CharacterID
		WHERE DC.DeleteFlag = 0 AND DC.WorldID = @intWorldID
	)
	SELECT DonateRank, CharacterID, CharacterName, DonateAmount, DonateDate
	FROM CTE2 
	WHERE DonateRank <= @intRankCount;
	';
	EXEC sp_executesql @nvcStmt, N'@dtmStartDate datetime, @dtmEndDate datetime, @intWorldID int, @intRankCount int', @dtmStartDate = @dtmStartDate, @dtmEndDate = @dtmEndDate, @intWorldID = @intWorldID, @intRankCount = @intRankCount;
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

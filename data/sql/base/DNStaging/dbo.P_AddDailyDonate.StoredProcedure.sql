USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_AddDailyDonate]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-01-13
description : ?? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_AddDailyDonate]
	@dtmBasisDate date
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

IF DATEPART(DAY,@dtmBasisDate) <> 1
	RETURN 0;

DECLARE @intReturnValue int;
DECLARE @nvcStmt nvarchar(max) = N''
	, @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName nvarchar(50) = N'CoinLogs'
	, @nvcStartDate nvarchar(8) = CONVERT(nvarchar(8), DATEADD(MONTH, -1, @dtmBasisDate), 112)
	, @nvcEndDate nvarchar(8) = CONVERT(nvarchar(8), DATEADD(DAY, -1, @dtmBasisDate), 112)		
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
INSERT dbo.ST_MonthlyDonate (WorldID, BasisDate, DonateAmount, DonateCharacterCount)
SELECT DC.WorldID, DATEADD(MONTH, -1, @dtmBasisDate) AS BasisDate, ABS(SUM(V.ChangedCoin)) AS DonateAmount, COUNT(DISTINCT DC.CharacterID) AS DonateCharacterCount
FROM (' + @nvcStmtOUT + N') V JOIN dbo.DimCharacters DC WITH (NOLOCK) ON V.CharacterID = DC.CharacterID
WHERE CoinChangeCode = 34
GROUP BY DC.WorldID
	';
	EXEC sp_executesql @nvcStmt, N'@dtmBasisDate date', @dtmBasisDate = @dtmBasisDate;
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

USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_AddST_DailyGameMoney]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 13
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-06-14
description : ?? ???? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
4 = ?? ?? at 2010-06-22
5 = ???? MailTypeCode 5, 6, 7? ?????. at 2010-08-28
6 = ?? ?? ?? ??? ?? ?? ?? at 2010-08-28
history :
8~11 = ????? VIEW ???? ???? ?? by ??? at 2012-02-07
12 = DWC ???? ?? ????? ?? by ??? at 2013-02-19
13 = ?? ??? ???? ????? ?? by ??? at 2013-03-19
**/

CREATE PROCEDURE [dbo].[P_AddST_DailyGameMoney]
	  @dtmBasisDate datetime --// ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dt2BasisDate datetime2(0) = CAST(CAST(@dtmBasisDate AS date) AS datetime2(0));
DECLARE @nvcStmt nvarchar(max) = N''
	, @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName1 nvarchar(50) = N'CoinLogs'
	, @nvcTblName2 nvarchar(50) = N'MailLogs'
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
	/**_# [ST_DailyGameMoney] ???? INSERT???.*/
	INSERT dbo.ST_DailyGameMoney (BasisDate, WorldID, GameMoneyChangeCode, GameMoneyAmount, GameMoneyChangeCount)
	SELECT CAST(A.LogDate AS date) AS BasisDate, B.WorldID, A.CoinChangeCode, CASE WHEN CoinChangeCode = 26 AND SUM(A.ChangedCoin) > 0 THEN - SUM(A.ChangedCoin) ELSE SUM(A.ChangedCoin) END, COUNT(*)
	FROM (' + @nvcStmtOUT1 + N') A
		INNER JOIN dbo.DimCharacters B ON A.CharacterID = B.CharacterID
	WHERE A.CoinTypeCode = 1 AND A.LogDate >= @dt2BasisDate AND A.LogDate < DATEADD(DAY, 1, @dt2BasisDate)
		AND A.CoinChangeCode NOT IN (6, 7, 8, 9, 11, 12, 22, 23)
		AND A.ChangedCoin <> 0
        AND B.AccountLevelCode = 0
	GROUP BY CAST(A.LogDate AS date), B.WorldID, A.CoinChangeCode
	UNION ALL
	SELECT CAST(A.LogDate AS date) AS BasisDate, B.WorldID,
		CASE C.MailTypeCode
			WHEN 2 THEN 80
			WHEN 3 THEN 81
			WHEN 4 THEN 82
			WHEN 5 THEN 83
			WHEN 6 THEN 84
			WHEN 7 THEN 85
		END
		, SUM(A.ChangedCoin), COUNT(*)
	FROM (' + @nvcStmtOUT1 + N') A
		INNER JOIN dbo.DimCharacters B ON A.CharacterID = B.CharacterID
		INNER JOIN (' + @nvcStmtOUT2 + N') C ON A.CoinChangeFKey = C.MailID AND B.WorldID = C.WorldID
	WHERE A.CoinTypeCode = 1 AND A.CoinChangeCode = 7 AND C.MailTypeCode > 1
		AND A.LogDate >= @dt2BasisDate AND A.LogDate < DATEADD(DAY, 1, @dt2BasisDate)
		AND C.SendDate >= @dt2BasisDate AND C.SendDate < DATEADD(DAY, 1, @dt2BasisDate)
		AND A.ChangedCoin <> 0
        AND B.AccountLevelCode = 0
	GROUP BY CAST(LogDate AS date), B.WorldID, C.MailTypeCode;
	';
	EXEC sp_executesql @nvcStmt, N'@dt2BasisDate datetime2(0)', @dt2BasisDate = @dt2BasisDate;
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

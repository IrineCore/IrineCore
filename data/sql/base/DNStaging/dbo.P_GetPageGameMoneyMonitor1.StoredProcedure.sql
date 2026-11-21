USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetPageGameMoneyMonitor1]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 7
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-07-08
description : 1? 10?? ?? ??? ??? ??? ????? ?????? ?????.
result set : 
	@intSelectClauseCode = 1 : LogDate, WorldName, AccountName, CharacterID, CharacterName, CharacterLevel, CoinChangeCode, GameMoneyAmount, MailID, CounterpartAccountName, CounterpartCharacterID, CounterpartCharacterName
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
2 = input parameter @inyCoinChangeCode ??      by 20110331.wgkim
3 = input parameter @inyCoinChangeCode ??      by 20110331.wgkim
4 = ????? VIEW ???? ???? ?? by ??? at 2012-02-07
**/

CREATE PROCEDURE [dbo].[P_GetPageGameMoneyMonitor1]
	  @intPageNumber int --// ??? ??.
	, @intPageSize int --// ??? ???.
	, @intSelectClauseCode int --// ?? ?? ??.
	, @dt2StartLogDate datetime2(0) = NULL --// ?? ?? ?? - ?? ??
	, @dt2EndLogDate datetime2(0) = NULL --// ?? ? ?? - ?? ??
	, @inyCoinChangeCode tinyint = NULL --// 3=??, 4=??? ??, 6=??????, 7=??????, 11=?? ??, 12=?? ??
    , @bitExcludeRestraintAccount bit = NULL --// ???? ?? ?? (0=??, 1=???)
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE
	@intReturnValue int,
	@nvcStmt nvarchar(max),
	@nvcSelectStmt nvarchar(max),
	@nvcSelectStmt2 nvarchar(max),
	@inbNullBitmap bigint,
	@intColumnCount int;
DECLARE	
	@nvcDBName nvarchar(50) = N'DNStaging',
	@nvcTblName nvarchar(50) = N'MailLogs',
	@nvcStartDate nvarchar(8) = CONVERT(nvarchar(8), @dt2StartLogDate, 112),
	@nvcEndDate nvarchar(8) = CONVERT(nvarchar(8), @dt2EndLogDate, 112),
	@nvcHint nvarchar(50) = N'NOLOCK',
	@nvcStmtOUT nvarchar(max) = N'';

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

	/**_# ?? ??? ?????. (SELECT ?).*/
	SET @nvcSelectStmt = 
		CASE
			WHEN @intSelectClauseCode = 1 THEN N'LogDate, CharacterID, CoinChangeCode, GameMoneyAmount, MailID, CounterpartCharacterID'
		END;

	SET @nvcSelectStmt2 = 
		CASE
			WHEN @intSelectClauseCode = 1 THEN N'L.LogDate, W.WorldName, AC1.AccountName, DC1.CharacterID, DC1.CharacterName, DC1.CharacterLevel, L.CoinChangeCode, L.GameMoneyAmount, L.MailID, AC2.AccountName AS CounterpartAccountName, DC2.CharacterID AS CounterpartCharacterID, DC2.CharacterName AS CounterpartCharacterName'
		END;

	SET @intColumnCount = 3;

	SET @inbNullBitmap = 
		CASE WHEN @dt2StartLogDate IS NOT NULL THEN POWER(2, 0) ELSE 0 END + 
		CASE WHEN @dt2EndLogDate IS NOT NULL THEN POWER(2, 1) ELSE 0 END + 
		CASE WHEN @inyCoinChangeCode IS NOT NULL THEN POWER(2, 2) ELSE 0 END;

	/**_# ?? ??? ?????. (FROM, WHERE ?).*/
	SET @intPageNumber = ISNULL(@intPageNumber, 1);
	SET @intPageSize = ISNULL(@intPageSize, 1000);

	SET @nvcStmt = N'
WITH PagingCTE AS (
	SELECT ROW_NUMBER() OVER(ORDER BY LogDate) AS RowNumber, ' + @nvcSelectStmt + N'
	FROM dbo.Mon_GameMoney1 WITH (NOLOCK)
'		+ dbo.FN_DynamicWhereClause2(N'LogDate', N'@dt2StartLogDate', @inbNullBitmap, 1, @intColumnCount, '>=')
		+ dbo.FN_DynamicWhereClause2(N'LogDate', N'@dt2EndLogDate', @inbNullBitmap, 2, @intColumnCount, '<=')
		+ dbo.FN_DynamicWhereClause2(N'CoinChangeCode', N'@inyCoinChangeCode', @inbNullBitmap, 3, @intColumnCount, '=') + N'
)
SELECT ' + @nvcSelectStmt2 + N'
FROM PagingCTE L
	INNER JOIN dbo.DimCharacters DC1 WITH (NOLOCK) ON L.CharacterID = DC1.CharacterID
	INNER JOIN dbo.DimAccounts AC1 WITH (NOLOCK) ON DC1.AccountID = AC1.AccountID
	INNER JOIN dbo.DimWorlds W WITH (NOLOCK) ON DC1.WorldID = W.WorldID
	LEFT OUTER JOIN dbo.SS_RestraintedAccounts R ON AC1.AccountID = R.AccountID
' + 

CASE WHEN @inyCoinChangeCode = 6 THEN N'
	LEFT OUTER JOIN (' + @nvcStmtOUT + N') ML ON (L.MailID = ML.MailID)
	LEFT OUTER JOIN dbo.DimCharacters DC2 WITH (NOLOCK) ON ML.ReceiverCharacterID = DC2.CharacterID
	LEFT OUTER JOIN dbo.DimAccounts AC2 WITH (NOLOCK) ON DC2.AccountID = AC2.AccountID'
     WHEN @inyCoinChangeCode = 7 THEN N'
	LEFT OUTER JOIN (' + @nvcStmtOUT + N') ML ON (L.MailID = ML.MailID)
	LEFT OUTER JOIN dbo.DimCharacters DC2 WITH (NOLOCK) ON ML.SenderCharacterID = DC2.CharacterID
	LEFT OUTER JOIN dbo.DimAccounts AC2 WITH (NOLOCK) ON DC2.AccountID = AC2.AccountID'
                                 ELSE N'
	LEFT OUTER JOIN dbo.DimCharacters DC2 WITH (NOLOCK) ON L.CounterpartCharacterID = DC2.CharacterID
	LEFT OUTER JOIN dbo.DimAccounts AC2 WITH (NOLOCK) ON DC2.AccountID = AC2.AccountID'
END +

N'
WHERE RowNumber BETWEEN (((@intPageNumber - 1) * @intPageSize) + 1) AND (@intPageNumber * @intPageSize)' +

CASE WHEN @bitExcludeRestraintAccount = 1 THEN N' AND R.AccountID IS NULL ' ELSE N'' END + 

'ORDER BY RowNumber
OPTION (MAXDOP 1)';

	/**_# ?? ??? ?????.*/
	EXEC dbo.sp_executesql @nvcStmt
		, N'
			  @dt2StartLogDate datetime2(0)
			, @dt2EndLogDate datetime2(0)
			, @intPageNumber int
			, @intPageSize int
			, @inyCoinChangeCode tinyint
		'
		, @dt2StartLogDate = @dt2StartLogDate
		, @dt2EndLogDate = @dt2EndLogDate
		, @intPageNumber = @intPageNumber
		, @intPageSize = @intPageSize
		, @inyCoinChangeCode = @inyCoinChangeCode;

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

USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetPageGameMoneyMonitor2]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-07-08
description : ?? ???? 50?? ?? ??? ??? ??? ????? ?????? ?????.
result set : 
	@intSelectClauseCode = 1 : BasisDate, CharacterID, GameMoneyAmount
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetPageGameMoneyMonitor2]
	  @intPageNumber int --// ??? ??.
	, @intPageSize int --// ??? ???.
	, @intSelectClauseCode int --// ?? ?? ??.
	, @datStartDate date = NULL --// ?? ?? ??.
	, @datEndDate date = NULL --// ?? ? ??.
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

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# ?? ??? ?????. (SELECT ?).*/
	SET @nvcSelectStmt = 
		CASE
			WHEN @intSelectClauseCode = 1 THEN N'BasisDate, CharacterID, GameMoneyAmount'
		END;

	SET @nvcSelectStmt2 = 
		CASE
			WHEN @intSelectClauseCode = 1 THEN N'L.BasisDate, AC.AccountName, W.WorldName, DC.CharacterID, DC.CharacterName, L.GameMoneyAmount'
		END;

	SET @intColumnCount = 2;

	SET @inbNullBitmap = 
		CASE WHEN @datStartDate IS NOT NULL THEN POWER(2, 0) ELSE 0 END +
		CASE WHEN @datEndDate IS NOT NULL THEN POWER(2, 1) ELSE 0 END;

	/**_# ?? ??? ?????. (FROM, WHERE ?).*/
	SET @intPageNumber = ISNULL(@intPageNumber, 1);
	SET @intPageSize = ISNULL(@intPageSize, 1000);

	SET @nvcStmt = N'
WITH PagingCTE AS (
	SELECT ROW_NUMBER() OVER(ORDER BY BasisDate, CharacterID) AS RowNumber, ' + @nvcSelectStmt + N'
	FROM dbo.Mon_GameMoney2 WITH (NOLOCK)
'		+ dbo.FN_DynamicWhereClause2(N'BasisDate', N'@datStartDate', @inbNullBitmap, 1, @intColumnCount, '>=')
		+ dbo.FN_DynamicWhereClause2(N'BasisDate', N'@datEndDate', @inbNullBitmap, 2, @intColumnCount, '<=') + N'
)
SELECT ' + @nvcSelectStmt2 + N'
FROM PagingCTE L
	INNER JOIN dbo.DimCharacters DC WITH (NOLOCK) ON L.CharacterID = DC.CharacterID
	INNER JOIN dbo.DimAccounts AC WITH (NOLOCK) ON DC.AccountID = AC.AccountID
	INNER JOIN dbo.DimWorlds W WITH (NOLOCK) ON DC.WorldID = W.WorldID
	LEFT OUTER JOIN dbo.SS_RestraintedAccounts R ON AC.AccountID = R.AccountID
WHERE L.RowNumber BETWEEN (((@intPageNumber - 1) * @intPageSize) + 1) AND (@intPageNumber * @intPageSize)' +

CASE WHEN @bitExcludeRestraintAccount = 1 THEN N' AND R.AccountID IS NULL ' ELSE N'' END + 

'ORDER BY L.RowNumber
OPTION (MAXDOP 1)';

	/**_# ?? ??? ?????.*/
	EXEC dbo.sp_executesql @nvcStmt
		, N'
			  @datStartDate date
			, @datEndDate date
			, @intPageNumber int
			, @intPageSize int
		'
		, @datStartDate = @datStartDate
		, @datEndDate = @datEndDate
		, @intPageNumber = @intPageNumber
		, @intPageSize = @intPageSize;
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

USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetPageGMCopyLog]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 7
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-02-19
description : ?? ? ?? ??? ????? ?????? ?????.
result set : GMCopyLogID, LogDate, SessionID, RestrictType, InitCoin, CurrentCoin, CharacterLevel, BanAmount, IllegalAmount, IP, CharacterName
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
7	??? ?? CharacterName? ????? ??
**/

CREATE PROCEDURE [dbo].[P_GetPageGMCopyLog]
	  @intPageNumber int --// ??? ??.
	, @intPageSize int --// ??? ???.
	, @intSelectClauseCode int --// ?? ?? ??.
	, @nvcRestrictType nvarchar(50) = NULL --// 
	, @vchIP varchar(15) = NULL --// 
	, @dtmStartLogDate datetime = NULL --// ?? ?? ??
	, @dtmEndLogDate datetime = NULL --// ?? ? ??
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE
	@intReturnValue int,
	@nvcStmt nvarchar(max),
	@nvcSelectStmt nvarchar(max),
	@inbNullBitmap bigint,
	@intColumnCount int,
	@intIP int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SET @intIP = dbo.FN_IP2IPNumber(@vchIP);
	SET @intColumnCount = 4;

	SET @inbNullBitmap = 
		CASE WHEN @nvcRestrictType IS NOT NULL THEN POWER(2, 0) ELSE 0 END + 
		CASE WHEN @intIP IS NOT NULL THEN POWER(2, 1) ELSE 0 END + 
		CASE WHEN @dtmStartLogDate IS NOT NULL THEN POWER(2, 2) ELSE 0 END + 
		CASE WHEN @dtmEndLogDate IS NOT NULL THEN POWER(2, 3) ELSE 0 END;

	/**_# ?? ??? ?????. (FROM, WHERE ?).*/
	SET @intPageNumber = ISNULL(@intPageNumber, 1);
	SET @intPageSize = ISNULL(@intPageSize, 1000);

	SET @nvcStmt = N'
WITH PagingCTE AS (
	SELECT ROW_NUMBER() OVER(ORDER BY GMCopyLogID DESC) AS rowNumber, GMCopyLogID, LogDate, SessionID, RestrictType, InitCoin, CurrentCoin, CharacterLevel, BanAmount, IllegalAmount, dbo.FN_IPNumber2IP(IP) AS IP, CharacterName
	FROM dbo.GMCopyLog WITH (NOLOCK)
'		+ dbo.FN_DynamicWhereClause2(N'RestrictType', N'@nvcRestrictType', @inbNullBitmap, 1, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'IP', N'@intIP', @inbNullBitmap, 2, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'LogDate', N'@dtmStartLogDate', @inbNullBitmap, 3, @intColumnCount, '>=')
		+ dbo.FN_DynamicWhereClause2(N'LogDate', N'@dtmEndLogDate', @inbNullBitmap, 4, @intColumnCount, '<=') + N'
)
SELECT GMCopyLogID, LogDate, SessionID, RestrictType, InitCoin, CurrentCoin, CharacterLevel, BanAmount, IllegalAmount, IP, CharacterName
FROM PagingCTE
WHERE rowNumber BETWEEN (((@intPageNumber - 1) * @intPageSize) + 1) AND (@intPageNumber * @intPageSize)
ORDER BY rowNumber
OPTION (MAXDOP 1)';

	/**_# ?? ??? ?????.*/
	EXEC dbo.sp_executesql @nvcStmt
		, N'
			  @nvcRestrictType nvarchar(50)
			, @intIP int
			, @dtmStartLogDate datetime
			, @dtmEndLogDate datetime
			, @intPageNumber int
			, @intPageSize int
		'
		, @nvcRestrictType = @nvcRestrictType
		, @intIP = @intIP
		, @dtmStartLogDate = @dtmStartLogDate
		, @dtmEndLogDate = @dtmEndLogDate
		, @intPageNumber = @intPageNumber
		, @intPageSize = @intPageSize;

	/**_## ??? ???? ?? ???? ???, 102111? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 102111;
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

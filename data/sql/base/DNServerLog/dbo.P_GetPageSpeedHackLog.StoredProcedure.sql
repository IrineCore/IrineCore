USE [dnserverlog]
GO
/****** Object:  StoredProcedure [dbo].[P_GetPageSpeedHackLog]    Script Date: 11/14/2026 8:58:01 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-03-22
description : SpeedHack ??? ????? ?????? ?????.
result set : LogDate/datetime2, WorldID/int, AccountID/int, CharacterID/bigint, CharacterName/nvarchar
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetPageSpeedHackLog]
	  @intPageNumber int --// ??? ??.
	, @intPageSize int --// ??? ???.
	, @intWorldID int = NULL --// ?? ID
	, @nvcCharacterName nvarchar(30) = NULL --// ??? ??
	, @dt2StartLogDate datetime2(0) = NULL --// ?? ?? ??
	, @dt2EndLogDate datetime2(0) = NULL --// ?? ? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE
	@intReturnValue int,
	@nvcStmt nvarchar(max),
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
	SET @intColumnCount = 4;

	SET @inbNullBitmap = 
		CASE WHEN @intWorldID IS NOT NULL THEN POWER(2, 0) ELSE 0 END + 
		CASE WHEN @nvcCharacterName IS NOT NULL THEN POWER(2, 1) ELSE 0 END + 
		CASE WHEN @dt2StartLogDate IS NOT NULL THEN POWER(2, 2) ELSE 0 END + 
		CASE WHEN @dt2EndLogDate IS NOT NULL THEN POWER(2, 3) ELSE 0 END;

	/**_# ?? ??? ?????. (FROM, WHERE ?).*/
	SET @intPageNumber = ISNULL(@intPageNumber, 1);
	SET @intPageSize = ISNULL(@intPageSize, 1000);

	SET @nvcStmt = N'
WITH PagingCTE AS (
	SELECT ROW_NUMBER() OVER(ORDER BY LogDate) AS RowNumber, LogDate, WorldID, AccountID, CharacterID, CharacterName
	FROM dbo.V_SpeedHackLogs WITH (NOLOCK)
'		+ dbo.FN_DynamicWhereClause2(N'WorldID', N'@intWorldID', @inbNullBitmap, 1, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'CharacterName', N'@nvcCharacterName', @inbNullBitmap, 2, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'LogDate', N'@dt2StartLogDate', @inbNullBitmap, 3, @intColumnCount, '>=')
		+ dbo.FN_DynamicWhereClause2(N'LogDate', N'@dt2EndLogDate', @inbNullBitmap, 4, @intColumnCount, '<=') + N'
)
SELECT LogDate, WorldID, AccountID, CharacterID, CharacterName
FROM PagingCTE
WHERE RowNumber BETWEEN (((@intPageNumber - 1) * @intPageSize) + 1) AND (@intPageNumber * @intPageSize)
ORDER BY RowNumber
OPTION (MAXDOP 1)';

	/**_# ?? ??? ?????.*/
	EXEC dbo.sp_executesql @nvcStmt
		, N'
			  @intWorldID int
			, @nvcCharacterName nvarchar(30)
			, @dt2StartLogDate datetime2(0)
			, @dt2EndLogDate datetime2(0)
			, @intPageNumber int
			, @intPageSize int
		'
		, @intWorldID = @intWorldID
		, @nvcCharacterName = @nvcCharacterName
		, @dt2StartLogDate = @dt2StartLogDate
		, @dt2EndLogDate = @dt2EndLogDate
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

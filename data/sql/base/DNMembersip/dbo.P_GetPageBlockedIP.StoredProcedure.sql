USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetPageBlockedIP]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetPageBlockedIP]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetPageBlockedIP] AS' 
END
GO
/**
version : 3
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-04
Description : ??? IP ?? ??? ??????.
return value :
0 = There is no error.
1 = The transaction is in an uncommittable state. Rolling back transaction.
100 = System error occurred.
101105 = ?? IP ??? ???? ????.
history :
3 = StartDate, EndDate ?? ??? ?? ?? at 2010-07-07
**/

ALTER PROCEDURE [dbo].[P_GetPageBlockedIP]
	  @intPageNumber int --// Page number.
	, @intPageSize int --// The row count per page.
	, @intSelectClauseCode int --// 1 = StartIP, EndIP, BlockedIPCode, [Description], AdminID, RegisterDate
	, @vchStartIP varchar(15) = NULL --// ?? IP ??.
	, @vchEndIP varchar(15) = NULL --// ? IP ??.
	, @inyBlockedIPCode tinyint = NULL --// 
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;

DECLARE
	@nvcStmt nvarchar(max),
	@nvcSelectStmt nvarchar(max),
	@intStartIP int,
	@intEndIP int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END;

BEGIN TRY
	SET @intStartIP = ISNULL(dbo.FN_IP2IPNumber(@vchStartIP), dbo.FN_IP2IPNumber(@vchEndIP));
	SET @intEndIP = ISNULL(dbo.FN_IP2IPNumber(@vchEndIP), @intStartIP);

	/**_# ?? ??? ?????. (SELECT ?).*/
	SET @nvcSelectStmt = 
		CASE
			WHEN @intSelectClauseCode = 1 THEN N'StartIP, EndIP, BlockedIPCode, [Description], AdminID, RegisterDate, StartDate, EndDate'
		END;

	/**_# ?? ??? ?????. (FROM, WHERE ?).*/
	SET @intPageNumber = ISNULL(@intPageNumber, 1);
	SET @intPageSize = ISNULL(@intPageSize, 1000);
	SET @nvcStmt = N'
WITH PagingCTE AS (
	SELECT ROW_NUMBER() OVER(ORDER BY StartIP, EndIP) AS rowNumber, ' + @nvcSelectStmt + N'
	FROM dbo.BlockedIPs WITH (NOLOCK)
	WHERE (StartDate IS NULL OR EndDate > GETDATE())' +
	CASE
		WHEN @vchStartIP IS NULL AND @inyBlockedIPCode IS NULL THEN N''
		ELSE
			CASE
				WHEN @vchStartIP IS NOT NULL THEN N' AND ((@intStartIP BETWEEN StartIP AND EndIP)
	OR (@intEndIP BETWEEN StartIP AND EndIP)
	OR (StartIP BETWEEN @intStartIP AND @intEndIP)
	OR (EndIP BETWEEN @intStartIP AND @intEndIP))'
				ELSE N''
			END +
			CASE
				WHEN @inyBlockedIPCode IS NOT NULL THEN N' AND BlockedIPCode = @inyBlockedIPCode'
				ELSE N''
			END
	END + N'
)
SELECT dbo.FN_IPNumber2IP(StartIP) AS StartIP, dbo.FN_IPNumber2IP(EndIP) AS EndIP, BlockedIPCode, [Description], AdminID, RegisterDate
FROM PagingCTE
WHERE rowNumber BETWEEN (((@intPageNumber - 1) * @intPageSize) + 1) AND (@intPageNumber * @intPageSize)
ORDER BY rowNumber
OPTION (MAXDOP 1)';

	/**_# ?? ??? ?????.*/
	EXEC dbo.sp_executesql @nvcStmt
		, N'@intStartIP int, @intEndIP int, @inyBlockedIPCode tinyint, @intPageNumber int, @intPageSize int'
		, @intStartIP = @intStartIP, @intEndIP = @intEndIP, @inyBlockedIPCode = @inyBlockedIPCode, @intPageNumber = @intPageNumber, @intPageSize = @intPageSize;

	/**_## ??? ???? ?? ???? ??? 101105? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 101105;
		GOTO ErrorHandler;
	END;
END TRY
BEGIN CATCH
	GOTO ErrorHandler;
END CATCH

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

IF @intReturnValue IS NULL OR @intReturnValue = 0
	EXEC @intReturnValue = dbo.P_AddErrorLog;

RETURN @intReturnValue;
GO

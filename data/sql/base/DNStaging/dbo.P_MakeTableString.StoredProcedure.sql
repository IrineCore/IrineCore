USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_MakeTableString]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : otooi@eyedentitygames.com
created date : 2012-02-08
description : ??? ??? ???? ??? ?? ???? ????. ???? ???? OUTPUT ????? ?????.
result set : 
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
1 = ????
**/

CREATE PROCEDURE [dbo].[P_MakeTableString]
	  @nvcDBName nvarchar(50) --// DB ??
	, @nvcTblName nvarchar(50) --// ??? ??
	, @nvcStartDate nvarchar(8) --// ?? ?? ?? ??
	, @nvcEndDate nvarchar(8) --// ?? ?? ?? ??
	, @nvcHint nvarchar(50) --// ??? ??
	, @nvcStmtOUT nvarchar(max) OUTPUT --// ?? ??? OUTPUT
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @nvcStmt nvarchar(max) = N''

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SET @nvcStmtOUT = N'';
	
	/** sys.databases ???? ???? ??? ?? ???? ???? ???? ????.**/
	SELECT @nvcStmt = @nvcStmt + 'UNION ALL SELECT N''UNION ALL SELECT * FROM '+name+'.dbo.'' + name + ''' + CASE WHEN @nvcHint IS NULL THEN N'' ELSE N' WITH (' + @nvcHint + N')' END + ''' AS [Stmt] FROM '
				+ name + '.sys.tables WHERE name LIKE N''' + @nvcTblName + N'%'' AND name BETWEEN N''' + @nvcTblName + N'_' + @nvcStartDate + ''' AND N''' + @nvcTblName + N'_' + @nvcEndDate + N'''' + char(13)
	FROM  
	(
		SELECT name FROM sys.databases
		WHERE name LIKE @nvcDBName + N'Log%' AND name >= @nvcDBName + N'Log_' + LEFT(CONVERT(nvarchar(6), DATEADD(MM, -3, GETDATE()), 112), 6)
		UNION ALL 
		SELECT @nvcDBName AS name
	)x;
	
	/** ???? ???? ??? ?? ???? OUTPUT ????? ??? ? ?? ???? ?????.**/
	SET @nvcStmt = N'
		SELECT @nvcStmtOUT = @nvcStmtOUT + Stmt + CHAR(13)
		FROM
		(' + STUFF(@nvcStmt, 1, 10, N'') + N') X
		SET @nvcStmtOUT = STUFF(@nvcStmtOUT, 1, 10, N'''')';
	
	EXEC dbo.sp_executesql @nvcStmt, N'@nvcStmtOUT nvarchar(max) OUTPUT', @nvcStmtOUT = @nvcStmtOUT OUTPUT;
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

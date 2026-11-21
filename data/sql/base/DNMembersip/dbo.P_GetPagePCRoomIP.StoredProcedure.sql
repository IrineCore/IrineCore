USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetPagePCRoomIP]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetPagePCRoomIP]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetPagePCRoomIP] AS' 
END
GO
/**
version : 5
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-05-04
description : PC? IP ??? ??????.
result set : IP/varchar, PCRoomGrade/tinyint
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101161 = ??? ???? PC? IP??? ????.
history :
5 = ?? ??? PCRoomGrade ?? at 2011-03-08
**/

ALTER PROCEDURE [dbo].[P_GetPagePCRoomIP]
	  @intPageNumber int --// ??? ??.
	, @intPageSize int --// ??? ???.
	, @vchStartIP varchar(15) = NULL --// ?? IP
	, @vchEndIP varchar(15) = NULL --// ? IP
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @nvcStmt nvarchar(max);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SET @nvcStmt = N'
WITH C1 AS (
	SELECT ROW_NUMBER() OVER(ORDER BY IP) AS Seq, IP, PCRoomGrade
	FROM dbo.PCRoomIPs WITH (NOLOCK)' + 
	CASE
		WHEN @vchStartIP IS NOT NULL OR @vchEndIP IS NOT NULL THEN N'
	WHERE '
			+ CASE WHEN @vchStartIP IS NOT NULL THEN N'IP >= ' + CAST(dbo.FN_IP2IPNumber(@vchStartIP) AS nvarchar(11)) + CASE WHEN @vchEndIP IS NOT NULL THEN N' AND ' ELSE N'' END ELSE N'' END
			+ CASE WHEN @vchEndIP IS NOT NULL THEN N'IP <= ' + CAST(dbo.FN_IP2IPNumber(@vchEndIP) AS nvarchar(11)) ELSE N'' END
		ELSE N''
	END + N'
)
SELECT dbo.FN_IPNumber2IP(IP) AS IP, PCRoomGrade
FROM C1
WHERE Seq BETWEEN (((@intPageNumber - 1) * @intPageSize) + 1) AND (@intPageNumber * @intPageSize)
ORDER BY Seq;';

	EXEC sp_executesql @nvcStmt, N'@intPageNumber int, @intPageSize int', @intPageNumber = @intPageNumber, @intPageSize = @intPageSize;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 101161;
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

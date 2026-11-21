USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetCountBlockedIPException]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetCountBlockedIPException]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetCountBlockedIPException] AS' 
END
GO
/**
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-07
Description : ?? ?? IP ?? ??? ??????? ???? ??????.
result set : recordCount
return value :
0 = There is no error.
1 = The transaction is in an uncommittable state. Rolling back transaction.
100 = System error occurred.
**/

ALTER PROCEDURE [dbo].[P_GetCountBlockedIPException]
	  @vchStartIP varchar(15) = NULL --// ?? IP ??.
	, @vchEndIP varchar(15) = NULL --// ? IP ??.
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;

DECLARE
	@nvcStmt nvarchar(max),
	@intStartIP int,
	@intEndIP int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END;

BEGIN TRY
	SET @intStartIP = COALESCE(dbo.FN_IP2IPNumber(@vchStartIP), dbo.FN_IP2IPNumber(@vchEndIP));
	SET @intEndIP = COALESCE(dbo.FN_IP2IPNumber(@vchEndIP), @intStartIP);

	/**_# ?? ??? ?????.*/
	SET @nvcStmt = N'
SELECT COUNT(*) AS recordCount
FROM dbo.BlockedIPExceptions WITH (NOLOCK)
' +
	CASE
		WHEN @vchStartIP IS NULL THEN N''
		ELSE N'WHERE
   (@intStartIP BETWEEN StartIP AND EndIP)
OR (@intEndIP BETWEEN StartIP AND EndIP)
OR (StartIP BETWEEN @intStartIP AND @intEndIP)
OR (EndIP BETWEEN @intStartIP AND @intEndIP)'
	END;

	/**_# ?? ??? ?????.*/
	EXEC dbo.sp_executesql @nvcStmt
		, N'@intStartIP int, @intEndIP int'
		, @intStartIP = @intStartIP, @intEndIP = @intEndIP;
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

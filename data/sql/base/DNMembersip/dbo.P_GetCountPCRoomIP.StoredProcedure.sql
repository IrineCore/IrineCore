USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetCountPCRoomIP]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetCountPCRoomIP]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetCountPCRoomIP] AS' 
END
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-05-04
description : PC? IP ??? ????? ?? ???? ??????.
result set : RecordCount/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

ALTER PROCEDURE [dbo].[P_GetCountPCRoomIP]
	  @vchStartIP varchar(15) = NULL --// ?? IP
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
SELECT COUNT(*) AS RecordCount
FROM dbo.PCRoomIPs WITH (NOLOCK)' + 
	CASE
		WHEN @vchStartIP IS NOT NULL OR @vchEndIP IS NOT NULL THEN N'
WHERE '
			+ CASE WHEN @vchStartIP IS NOT NULL THEN N'IP >= ' + CAST(dbo.FN_IP2IPNumber(@vchStartIP) AS nvarchar(11)) + CASE WHEN @vchEndIP IS NOT NULL THEN N' AND ' ELSE N'' END ELSE N'' END
			+ CASE WHEN @vchEndIP IS NOT NULL THEN N'IP <= ' + CAST(dbo.FN_IP2IPNumber(@vchEndIP) AS nvarchar(11)) ELSE N'' END
		ELSE N''
	END;

	EXEC sp_executesql @nvcStmt;
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

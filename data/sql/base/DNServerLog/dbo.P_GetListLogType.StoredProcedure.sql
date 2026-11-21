USE [dnserverlog]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListLogType]    Script Date: 11/14/2026 8:58:01 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-03-24
description : ?? ?? ??? ?????.
result set : LogType/tinyint, LogTypeName/nvarchar, HideFlag/bit
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListLogType]
	  @bitHideFlag bit --// NULL=??, 0=???, 1=???
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT LogType, LogTypeName, HideFlag
	FROM dbo.LogTypes WITH (NOLOCK)
	WHERE HideFlag = COALESCE(@bitHideFlag, HideFlag)
	ORDER BY LogType;
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

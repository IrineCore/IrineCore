USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListAbuseLog]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-02-21
description : ??? ??? ?????.
result set : LogDate, AbuseCode, CharacterID, LogMessage
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListAbuseLog]
	  @dtmStartDate datetime --// ?? ?? ??
	, @dtmEndDate datetime --// ?? ? ??
	, @inyAbuseCode tinyint --// 1=??<->?? ??, 2=?? ??, 3=????? ?? ???? ??
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
	SELECT LogDate, AbuseCode, CharacterID, LogMessage
	FROM dbo.AbuseLogs WITH (NOLOCK)
	WHERE LogDate BETWEEN @dtmStartDate AND @dtmEndDate
		AND AbuseCode = COALESCE(@inyAbuseCode, AbuseCode)
	ORDER BY LogDate;
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

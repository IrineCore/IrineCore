USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddAbuseLog]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-02-21
description : ??? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_AddAbuseLog]
	  @inyAbuseCode tinyint --// 1=??<->?? ??, 2=?? ??, 3=????? ?? ???? ??, 4=?? ??, 5=DB ??
	, @inbCharacterID bigint --// ??? ID
	, @nvcLogMessage nvarchar(2000) --// ??? ?? ???
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
	/**_# [AbuseLogs] ???? INSERT???.*/
	INSERT dbo.AbuseLogs (LogDate, AbuseCode, CharacterID, LogMessage)
	VALUES (GETDATE(), @inyAbuseCode, @inbCharacterID, @nvcLogMessage);
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

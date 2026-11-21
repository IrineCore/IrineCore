USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddAbuseMonitor]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-05-16
description : ???? ??? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_AddAbuseMonitor]
	  @inbCharacterID bigint --// ??? ID
	, @intAbuseCount1 int --// ???
	, @intAbuseCount2 int = 0 --// ??? (chat)
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
	UPDATE dbo.AbuseMonitor
	SET AbuseCount1 += @intAbuseCount1
		, AbuseCount2 += @intAbuseCount2
	WHERE CharacterID = @inbCharacterID;

	IF @@ROWCOUNT = 0
		INSERT dbo.AbuseMonitor (CharacterID, AbuseCount1, AbuseCount2)
		VALUES (@inbCharacterID, @intAbuseCount1, @intAbuseCount2);
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

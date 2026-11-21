USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_UpsTempSecondAuthStatus]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2011-09-20
description : TempSecondAuthStatus UPSERT ???.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_UpsTempSecondAuthStatus]
WITH EXECUTE AS 'SPExecutor'
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
	-- ?? ??? 1 ?? ??? ??? ??
	DELETE dbo.TempSecondAuthStatus
	WHERE AccountID IN (SELECT AccountID FROM dbo.SecondAuthLogsBuffer WITH (NOLOCK) WHERE ISNULL(SecondAuthCode, 0) <= 1)

	-- ?? ??? 1 ?? ?? ?? (?? ???? ?? ??)
	UPDATE dbo.TempSecondAuthStatus
	SET SecondAuthCode = SALB.SecondAuthCode
	FROM dbo.TempSecondAuthStatus TSAS
		   JOIN dbo.SecondAuthLogsBuffer SALB WITH (NOLOCK) ON (TSAS.AccountID = SALB.AccountID)
	WHERE ISNULL(SALB.SecondAuthCode, 0) > 1

	-- ?? ??? 1 ?? ?? ?? (?? ???? ?? ??)
	INSERT dbo.TempSecondAuthStatus (AccountID, SecondAuthCode)
	SELECT AccountID
		 , SecondAuthCode
	FROM dbo.SecondAuthLogsBuffer WITH (NOLOCK)
	WHERE ISNULL(SecondAuthCode, 0) > 1
	   AND AccountID NOT IN (SELECT AccountID FROM dbo.TempSecondAuthStatus WITH (NOLOCK))
END TRY
BEGIN CATCH
    -- ?? ??
    INSERT dbo.DailySnapshotErrorLog (ErrorDate, ErrorStep, ErrorNumber, ErrorMessage)
    SELECT GETDATE(), N'dbo.TempSecondAuthStatus Fail..',  ERROR_NUMBER(), ERROR_MESSAGE()

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

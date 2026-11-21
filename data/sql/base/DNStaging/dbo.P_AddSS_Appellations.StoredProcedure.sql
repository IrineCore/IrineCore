USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_AddSS_Appellations]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 10
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2011-08-22

description :
	??? ??? SS_Appellations? ?????.
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
	6 = dbo.SS_AppellationCounts ?? ?? ??     by 20110824.wgkim
	7 = ??? ???? ???? ???? DISTINCT? ??? ??	by 20110826.ohtack
	8 = ??? ?? ?? ?? ??	by 20110906.ohtack
	9 = SS_AppellationCounts ??? TRUNCATE ?? ??	by 20110907.ohtack
	10 = MAXDOP OPTION ?? by ??? at 2011-12-20
**/

CREATE PROCEDURE [dbo].[P_AddSS_Appellations]

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

	/**_# [AppellationsBuffer] ???? ???? ?????. */
	IF NOT EXISTS (SELECT * FROM sys.indexes WHERE name = N'IX_NC_AppellationsBuffer')
		CREATE CLUSTERED INDEX IX_NC_AppellationsBuffer ON dbo.AppellationsBuffer (CharacterID);

	/**_# [SS_Appellations] ????? [AppellationsBuffer]? ?? Character ? ???? DELETE??? */
	DELETE dbo.SS_Appellations
	  FROM dbo.SS_Appellations A
	 WHERE EXISTS (SELECT * FROM dbo.AppellationsBuffer B WITH (NOLOCK) WHERE A.CharacterID = B.CharacterID);

	/**_# [SS_Appellations] ???? [AppellationsBuffer]? ???? INSERT ??? */
	INSERT dbo.SS_Appellations (CharacterID, AppellationID, ChoiceFlag)
	SELECT DISTINCT CharacterID, AppellationID, ChoiceFlag
	  FROM dbo.AppellationsBuffer WITH (NOLOCK);
	
	/**_# AppellationsBuffer ???? ??? ???? ?????. */
	IF EXISTS (SELECT * FROM sys.indexes WHERE name = N'IX_NC_AppellationsBuffer')
		DROP INDEX IX_NC_AppellationsBuffer ON dbo.AppellationsBuffer;

	/**_# SS_AppellationCounts? ?? ???? ?????. */
	TRUNCATE TABLE dbo.SS_AppellationCounts;

	/**_# SS_AppellationCounts ???? ????? ??? ??? ?? ??? ?????. */
    INSERT dbo.SS_AppellationCounts (CharacterID, AppellationCount)
    SELECT CharacterID
         , COUNT(AppellationID)
      FROM dbo.SS_Appellations WITH (NOLOCK)
     GROUP BY CharacterID;

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

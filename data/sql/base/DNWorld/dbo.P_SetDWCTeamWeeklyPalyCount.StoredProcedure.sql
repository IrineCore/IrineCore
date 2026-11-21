USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_SetDWCTeamWeeklyPalyCount]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-01-11
description : DWC ?? ??? ??? ? ???.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103361 = ???? ?? DWC????.
**/

CREATE PROCEDURE [dbo].[P_SetDWCTeamWeeklyPalyCount]
	  @intDWCTeamID int --// DWC ?ID
	, @intDWCPoint int = NULL --//(OPTIONAL)DWC ???(?? ??? ??? ?? ???? ???? ?? ??)
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
	/**_# [DWCTeams] ???? UPDATE???.*/
	UPDATE dbo.DWCTeams
	SET WeeklyPlayCount = 0
	  , DWCPoint += ISNULL(@intDWCPoint, 0)
	WHERE DWCTeamID = @intDWCTeamID AND CloseFlag = 0;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103361;
		GOTO ErrorHandler;
	END

	SET @intDWCTeamID = SCOPE_IDENTITY();
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

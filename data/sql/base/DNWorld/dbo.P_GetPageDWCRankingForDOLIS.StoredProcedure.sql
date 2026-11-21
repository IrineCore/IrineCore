USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetPageDWCRankingForDOLIS]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-01-15
description : DWC ??? ?????.(???)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetPageDWCRankingForDOLIS]
	  @intPageNumber int --// ??? ??.
	, @intPageSize int --// ??? ???.	
	, @intDWCSequence int = NULL--// (OPTIONAL)DWC ??(?? ??? ????? ??? NULL? ??
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
	/**_# [DWCRanking] ???? ?????.*/
	IF @intDWCSequence IS NULL
	BEGIN
		SELECT Ranking, DWCTeamID, DWCTeamName, Win, Lose, Draw, DWCPoint, ChangedRanking, DWCTeamLeaderCharacterInfo, DWCTeamMemberCharacterInfo
		FROM dbo.DWCRanking WITH(NOLOCK, INDEX(PK_DWCRanking), FORCESEEK)
		WHERE Ranking BETWEEN (((@intPageNumber - 1) * @intPageSize) + 1) AND (@intPageNumber * @intPageSize)
		OPTION(KEEPFIXED PLAN);
	END
	ELSE
	BEGIN
		SELECT Ranking, DWCTeamID, DWCTeamName, Win, Lose, Draw, DWCPoint, ChangedRanking, DWCTeamLeaderCharacterInfo, DWCTeamMemberCharacterInfo
		FROM dbo.PastDWCRanking WITH(NOLOCK)
		WHERE DWCSequence = @intDWCSequence
          AND Ranking BETWEEN (((@intPageNumber - 1) * @intPageSize) + 1) AND (@intPageNumber * @intPageSize);
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

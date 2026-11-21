USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetPageDWCRanking]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-01-14
description : DWC ?? ??? ????? ?????? ?????.
result set : Ranking/int, DWCTeamID/int, DWCTeamName/nvarchar(50), Win/int, Lose/int, Draw/int, DWCPoint/int, ChangedRanking/int, DWCTeamLeaderCharacterInfo/nvarchar(80), DWCTeamMemberCharacterInfo/nvarchar(160)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetPageDWCRanking]
	  @intPageNumber int --// ??? ??.
	, @intPageSize int --// ??? ???.
	, @intTotalRankingCount int OUTPUT--// ?? ?? ???	
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
	SELECT @intTotalRankingCount = TotalCount
	FROM dbo.DWCRankingTotalCount WITH(NOLOCK)

	/**_# [DWCRanking] ???? ?????.*/
	SELECT Ranking, DWCTeamID, DWCTeamName, Win, Lose, Draw, DWCPoint, ChangedRanking, DWCTeamLeaderCharacterInfo, DWCTeamMemberCharacterInfo
	FROM dbo.DWCRanking WITH(NOLOCK, INDEX(PK_DWCRanking), FORCESEEK)
	WHERE Ranking BETWEEN (((@intPageNumber - 1) * @intPageSize) + 1) AND (@intPageNumber * @intPageSize)
	OPTION(KEEPFIXED PLAN);
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

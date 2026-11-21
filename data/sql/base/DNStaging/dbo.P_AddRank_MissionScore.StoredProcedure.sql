USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_AddRank_MissionScore]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-02-22
description : ?? ?? ??? ?????
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_AddRank_MissionScore]
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @datBasisDate date = GETDATE() - 1;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	TRUNCATE TABLE dbo.Rank_MissionScore;

	WITH C0 AS (
		SELECT A.CharacterID, DC.WorldID, DCS.CharacterLevel, DCS.JobCode, A.MissionScore, DC.CharacterClassCode
		FROM dbo.SS_AchieveMissionCounts A WITH (NOLOCK)
			INNER JOIN dbo.DimCharacters DC WITH (NOLOCK) ON A.CharacterID = DC.CharacterID
			INNER JOIN dbo.DailyCharacterStatus DCS WITH (NOLOCK) ON A.CharacterID = DCS.CharacterID
		WHERE DC.DeleteFlag = CAST(0 AS bit) 
          AND DCS.BasisDate = @datBasisDate
          AND NOT EXISTS (
			SELECT * FROM dbo.SS_RestraintedAccounts WITH (NOLOCK) WHERE AccountID = DC.AccountID
		)
	), C1 AS (
		SELECT CharacterID, RANK() OVER(ORDER BY MissionScore DESC) AS TotalRank FROM C0
	), C2 AS (
		SELECT CharacterID, RANK() OVER(PARTITION BY WorldID ORDER BY MissionScore DESC) AS WorldRank FROM C0
	), C3 AS (
		SELECT CharacterID, RANK() OVER(PARTITION BY WorldID, CharacterClassCode ORDER BY MissionScore DESC) AS WorldCharacterClassRank FROM C0
	), C4 AS (
		SELECT CharacterID, RANK() OVER(PARTITION BY CharacterClassCode ORDER BY MissionScore DESC) AS CharacterClassRank FROM C0
	)
	SELECT C0.CharacterID, C0.WorldID, C0.CharacterLevel, C0.JobCode, C0.MissionScore, C1.TotalRank, C2.WorldRank, C3.WorldCharacterClassRank, C4.CharacterClassRank
	INTO #Rank_MissionScore
	FROM C0
		INNER JOIN C1 ON C0.CharacterID = C1.CharacterID
		INNER JOIN C2 ON C0.CharacterID = C2.CharacterID
		INNER JOIN C3 ON C0.CharacterID = C3.CharacterID
		INNER JOIN C4 ON C0.CharacterID = C4.CharacterID
	ORDER BY C0.CharacterID;

	INSERT dbo.Rank_MissionScore (CharacterID, WorldID, CharacterLevel, JobCode, MissionScore, TotalRank, WorldRank, WorldCharacterClassRank, CharacterClassRank)
	SELECT CharacterID, WorldID, CharacterLevel, JobCode, MissionScore, TotalRank, WorldRank, WorldCharacterClassRank, CharacterClassRank
	FROM #Rank_MissionScore

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

USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddPVPRanking]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 15
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-07-26
description : PVP ??? ?? ? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_AddPVPRanking]	  
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	  , @intPVPCharacterClassScoresCount int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT @intPVPCharacterClassScoresCount = ISNULL(COUNT(*), 0)
	FROM dbo.PVPCharacterClassScores WITH (NOLOCK);

	IF @intPVPCharacterClassScoresCount IS NULL
		SET @intPVPCharacterClassScoresCount = 0;

	/**_# ?? ?? ???? ?????.*/
	IF OBJECT_ID('tempdb..#tblPVPRanking') IS NOT NULL
		DROP TABLE #tblPVPRanking;

	CREATE TABLE #tblPVPRanking (
		CharacterID	bigint	NOT NULL PRIMARY KEY,
		CharacterName	nvarchar(30)	NOT NULL,
		CharacterClassCode	tinyint	NOT NULL,
		CharacterSubClassCode	tinyint	NOT NULL,
		JobCode	tinyint	NOT NULL,
		GuildID	int	NULL,
		GuildName	nvarchar(50)	NULL,
		PVPExp	int	NOT NULL,
		[Kill]	int	NOT NULL,
		Death	int	NOT NULL,
		PVPClassRank	int	NOT NULL,
		PVPClassRatio	float	NOT NULL,
		TotalRank	int	NOT NULL,
		CharacterClassRank	int	NOT NULL,
		CharacterSubClassRank	int	NOT NULL,
		ChangedTotalRank	int	NULL,
		ChangedCharacterClassRank	int	NULL,
		ChangedCharacterSubClassRank	int	NULL,	
		CharacterLevel	tinyint	NULL,
		PVPLevel	tinyint	NULL
	);
	
	/**_# ??? ??? ?????.*/
	IF @intPVPCharacterClassScoresCount > 0
		WITH PC AS (
			SELECT CharacterID, SUM(Win) AS [Kill], SUM(Lose) AS Death
			FROM dbo.PVPCharacterClassScores WITH (NOLOCK)
			GROUP BY CharacterID
		), BR AS (
		SELECT PS.CharacterID, CS.CharacterLevel, CH.CharacterClassCode, 
			   CASE WHEN CS.JobCode IN (11,23,24) THEN 11 
			   WHEN CS.JobCode IN (12,25,26) THEN 12 
			   WHEN CS.JobCode IN (14,29,30) THEN 14
			   WHEN CS.JobCode IN (15,31,32) THEN 15
			   WHEN CS.JobCode IN (17,35,36) THEN 17
			   WHEN CS.JobCode IN (18,37,38) THEN 18
			   WHEN CS.JobCode IN (20,41,42) THEN 20
			   WHEN CS.JobCode IN (22,43,44) THEN 22
			   WHEN CS.JobCode IN (46,47,48) THEN 46
			   WHEN CS.JobCode IN (49,50,51) THEN 49
			   WHEN CS.JobCode IN (54,55,56) THEN 54
			   WHEN CS.JobCode IN (57,58,59) THEN 57
			   WHEN CS.JobCode IN (62,63,64) THEN 62
			   WHEN CS.JobCode IN (67,68,69) THEN 67
			   ELSE CS.JobCode END AS CharacterSubClassCode, 
			   ISNULL(PR.Ranking, 0) AS PVPClassRank, ISNULL(PR.RankRatio, 0) AS PVPClassRankRatio, G.GuildID, G.GuildName, CH.CharacterName, CS.JobCode,
			   ISNULL(PC.[Kill], 0) AS [Kill], ISNULL(PC.Death, 0) AS Death, PS.PVPExp, PS.PVPLevel
		FROM dbo.PVPScores PS WITH (NOLOCK) 
			 INNER JOIN dbo.Characters CH WITH (NOLOCK, INDEX(PK_Characters), FORCESEEK) ON PS.CharacterID = CH.CharacterID
			 INNER JOIN dbo.CharacterStatus CS WITH (NOLOCK, INDEX(PK_CharacterStatus), FORCESEEK) ON CH.CharacterID = CS.CharacterID
			 LEFT OUTER JOIN PC WITH (NOLOCK) ON PS.CharacterID = PC.CharacterID     
			 LEFT OUTER JOIN dbo.GuildMembers GM WITH (NOLOCK, INDEX(IX_UN_GuildMembers_CharacterID), FORCESEEK) ON CH.CharacterID = GM.CharacterID
			 LEFT OUTER JOIN dbo.Guilds G WITH (NOLOCK, INDEX(PK_Guilds), FORCESEEK) ON GM.GuildID = G.GuildID
			 LEFT OUTER JOIN dbo.PVPTopRanker PR WITH (NOLOCK, INDEX(PK_PVPTopRanker), FORCESEEK) ON CH.CharacterID = PR.CharacterID
		WHERE CH.DeleteFlag = 0 AND CH.AccountLevelCode < 99
		)
		INSERT #tblPVPRanking (CharacterID, CharacterName, CharacterClassCode, CharacterSubClassCode, JobCode, GuildID, GuildName, PVPExp, [Kill], Death, PVPClassRank
			   , PVPClassRatio, TotalRank, CharacterClassRank, CharacterSubClassRank, ChangedTotalRank, ChangedCharacterClassRank, ChangedCharacterSubClassRank, CharacterLevel, PVPLevel)
		SELECT CharacterID, CharacterName, CharacterClassCode, CharacterSubClassCode, JobCode, GuildID, GuildName, PVPExp, [Kill], Death, PVPClassRank, PVPClassRankRatio,
			   RANK() OVER(ORDER BY PVPExp DESC, [Kill] DESC, CASE WHEN [Kill] + Death = 0 THEN 0 ELSE (CAST ([Kill] AS float) / ([Kill] + Death)) * 100 END DESC, CharacterID DESC) AS TotalRank, 
			   RANK() OVER(PARTITION BY CharacterClassCode ORDER BY PVPExp DESC, [Kill] DESC, CASE WHEN [Kill] + Death = 0 THEN 0 ELSE (CAST ([Kill] AS float) / ([Kill] + Death)) * 100 END DESC, CharacterID DESC) AS CharacterClassRanking,
			   RANK() OVER(PARTITION BY CharacterSubClassCode ORDER BY PVPExp DESC, [Kill] DESC, CASE WHEN [Kill] + Death = 0 THEN 0 ELSE (CAST ([Kill] AS float) / ([Kill] + Death)) * 100 END DESC, CharacterID DESC) AS CharacterSubClassRank,
			   0,0,0, CharacterLevel, PVPLevel	   
		FROM BR;
	ELSE
		WITH BR AS (
		SELECT PS.CharacterID, CS.CharacterLevel, CH.CharacterClassCode, 
			   CASE WHEN CS.JobCode IN (11,23,24) THEN 11 
			   WHEN CS.JobCode IN (12,25,26) THEN 12 
			   WHEN CS.JobCode IN (14,29,30) THEN 14
			   WHEN CS.JobCode IN (15,31,32) THEN 15
			   WHEN CS.JobCode IN (17,35,36) THEN 17
			   WHEN CS.JobCode IN (18,37,38) THEN 18
			   WHEN CS.JobCode IN (20,41,42) THEN 20
			   WHEN CS.JobCode IN (22,43,44) THEN 22
			   WHEN CS.JobCode IN (46,47,48) THEN 46
			   WHEN CS.JobCode IN (49,50,51) THEN 49
			   WHEN CS.JobCode IN (54,55,56) THEN 54
			   WHEN CS.JobCode IN (57,58,59) THEN 57
			   WHEN CS.JobCode IN (62,63,64) THEN 62
			   WHEN CS.JobCode IN (67,68,69) THEN 67
			   ELSE CS.JobCode END AS CharacterSubClassCode, 
			   ISNULL(PR.Ranking, 0) AS PVPClassRank, ISNULL(PR.RankRatio, 0) AS PVPClassRankRatio, G.GuildID, G.GuildName, CH.CharacterName, CS.JobCode,
			   ISNULL(PS.vsWarriorKOWin, 0) + ISNULL(PS.vsArcherKOWin, 0) + ISNULL(PS.vsClericKOWin, 0) + ISNULL(PS.vsSorceressKOWin, 0) + ISNULL(PS.vsAcademicKOWin, 0) AS [Kill], 
			   ISNULL(PS.vsWarriorKOLose, 0) + ISNULL(PS.vsArcherKOLose, 0) + ISNULL(PS.vsClericKOLose, 0) + ISNULL(PS.vsSorceressKOLose, 0) + ISNULL(PS.vsAcademicKOLose, 0) AS Death, 
			   PS.PVPExp, PS.PVPLevel
		FROM dbo.PVPScores PS WITH (NOLOCK) 
			 INNER JOIN dbo.Characters CH WITH (NOLOCK, INDEX(PK_Characters), FORCESEEK) ON PS.CharacterID = CH.CharacterID
			 INNER JOIN dbo.CharacterStatus CS WITH (NOLOCK, INDEX(PK_CharacterStatus), FORCESEEK) ON CH.CharacterID = CS.CharacterID			 
			 LEFT OUTER JOIN dbo.GuildMembers GM WITH (NOLOCK, INDEX(IX_UN_GuildMembers_CharacterID), FORCESEEK) ON CH.CharacterID = GM.CharacterID
			 LEFT OUTER JOIN dbo.Guilds G WITH (NOLOCK, INDEX(PK_Guilds), FORCESEEK) ON GM.GuildID = G.GuildID
			 LEFT OUTER JOIN dbo.PVPTopRanker PR WITH (NOLOCK, INDEX(PK_PVPTopRanker), FORCESEEK) ON CH.CharacterID = PR.CharacterID
		WHERE CH.DeleteFlag = 0  AND CH.AccountLevelCode < 99
		)
		INSERT #tblPVPRanking (CharacterID, CharacterName, CharacterClassCode, CharacterSubClassCode, JobCode, GuildID, GuildName, PVPExp, [Kill], Death, PVPClassRank
			   , PVPClassRatio, TotalRank, CharacterClassRank, CharacterSubClassRank, ChangedTotalRank, ChangedCharacterClassRank, ChangedCharacterSubClassRank, CharacterLevel, PVPLevel)
		SELECT CharacterID, CharacterName, CharacterClassCode, CharacterSubClassCode, JobCode, GuildID, GuildName, PVPExp, [Kill], Death, PVPClassRank, PVPClassRankRatio,
			   RANK() OVER(ORDER BY PVPExp DESC, [Kill] DESC, CASE WHEN [Kill] + Death = 0 THEN 0 ELSE (CAST ([Kill] AS float) / ([Kill] + Death)) * 100 END DESC, CharacterID DESC) AS TotalRank, 
			   RANK() OVER(PARTITION BY CharacterClassCode ORDER BY PVPExp DESC, [Kill] DESC, CASE WHEN [Kill] + Death = 0 THEN 0 ELSE (CAST ([Kill] AS float) / ([Kill] + Death)) * 100 END DESC, CharacterID DESC) AS CharacterClassRanking,
			   RANK() OVER(PARTITION BY CharacterSubClassCode ORDER BY PVPExp DESC, [Kill] DESC, CASE WHEN [Kill] + Death = 0 THEN 0 ELSE (CAST ([Kill] AS float) / ([Kill] + Death)) * 100 END DESC, CharacterID DESC) AS CharacterSubClassRank,
			   0,0,0, CharacterLevel, PVPLevel	   
		FROM BR;
	
	UPDATE #tblPVPRanking
	SET ChangedTotalRank = CASE WHEN PR.TotalRank IS NULL THEN NULL ELSE ISNULL(PR.TotalRank,0) - ISNULL(NR.TotalRank,0) END,
		ChangedCharacterClassRank = CASE WHEN PR.CharacterClassRank IS NULL THEN NULL ELSE ISNULL(PR.CharacterClassRank,0) - ISNULL(NR.CharacterClassRank,0) END,
		ChangedCharacterSubClassRank = CASE WHEN PR.CharacterSubClassRank IS NULL THEN NULL ELSE ISNULL(PR.CharacterSubClassRank,0) - ISNULL(NR.CharacterSubClassRank,0) END
	FROM #tblPVPRanking NR LEFT OUTER JOIN dbo.PVPRanking PR WITH (NOLOCK) 
		 ON NR.CharacterID = PR.CharacterID

	/**_# ?? ?? ???? TRUNCATE ?? ??? ?????? INSERT ???.*/
	TRUNCATE TABLE dbo.PVPRanking;

	INSERT dbo.PVPRanking (CharacterID, CharacterName, CharacterClassCode, CharacterSubClassCode, JobCode, GuildID, GuildName, PVPExp, [Kill], Death, PVPClassRank, PVPClassRatio, TotalRank, CharacterClassRank, CharacterSubClassRank, ChangedTotalRank, ChangedCharacterClassRank, ChangedCharacterSubClassRank, CharacterLevel, PVPLevel)
	SELECT CharacterID, CharacterName, CharacterClassCode, CharacterSubClassCode, JobCode, GuildID, GuildName, PVPExp, [Kill], Death, PVPClassRank, PVPClassRatio, TotalRank, CharacterClassRank, CharacterSubClassRank, ChangedTotalRank, ChangedCharacterClassRank, ChangedCharacterSubClassRank, CharacterLevel, PVPLevel
	FROM #tblPVPRanking;
	
	
	/**_# ?? ?? ?? TotalCount? ?????.*/
	TRUNCATE TABLE dbo.PVPRankingCount;
	
	INSERT dbo.PVPRankingCount(CharacterClassCode, TotalCount)
	SELECT CharacterClassCode, COUNT(*) 
	FROM dbo.PVPRanking WITH(NOLOCK)
	GROUP BY CharacterClassCode;

	INSERT dbo.PVPRankingCount(CharacterSubClassCode, TotalCount)
	SELECT CharacterSubClassCode, COUNT(*) 
	FROM dbo.PVPRanking WITH(NOLOCK)
	GROUP BY CharacterSubClassCode;

	INSERT dbo.PVPRankingCount(GuildName, TotalCount)
	SELECT GuildName, COUNT(*) 
	FROM dbo.PVPRanking WITH(NOLOCK)
	WHERE GuildName IS NOT NULL
	GROUP BY GuildName;

	INSERT dbo.PVPRankingCount(Total, TotalCount)
	SELECT 1, COUNT(*) 
	FROM dbo.PVPRanking WITH(NOLOCK);

	/**_# ?????? DROP ???.*/
	DROP TABLE #tblPVPRanking;
	
	/**_# ??? ???? ????? UPSERT ???.*/
	UPDATE dbo.RankingTotalCompleteTime
	SET CompleteDate = GETDATE()
	WHERE RankingCode = 1
	
	IF @@ROWCOUNT = 0
		INSERT dbo.RankingTotalCompleteTime(RankingCode, CompleteDate)
		VALUES (1, GETDATE());
	 
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

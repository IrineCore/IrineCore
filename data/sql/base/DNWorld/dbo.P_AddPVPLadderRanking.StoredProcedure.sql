USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddPVPLadderRanking]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 11
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-08-06
description : PVP?? ??? ?? & ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_AddPVPLadderRanking]	  
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
	/**_# ?? ?? ???? ?????.*/
	IF OBJECT_ID('tempdb..#tblPVPLadderRanking') IS NOT NULL
		DROP TABLE #tblPVPLadderRanking;

	CREATE TABLE #tblPVPLadderRanking (
		CharacterID	bigint	NOT NULL,		
		CharacterName	nvarchar(30)	NOT NULL,
		CharacterClassCode	tinyint	NOT NULL,
		CharacterSubClassCode	tinyint	NOT NULL,
		JobCode	tinyint	NOT NULL,
		GuildID	int	NULL,
		GuildName	nvarchar(50)	NULL,
		PVPLadderGradePoint	int	NOT NULL,
		Win	int	NOT NULL,
		Lose	int	NOT NULL,
		Draw	int	NOT NULL,
		WinRatio	int	NOT NULL,
		PVPClassRank	int	NOT NULL,
		PVPClassRatio	float	NOT NULL,
		TotalRank	bigint	NOT NULL,
		CharacterClassRank	bigint	NOT NULL,
		CharacterSubClassRank	bigint	NOT NULL,
		ChangedTotalRank	bigint	NULL,
		ChangedCharacterClassRank	bigint	NULL,
		ChangedCharacterSubClassRank	bigint	NULL,	
		PVPLadderCode	tinyint	NULL
	);

	/**_# ??? ??? ?????.*/
	WITH LR AS (
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
		   ISNULL(PR.Ranking, 0) AS PVPClassRank, ISNULL(PR.RankRatio, 0) AS PVPClassRankRatio,
		   G.GuildID, G.GuildName, CH.CharacterName, CS.JobCode, PS.PVPLadderCode, PS.PVPLadderGradePoint, PS.Win, PS.Lose, PS.Draw, PS.WinRatio
	FROM dbo.VIX_PVPLadderRanking PS WITH (NOLOCK) 
		 INNER JOIN dbo.Characters CH WITH (NOLOCK, INDEX(PK_Characters), FORCESEEK) ON PS.CharacterID = CH.CharacterID
		 INNER JOIN dbo.CharacterStatus CS WITH (NOLOCK, INDEX(PK_CharacterStatus), FORCESEEK) ON CH.CharacterID = CS.CharacterID	 
		 LEFT OUTER JOIN dbo.GuildMembers GM WITH (NOLOCK, INDEX(IX_UN_GuildMembers_CharacterID), FORCESEEK) ON CH.CharacterID = GM.CharacterID
		 LEFT OUTER JOIN dbo.Guilds G WITH (NOLOCK, INDEX(PK_Guilds), FORCESEEK) ON GM.GuildID = G.GuildID	 
		 LEFT OUTER JOIN dbo.PVPTopRanker PR WITH (NOLOCK, INDEX(PK_PVPTopRanker), FORCESEEK) ON CH.CharacterID = PR.CharacterID
	WHERE CH.DeleteFlag = 0 AND CH.AccountLevelCode < 99
	)
	INSERT #tblPVPLadderRanking (CharacterID, CharacterName, CharacterClassCode, CharacterSubClassCode, JobCode, GuildID, GuildName, PVPLadderGradePoint, Win, Lose, Draw, WinRatio, PVPClassRank, PVPClassRatio, TotalRank, CharacterClassRank, CharacterSubClassRank, ChangedTotalRank, ChangedCharacterClassRank, ChangedCharacterSubClassRank, PVPLadderCode)
	SELECT CharacterID, CharacterName, CharacterClassCode, CharacterSubClassCode, JobCode, GuildID, GuildName, PVPLadderGradePoint, Win, Lose, Draw, WinRatio, PVPClassRank, PVPClassRankRatio, 
		   RANK() OVER(PARTITION BY PVPLadderCode ORDER BY PVPLadderGradePoint DESC, Win DESC, WinRatio DESC, CharacterLevel ASC, CharacterID DESC) AS TotalRank, 
		   RANK() OVER(PARTITION BY PVPLadderCode, CharacterClassCode ORDER BY PVPLadderGradePoint DESC, Win DESC, WinRatio DESC, CharacterLevel ASC, CharacterID DESC) AS CharacterClassRanking,
		   RANK() OVER(PARTITION BY PVPLadderCode, CharacterSubClassCode ORDER BY PVPLadderGradePoint DESC, Win DESC, WinRatio DESC, CharacterLevel ASC, CharacterID DESC) AS CharacterSubClassRank,
		   0,0,0, PVPLadderCode	   
	FROM LR;	

	UPDATE #tblPVPLadderRanking
	SET ChangedTotalRank = CASE WHEN PR.TotalRank IS NULL THEN NULL ELSE ISNULL(PR.TotalRank,0) - ISNULL(NR.TotalRank,0) END,
		ChangedCharacterClassRank = CASE WHEN PR.CharacterClassRank IS NULL THEN NULL ELSE ISNULL(PR.CharacterClassRank,0) - ISNULL(NR.CharacterClassRank,0) END,
		ChangedCharacterSubClassRank = CASE WHEN PR.CharacterSubClassRank IS NULL THEN NULL ELSE ISNULL(PR.CharacterSubClassRank,0) - ISNULL(NR.CharacterSubClassRank,0) END
	FROM #tblPVPLadderRanking NR LEFT OUTER JOIN dbo.PVPLadderRanking PR WITH (NOLOCK) 
		 ON NR.CharacterID = PR.CharacterID

	/**_# ?? ?? ???? TRUNCATE ?? ??? ?????? INSERT ???.*/
	TRUNCATE TABLE dbo.PVPLadderRanking;

	INSERT dbo.PVPLadderRanking (CharacterID, CharacterName, CharacterClassCode, CharacterSubClassCode, JobCode, GuildID, GuildName, PVPLadderGradePoint, Win, Lose, Draw, WinRatio, PVPClassRank, PVPClassRatio, TotalRank, CharacterClassRank, CharacterSubClassRank, ChangedTotalRank, ChangedCharacterClassRank, ChangedCharacterSubClassRank, PVPLadderCode)
	SELECT CharacterID, CharacterName, CharacterClassCode, CharacterSubClassCode, JobCode, GuildID, GuildName, PVPLadderGradePoint, Win, Lose, Draw, WinRatio, PVPClassRank, PVPClassRatio, TotalRank, CharacterClassRank, CharacterSubClassRank, ChangedTotalRank, ChangedCharacterClassRank, ChangedCharacterSubClassRank, PVPLadderCode
	FROM #tblPVPLadderRanking;
	
	/**_# ?? ?? ?? TotalCount? ?????.*/
	TRUNCATE TABLE dbo.PVPLadderRankingCount;
	
	INSERT dbo.PVPLadderRankingCount(PVPLadderCode, CharacterClassCode, TotalCount)
	SELECT PVPLadderCode, CharacterClassCode, COUNT(*) 
	FROM dbo.PVPLadderRanking WITH(NOLOCK)
	GROUP BY PVPLadderCode, CharacterClassCode;

	INSERT dbo.PVPLadderRankingCount(PVPLadderCode, CharacterSubClassCode, TotalCount)
	SELECT PVPLadderCode, CharacterSubClassCode, COUNT(*) 
	FROM dbo.PVPLadderRanking WITH(NOLOCK)
	GROUP BY PVPLadderCode, CharacterSubClassCode;

	INSERT dbo.PVPLadderRankingCount(PVPLadderCode, GuildName, TotalCount)
	SELECT PVPLadderCode, GuildName, COUNT(*) 
	FROM dbo.PVPLadderRanking WITH(NOLOCK)
	WHERE GuildName IS NOT NULL
	GROUP BY PVPLadderCode, GuildName;

	INSERT dbo.PVPLadderRankingCount(PVPLadderCode, Total, TotalCount)
	SELECT PVPLadderCode, 1, COUNT(*) 
	FROM dbo.PVPLadderRanking WITH(NOLOCK)
	GROUP BY PVPLadderCode;
	
	/**_# ?????? DROP ???.*/
	DROP TABLE #tblPVPLadderRanking;
	
	/**_# ??? ???? ????? UPDATE ???.*/
	UPDATE dbo.RankingTotalCompleteTime
	SET CompleteDate = GETDATE()
	WHERE RankingCode = 2
	
	IF @@ROWCOUNT = 0
		INSERT dbo.RankingTotalCompleteTime(RankingCode, CompleteDate)
		VALUES (2, GETDATE());
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

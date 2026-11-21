USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListPVPLadderScores]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 6
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2011-05-17

description :
	PVP ?? ?? ??
result set : 
	@inyDivision = 1
		Ranking						int
		CharacterName				nvarchar(30)
		CharacterLevel				tinyint
		JobName						nvarchar(50)
		PVPLadderCode				tinyint
		PVPLadderGradePoint			int
		HiddenPVPLadderGradePoint	int
		Win							int
		Lose						int
		Draw						int
		TodayWin					smallint
		TodayLose					smallint
		TodayDraw					smallint
		ConsecutiveWin				smallint
		ConsecutiveLose				smallint
		LastPVPLadderScoreDate		datetime2(0)
		WeeklyPVPCount				smallint
		CharacterID					bigint
		AccountID					int
	@inyDivision = 2
		CharacterName				nvarchar(30)
		CharacterLevel				tinyint
		JobName						nvarchar(50)
		OpponentJobName				nvarchar(50)
		Win							int
		Lose						int
		Draw						int
		CharacterID					bigint
		AccountID					int
	@inyDivision = 3
		CharacterName				nvarchar(30)
		CharacterLevel				tinyint
		JobName						nvarchar(50)
		OpponentJobName				nvarchar(50)
		KOWin						int
		KOLose						int
		CharacterID					bigint
		AccountID					int
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
	3 = ?? ??
**/

CREATE PROCEDURE [dbo].[P_GetListPVPLadderScores]
	  @inyDivision tinyint --// 1=PVP ?? ??, 2=PVP ?? ?? - 1:1 ??? ?? ??, 3=PVP ???? ?? ??? ?/?? ??
	, @intWorldID int --// ??ID
	, @datBasisDate date --// ?? ??
	, @inbCharacterID bigint = NULL --// ??? ID (@inyDivision=2 ??? @inyDivision=3 ????? ??)
	, @inyPVPLadderCode tinyint = NULL --// 1=1:1, 2=2:2, 3=3:3, 4=4:4 (@inyDivision=2 ??? @inyDivision=3 ????? ????)
	, @intRankingCount int = NULL --// ?? ?? ??? (@inyDivision=1 ? ???? ??)
	, @nvcCharacterName nvarchar(30) = NULL --// ?? ??? ?? (@inyDivision=1 ? ???? ????)
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

	IF @inyDivision = 1
	BEGIN
		WITH CTE AS (
			SELECT RANK() OVER(PARTITION BY LS.PVPLadderCode ORDER BY LS.PVPLadderGradePoint DESC) AS Ranking
				 , DC.CharacterName
				 , DC.CharacterLevel
				 , DJ.JobName
				 , LS.PVPLadderCode
				 , LS.PVPLadderGradePoint
				 , LS.HiddenPVPLadderGradePoint
				 , LS.Win
				 , LS.Lose
				 , LS.Draw
				 , LS.TodayWin
				 , LS.TodayLose
				 , LS.TodayDraw
				 , LS.ConsecutiveWin
				 , LS.ConsecutiveLose
				 , LS.LastPVPLadderScoreDate
				 , LS.WeeklyPVPCount
				 , LS.CharacterID
				 , DC.AccountID				 
			  FROM dbo.DailyPVPLadderScores LS WITH (NOLOCK)
				   INNER JOIN dbo.DimCharacters DC WITH (NOLOCK) ON (LS.CharacterID = DC.CharacterID)
				   INNER JOIN dbo.DimJobs DJ WITH (NOLOCK) ON (DC.JobCode = DJ.JobCode)
			 WHERE (LS.WorldID = @intWorldID) AND DC.DeleteFlag = 0
			   AND (LS.BasisDate = @datBasisDate)		   
			   AND LS.PVPLadderCode = COALESCE(@inyPVPLadderCode, LS.PVPLadderCode)
		 )
		 SELECT Ranking, CharacterName, CharacterLevel, JobName, PVPLadderCode, PVPLadderGradePoint, HiddenPVPLadderGradePoint, Win, Lose, Draw, TodayWin, TodayLose, TodayDraw, ConsecutiveWin, ConsecutiveLose, LastPVPLadderScoreDate, WeeklyPVPCount, CharacterID, AccountID 
		 FROM CTE	
		 WHERE CharacterID = COALESCE(@inbCharacterID, CharacterID)
		   AND CharacterName = COALESCE(@nvcCharacterName, CharacterName)
		   AND Ranking <= CASE WHEN @intRankingCount IS NOT NULL THEN @intRankingCount ELSE Ranking END;

	END
	ELSE IF @inyDivision = 2 AND @inbCharacterID IS NOT NULL
	BEGIN

		SELECT DC.CharacterName
			 , DC.CharacterLevel
			 , DJ.JobName
			 , DJ2.JobName AS OpponentJobName
		     , LS.Win
		     , LS.Lose
		     , LS.Draw
			 , LS.CharacterID
			 , DC.AccountID
		  FROM dbo.DailyPVPLadderScoresByJob LS WITH (NOLOCK)
			   INNER JOIN dbo.DimCharacters DC WITH (NOLOCK) ON (LS.CharacterID = DC.CharacterID)
			   INNER JOIN dbo.DimJobs DJ WITH (NOLOCK) ON (DC.JobCode = DJ.JobCode)
			   INNER JOIN dbo.DimJobs DJ2 WITH (NOLOCK) ON (LS.JobCode = DJ.JobCode)
		 WHERE (LS.WorldID = @intWorldID)
		   AND (LS.BasisDate = @datBasisDate)
		   AND (LS.CharacterID = @inbCharacterID);

	END
	ELSE IF @inyDivision = 3 AND @inbCharacterID IS NOT NULL
	BEGIN
		SELECT DC.CharacterName
			 , DC.CharacterLevel
			 , DJ.JobName
			 , DJ2.JobName AS OpponentJobName
		     , LS.KOWin
		     , LS.KOLose
			 , LS.CharacterID
			 , DC.AccountID
		  FROM dbo.DailyPVPLadderKillDeathByJob LS WITH (NOLOCK)
			   INNER JOIN dbo.DimCharacters DC WITH (NOLOCK) ON (LS.CharacterID = DC.CharacterID)
			   INNER JOIN dbo.DimJobs DJ WITH (NOLOCK) ON (DC.JobCode = DJ.JobCode)
			   INNER JOIN dbo.DimJobs DJ2 WITH (NOLOCK) ON (LS.JobCode = DJ.JobCode)
		 WHERE (LS.WorldID = @intWorldID)
		   AND (LS.BasisDate = @datBasisDate)
		   AND (LS.CharacterID = @inbCharacterID);

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

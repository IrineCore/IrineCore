USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddDWCRanking]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 22
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-01-11
description : ?? DWC ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_AddDWCRanking]
	@dt2BasisDate datetime2(0) = NULL
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @dt2Now datetime2(0) = CASE WHEN @dt2BasisDate IS NULL THEN GETDATE() ELSE @dt2BasisDate END
DECLARE @dt2DWCStartDate datetime2(0)
DECLARE @dt2DWCEndDate datetime2(0)
DECLARE @dt2DWCStopDate datetime2(0) = NULL
DECLARE @dt2DWCReopenDate datetime2(0) = NULL
DECLARE @intDWCSequence int
DECLARE @tblBadDWCTeamIDs TABLE (DWCTeamID int, DWCPoint int)
CREATE TABLE #PastDWCRanking
(
	DWCTeamID int NOT NULL,
	Ranking int NOT NULL
)

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	/**_# ?? ??? DWC? ??? ??? ???.*/
	SELECT @dt2DWCStartDate = DWCStartTime
         , @dt2DWCEndDate = DWCEndTime
         , @intDWCSequence = DWCSequence
		 , @dt2DWCStopDate = DWCStopTime
		 , @dt2DWCReopenDate = DWCReopenTime
	FROM dbo.DWCChannels WITH(NOLOCK)
	WHERE EnableFlag = 1

	IF @@ROWCOUNT > 0	
	BEGIN
		/**_# ?? ??? ??? ???????? ??? ? ? ?? ???? */
		SET @dt2DWCStartDate = CASE WHEN @dt2DWCStopDate IS NOT NULL AND @dt2DWCReopenDate IS NOT NULL THEN DATEADD(DAY, DATEDIFF(DAY, @dt2DWCStopDate, @dt2DWCReopenDate), @dt2DWCStartDate) ELSE @dt2DWCStartDate END

		/**_# ?? ??? 7???? ?? ??? ?? ??? ? ??? ?? ?? ? ??? ????. */
		IF DATEDIFF(DAY, @dt2DWCStartDate, @dt2Now) % 7 = 0
		BEGIN
			/**_# 1800? ?? ???? ????? ??? ??? 10? ??? ?? ???. */
			INSERT dbo.DWCTeamPointPlayHistory (DWCTeamID, DWCPointChangeCode, DWCPointBefore, DWCPoint, RegisterDate)
			SELECT DWCTeamID, 4, DWCPoint, DWCPoint-100, @dt2Now
			FROM dbo.DWCTeams WITH(NOLOCK)
			WHERE DWCPoint >= 1800
			  AND WeeklyPlayCount < 10
			  AND CloseFlag = 0;

			/**_# 1800? ?? ???? ????? ??? ??? 10? ??? ???? ????. */
			UPDATE dbo.DWCTeams
			SET DWCPoint = CASE WHEN DWCPoint >= 1800 AND WeeklyPlayCount < 10 THEN DWCPoint - 100 ELSE DWCPoint END
			  , WeeklyPlayCount = 0
			WHERE CloseFlag = 0;
		END

		INSERT #PastDWCRanking(DWCTeamID, Ranking)
		SELECT DWCTeamID, Ranking
		FROM dbo.DWCRanking WITH(NOLOCK)
			
		ALTER TABLE #PastDWCRanking ADD CONSTRAINT PK_#PastDWCRanking PRIMARY KEY (DWCTeamID)

		TRUNCATE TABLE dbo.DWCRanking;
		TRUNCATE TABLE dbo.DWCRankingTotalCount;

		/**_# [DWCRanking] ???? INSERT???.*/
		WITH C1 AS
		(
			SELECT ROW_NUMBER() OVER(ORDER BY DT.DWCPoint DESC, CAST(DT.Win as float)/CASE WHEN (DT.Win+DT.Lose) = 0 THEN 1 ELSE (DT.Win+DT.Lose) END DESC) AS Ranking, DT.DWCTeamID, DWCTeamName, DT.Win, DT.Lose, DT.Draw, DT.DWCPoint, ISNULL(PDR.Ranking, 0) AS ChangedRanking
			FROM dbo.DWCTeams DT WITH(NOLOCK)
				LEFT OUTER JOIN #PastDWCRanking PDR WITH(NOLOCK, INDEX(PK_#PastDWCRanking), FORCESEEK) ON DT.DWCTeamID = PDR.DWCTeamID
			WHERE DWCPoint > 0
              AND CloseFlag = 0 
		)
		INSERT dbo.DWCRanking (Ranking, DWCTeamID, DWCTeamName, Win, Lose, Draw, DWCPoint, ChangedRanking, DWCTeamLeaderCharacterInfo, DWCTeamMemberCharacterInfo)
		SELECT Ranking, DWCTeamID, DWCTeamName, Win, Lose, Draw, DWCPoint, ChangedRanking
			, STUFF((SELECT
					 N'|' + B.CharacterName
				   , N',' + CAST(C.JobCode AS nvarchar(20))
				FROM dbo.DWCTeamMembers A WITH(NOLOCK)
					INNER JOIN dbo.Characters B WITH(NOLOCK) ON A.CharacterID = B.CharacterID
					INNER JOIN dbo.CharacterStatus C ON B.CharacterID = C.CharacterID
				WHERE A.DWCTeamID = C1.DWCTeamID AND A.TeamLeaderFlag = 1
				FOR XML PATH(N'')), 1, 1, N'') AS DWCTeamLeaderCharacterInfo
			, STUFF((SELECT
					 N'|' + B.CharacterName
				   , N',' + CAST(C.JobCode AS nvarchar(20))
				FROM dbo.DWCTeamMembers A WITH(NOLOCK)
					INNER JOIN dbo.Characters B WITH(NOLOCK) ON A.CharacterID = B.CharacterID
					INNER JOIN dbo.CharacterStatus C WITH(NOLOCK) ON B.CharacterID = C.CharacterID
				WHERE A.DWCTeamID = C1.DWCTeamID AND A.TeamLeaderFlag = 0
				FOR XML PATH(N'')), 1, 1, N'') AS DWCTeamMemberCharacterInfo
		FROM C1

		INSERT dbo.DWCRankingTotalCount
		SELECT @@ROWCOUNT;
		/**_# ?? ???? ?? DWC? ?????? ?? ??? ?? ???? ????.*/
		IF @dt2DWCEndDate < @dt2Now
		BEGIN
			IF NOT EXISTS(SELECT TOP 1 * FROM dbo.PastDWCRanking WITH(NOLOCK) WHERE DWCSequence = @intDWCSequence)
				INSERT PastDWCRanking(DWCSequence, Ranking, DWCTeamID, DWCTeamName, Win, Lose, Draw, DWCPoint, ChangedRanking, DWCTeamLeaderCharacterInfo, DWCTeamMemberCharacterInfo)
				SELECT @intDWCSequence, Ranking, DWCTeamID, DWCTeamName, Win, Lose, Draw, DWCPoint, ChangedRanking, DWCTeamLeaderCharacterInfo, DWCTeamMemberCharacterInfo
				FROM dbo.DWCRanking WITH(NOLOCK)

			IF NOT EXISTS(SELECT TOP 1 * FROM dbo.PastDWCTeams WITH(NOLOCK) WHERE DWCSequence = @intDWCSequence)
				INSERT dbo.PastDWCTeams(DWCSequence, DWCTeamID, DWCTeamName, Win, Lose, Draw, DWCPoint, CreateDate)
				SELECT @intDWCSequence, DWCTeamID, DWCTeamName, Win, Lose, Draw, DWCPoint, CreateDate 
				FROM dbo.DWCTeams WITH(NOLOCK)
				WHERE CloseFlag = 0

			IF NOT EXISTS(SELECT TOP 1 * FROM dbo.PastDWCTeamMembers WITH(NOLOCK) WHERE DWCSequence = @intDWCSequence)
				INSERT dbo.PastDWCTeamMembers(DWCSequence, DWCTeamID, CharacterID, JobCode, TeamLeaderFlag, RegisterDate)
				SELECT @intDWCSequence, A.DWCTeamID, B.CharacterID, B.JobCode, B.TeamLeaderFlag, B.RegisterDate
				FROM dbo.DWCTeams A WITH(NOLOCK)
					INNER JOIN dbo.DWCTeamMembers B WITH(NOLOCK) ON A.DWCTeamID = B.DWCTeamID
				WHERE A.CloseFlag = 0

			IF NOT EXISTS(SELECT TOP 1 * FROM dbo.PastDWCTeamPointPlayHistory WITH(NOLOCK) WHERE DWCSequence = @intDWCSequence)
				INSERT dbo.PastDWCTeamPointPlayHistory(DWCSequence, DWCTeamID, vsDWCTeamID, DWCPointChangeCode, DWCPointBefore, DWCPoint, RegisterDate)
				SELECT @intDWCSequence, A.DWCTeamID, B.vsDWCTeamID, B.DWCPointChangeCode, B.DWCPointBefore, B.DWCPoint, B.RegisterDate
				FROM dbo.DWCTeams A WITH(NOLOCK)
					INNER JOIN dbo.DWCTeamPointPlayHistory B WITH(NOLOCK) ON A.DWCTeamID = B.DWCTeamID
				WHERE A.CloseFlag = 0
			
			UPDATE dbo.DWCChannels
			SET EnableFlag = 0
			WHERE EnableFlag = 1
		END
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

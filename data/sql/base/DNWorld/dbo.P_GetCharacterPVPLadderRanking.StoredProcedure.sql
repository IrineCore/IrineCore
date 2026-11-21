USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetCharacterPVPLadderRanking]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-08-06
description : ???? PVP ?? ??? ?????.
result set : CharacterName/nvarchar, JobCode/tinyint, GuildName/nvarchar, PVPLadderGradePoint/int, Win/int, Lose/int, Draw/int, PVPExp/int, PVPClassRank/int, PVPClassRatio/float, TotalRank/bigint, CharacterClassRank/bigint, CharacterSubClassRank/bigint, ChangedTotalRank/int, ChangedCharacterClassRank/int, ChangedCharacterSubClassRank/int, CharacterLevel/int, PVPLevel/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetCharacterPVPLadderRanking]
	  @inbCharacterID bigint = NULL --// ???ID
	, @nvcCharacterName nvarchar(30) = NULL --// ??? ??	
	, @inyPVPLadderCode tinyint = 1 --// PVP ???? ?? (1=1:1,3=3:3)
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
	SET @inyPVPLadderCode = ISNULL(@inyPVPLadderCode,1);

	/**_# [PVPLadderRanking] ???? SELECT???.*/
	IF @inbCharacterID IS NOT NULL
		SELECT PR.CharacterName, PR.JobCode, PR.GuildName, PR.PVPLadderGradePoint, PR.Win, PR.Lose, PR.Draw, PS.PVPExp, PR.PVPClassRank, PR.PVPClassRatio, PR.TotalRank, PR.CharacterClassRank, PR.CharacterSubClassRank, CASE WHEN PR.ChangedTotalRank IS NULL THEN 2 
				 WHEN PR.ChangedTotalRank > 0 THEN 1 
				 WHEN PR.ChangedTotalRank < 0 THEN -1 
				 ELSE 0 END AS ChangedTotalRank,
			CASE WHEN PR.ChangedCharacterClassRank IS NULL THEN 2 
				 WHEN PR.ChangedCharacterClassRank > 0 THEN 1 
				 WHEN PR.ChangedCharacterClassRank < 0 THEN -1 
				 ELSE 0 END AS ChangedCharacterClassRank,
 			CASE WHEN PR.ChangedCharacterSubClassRank IS NULL THEN 2 
				 WHEN PR.ChangedCharacterSubClassRank > 0 THEN 1 
				 WHEN PR.ChangedCharacterSubClassRank < 0 THEN -1 
				 ELSE 0 END AS ChangedCharacterSubClassRank,
			CH.CharacterLevel, PS.PVPLevel
		FROM dbo.PVPLadderRanking PR WITH (NOLOCK) INNER JOIN dbo.CharacterStatus CH WITH (NOLOCK) ON PR.CharacterID = CH.CharacterID
	         INNER JOIN dbo.PVPScores PS WITH (NOLOCK) ON PR.CharacterID = PS.CharacterID 
		WHERE PR.CharacterID = @inbCharacterID AND PVPLadderCode = @inyPVPLadderCode;
	ELSE IF @nvcCharacterName IS NOT NULL
		SELECT PR.CharacterName, PR.JobCode, PR.GuildName, PR.PVPLadderGradePoint, PR.Win, PR.Lose, PR.Draw, PS.PVPExp, PR.PVPClassRank, PR.PVPClassRatio, PR.TotalRank, PR.CharacterClassRank, PR.CharacterSubClassRank, CASE WHEN PR.ChangedTotalRank IS NULL THEN 2 
				 WHEN PR.ChangedTotalRank > 0 THEN 1 
				 WHEN PR.ChangedTotalRank < 0 THEN -1 
				 ELSE 0 END AS ChangedTotalRank,
			CASE WHEN PR.ChangedCharacterClassRank IS NULL THEN 2 
				 WHEN PR.ChangedCharacterClassRank > 0 THEN 1 
				 WHEN PR.ChangedCharacterClassRank < 0 THEN -1 
				 ELSE 0 END AS ChangedCharacterClassRank,
 			CASE WHEN PR.ChangedCharacterSubClassRank IS NULL THEN 2 
				 WHEN PR.ChangedCharacterSubClassRank > 0 THEN 1 
				 WHEN PR.ChangedCharacterSubClassRank < 0 THEN -1 
				 ELSE 0 END AS ChangedCharacterSubClassRank,
			CH.CharacterLevel, PS.PVPLevel
		FROM dbo.PVPLadderRanking PR WITH (NOLOCK) INNER JOIN dbo.CharacterStatus CH WITH (NOLOCK) ON PR.CharacterID = CH.CharacterID
			 INNER JOIN dbo.PVPScores PS WITH (NOLOCK) ON PR.CharacterID = PS.CharacterID 
		WHERE PR.CharacterName = @nvcCharacterName AND PVPLadderCode = @inyPVPLadderCode;
	ELSE
		SELECT PR.CharacterName, PR.JobCode, PR.GuildName, PR.PVPLadderGradePoint, PR.Win, PR.Lose, PR.Draw, PS.PVPExp, PR.PVPClassRank, PR.PVPClassRatio, PR.TotalRank, PR.CharacterClassRank, PR.CharacterSubClassRank, CASE WHEN PR.ChangedTotalRank IS NULL THEN 2 
				 WHEN PR.ChangedTotalRank > 0 THEN 1 
				 WHEN PR.ChangedTotalRank < 0 THEN -1 
				 ELSE 0 END AS ChangedTotalRank,
			CASE WHEN PR.ChangedCharacterClassRank IS NULL THEN 2 
				 WHEN PR.ChangedCharacterClassRank > 0 THEN 1 
				 WHEN PR.ChangedCharacterClassRank < 0 THEN -1 
				 ELSE 0 END AS ChangedCharacterClassRank,
 			CASE WHEN PR.ChangedCharacterSubClassRank IS NULL THEN 2 
				 WHEN PR.ChangedCharacterSubClassRank > 0 THEN 1 
				 WHEN PR.ChangedCharacterSubClassRank < 0 THEN -1 
				 ELSE 0 END AS ChangedCharacterSubClassRank,
			CH.CharacterLevel, PS.PVPLevel
		FROM dbo.PVPLadderRanking PR WITH (NOLOCK) INNER JOIN dbo.CharacterStatus CH WITH (NOLOCK) ON PR.CharacterID = CH.CharacterID
			 INNER JOIN dbo.PVPScores PS WITH (NOLOCK) ON PR.CharacterID = PS.CharacterID 
		WHERE PR.CharacterID = @inbCharacterID AND PR.CharacterName = @nvcCharacterName AND PVPLadderCode = @inyPVPLadderCode;
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

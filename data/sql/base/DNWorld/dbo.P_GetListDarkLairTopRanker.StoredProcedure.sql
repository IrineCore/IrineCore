USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListDarkLairTopRanker]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 6
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-08-22
description : ???? ? ?? ??? ?????.
result set : Ranking/int, PartyName/nvarchar, PlayRound/smallint, PlayTime/int, CharacterNames/nvarchar, CharacterLevels/nvarchar, JobCodes/nvarchar
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history
2 = BlockFlag = 1 ? ???? ?? ??? ?? 2010-10-07
3 = BlockFlag = 0 ? ???? ?? ??? ?? 2010-10-19
4 = Ranking?? ????? ?? at 2011-01-19
**/

CREATE PROCEDURE [dbo].[P_GetListDarkLairTopRanker]
	  @intMapID int --// ? ID
	, @inyPartyUserCount tinyint --// ?? ?? (1 ~ 4)
	, @inyRankerCount tinyint --// ??? ? ?? ? (1 ~ 255)
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
	DECLARE @tblTopRank table (
		Ranking int NOT NULL,
		DarkLairRankingID bigint NOT NULL,
		PlayRound smallint NOT NULL,
		PlayTime int NOT NULL
	);

	INSERT @tblTopRank (Ranking, DarkLairRankingID, PlayRound, PlayTime)
	SELECT TOP (@inyRankerCount) RANK() OVER (ORDER BY PlayRound DESC, PlayTime, DarkLairRankingID) AS Ranking, DarkLairRankingID, PlayRound, PlayTime
	FROM dbo.DarkLairRanking WITH (NOLOCK, INDEX(IX_NNIF_DarkLairRanking_MapID_PartyUserCount_PlayRound_PlayTime), FORCESEEK)
	WHERE MapID = @intMapID AND PartyUserCount = @inyPartyUserCount AND BestFlag = CAST(1 AS bit) AND BlockFlag = CAST(0 AS bit)
	ORDER BY PlayRound DESC, PlayTime, DarkLairRankingID
	OPTION (KEEPFIXED PLAN, MAXDOP 1);

	SELECT C1.Ranking, DLR.PartyName, C1.PlayRound, C1.PlayTime
		, STUFF((
			SELECT N',' + CA.CharacterName
			FROM dbo.DarkLairPartyMembers DLM WITH (NOLOCK)
				INNER JOIN dbo.Characters CA WITH (NOLOCK) ON DLM.CharacterID = CA.CharacterID
			WHERE DLM.DarkLairRankingID = C1.DarkLairRankingID AND DLM.CompleteFlag = CAST(1 AS bit)
			ORDER BY DLM.CharacterID
			FOR XML PATH(N'')), 1, 1, N'') AS CharacterNames
		, STUFF((
			SELECT N',' + CAST(CharacterLevel AS nvarchar(20))
			FROM dbo.DarkLairPartyMembers WITH (NOLOCK)
			WHERE DarkLairRankingID = C1.DarkLairRankingID AND CompleteFlag = CAST(1 AS bit)
			ORDER BY CharacterID
			FOR XML PATH(N'')), 1, 1, N'') AS Levels
		, STUFF((
			SELECT N',' + CAST(JobCode AS nvarchar(20))
			FROM dbo.DarkLairPartyMembers WITH (NOLOCK)
			WHERE DarkLairRankingID = C1.DarkLairRankingID AND CompleteFlag = CAST(1 AS bit)
			ORDER BY CharacterID
			FOR XML PATH(N'')), 1, 1, N'') AS JobCodes
	FROM @tblTopRank C1
		INNER JOIN dbo.DarkLairRanking DLR WITH (NOLOCK, FORCESEEK) ON C1.DarkLairRankingID = DLR.DarkLairRankingID
	ORDER BY C1.Ranking
	OPTION (KEEPFIXED PLAN, MAXDOP 1);

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

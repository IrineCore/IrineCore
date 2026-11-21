USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddDarkLairRanking]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 21
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-02-01
description : ???? ??? ???? ?? ???? ??? ?? Top n?? ??? ?????.
result set : Ranking/int, PartyName/nvarchar, PlayRound/smallint, PlayTime/int, CharacterNames/nvarchar, CharacterLevels/nvarchar, JobCodes/nvarchar
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103179 = @tblCharacterIDs? ???? @inyPartyUserCount? ???? ????.
103180 = @tblCompleteFlags? ???? @inyPartyUserCount? ???? ????.
103181 = @vchCharacterLevel? ???? @inyPartyUserCount? ???? ????.
103182 = @vchJobCodes? ???? @inyPartyUserCount? ???? ????.
history :
2 = TF_Split() ?? ?? IF_Split() ??? ????? ?? & CharacterLevel? JobCode? CharacterStatus ???? ???? ????? ?? at 2010-07-13
3 = CharacterLevel, JobCode? input parameter?? ????? ?? at 2010-07-19
4 = DarkLairPersonalBest ???? INSERT? ??? ?? ?? at 2010-07-22
5 = DarkLairPersonalBest ???? INSERT? ??? ?? ? ?? at 2010-07-29
6 = ?? ?? ?? ??? at 2010-08-22
7 = dead lock ?? ?? at 2010-09-16
9 = Ranking?? ????? ?? at 2011-01-19
15 = TF_Split(), TF_NSplit() ??? ????? ?? by ??? at 2011-11-01
16 = ?? ??? ??? ???? ?? ?? ?? by ??? at 2012-09-10
19 = ?? ?? ?? ?? by ??? at 2013-01-04
**/

CREATE PROCEDURE [dbo].[P_AddDarkLairRanking]
	  @intMapID int --// ? ID
	, @inyPartyUserCount tinyint --// ?? ?? (1 ~ 4)
	, @insPlayRound smallint --// ??? ??? ?
	, @intPlayTime int --// ??? ?? (?)
	, @nvcPartyName nvarchar(100) --// ?? ??
	, @vchCharacterIDs varchar(100) --// ,? ?????? ?? ???? ??? ID ???
	, @vchCompleteFlags varchar(100) --// ,? ?????? ?? ???? ???? ??? ?? ??? ex) 1,1,0,1
	, @vchCharacterLevel varchar(100) --// ,? ?????? ?? ???? ??? ??
	, @vchJobCodes varchar(100) --// ,? ?????? ?? ???? ??? ??
	, @intTopCount int --// ?? @intTopCount?? ??? ????? ?????.
	, @intRanking int OUTPUT --// ??. 10000? ??? ??, ?? 10000? ?????.
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @inbDarkLairRankingID bigint;

DECLARE @tblResults table (
	CharacterID bigint NOT NULL PRIMARY KEY,
	CompleteFlag bit NOT NULL,
	CharacterLevel tinyint NOT NULL,
	JobCode tinyint NOT NULL
);

DECLARE @tblResult2 table (
	CharacterID bigint NOT NULL,
	OldDarkLairRankingID bigint NOT NULL,
	DarkLairRankingID bigint NOT NULL
);

DECLARE @tblTopRank table (
	Ranking int NOT NULL,
	DarkLairRankingID bigint NOT NULL,
	PlayRound smallint NOT NULL,
	PlayTime int NOT NULL
);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	INSERT @tblResults (CharacterID, CompleteFlag, CharacterLevel, JobCode)
	SELECT A.string, B.string, C.string, D.string
	FROM dbo.TF_Split(@vchCharacterIDs, ',') A
		INNER JOIN dbo.TF_Split(@vchCompleteFlags, ',') B ON A.seq = B.seq
		INNER JOIN dbo.TF_Split(@vchCharacterLevel, ',') C ON A.seq = C.seq
		INNER JOIN dbo.TF_Split(@vchJobCodes, ',') D ON A.seq = D.seq
	WHERE A.string > '' AND B.string > '' AND C.string > '' AND D.string > ''
	OPTION (FORCE ORDER, LOOP JOIN, KEEPFIXED PLAN);

	IF @@ROWCOUNT <> @inyPartyUserCount
	BEGIN
		SET @intReturnValue = 103179;
		GOTO ErrorHandler;
	END

	/**_# ?? ?? ??? ??? INSERT ?? ?? ?????.*/
	IF EXISTS (SELECT * 
				FROM @tblResults A
					INNER JOIN dbo.Characters B WITH(NOLOCK) ON A.CharacterID = B.CharacterID
				WHERE B.AccountLevelCode > 0 )
	BEGIN
		SET @intReturnValue = 0;

		SELECT 0 AS Ranking, 0 AS PartyName, 0 AS PlayRound, 0 AS PlayTime, 0 AS CharacterNames, 0 AS Levels, 0 AS JobCodes
		WHERE 0=1

		SET @intRanking = 10000;
		RETURN;
	END

	/**_# [DarkLairRanking] ???? INSERT???.*/
	INSERT dbo.DarkLairRanking (MapID, PartyUserCount, PlayRound, PlayTime, PartyName, RegisterDate, BestFlag)
	VALUES (@intMapID, @inyPartyUserCount, @insPlayRound, @intPlayTime, @nvcPartyName, GETDATE(), 1)
	OPTION (KEEPFIXED PLAN);

	SET @inbDarkLairRankingID = SCOPE_IDENTITY();
	/**_# [DarkLairPartyMembers] ???? INSERT???.*/
	INSERT dbo.DarkLairPartyMembers (DarkLairRankingID, CharacterID, CompleteFlag, CharacterLevel, JobCode)
	SELECT @inbDarkLairRankingID, CharacterID, CompleteFlag, CharacterLevel, JobCode
	FROM @tblResults
	ORDER BY CharacterID
	OPTION (KEEPFIXED PLAN);

	/**_# [DarkLairPersonalBest] ???? UPDATE / INSERT???.*/
	INSERT @tblResult2 (CharacterID, OldDarkLairRankingID, DarkLairRankingID)
	SELECT R.CharacterID, ISNULL(DLR.DarkLairRankingID, 0), 
		CASE
			WHEN DLR.DarkLairRankingID IS NULL THEN @inbDarkLairRankingID -- ?? ???
			ELSE
				CASE
					WHEN DLR.PlayRound > @insPlayRound OR (DLR.PlayRound = @insPlayRound AND DLR.PlayTime < @intPlayTime) THEN DLR.DarkLairRankingID -- ??? ?? ?? ??
					ELSE @inbDarkLairRankingID -- ??? ?? ??
				END
		END
	FROM @tblResults R
		LEFT OUTER JOIN dbo.DarkLairPersonalBest DLB WITH (NOLOCK, INDEX(PK_DarkLairPersonalBest), FORCESEEK)
			ON R.CharacterID = DLB.CharacterID AND DLB.MapID = @intMapID AND DLB.PartyUserCount = @inyPartyUserCount
		LEFT OUTER JOIN dbo.DarkLairRanking DLR WITH (NOLOCK, INDEX(PK_DarkLairRanking), FORCESEEK)
			ON DLB.DarkLairRankingID = DLR.DarkLairRankingID
	WHERE R.CompleteFlag = CAST(1 AS bit)
	OPTION (KEEPFIXED PLAN, FORCE ORDER, LOOP JOIN);

	UPDATE dbo.DarkLairPersonalBest
	SET DarkLairRankingID = T.DarkLairRankingID
	FROM @tblResult2 T
		INNER JOIN dbo.DarkLairPersonalBest DLB WITH (INDEX(PK_DarkLairPersonalBest), FORCESEEK) ON T.CharacterID = DLB.CharacterID AND DLB.MapID = @intMapID AND DLB.PartyUserCount = @inyPartyUserCount
	WHERE T.OldDarkLairRankingID > 0
		AND T.OldDarkLairRankingID <> T.DarkLairRankingID
	OPTION (KEEPFIXED PLAN, FORCE ORDER, LOOP JOIN);

	INSERT dbo.DarkLairPersonalBest (CharacterID, MapID, PartyUserCount, DarkLairRankingID)
	SELECT CharacterID, @intMapID, @inyPartyUserCount, DarkLairRankingID
	FROM @tblResult2
	WHERE OldDarkLairRankingID = 0
	OPTION (KEEPFIXED PLAN);

	/**_# [DarkLairRanking] ????? Best ???? ?????.*/
	WITH S (DarkLairRankingID) AS (
		SELECT @inbDarkLairRankingID UNION
		SELECT DarkLairRankingID FROM @tblResult2 UNION
		SELECT OldDarkLairRankingID FROM @tblResult2
	)
	UPDATE dbo.DarkLairRanking
	SET BestFlag = 
		CASE
			WHEN EXISTS (
				SELECT * 
				FROM dbo.DarkLairPersonalBest WITH (NOLOCK, INDEX(IX_NN_DarkLairPersonalBest_DarkLairRankingID), FORCESEEK) 
				WHERE DarkLairRankingID = DR.DarkLairRankingID
			) THEN 1
			ELSE 0
		END
	FROM S
		INNER JOIN dbo.DarkLairRanking DR WITH (INDEX(PK_DarkLairRanking), FORCESEEK) ON S.DarkLairRankingID = DR.DarkLairRankingID
	OPTION (KEEPFIXED PLAN);

	/**_# ??? ????.*/
	SET @intRanking = 1;

	SELECT TOP (9999) @intRanking += 1
	FROM dbo.DarkLairRanking DR WITH (NOLOCK, INDEX(IX_NNIF_DarkLairRanking_MapID_PartyUserCount_PlayRound_PlayTime), FORCESEEK)
	WHERE MapID = @intMapID 
		AND PartyUserCount = @inyPartyUserCount 
		AND BestFlag = CAST(1 AS bit)
		AND (PlayRound > @insPlayRound OR (PlayRound = @insPlayRound AND PlayTime < @intPlayTime))
	ORDER BY PlayRound DESC, PlayTime
	OPTION (KEEPFIXED PLAN, MAXDOP 1);

	/**_# ?? @intTopCount?? ??? ?????.*/
	INSERT @tblTopRank (Ranking, DarkLairRankingID, PlayRound, PlayTime)
	SELECT TOP (@intTopCount) RANK() OVER (ORDER BY PlayRound DESC, PlayTime) AS Ranking, DarkLairRankingID, PlayRound, PlayTime
	FROM dbo.DarkLairRanking WITH (NOLOCK, INDEX(IX_NNIF_DarkLairRanking_MapID_PartyUserCount_PlayRound_PlayTime), FORCESEEK)
	WHERE MapID = @intMapID AND PartyUserCount = @inyPartyUserCount AND BestFlag = CAST(1 AS bit) AND BlockFlag = 0
	ORDER BY PlayRound DESC, PlayTime
	OPTION (OPTIMIZE FOR(@intTopCount = 10), KEEPFIXED PLAN, MAXDOP 1);

	SELECT C1.Ranking, DLR.PartyName, C1.PlayRound, C1.PlayTime
		, STUFF((
			SELECT N',' + CA.CharacterName
			FROM dbo.DarkLairPartyMembers DLM WITH (NOLOCK, FORCESEEK)
				INNER JOIN dbo.Characters CA WITH (NOLOCK, FORCESEEK) ON DLM.CharacterID = CA.CharacterID
			WHERE DLM.DarkLairRankingID = C1.DarkLairRankingID AND DLM.CompleteFlag = CAST(1 AS bit)
			ORDER BY DLM.CharacterID
			FOR XML PATH(N'')), 1, 1, N'') AS CharacterNames
		, STUFF((
			SELECT N',' + CAST(CharacterLevel AS nvarchar(20))
			FROM dbo.DarkLairPartyMembers WITH (NOLOCK, FORCESEEK)
			WHERE DarkLairRankingID = C1.DarkLairRankingID AND CompleteFlag = CAST(1 AS bit)
			ORDER BY CharacterID
			FOR XML PATH(N'')), 1, 1, N'') AS Levels
		, STUFF((
			SELECT N',' + CAST(JobCode AS nvarchar(20))
			FROM dbo.DarkLairPartyMembers WITH (NOLOCK, FORCESEEK)
			WHERE DarkLairRankingID = C1.DarkLairRankingID AND CompleteFlag = CAST(1 AS bit)
			ORDER BY CharacterID
			FOR XML PATH(N'')), 1, 1, N'') AS JobCodes
	FROM @tblTopRank C1
		INNER JOIN dbo.DarkLairRanking DLR WITH (NOLOCK, INDEX(PK_DarkLairRanking), FORCESEEK) ON C1.DarkLairRankingID = DLR.DarkLairRankingID
	ORDER BY C1.Ranking
	OPTION (KEEPFIXED PLAN, FORCE ORDER, LOOP JOIN, MAXDOP 1);
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

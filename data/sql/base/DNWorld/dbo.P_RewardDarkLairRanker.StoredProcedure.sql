USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_RewardDarkLairRanker]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 19
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-09-13
description : ??? ????? ? ????? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
10 = ??? ??? LOOP? ??? ????? ?? at 2012-11-14 by ???
11 = ??? ??? ???? ???? ?????? ???? ??? ?? at 2012-11-14 by ???
**/

CREATE PROCEDURE [dbo].[P_RewardDarkLairRanker]
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	  , @dtsSendDate smalldatetime = GETDATE()
      , @i int = 1
      , @intRanking int
      , @inbCharacterID bigint
      , @intMailScheduleID int;


DECLARE @tblReward table
(
	Ranking int, ItemID int, SenderID varchar(10), SubjectID varchar(10), ContentID varchar(10)
)

DECLARE @tblRewardTarget table
(
	Seq int identity(1, 1), Ranking int, PartyUserCount int, CharacterID bigint
)

DECLARE @tblRewardTarget2 table
(
	Seq int identity(1, 1), Ranking int, PartyUserCount int, CharacterID bigint
)

DECLARE @tblDarkLairRankingID table
(
	Seq int identity(1, 1) PRIMARY KEY, DarkLairRankingID bigint
)

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# ?????? ??? ??? ??? INSERT ???.*/
	INSERT @tblReward(Ranking, ItemID , SenderID , SubjectID , ContentID)
	SELECT 1, 268442067, 1000009487, 1000050071, 1000050081 UNION ALL
	SELECT 1, 536872085, NULL, NULL, NULL UNION ALL
	SELECT 2, 268442068, 1000009487, 1000050072, 1000050081 UNION ALL
	SELECT 2, 536872086, NULL, NULL, NULL UNION ALL
	SELECT 3, 268442069, 1000009487, 1000050073, 1000050081 UNION ALL
	SELECT 3, 536872087, NULL, NULL, NULL UNION ALL
	SELECT 4, 268442070, 1000009487, 1000050074, 1000050081 UNION ALL
	SELECT 5, 268442071, 1000009487, 1000050075, 1000050081 UNION ALL
	SELECT 6, 268442072, 1000009487, 1000050076, 1000050081 UNION ALL
	SELECT 7, 268442073, 1000009487, 1000050077, 1000050081 UNION ALL
	SELECT 8, 268442074, 1000009487, 1000050078, 1000050081 UNION ALL
	SELECT 9, 268442075, 1000009487, 1000050079, 1000050081 UNION ALL
	SELECT 10, 268442076, 1000009487, 1000050080, 1000050081;

	/**_# ???? ??? ? ?? 10??? ??? ?? ?????? Insert???.*/
	WITH CTE AS	
	(
		SELECT RANK() OVER (PARTITION BY PartyUserCount ORDER BY PlayRound DESC, PlayTime, DarkLairRankingID) AS Ranking, PartyUserCount, DarkLairRankingID
		FROM dbo.DarkLairRanking WITH (NOLOCK, INDEX(IX_NNIF_DarkLairRanking_MapID_PartyUserCount_PlayRound_PlayTime), FORCESEEK)
		WHERE MapID = 1000012601 AND BestFlag = CAST(1 AS bit) AND BlockFlag = CAST(0 AS bit)
	)
	INSERT @tblRewardTarget (Ranking, PartyUserCount, CharacterID)
	SELECT A.Ranking, A.PartyUserCount, B.CharacterID
	FROM CTE A 
		INNER JOIN dbo.DarkLairPartyMembers B WITH(NOLOCK) ON A.DarkLairRankingID = B.DarkLairRankingID
	WHERE A.Ranking <= 10
	OPTION (KEEPFIXED PLAN);
	
	/**_# ?? ??? ? ??? ???? ??? ?? ? ??? ??(?? ?? ??? ?? ? ??)*/
	WITH CTE AS(
		SELECT ROW_NUMBER() OVER(PARTITION BY PartyUserCount, CharacterID ORDER BY Ranking) AS DuplicationSeq
		FROM @tblRewardTarget
	)
	DELETE CTE	
	WHERE DuplicationSeq > 1;

	INSERT @tblRewardTarget2 (Ranking, PartyUserCount, CharacterID)
	SELECT Ranking, PartyUserCount, CharacterID
	FROM @tblRewardTarget

	/**_# ?? ???? ?? ???? ?????.(? ???? ????? ???? ??? ??)*/
	WHILE 1=1
	BEGIN
		SELECT @intRanking = Ranking, @inbCharacterID = CharacterID
		FROM @tblRewardTarget2
		WHERE Seq = @i;
		
		IF @@ROWCOUNT = 0
			BREAK;

		INSERT dbo.MailSchedules (SendDate, SenderName, [Subject], Content, AttachCoin, MailScheduleExpireDate, MailTypeCode, NewFlag, DeleteFlag)
		SELECT @dtsSendDate, SenderID, SubjectID, ContentID, 0, DATEADD(DAY, 7, @dtsSendDate), 1, 1, 0
		FROM @tblReward
		WHERE Ranking = @intRanking 
          AND SubjectID IS NOT NULL;

		SET @intMailScheduleID = SCOPE_IDENTITY();
		INSERT dbo.AttachItemsForMailSchedule(MailScheduleID, ItemID, ItemCount, ItemDurability, RandomSeed, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, EternityFlag, ItemLifespan)
		SELECT @intMailScheduleID, ItemID, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0
		FROM @tblReward
		WHERE Ranking = @intRanking;

		INSERT dbo.MailSchedulesTarget(MailScheduleID, CharacterID, DeleteFlag)
		SELECT @intMailScheduleID, @inbCharacterID, 0

		SET @i += 1;
	END

	--?? ??? ?? ?? ???? ????? ??.
	UPDATE dbo.DarkLairRanking
	SET BlockFlag = 1
	FROM dbo.DarkLairRanking A
	WHERE MapID = 1000012601 
      AND A.RegisterDate BETWEEN CAST(@dtsSendDate AS date) AND @dtsSendDate;

	INSERT @tblDarkLairRankingID (DarkLairRankingID)
	SELECT DarkLairRankingID
	FROM dbo.DarkLairRanking WITH(NOLOCK, INDEX(IX_NN_DarkLairRanking_MapID), FORCESEEK)
	WHERE MapID = 1000012601 
	  AND RegisterDate BETWEEN CAST(DATEADD(MONTH, -1, @dtsSendDate) AS date) AND @dtsSendDate
      AND BlockFlag = 0;

	SET @i = 1
	
	WHILE 1 = 1
	BEGIN
		IF NOT EXISTS(SELECT * FROM @tblDarkLairRankingID WHERE Seq BETWEEN @i AND @i + 99)
			BREAK;

		DELETE dbo.DarkLairPersonalBest
		FROM @tblDarkLairRankingID A
			INNER JOIN dbo.DarkLairPersonalBest B ON A.DarkLairRankingID = B.DarkLairRankingID
		WHERE Seq BETWEEN @i AND @i + 99;

		DELETE dbo.DarkLairPartyMembers
		FROM @tblDarkLairRankingID A
			INNER JOIN dbo.DarkLairPartyMembers B ON A.DarkLairRankingID = B.DarkLairRankingID
		WHERE Seq BETWEEN @i AND @i + 99;

		DELETE dbo.DarkLairRanking
		FROM @tblDarkLairRankingID A
			INNER JOIN dbo.DarkLairRanking B ON A.DarkLairRankingID = B.DarkLairRankingID
		WHERE Seq BETWEEN @i AND @i + 99;

		SET @i += 100;

		WAITFOR DELAY '00:00:01'
	END

	INSERT dbo.ChallengeDarkLairRewardCharacters (BasisDate, Ranking, PartyMemberCount, CharacterID)
	SELECT DATEADD(MONTH, -1, @dtsSendDate), Ranking, PartyUserCount, CharacterID 
	FROM @tblRewardTarget2

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

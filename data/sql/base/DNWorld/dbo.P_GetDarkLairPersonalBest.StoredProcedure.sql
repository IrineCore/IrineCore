USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetDarkLairPersonalBest]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-02-01
description : ???? ?? ??? ??? ??? ?????.
result set : CharacterName/nvarchar, CharacterLevel/tinyint, JobCode/tinyint
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
2 = ?? ?? ??? at 2010-08-22
**/

CREATE PROCEDURE [dbo].[P_GetDarkLairPersonalBest]
	  @inbCharacterID bigint --// ??? ID
	, @intMapID int --// ? ID
	, @inyPartyUserCount tinyint --// ?? ??
	, @intRanking int OUTPUT --// ?? ??? ??? ??. 10000? ??? ??, ?? 10000? ?????.
	, @nvcPartyName nvarchar(100) OUTPUT --// ?? ??
	, @insPlayRound smallint OUTPUT --// ??? ??? ?
	, @intPlayTime int OUTPUT --// ??? ?? (?)
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @inbDarkLairRankingID bigint;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# ?? ???? ?? ??? ?????.*/
	SELECT @insPlayRound = DLR.PlayRound, @intPlayTime = DLR.PlayTime, @nvcPartyName = DLR.PartyName, @inbDarkLairRankingID = DLR.DarkLairRankingID
	FROM dbo.DarkLairPersonalBest DLB WITH (NOLOCK)
		INNER JOIN dbo.DarkLairRanking DLR WITH (NOLOCK) ON DLB.DarkLairRankingID = DLR.DarkLairRankingID
	WHERE DLB.CharacterID = @inbCharacterID AND DLB.MapID = @intMapID AND DLB.PartyUserCount = @inyPartyUserCount;

	/**_# ?? ???? ?? ?? ??? ?????.*/
	SET @intRanking = 1;

	SELECT TOP (9999) @intRanking += 1
	FROM dbo.DarkLairRanking DLR1 WITH (NOLOCK)
		INNER JOIN dbo.DarkLairRanking DLR2 WITH (NOLOCK, INDEX (IX_NNIF_DarkLairRanking_MapID_PartyUserCount_PlayRound_PlayTime), FORCESEEK) ON DLR1.MapID = DLR2.MapID AND DLR1.PartyUserCount = DLR2.PartyUserCount
			AND (DLR2.PlayRound > DLR1.PlayRound OR (DLR2.PlayRound = DLR1.PlayRound AND DLR2.PlayTime < DLR1.PlayTime))
	WHERE DLR1.DarkLairRankingID = @inbDarkLairRankingID AND DLR2.BestFlag = CAST(1 AS bit)
	OPTION (KEEPFIXED PLAN, MAXDOP 1);

	/**_# ?? ??? ??? ???? ?????.*/
	SELECT CA.CharacterName, DLM.CharacterLevel, DLM.JobCode
	FROM dbo.DarkLairPartyMembers DLM WITH (NOLOCK)
		INNER JOIN dbo.Characters CA WITH (NOLOCK) ON DLM.CharacterID = CA.CharacterID
	WHERE DLM.DarkLairRankingID = @inbDarkLairRankingID AND DLM.CompleteFlag = CAST(1 AS bit);
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

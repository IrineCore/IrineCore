USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetPVPScoreByCharacterName2]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-03-07
description : ??? ???? PVP ??? ?????.
result set : CharacterClassCode/tinyint, Win/int, Lose/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
4 = (62971) PVP ?? ? ?? ??? ?? by ??? at 2012-07-17
**/

CREATE PROCEDURE [dbo].[P_GetPVPScoreByCharacterName2]
	  @nvcCharacterName nvarchar(30) --// ??? ??
	, @intPVPWin int OUTPUT--// PVP ?
	, @intPVPLose int OUTPUT --// PVP ?
	, @intPVPDraw int OUTPUT --// PVP ???
	, @intPVPGiveUp int OUTPUT --// PVP ??
	, @inyPVPLevel tinyint OUTPUT --// PVP ??
	, @intPVPExp int OUTPUT --// PVP ???
	, @intPVPPlayTime int OUTPUT --// PVP ??? ?? (?)
	, @inbPVPKillPoint bigint OUTPUT --// PVP ? ???
	, @inbPVPAssisPoint bigint OUTPUT --// PVP ???? ???
	, @inbPVPSupportPoint bigint OUTPUT --// PVP ??? ???
	, @intPVPRanking int = NULL OUTPUT --// PVP ??
	, @fltPVPRankingRatio float = NULL OUTPUT --// PVP ???????
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @inbCharacterID bigint;
SET @intPVPWin = 0;
SET @intPVPLose = 0;
SET @intPVPDraw = 0;
SET @intPVPGiveUp = 0;
SET @inyPVPLevel = 0;
SET @intPVPExp = 0;
SET @intPVPPlayTime = 0;
SET @inbPVPKillPoint = 0;
SET @inbPVPAssisPoint = 0;
SET @inbPVPSupportPoint  = 0; 

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT @inbCharacterID = CharacterID
	FROM dbo.Characters WITH (NOLOCK)
	WHERE CharacterName = @nvcCharacterName AND DeleteFlag = CAST(0 AS bit);

	SELECT @intPVPWin = PVPWin
		 , @intPVPLose = PVPLose
		 , @intPVPDraw = PVPDraw
		 , @intPVPGiveUp = PVPGiveUp
		 , @inyPVPLevel = PVPLevel
		 , @intPVPExp = PVPExp
		 , @intPVPPlayTime = PVPPlayTime
		 , @inbPVPKillPoint = PVPKillPoint
		 , @inbPVPAssisPoint = PVPAssisPoint
		 , @inbPVPSupportPoint  = PVPSupportPoint 
	FROM dbo.PVPScores WITH(NOLOCK, INDEX(PK_PVPScores), FORCESEEK)
	WHERE CharacterID = @inbCharacterID
	OPTION(KEEPFIXED PLAN);
	
	SELECT CharacterClassCode, Win, Lose
	FROM dbo.PVPCharacterClassScores A WITH(NOLOCK, INDEX(PK_PVPCharacterClassScores), FORCESEEK)
	WHERE CharacterID = @inbCharacterID
	OPTION(KEEPFIXED PLAN);

	SELECT @intPVPRanking = Ranking
           , @fltPVPRankingRatio = RankRatio
	FROM dbo.PVPTopRanker WITH (NOLOCK)
	WHERE CharacterID = @inbCharacterID;

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

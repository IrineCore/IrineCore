USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddPVPTopRanker]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 6
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-07-17
description : PVP ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
6 = GM ?? ?? ???? ?? by ??? at 2013-05-09
**/

CREATE PROCEDURE [dbo].[P_AddPVPTopRanker]	  
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;
DECLARE @intReturnValue int;
DECLARE @intPVPRankCriteria int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# ????? ?????? SELECT ???.*/
	SELECT @intPVPRankCriteria = ExpCriteria FROM dbo.PVPRankCriteria WITH (NOLOCK);

	IF @intPVPRankCriteria IS NULL
	BEGIN
		IF EXISTS (SELECT * FROM dbo.PVPRankCriteria WITH (NOLOCK))
			UPDATE dbo.PVPRankCriteria SET ExpCriteria = 403400;
		ELSE
			INSERT dbo.PVPRankCriteria 
			SELECT 403400;

		SET @intPVPRankCriteria = 403400;
	END

	/**_# [PVPTopRanker] ???? TRUNCATE ???.*/
	TRUNCATE TABLE dbo.PVPTopRanker;

	/**_# [PVPTopRanker] ???? INSERT ???.*/
	;WITH R AS (
		SELECT RANK() OVER (ORDER BY PS.PVPExp DESC) AS ExpRank, PS.CharacterID
		FROM dbo.PVPScores PS WITH (NOLOCK) INNER JOIN dbo.Characters CH WITH (NOLOCK) ON PS.CharacterID = CH.CharacterID
		WHERE PS.PVPExp >= @intPVPRankCriteria AND CH.DeleteFlag = 0 AND CH.AccountLevelCode < 99
	), C AS (
		SELECT Count(*) AS CharacterCount
		FROM R
	)
	INSERT dbo.PVPTopRanker (CharacterID, Ranking, RankRatio)
	SELECT R.CharacterID, R.ExpRank, CASE WHEN ExpRank <= 101 THEN 0 ELSE Round((CAST((R.ExpRank - 101) AS float) / CAST((C.CharacterCount - 101) AS float)) * 100,2) END  AS RankRatio
	FROM R, C;

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

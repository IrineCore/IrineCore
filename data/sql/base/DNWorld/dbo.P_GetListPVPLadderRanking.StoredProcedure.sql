USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListPVPLadderRanking]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-03-29
description : PVP ?? ??? ?????.
result set : RankingNumber/int, CharacterID/bigint, PVPLadderGradePoint/int, CharacterName/nvarchar, JobCode/tinyint, Win/int, Lose/int, Draw/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListPVPLadderRanking]
	  @inbCharacterID bigint --// ??? ID
	, @inyPVPLadderCode tinyint --// 1=1:1, 2=2:2, 3=3:3, 4=4:4
	, @intRankNumber int OUTPUT --// ?? ??
	, @intPVPLadderGradePoint int OUTPUT --// PVP ?? ??
	, @nvcCharacterName nvarchar(30) OUTPUT --// ??? ??
	, @inyJobCode tinyint OUTPUT --// ?? ??
	, @intWin int OUTPUT --// ?? ??
	, @intLose int OUTPUT --// ?? ??
	, @intDraw int OUTPUT --// ??? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @intWinRatio int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT TOP (10) RANK() OVER(ORDER BY PVPLadderGradePoint DESC) AS RankingNumber
		, CharacterID, PVPLadderGradePoint, CharacterName, JobCode, Win, Lose, Draw
	FROM dbo.VIX_PVPLadderRanking WITH (NOLOCK, NOEXPAND)
	WHERE PVPLadderCode = @inyPVPLadderCode
	ORDER BY PVPLadderGradePoint DESC, WinRatio DESC, Lose;

	SELECT @intPVPLadderGradePoint = PVPLadderGradePoint
		, @nvcCharacterName = CharacterName
		, @inyJobCode = JobCode
		, @intWin = Win
		, @intLose = Lose
		, @intDraw = Draw
		, @intWinRatio = WinRatio
	FROM dbo.VIX_PVPLadderRanking WITH (NOLOCK, NOEXPAND)
	WHERE PVPLadderCode = @inyPVPLadderCode AND CharacterID = @inbCharacterID;

	IF @@ROWCOUNT > 0
	BEGIN
		SET @intRankNumber = 1;

		SELECT @intRankNumber += 1
		FROM dbo.VIX_PVPLadderRanking WITH (NOLOCK, NOEXPAND)
		WHERE PVPLadderCode = @inyPVPLadderCode
			AND PVPLadderGradePoint > @intPVPLadderGradePoint
		ORDER BY PVPLadderGradePoint DESC, WinRatio DESC, Lose;
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

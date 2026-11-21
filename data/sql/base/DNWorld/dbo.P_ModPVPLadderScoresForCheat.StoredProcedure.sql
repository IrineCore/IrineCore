USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModPVPLadderScoresForCheat]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : wgkim@eyedentitygames
created date : 2011-02-22
description :
	PVP ?? - ?????, ??, ???? ???? ??SP ???.
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
	2 = ???? ?? ?? - 20110222.wgkim
	3 = ???? null ? ??? ?? ?? ?? ??
	4 = ?? ??? ?? PVP ??? ??? 1? ???? ??? 0 ?? ??     by ??? at 2011-11-17
**/

CREATE PROCEDURE [dbo].[P_ModPVPLadderScoresForCheat]
	  @inbCharacterID bigint --// ??? ID
	, @inyPVPLadderCode tinyint --// 1=1:1, 2=2:2, 3=3:3, 4=4:4
	, @intPVPLadderPoint int = null --// PVP ?? ???
	, @intPVPLadderGradePoint int = null --// PVP ?? ??
	, @intHiddenPVPLadderGradePoint int = null --// PVP ?? ???

--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @dt2Now datetime2(0) = GETDATE();

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

    /**_# [CharacterStatus] ???? PVP ?? ???? UPDATE???.*/
    UPDATE dbo.CharacterStatus
    SET PVPLadderPoint = CASE WHEN @intPVPLadderPoint IS NULL THEN PVPLadderPoint ELSE @intPVPLadderPoint END
    WHERE CharacterID = @inbCharacterID;


	/**_# [PVPLadderScores] ???? PVP ?? ??, PVP ?? ???? UPDATE???.*/
	UPDATE dbo.PVPLadderScores
	SET PVPLadderGradePoint = CASE WHEN @intPVPLadderGradePoint IS NULL THEN PVPLadderGradePoint ELSE @intPVPLadderGradePoint END 
		, HiddenPVPLadderGradePoint = CASE WHEN @intHiddenPVPLadderGradePoint IS NULL THEN HiddenPVPLadderGradePoint ELSE @intHiddenPVPLadderGradePoint END 
	WHERE CharacterID = @inbCharacterID AND PVPLadderCode = @inyPVPLadderCode;

	/**_## [PVPLadderScores] ???? UPDATE? ?? ??? ??? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
	    INSERT dbo.PVPLadderScores (CharacterID, PVPLadderCode, PVPLadderGradePoint, HiddenPVPLadderGradePoint, Win, Lose, Draw, TodayWin, TodayLose, TodayDraw, ConsecutiveWin, ConsecutiveLose, LastPVPLadderScoreDate, WeeklyPVPCount)
	    VALUES (@inbCharacterID, @inyPVPLadderCode, CASE WHEN @intPVPLadderGradePoint IS NULL THEN 0 ELSE @intPVPLadderGradePoint END, CASE WHEN @intHiddenPVPLadderGradePoint IS NULL THEN 0 ELSE @intHiddenPVPLadderGradePoint END, 0, 0, 0, 0, 0, 0, 0, 0, @dt2Now, 0);
	END;

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

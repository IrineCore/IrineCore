USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModPVPScore]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-02-01
description : PVP ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103102 = ???? ???? ????.
history :
2~3 = PVPScores ? UPDATE ?? Data ? ???? ??? INSERT (?? ??) - 20110111.wgkim
4 = ?? ??? ?? (????)     by 20110520.wgkim
**/

CREATE PROCEDURE [dbo].[P_ModPVPScore]
	  @inbCharacterID bigint --// ??? ID
	, @intPVPWin int --// PVP ?
	, @intPVPLose int --// PVP ?
	, @intPVPDraw int --// PVP ???
	, @intPVPGiveUp int --// PVP ??
	, @inyPVPLevel tinyint --// PVP ??
	, @intPVPExp int --// PVP ???
	, @intPVPPlayTime int --// PVP ??? ?? (?)
	, @inbPVPKillPoint bigint --// PVP ? ???
	, @inbPVPAssisPoint bigint --// PVP ???? ???
	, @inbPVPSupportPoint bigint --// PVP ??? ???
	, @intVsWarriorKOWin int --// ????? KO ?
	, @intVsWarriorKOLose int --// ????? KO ?
	, @intVsArcherKOWin int --// ???? KO ?
	, @intVsArcherKOLose int --// ???? KO ?
	, @intVsSorceressKOWin int --// ?????? KO ?
	, @intVsSorceressKOLose int --// ?????? KO ?
	, @intVsClericKOWin int --// ????? KO ?
	, @intVsClericKOLose int --// ????? KO ?
	, @intVsAcademicKOWin int = 0 --// ?????? KO ?
	, @intVsAcademicKOLose int = 0 --// ?????? KO ?
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

	/**_# [PVPScores]???? UPDATE???.*/
	UPDATE dbo.PVPScores
	SET PVPWin = @intPVPWin
		, PVPLose = @intPVPLose
		, PVPDraw = @intPVPDraw
		, PVPGiveUp = @intPVPGiveUp
		, PVPLevel = @inyPVPLevel
		, PVPExp = @intPVPExp
		, PVPPlayTime = @intPVPPlayTime
		, PVPKillPoint = @inbPVPKillPoint
		, PVPAssisPoint = @inbPVPAssisPoint
		, PVPSupportPoint = @inbPVPSupportPoint
		, vsWarriorKOWin = @intVsWarriorKOWin
		, vsWarriorKOLose = @intVsWarriorKOLose
		, vsArcherKOWin = @intVsArcherKOWin
		, vsArcherKOLose = @intVsArcherKOLose
		, vsSorceressKOWin = @intVsSorceressKOWin
		, vsSorceressKOLose = @intVsSorceressKOLose
		, vsClericKOWin = @intVsClericKOWin
		, vsClericKOLose = @intVsClericKOLose
		, vsAcademicKOWin = @intVsAcademicKOWin
		, vsAcademicKOLose = @intVsAcademicKOLose
	WHERE CharacterID = @inbCharacterID;

	/**_## UPDATE? ?? ??? [PVPScores]???? INSERT???.*/
	IF @@ROWCOUNT = 0
	BEGIN
		--SET @intReturnValue = 103102;
		--GOTO ErrorHandler;

	    INSERT dbo.PVPScores (CharacterID, PVPWin, PVPLose, PVPDraw, PVPGiveUp, PVPLevel, PVPExp, PVPPlayTime, PVPKillPoint, PVPAssisPoint, PVPSupportPoint, vsWarriorKOWin, vsWarriorKOLose, vsArcherKOWin, vsArcherKOLose, vsSorceressKOWin, vsSorceressKOLose, vsClericKOWin, vsClericKOLose, vsAcademicKOWin, vsAcademicKOLose)
	    VALUES (@inbCharacterID, @intPVPWin, @intPVPLose, @intPVPDraw, @intPVPGiveUp, @inyPVPLevel, @intPVPExp, @intPVPPlayTime, @inbPVPKillPoint, @inbPVPAssisPoint, @inbPVPSupportPoint, @intVsWarriorKOWin, @intVsWarriorKOLose, @intVsArcherKOWin, @intVsArcherKOLose,@intVsSorceressKOWin, @intVsSorceressKOLose,@intVsClericKOWin, @intVsClericKOLose, @intVsAcademicKOWin, @intVsAcademicKOLose)
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

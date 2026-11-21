USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListPVPScoresForSnapShot]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-03-26
description : PVPScores? DailySnapShot?? ?????
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListPVPScoresForSnapShot]
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
	IF EXISTS(SELECT TOP 1* FROM dbo.PVPCharacterClassScores WITH(NOLOCK))
	BEGIN
		SELECT A.CharacterID
			 , PVPWin
			 , PVPLose
			 , PVPDraw
			 , PVPGiveUp
			 , PVPLevel
			 , PVPExp
			 , PVPPlayTime
			 , PVPKillPoint
			 , PVPAssisPoint
			 , PVPSupportPoint 
			 , SUM(CASE WHEN B.CharacterClassCode = 1 THEN B.Win ELSE 0 END) AS vsWarriorKOWin
			 , SUM(CASE WHEN B.CharacterClassCode = 1 THEN B.Lose ELSE 0 END) AS vsWarriorKOLose
			 , SUM(CASE WHEN B.CharacterClassCode = 2 THEN B.Win ELSE 0 END) AS vsArcherKOWin
			 , SUM(CASE WHEN B.CharacterClassCode = 2 THEN B.Lose ELSE 0 END) AS vsArcherKOLose
			 , SUM(CASE WHEN B.CharacterClassCode = 3 THEN B.Win ELSE 0 END) AS vsSorceressKOWin
			 , SUM(CASE WHEN B.CharacterClassCode = 3 THEN B.Lose ELSE 0 END) AS vsSorceressKOLose
			 , SUM(CASE WHEN B.CharacterClassCode = 4 THEN B.Win ELSE 0 END) AS vsClericKOWin
			 , SUM(CASE WHEN B.CharacterClassCode = 4 THEN B.Lose ELSE 0 END) AS vsClericKOLose
			 , SUM(CASE WHEN B.CharacterClassCode = 5 THEN B.Win ELSE 0 END) AS vsAcademicKOWin
			 , SUM(CASE WHEN B.CharacterClassCode = 5 THEN B.Lose ELSE 0 END) AS vsAcademicKOLose
			 , SUM(CASE WHEN B.CharacterClassCode = 6 THEN B.Win ELSE 0 END) AS vsKaliKOWin
			 , SUM(CASE WHEN B.CharacterClassCode = 6 THEN B.Lose ELSE 0 END) AS vsKaliKOLose
			 , SUM(CASE WHEN B.CharacterClassCode = 7 THEN B.Win ELSE 0 END) AS vsAssasinKOWin
			 , SUM(CASE WHEN B.CharacterClassCode = 7 THEN B.Lose ELSE 0 END) AS vsAssasinKOLose
		FROM dbo.PVPScores A WITH(NOLOCK) 
			LEFT OUTER JOIN dbo.PVPCharacterClassScores B WITH(NOLOCK) ON A.CharacterID = B.CharacterID
		GROUP BY A.CharacterID , PVPWin, PVPLose, PVPDraw, PVPGiveUp, PVPLevel, PVPExp, PVPPlayTime, PVPKillPoint, PVPAssisPoint, PVPSupportPoint 
	END
	ELSE
	BEGIN
		SELECT A.CharacterID
			 , PVPWin
			 , PVPLose
			 , PVPDraw
			 , PVPGiveUp
			 , PVPLevel
			 , PVPExp
			 , PVPPlayTime
			 , PVPKillPoint
			 , PVPAssisPoint
			 , PVPSupportPoint 
			 , vsWarriorKOWin
			 , vsWarriorKOLose
			 , vsArcherKOWin
			 , vsArcherKOLose
			 , vsSorceressKOWin
			 , vsSorceressKOLose
			 , vsClericKOWin
			 , vsClericKOLose
			 , vsAcademicKOWin
			 , vsAcademicKOLose
			 , 0 AS vsKaliKOWin
			 , 0 AS vsKaliKOLose
			 , 0 AS vsAssasinKOWin
			 , 0 AS vsAssasinKOLose
		FROM dbo.PVPScores A WITH(NOLOCK) 
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

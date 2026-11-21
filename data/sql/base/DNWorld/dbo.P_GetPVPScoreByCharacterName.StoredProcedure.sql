USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetPVPScoreByCharacterName]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 6
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-07-08
description : ??? ???? PVP ??? ?????.
result set : PVPWin/int, PVPLose/int, PVPDraw/int, PVPGiveUp/int, PVPLevel/tinyint, PVPExp/int, PVPPlayTime/int, PVPKillPoint/bigint, PVPAssisPoint/bigint, PVPSupportPoint/bigint, vsWarriorKOWin/int, vsWarriorKOLose/int, vsArcherKOWin/int, vsArcherKOLose/int, vsSorceressKOWin/int, vsSorceressKOLose/int, vsClericKOWin/int, vsClericKOLose
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103102 = ???? ???? ????.
history :
2~4 = PVPScore ? CharacterID ? ???? ??? ??? ????? ????? ?? - 20110111.wgkim
5 = ??? ?? 2? ???? ?? ?? at 2010-01-19
6 = ?? ??? ?? (????) ? ?? ??     by 20110520.wgkim
**/

CREATE PROCEDURE [dbo].[P_GetPVPScoreByCharacterName]
	  @nvcCharacterName nvarchar(30) --// ??? ??
	, @inyVersion tinyint = 0 --// ?? ??? ???? ?? ???? @inyVersion=1 ? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @inbCharacterID bigint;

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

	IF @inyVersion = 1
	BEGIN

		IF EXISTS (SELECT * FROM dbo.PVPScores WITH (NOLOCK) WHERE CharacterID = @inbCharacterID)
			SELECT PVPWin, PVPLose, PVPDraw, PVPGiveUp, PVPLevel, PVPExp, PVPPlayTime, PVPKillPoint, PVPAssisPoint, PVPSupportPoint, vsWarriorKOWin, vsWarriorKOLose, vsArcherKOWin, vsArcherKOLose, vsSorceressKOWin, vsSorceressKOLose, vsClericKOWin, vsClericKOLose, vsAcademicKOWin, vsAcademicKOLose
			FROM dbo.PVPScores WITH (NOLOCK)
			WHERE CharacterID = @inbCharacterID;

		ELSE
			SELECT 0 AS PVPWin, 0 AS PVPLose, 0 AS PVPDraw, 0 AS PVPGiveUp, 1 AS PVPLevel, 0 AS PVPExp, 0 AS PVPPlayTime, 0 AS PVPKillPoint, 0 AS PVPAssisPoint, 0 AS PVPSupportPoint, 0 AS vsWarriorKOWin, 0 AS vsWarriorKOLose, 0 AS vsArcherKOWin, 0 AS vsArcherKOLose, 0 AS vsSorceressKOWin, 0 AS vsSorceressKOLose, 0 AS vsClericKOWin, 0 AS vsClericKOLose, 0 AS vsAcademicKOWin, 0 AS vsAcademicKOLose

	END
	ELSE
	BEGIN

		IF EXISTS (SELECT * FROM dbo.PVPScores WITH (NOLOCK) WHERE CharacterID = @inbCharacterID)
			SELECT PVPWin, PVPLose, PVPDraw, PVPGiveUp, PVPLevel, PVPExp, PVPPlayTime, PVPKillPoint, PVPAssisPoint, PVPSupportPoint, vsWarriorKOWin, vsWarriorKOLose, vsArcherKOWin, vsArcherKOLose, vsSorceressKOWin, vsSorceressKOLose, vsClericKOWin, vsClericKOLose
			FROM dbo.PVPScores WITH (NOLOCK)
			WHERE CharacterID = @inbCharacterID;

		ELSE
			SELECT 0 AS PVPWin, 0 AS PVPLose, 0 AS PVPDraw, 0 AS PVPGiveUp, 1 AS PVPLevel, 0 AS PVPExp, 0 AS PVPPlayTime, 0 AS PVPKillPoint, 0 AS PVPAssisPoint, 0 AS PVPSupportPoint, 0 AS vsWarriorKOWin, 0 AS vsWarriorKOLose, 0 AS vsArcherKOWin, 0 AS vsArcherKOLose, 0 AS vsSorceressKOWin, 0 AS vsSorceressKOLose, 0 AS vsClericKOWin, 0 AS vsClericKOLose

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

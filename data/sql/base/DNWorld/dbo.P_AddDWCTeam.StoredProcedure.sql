USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddDWCTeam]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 12
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-01-10
description : DWC ?? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103357 = ?? ???? DWC???? ???? ???? ????.
103358 = ?? ???? DWC ?? ???.
**/

CREATE PROCEDURE [dbo].[P_AddDWCTeam]
	  @nvcDWCTeamName nvarchar(50) --// DWC ? ??
	, @inbCharacterID bigint --// ??? ???ID
	, @intDWCTeamID int = NULL OUTPUT --// DWC ?ID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @dt2Now datetime2(0) = GETDATE()

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	IF NOT EXISTS (SELECT * FROM dbo.Characters WITH(NOLOCK, INDEX(PK_Characters), FORCESEEK) WHERE CharacterID = @inbCharacterID AND AccountLevelCode = 110)
	BEGIN
		SET @intReturnValue = 103357;
		GOTO ErrorHandler;
	END

	BEGIN TRAN

		/**_# [DWCTeams] ???? INSERT???.*/
		INSERT dbo.DWCTeams (DWCTeamName, CloseFlag, Win, Lose, Draw, DWCPoint, CreateDate, TodayWin, TodayLose, TodayDraw, ConsecutiveWin, ConsecutiveLose, WeeklyPlayCount, LastPlayDate, DWCHiddenPoint)
		VALUES (@nvcDWCTeamName, 0, 0, 0, 0, 0, @dt2Now, 0, 0, 0, 0, 0, 0, @dt2Now, 0)

		SET @intDWCTeamID = SCOPE_IDENTITY();

		/**_# [DWCTeamMembers] ???? INSERT???.*/
		INSERT dbo.DWCTeamMembers (DWCTeamID, CharacterID, JobCode, TeamLeaderFlag, RegisterDate)
		SELECT @intDWCTeamID, CharacterID, JobCode, 1, @dt2Now
		FROM dbo.CharacterStatus WITH(NOLOCK, INDEX(PK_CharacterStatus), FORCESEEK)
		WHERE CharacterID = @inbCharacterID
		OPTION (KEEPFIXED PLAN);

	COMMIT TRAN

	/**_# DWC ? ??? ?????. */
	INSERT dbo.DWCTeamHistory (DWCTeamID, DWCTeamHistoryCode, CharacterID, RegisterDate)
	VALUES (@intDWCTeamID, 1, @inbCharacterID, @dt2Now)	

END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 2601 AND ERROR_MESSAGE() LIKE N'%IX_UNF_DWCTeams_DWCTeamName%'
		SET @intReturnValue = 103358;

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

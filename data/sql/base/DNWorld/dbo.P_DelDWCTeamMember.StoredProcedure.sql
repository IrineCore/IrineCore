USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DelDWCTeamMember]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-01-10
description : DWC???? ?? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103360 = ?? ??? ???? ?? ??????.
**/

CREATE PROCEDURE [dbo].[P_DelDWCTeamMember]
	  @intDWCTeamID int --// DWC ?ID
	, @inbCharacterID bigint --// ??? ID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @inbTeamLeaderFlag tinyint;
DECLARE @dt2Now datetime2(0) = GETDATE()
DECLARE @intDWCPointBefore int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT @inbTeamLeaderFlag = TeamLeaderFlag 
	FROM dbo.DWCTeamMembers WITH(NOLOCK, INDEX(PK_DWCTeamMembers), FORCESEEK) 
	WHERE DWCTeamID = @intDWCTeamID AND CharacterID = @inbCharacterID;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103360;
		GOTO ErrorHandler;
	END

	/**_# ??? ???? ?? ???? ???? DELETE???.*/
	IF @inbTeamLeaderFlag = 1
	BEGIN
		BEGIN TRAN
			/**_# [DWCTeamMembers] ???? DELETE???.*/
			DELETE dbo.DWCTeamMembers
			WHERE DWCTeamID = @intDWCTeamID;

			/**_# [DWCTeams] Close ???.*/
			UPDATE dbo.DWCTeams
			SET CloseFlag = 1, CloseDate = @dt2Now
			WHERE DWCTeamID = @intDWCTeamID;
		COMMIT TRAN

		/**_# DWC ? ??? ?????. */
		INSERT dbo.DWCTeamHistory (DWCTeamID, DWCTeamHistoryCode, RegisterDate)
		VALUES (@intDWCTeamID, 4, @dt2Now)	
	END
	/**_# ??? ???? ?? ??? ? ??? Reset ???.*/
	ELSE
	BEGIN
		BEGIN TRAN
			/**_# [DWCTeamMembers] ???? DELETE???.*/
			DELETE dbo.DWCTeamMembers
			WHERE DWCTeamID = @intDWCTeamID AND CharacterID = @inbCharacterID;

			/**_# [DWCTeams] ??? ?? ???.*/
			UPDATE dbo.DWCTeams
			SET @intDWCPointBefore = DWCPoint
			  ,	Win = 0
			  , Lose = 0
			  , Draw = 0
			  , DWCPoint = 0
			  , TodayWin = 0
			  , TodayLose = 0
			  , TodayDraw = 0
			  , ConsecutiveWin = 0
			  , ConsecutiveLose = 0
			  , WeeklyPlayCount = 0
			  , DWCHiddenPoint = 0
			WHERE DWCTeamID = @intDWCTeamID;
		COMMIT TRAN

		/**_# DWC ? ??? ?????. */
		INSERT dbo.DWCTeamHistory (DWCTeamID, DWCTeamHistoryCode, CharacterID, RegisterDate)
		VALUES (@intDWCTeamID, 3, @inbCharacterID, @dt2Now)	

		/**_# DWC ??? ??? ??? ?????. */
		INSERT dbo.DWCTeamPointPlayHistory (DWCTeamID, DWCPointChangeCode, DWCPointBefore, DWCPoint, RegisterDate)
		VALUES (@intDWCTeamID, 7, @intDWCPointBefore, 0, @dt2Now)	

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

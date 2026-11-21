USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DelPVPLadderScores]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2011-06-28

description :
	?? ??? ???
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
	3 = ?? ??
**/

CREATE PROCEDURE [dbo].[P_DelPVPLadderScores]
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

	BEGIN TRAN

		IF EXISTS (SELECT * FROM dbo.PVPLadderScores WITH (NOLOCK))
		BEGIN
			IF OBJECT_ID(N'dbo.PVPLadderScoresPast', N'U') IS NOT NULL DROP TABLE dbo.PVPLadderScoresPast;

			SELECT * INTO dbo.PVPLadderScoresPast FROM dbo.PVPLadderScores;

			DELETE dbo.PVPLadderScores;
		END

		IF EXISTS (SELECT * FROM dbo.PVPLadderScoresByJob WITH (NOLOCK))
		BEGIN
			IF OBJECT_ID(N'dbo.PVPLadderScoresByJobPast', N'U') IS NOT NULL DROP TABLE dbo.PVPLadderScoresByJobPast;

			SELECT * INTO dbo.PVPLadderScoresByJobPast FROM dbo.PVPLadderScoresByJob;

			DELETE dbo.PVPLadderScoresByJob;
		END

		IF EXISTS (SELECT * FROM dbo.PVPLadderKillDeathByJob WITH (NOLOCK))
		BEGIN
			IF OBJECT_ID(N'dbo.PVPLadderKillDeathByJobPast', N'U') IS NOT NULL DROP TABLE dbo.PVPLadderKillDeathByJobPast;

			SELECT * INTO dbo.PVPLadderKillDeathByJobPast FROM dbo.PVPLadderKillDeathByJob;

			DELETE dbo.PVPLadderKillDeathByJob;
		END

		IF EXISTS (SELECT * FROM dbo.CharacterStatus WITH (NOLOCK) WHERE PVPLadderPoint > 0)
		BEGIN
			IF OBJECT_ID(N'dbo.PVPLadderPointPast', N'U') IS NOT NULL DROP TABLE dbo.PVPLadderPointPast;

			SELECT * INTO dbo.PVPLadderPointPast FROM dbo.CharacterStatus;

			UPDATE dbo.CharacterStatus SET PVPLadderPoint = 0;
		END

	COMMIT TRAN

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

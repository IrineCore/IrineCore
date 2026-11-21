USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddDWCTeamMember]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 6
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-01-10
description : DWC ?? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103357 = ?? ???? DWC???? ???? ???? ????.
103359 = ?? ???? ?? DWC ?? ??? ????
103392 = DWC?? ??? ? ?? ??? ???????.
**/

CREATE PROCEDURE [dbo].[P_AddDWCTeamMember]
	  @intDWCTeamID int --// DWC ? ID
	, @inbCharacterID bigint --// ???ID
	, @intMaxDWCMemberCount int = NULL--// ?? DWC ?? ?
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @dt2Now datetime2(0) = GETDATE()
DECLARE @intCurrentDWCMemberCount int;

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

	IF @intMaxDWCMemberCount IS NOT NULL
	BEGIN
	
		SELECT @intCurrentDWCMemberCount = COUNT(*) 
		FROM dbo.DWCTeamMembers WITH (NOLOCK, INDEX(PK_DWCTeamMembers), FORCESEEK) 
		WHERE DWCTeamID = @intDWCTeamID;

		IF @intCurrentDWCMemberCount >= @intMaxDWCMemberCount
		BEGIN
			SET @intReturnValue = 103392;
			GOTO ErrorHandler;
		END
	END

	/**_# [DWCTeamMembers] ???? INSERT???.*/
	INSERT dbo.DWCTeamMembers (DWCTeamID, CharacterID, JobCode, TeamLeaderFlag, RegisterDate)
	SELECT @intDWCTeamID, CharacterID, JobCode, 0, @dt2Now
	FROM dbo.CharacterStatus WITH(NOLOCK, INDEX(PK_CharacterStatus), FORCESEEK)
	WHERE CharacterID = @inbCharacterID
	OPTION (KEEPFIXED PLAN);

	COMMIT TRAN

	/**_# DWC ? ??? ?????. */
	INSERT dbo.DWCTeamHistory (DWCTeamID, DWCTeamHistoryCode, CharacterID, RegisterDate)
	VALUES (@intDWCTeamID, 2, @inbCharacterID, @dt2Now)	

END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 2601 AND ERROR_MESSAGE() LIKE N'%IX_UN_DWCTeamMembers_CharacterID%'
		SET @intReturnValue = 103359;

	ELSE IF ERROR_NUMBER() = 2627 AND ERROR_MESSAGE() LIKE N'%PK_DWCTeamMembers%'
		SET @intReturnValue = 103359;

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

USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListDWCTeamMember]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-01-16
description : DWC ? ??? ?????.
result set : AccountID/int ,CharacterID/bigint, CharacterName/nvarchar(50), JobCode/tinyint, TeamLeaderFlag/bit
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListDWCTeamMember]
	  @intDWCTeamID int = NULL --// DWC ?ID
	, @nvcDWCTeamName nvarchar(50) = NULL --// DWC ? ??
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

	IF @intDWCTeamID > 0 
		SELECT C.AccountID, C.CharacterID, C.CharacterName, DTM.JobCode, DTM.TeamLeaderFlag
		FROM dbo.DWCTeams DT WITH(NOLOCK, INDEX(PK_DWCTeams), FORCESEEK)
			INNER LOOP JOIN dbo.DWCTeamMembers DTM WITH(NOLOCK, INDEX(PK_DWCTeamMembers), FORCESEEK) ON DT.DWCTeamID = DTM.DWCTeamID
			INNER LOOP JOIN dbo.Characters C WITH(NOLOCK, INDEX(PK_Characters), FORCESEEK) ON C.CharacterID = DTM.CharacterID
		WHERE DT.DWCTeamID = @intDWCTeamID AND DT.CloseFlag = CAST(0 AS bit)
		OPTION (KEEPFIXED PLAN);
	ELSE 
		SELECT C.AccountID, C.CharacterID, C.CharacterName, DTM.JobCode, DTM.TeamLeaderFlag
		FROM dbo.DWCTeams DT WITH(NOLOCK, INDEX(IX_UNF_DWCTeams_DWCTeamName), FORCESEEK)
			INNER LOOP JOIN dbo.DWCTeamMembers DTM WITH(NOLOCK, INDEX(PK_DWCTeamMembers), FORCESEEK) ON DT.DWCTeamID = DTM.DWCTeamID
			INNER LOOP JOIN dbo.Characters C WITH(NOLOCK, INDEX(PK_Characters), FORCESEEK) ON C.CharacterID = DTM.CharacterID
		WHERE DT.DWCTeamName = @nvcDWCTeamName AND DT.CloseFlag = CAST(0 AS bit)
		OPTION (KEEPFIXED PLAN);
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

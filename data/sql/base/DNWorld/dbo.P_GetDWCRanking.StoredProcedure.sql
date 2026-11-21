USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetDWCRanking]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 6
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-02-04
description : ?? ?? ?? ???? ?? ? ??? ?????.
result set : Ranking/int, DWCTeamID/int, DWCTeamName/nvarchar(50), Win/int, Lose/int, Draw/int, DWCPoint/int, ChangedRanking/int, DWCTeamLeaderCharacterInfo/nvarchar(80), DWCTeamMemberCharacterInfo/nvarchar(160)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103361 = ???? ?? DWC????.
**/

CREATE PROCEDURE [dbo].[P_GetDWCRanking]
	  @nvcDWCTeamName nvarchar(50) = NULL--// DWC ? ??
	, @nvcDWCCharacterName nvarchar(50) = NULL --// DWC ??? ??
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
	IF @nvcDWCTeamName > ''
		SELECT B.Ranking, B.DWCTeamID, B.DWCTeamName, B.Win, B.Lose, B.Draw, B.DWCPoint, B.ChangedRanking, B.DWCTeamLeaderCharacterInfo, B.DWCTeamMemberCharacterInfo 
		FROM dbo.DWCTeams A WITH(NOLOCK, INDEX(IX_UNF_DWCTeams_DWCTeamName), FORCESEEK)
			INNER JOIN dbo.DWCRanking B WITH(NOLOCK, INDEX(IX_UN_DWCRanking_DWCTeamID), FORCESEEK) ON A.DWCTeamID = B.DWCTeamID
		WHERE A.DWCTeamName = @nvcDWCTeamName AND A.CloseFlag = CAST(0 AS bit)
		OPTION(KEEPFIXED PLAN);
	ELSE IF @nvcDWCCharacterName > ''
		SELECT C.Ranking, C.DWCTeamID, C.DWCTeamName, C.Win, C.Lose, C.Draw, C.DWCPoint, C.ChangedRanking, C.DWCTeamLeaderCharacterInfo, C.DWCTeamMemberCharacterInfo
		FROM dbo.Characters A WITH(NOLOCK, INDEX(IX_UNF_Characters_CharacterName), FORCESEEK) 
			INNER JOIN dbo.DWCTeamMembers B WITH(NOLOCK, INDEX(IX_UN_DWCTeamMembers_CharacterID), FORCESEEK) ON A.CharacterID = B.CharacterID
			INNER JOIN dbo.DWCRanking C WITH(NOLOCK, INDEX(IX_UN_DWCRanking_DWCTeamID), FORCESEEK) ON B.DWCTeamID = C.DWCTeamID
		WHERE A.CharacterName = @nvcDWCCharacterName AND A.DeleteFlag = CAST(0 AS bit)
		OPTION(KEEPFIXED PLAN);	

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103361;
		GOTO ErrorHandler;
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

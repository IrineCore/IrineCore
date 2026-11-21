USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListDWCTeamHistory]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-01-14
description : DWC ? ????? ?????.
result set : DWCTeamHistoryCode/tinyint, CharacterID/bigint, CharacterName/nvarchar(50) ,RegisterDate/datetime2(0)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103361 = ???? ?? DWC????.
**/

CREATE PROCEDURE [dbo].[P_GetListDWCTeamHistory]
	  @intDWCTeamID int = 0 --// DWC ?ID
	, @nvcDWCTeamName nvarchar(50) = NULL--// DWC ???
	, @dt2StartDate datetime2(0) --// ?? ?? ??
	, @dt2EndDate datetime2(0) --// ?? ?? ??
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
	/**_# [DWCTeamHistory] ???? ?????.*/
	IF @intDWCTeamID > 0
		SELECT DWCTeamHistoryCode, A.CharacterID, C.CharacterName, RegisterDate 
		FROM dbo.DWCTeamHistory A WITH(NOLOCK, INDEX(PK_DWCTeamHistory), FORCESEEK)
			INNER JOIN dbo.Characters C WITH(NOLOCK, INDEX(PK_Characters), FORCESEEK) ON A.CharacterID = C.CharacterID
		WHERE DWCTeamID = @intDWCTeamID
          AND RegisterDate BETWEEN @dt2StartDate AND @dt2EndDate
		OPTION(KEEPFIXED PLAN);
	ELSE IF @nvcDWCTeamName IS NOT NULL
		SELECT DWCTeamHistoryCode, B.CharacterID, C.CharacterName ,RegisterDate 
		FROM dbo.DWCTeams A WITH(NOLOCK, INDEX(IX_UNF_DWCTeams_DWCTeamName), FORCESEEK)
			INNER JOIN dbo.DWCTeamHistory B WITH(NOLOCK, INDEX(PK_DWCTeamHistory), FORCESEEK) ON B.DWCTeamID = A.DWCTeamID
			INNER JOIN dbo.Characters C WITH(NOLOCK, INDEX(PK_Characters), FORCESEEK) ON B.CharacterID = C.CharacterID
		WHERE A.DWCTeamName = @nvcDWCTeamName
          AND A.CloseFlag = 0
          AND RegisterDate BETWEEN @dt2StartDate AND @dt2EndDate
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

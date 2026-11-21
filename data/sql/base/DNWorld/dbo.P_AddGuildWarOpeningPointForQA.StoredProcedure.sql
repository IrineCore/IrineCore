USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddGuildWarOpeningPointForQA]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : wgkim@eyedentitygames
created date : 2011-11-23

description :
    ?? ???? ?? ?? ?? ?? (QA ??)
return value :
    0 = ??? ????.
    1 = ????? Commit? ? ?? ?????. ????? Rollback???.
    100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
    1 = ?? ??
	2 = TeamColorCode ???? ?? ??     by ??? at 2012.01.06
**/

CREATE PROCEDURE [dbo].[P_AddGuildWarOpeningPointForQA]
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @intGuildID int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	INSERT dbo.GuildWarOpeningPointRecord (CharacterID, MissionID, OpeningPoint, GuildID, TeamColorCode, RegisterDate)
	SELECT B.CharacterID, 500000 + ((CONVERT(INT, ROUND(RAND() * 10, 0)) * B.CharacterID) % 15), (CONVERT(INT, ROUND(RAND() * 10, 0)) * B.CharacterID) % 200, A.GuildID, A.TeamColorCode, GETDATE()
	  FROM dbo.Guilds A WITH (NOLOCK)
		   INNER JOIN dbo.GuildMembers B WITH (NOLOCK) ON (A.GuildID = B.GuildID)
	 WHERE A.TeamColorCode IS NOT NULL;

	INSERT dbo.GuildWarOpeningPointRecord (CharacterID, MissionID, OpeningPoint, GuildID, TeamColorCode, RegisterDate)
	SELECT B.CharacterID, 500000 + ((CONVERT(INT, ROUND(RAND() * 10, 0)) * B.CharacterID) % 15), (CONVERT(INT, ROUND(RAND() * 10, 0)) * B.CharacterID) % 200, A.GuildID, A.TeamColorCode, GETDATE()
	  FROM dbo.Guilds A WITH (NOLOCK)
		   INNER JOIN dbo.GuildMembers B WITH (NOLOCK) ON (A.GuildID = B.GuildID)
	 WHERE A.TeamColorCode IS NOT NULL;

	INSERT dbo.GuildWarOpeningPointRecord (CharacterID, MissionID, OpeningPoint, GuildID, TeamColorCode, RegisterDate)
	SELECT B.CharacterID, 500000 + ((CONVERT(INT, ROUND(RAND() * 10, 0)) * B.CharacterID) % 15), (CONVERT(INT, ROUND(RAND() * 10, 0)) * B.CharacterID) % 200, A.GuildID, A.TeamColorCode, GETDATE()
	  FROM dbo.Guilds A WITH (NOLOCK)
		   INNER JOIN dbo.GuildMembers B WITH (NOLOCK) ON (A.GuildID = B.GuildID)
	 WHERE A.TeamColorCode IS NOT NULL;

	INSERT dbo.GuildWarOpeningPointRecord (CharacterID, MissionID, OpeningPoint, GuildID, TeamColorCode, RegisterDate)
	SELECT B.CharacterID, 500000 + ((CONVERT(INT, ROUND(RAND() * 10, 0)) * B.CharacterID) % 15), (CONVERT(INT, ROUND(RAND() * 10, 0)) * B.CharacterID) % 200, A.GuildID, A.TeamColorCode, GETDATE()
	  FROM dbo.Guilds A WITH (NOLOCK)
		   INNER JOIN dbo.GuildMembers B WITH (NOLOCK) ON (A.GuildID = B.GuildID)
	 WHERE A.TeamColorCode IS NOT NULL;

	INSERT dbo.GuildWarOpeningPointRecord (CharacterID, MissionID, OpeningPoint, GuildID, TeamColorCode, RegisterDate)
	SELECT B.CharacterID, 500000 + ((CONVERT(INT, ROUND(RAND() * 10, 0)) * B.CharacterID) % 15), (CONVERT(INT, ROUND(RAND() * 10, 0)) * B.CharacterID) % 200, A.GuildID, A.TeamColorCode, GETDATE()
	  FROM dbo.Guilds A WITH (NOLOCK)
		   INNER JOIN dbo.GuildMembers B WITH (NOLOCK) ON (A.GuildID = B.GuildID)
	 WHERE A.TeamColorCode IS NOT NULL;

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

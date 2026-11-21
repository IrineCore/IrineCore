USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetPageGuildJoinCharactersForGuild]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2011-11-21
description : ?? ??? ??? ??? ?? ?? ?????? ?????.
result set : CharacterID/bigint, CharacterLevel/tinyint, JobCode/tinyint, CharacterName/nvarchar
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetPageGuildJoinCharactersForGuild]
	  @intStartNumber int --// ?? ??
	, @intEndNumber int --// ? ??
	, @intGuildID bigint --// ??ID
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
	/**_# ?? ??? ?? ??? ?? ??? ?????.*/
	;WITH PagingCTE AS (
		SELECT ROW_NUMBER() OVER(ORDER BY GuildID) AS PageNo, GuildID, CharacterID
		FROM dbo.GuildJoinCharacters WITH (NOLOCK, INDEX(PK_GuildJoinCharacters), FORCESEEK)
		WHERE GuildID = @intGuildID
	)
	SELECT A.CharacterID, B.CharacterLevel, B.JobCode, C.CharacterName
	FROM PagingCTE A
		INNER LOOP JOIN dbo.CharacterStatus B WITH (NOLOCK, INDEX(PK_CharacterStatus), FORCESEEK) ON A.CharacterID = B.CharacterID
		INNER LOOP JOIN dbo.Characters C WITH (NOLOCK, INDEX(PK_Characters), FORCESEEK) ON A.CharacterID = C.CharacterID
	WHERE A.PageNo BETWEEN @intStartNumber AND @intEndNumber
	OPTION (FORCE ORDER, LOOP JOIN, KEEPFIXED PLAN);
	
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

USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListGuildWarPopularityVote]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : wgkim@eyedentitygames
created date : 2011-03-08

description :
    .??? ?? ?? ??
result set :
    GuildID	int (or) CharacterID	bigint
    GuildName	nvarchar(50) (or) CharacterName	nvarchar(30)
    PopularityVoteCount int
return value :
    0 = ??? ????.
    1 = ????? Commit? ? ?? ?????. ????? Rollback???.
    100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
    2 = ?? ??
**/

CREATE PROCEDURE [dbo].[P_GetListGuildWarPopularityVote]
	  @inbCharacterID bigint = NULL --// ??? ID
	, @intGuildID int = NULL --// ?? ?? ID
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

	/**_# ??? ID ? ???? ???? ?? ???? ??? ??? ?????. */
    IF @inbCharacterID IS NOT NULL
    BEGIN
        SELECT G.GuildID
             , G.GuildName
             , 0 AS PopularityVoteCount
          FROM dbo.GuildWarPopularityVote GWPV WITH (NOLOCK)
               INNER JOIN dbo.Guilds G WITH (NOLOCK) ON (GWPV.GuildID = G.GuildID)
         WHERE GWPV.CharacterID = @inbCharacterID;
    END
	/**_# ?? ID ? ???? ???? ?? ??? ??? ???? ?????. */
    ELSE IF @intGuildID IS NOT NULL
    BEGIN
        SELECT C.CharacterID
             , C.CharacterName
             , 0 AS PopularityVoteCount
          FROM dbo.GuildWarPopularityVote GWPV WITH (NOLOCK)
               INNER JOIN dbo.Characters C WITH (NOLOCK) ON (GWPV.CharacterID = C.CharacterID)
         WHERE GWPV.GuildID = @intGuildID;
    END
	/**_# ??? ID ? ?? ID ? ?? NULL ? ???? ???? ?? ?? 1? ??? ?????. */
    ELSE 
    BEGIN
        SELECT TOP 1
               G.GuildID
             , G.GuildName
             , COUNT(GWPV.CharacterID) AS PopularityVoteCount
          FROM dbo.GuildWarPopularityVote GWPV WITH (NOLOCK)
               INNER JOIN dbo.Guilds G WITH (NOLOCK) ON (GWPV.GuildID = G.GuildID)
         GROUP BY G.GuildID, G.GuildName
         ORDER BY COUNT(GWPV.CharacterID) DESC
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

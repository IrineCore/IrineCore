USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListGuildWarOpeningPointGuildTotalByMission]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : wgkim@eyedentitygames
created date : 2011-03-10

description :
    ??? ?? ??? ?? ??? ?? ??
result set :
    Ranking	int
    GuildWarMissionTypeCode	tinyint
    GuildID	int
    GuildName	nvarchar(50)
    CharacterID	bigint
    CharacterName	nvarchar(30)
    OpeningPointSum	int
return value :
    0 = ??? ????.
    1 = ????? Commit? ? ?? ?????. ????? Rollback???.
    100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
    2 = ?? ??
**/

CREATE PROCEDURE [dbo].[P_GetListGuildWarOpeningPointGuildTotalByMission]
    @intGuildID int = NULL --// ?? ID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
      , @inyFinalTeamCount tinyint;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	/**_# ?? ID ? ?? ???? ? ??? 1?? ?????. */
    IF @intGuildID IS NULL
    BEGIN

        SELECT PT.Ranking           -- ??
             , PT.GuildWarMissionTypeCode -- 0=??, 1=??, 2=??, 3=??, 4=??, 5=??
             , PT.GuildID           -- ??ID
             , G.GuildName          -- ???
             , C.CharacterID        -- ???ID
             , C.CharacterName      -- ????
             , PT.OpeningPointSum   -- ????
          FROM dbo.GuildWarOpeningPointGuildTotal PT WITH (NOLOCK)
               INNER JOIN dbo.Guilds G WITH (NOLOCK) ON (PT.GuildID = G.GuildID)
               INNER JOIN dbo.GuildMembers GM WITH (NOLOCK) ON (G.GuildID = GM.GuildID AND GM.GuildMemberRoleCode = 0)
               INNER JOIN dbo.Characters C WITH (NOLOCK) ON (GM.CharacterID = C.CharacterID)
         WHERE PT.Ranking = 1
         ORDER BY PT.GuildWarMissionTypeCode;

    END
	/**_# ?? ID ? ?? ???? ?? ??? ??? ??? ?????. */
    ELSE
    BEGIN

        SELECT PT.Ranking           -- ??
             , PT.GuildWarMissionTypeCode -- 0=??, 1=??, 2=??, 3=??, 4=??, 5=??
             , PT.GuildID           -- ??ID
             , G.GuildName          -- ???
             , C.CharacterID        -- ???ID
             , C.CharacterName      -- ????
             , PT.OpeningPointSum   -- ????
          FROM dbo.GuildWarOpeningPointGuildTotal PT WITH (NOLOCK)
               INNER JOIN dbo.Guilds G WITH (NOLOCK) ON (PT.GuildID = G.GuildID)
               INNER JOIN dbo.GuildMembers GM WITH (NOLOCK) ON (G.GuildID = GM.GuildID AND GM.GuildMemberRoleCode = 0)
               INNER JOIN dbo.Characters C WITH (NOLOCK) ON (GM.CharacterID = C.CharacterID)
         WHERE PT.GuildID = @intGuildID
         ORDER BY PT.GuildWarMissionTypeCode;

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

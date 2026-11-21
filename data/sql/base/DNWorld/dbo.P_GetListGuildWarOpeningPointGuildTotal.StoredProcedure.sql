USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListGuildWarOpeningPointGuildTotal]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 10
author : ???
e-mail : wgkim@eyedentitygames
created date : 2011-03-10

description :
    ??? ?? ??? ?? ?? ??
result set :
    @intVersion = 1: Ranking/int, GuildID/int, GuildName/nvarchar(50), CharacterID/bigint, CharacterName/nvarchar(30), GuildLevel/tinyint, JoinMembersCount/int, OpeningPointSum/int
    @intVersion = 2: Ranking/int, GuildID/int, GuildName/nvarchar(50), CharacterID/bigint, CharacterName/nvarchar(30), GuildLevel/tinyint, JoinMembersCount/int, OpeningPointSum/int, GuildMaxMemberCount/int 
return value :
    0 = ??? ????.
    1 = ????? Commit? ? ?? ?????. ????? Rollback???.
    100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
    7 = ?? ??
	8 = ???? ????? ?? ? ???? ??     by 20110830.wgkim
	9~10 = ?? ?? ?? ??? Result Set ?? by ??? at 2011-12-06
**/

CREATE PROCEDURE [dbo].[P_GetListGuildWarOpeningPointGuildTotal]
      @insGuildWarScheduleID smallint --// ??? ??
    , @intGuildID int = NULL --// ?? ID
	, @intVersion int = 1 --// ??? SP version(?? ?? ?? ?? 2 ??)
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
	IF @intVersion = 1
	BEGIN
		/**_# ?? ID ? ?? ???? ?? ???? ?????. */
		IF @intGuildID IS NULL
		BEGIN

			-- ?? ?? ? ?? ?????. (1=??, 2=4?, 3=8?, 4=16?)
			SELECT @inyFinalTeamCount = CASE WHEN MAX(MatchTypeCode) = 4 THEN 16
											 WHEN MAX(MatchTypeCode) = 3 THEN 8
											 WHEN MAX(MatchTypeCode) = 2 THEN 4
											 WHEN MAX(MatchTypeCode) = 1 THEN 2 ELSE 0
										END
			  FROM dbo.GuildWarFinalSchedule WITH (NOLOCK)
			 WHERE GuildWarScheduleID = @insGuildWarScheduleID

			-- ?? ?? ?? ?????.
			SELECT TOP (@inyFinalTeamCount)
				   PT.Ranking           -- ??
				 , PT.GuildID           -- ??ID
				 , G.GuildName          -- ???
				 , C.CharacterID        -- ???ID
				 , C.CharacterName      -- ????
				 , G.GuildLevel         -- ????
				 , (SELECT COUNT(*) FROM dbo.GuildMembers P WITH (NOLOCK) WHERE P.GuildID = PT.GuildID) AS JoinMembersCount -- ???? ????
				 , PT.OpeningPointSum   -- ????
			  FROM dbo.GuildWarOpeningPointGuildTotal PT WITH (NOLOCK)
				   INNER JOIN dbo.Guilds G WITH (NOLOCK) ON (PT.GuildID = G.GuildID)
				   INNER JOIN dbo.GuildMembers GM WITH (NOLOCK) ON (G.GuildID = GM.GuildID AND GM.GuildMemberRoleCode = 0)
				   INNER JOIN dbo.Characters C WITH (NOLOCK) ON (GM.CharacterID = C.CharacterID)
			 WHERE PT.GuildWarMissionTypeCode = 0
			 ORDER BY PT.Ranking, G.GuildName;

		END
		/**_# ?? ID ? ?? ???? ?? ??? ??? ?????. */
		ELSE
		BEGIN

			SELECT PT.Ranking           -- ??
				 , PT.GuildID           -- ??ID
				 , G.GuildName          -- ???
				 , C.CharacterID        -- ???ID
				 , C.CharacterName      -- ????
				 , G.GuildLevel         -- ????
				 , (SELECT COUNT(*) FROM dbo.GuildMembers P WITH (NOLOCK) WHERE P.GuildID = PT.GuildID) AS JoinMembersCount -- ???? ????
				 , PT.OpeningPointSum   -- ????
			  FROM dbo.GuildWarOpeningPointGuildTotal PT WITH (NOLOCK)
				   INNER JOIN dbo.Guilds G WITH (NOLOCK) ON (PT.GuildID = G.GuildID)
				   INNER JOIN dbo.GuildMembers GM WITH (NOLOCK) ON (G.GuildID = GM.GuildID AND GM.GuildMemberRoleCode = 0)
				   INNER JOIN dbo.Characters C WITH (NOLOCK) ON (GM.CharacterID = C.CharacterID)
			 WHERE PT.GuildWarMissionTypeCode = 0
			   AND PT.GuildID = @intGuildID;

		END
	END
	ELSE IF @intVersion = 2
	BEGIN
		/**_# ?? ID ? ?? ???? ?? ???? ?????. */
		IF @intGuildID IS NULL
		BEGIN

			-- ?? ?? ? ?? ?????. (1=??, 2=4?, 3=8?, 4=16?)
			SELECT @inyFinalTeamCount = CASE WHEN MAX(MatchTypeCode) = 4 THEN 16
											 WHEN MAX(MatchTypeCode) = 3 THEN 8
											 WHEN MAX(MatchTypeCode) = 2 THEN 4
											 WHEN MAX(MatchTypeCode) = 1 THEN 2 ELSE 0
										END
			  FROM dbo.GuildWarFinalSchedule WITH (NOLOCK)
			 WHERE GuildWarScheduleID = @insGuildWarScheduleID

			-- ?? ?? ?? ?????.
			SELECT TOP (@inyFinalTeamCount)
				   PT.Ranking           -- ??
				 , PT.GuildID           -- ??ID
				 , G.GuildName          -- ???
				 , C.CharacterID        -- ???ID
				 , C.CharacterName      -- ????
				 , G.GuildLevel         -- ????
				 , (SELECT COUNT(*) FROM dbo.GuildMembers P WITH (NOLOCK) WHERE P.GuildID = PT.GuildID) AS JoinMembersCount -- ???? ????
				 , PT.OpeningPointSum   -- ????
				 , G.GuildMaxMemberCount-- ?? ?? ?? ??
			  FROM dbo.GuildWarOpeningPointGuildTotal PT WITH (NOLOCK)
				   INNER JOIN dbo.Guilds G WITH (NOLOCK) ON (PT.GuildID = G.GuildID)
				   INNER JOIN dbo.GuildMembers GM WITH (NOLOCK) ON (G.GuildID = GM.GuildID AND GM.GuildMemberRoleCode = 0)
				   INNER JOIN dbo.Characters C WITH (NOLOCK) ON (GM.CharacterID = C.CharacterID)
			 WHERE PT.GuildWarMissionTypeCode = 0
			 ORDER BY PT.Ranking, G.GuildName;

		END
		/**_# ?? ID ? ?? ???? ?? ??? ??? ?????. */
		ELSE
		BEGIN

			SELECT PT.Ranking           -- ??
				 , PT.GuildID           -- ??ID
				 , G.GuildName          -- ???
				 , C.CharacterID        -- ???ID
				 , C.CharacterName      -- ????
				 , G.GuildLevel         -- ????
				 , (SELECT COUNT(*) FROM dbo.GuildMembers P WITH (NOLOCK) WHERE P.GuildID = PT.GuildID) AS JoinMembersCount -- ???? ????
				 , PT.OpeningPointSum   -- ????
				 , G.GuildMaxMemberCount-- ?? ?? ?? ??
			  FROM dbo.GuildWarOpeningPointGuildTotal PT WITH (NOLOCK)
				   INNER JOIN dbo.Guilds G WITH (NOLOCK) ON (PT.GuildID = G.GuildID)
				   INNER JOIN dbo.GuildMembers GM WITH (NOLOCK) ON (G.GuildID = GM.GuildID AND GM.GuildMemberRoleCode = 0)
				   INNER JOIN dbo.Characters C WITH (NOLOCK) ON (GM.CharacterID = C.CharacterID)
			 WHERE PT.GuildWarMissionTypeCode = 0
			   AND PT.GuildID = @intGuildID;

		END
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

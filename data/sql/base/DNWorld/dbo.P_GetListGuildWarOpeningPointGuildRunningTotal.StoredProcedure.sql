USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListGuildWarOpeningPointGuildRunningTotal]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 6
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2011-09-06

description :
	??? ?? ??? ?? ?? ?? ??
result set : 
	@intVersion = 1 : Ranking/int, GuildID/int, GuildName/nvarchar(50), CharacterID/bigint, CharacterName/nvarchar(30), GuildLevel/tinyint, JoinMembersCount/int, OpeningPointSum/int
	@intVersion = 2 : Ranking/int, GuildID/int, GuildName/nvarchar(50), CharacterID/bigint, CharacterName/nvarchar(30), GuildLevel/tinyint, JoinMembersCount/int, OpeningPointSum/int, GuildMaxMemberCount/int
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
	3 = ?? ??
	4~6 = ?? ?? ?? ??? Result Set ?? by ??? at 2011-12-06
**/

CREATE PROCEDURE [dbo].[P_GetListGuildWarOpeningPointGuildRunningTotal]
	  @inyRanking tinyint = 16 --// ????? ??
	, @intGuildID int = NULL --// ?? ID (null ? ???? ?? ?? ??)
	, @intVersion int = 1 --// ??? SP version(?? ?? ?? ?? 2 ??)
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
	IF @intVersion = 1
	BEGIN
		IF @intGuildID IS NULL
		BEGIN

			-- ?? ?? ??
			SELECT TOP (@inyRanking)
				   RANK() OVER(ORDER BY PT.OpeningPointSum DESC) AS Ranking
				 , PT.GuildID           -- ??ID
				 , G.GuildName          -- ???
				 , C.CharacterID        -- ???ID
				 , C.CharacterName      -- ????
				 , G.GuildLevel         -- ????
				 , (SELECT COUNT(*) FROM dbo.GuildMembers GM WITH (NOLOCK) WHERE GM.GuildID = PT.GuildID) AS JoinMembersCount -- ????
				 , PT.OpeningPointSum   -- ????
			  FROM dbo.GuildWarOpeningPointGuildRunningTotal PT WITH (NOLOCK)
				   INNER JOIN dbo.Guilds G WITH (NOLOCK) ON (PT.GuildID = G.GuildID)
				   INNER JOIN dbo.GuildMembers GM WITH (NOLOCK) ON (G.GuildID = GM.GuildID AND GM.GuildMemberRoleCode = 0)
				   INNER JOIN dbo.Characters C WITH (NOLOCK) ON (GM.CharacterID = C.CharacterID);

		END
		ELSE
		BEGIN

			-- ?? ?? ??
			;WITH T1 AS (
				SELECT RANK() OVER(ORDER BY OpeningPointSum DESC) AS Ranking, GuildID, OpeningPointSum
				  FROM dbo.GuildWarOpeningPointGuildRunningTotal WITH (NOLOCK)
			)
			SELECT T1.Ranking
				 , T1.GuildID           -- ??ID
				 , G.GuildName          -- ???
				 , C.CharacterID        -- ???ID
				 , C.CharacterName      -- ????
				 , G.GuildLevel         -- ????
				 , (SELECT COUNT(*) FROM dbo.GuildMembers GM WITH (NOLOCK) WHERE GM.GuildID = T1.GuildID) AS JoinMembersCount -- ????
				 , T1.OpeningPointSum   -- ????
			  FROM T1
				   INNER JOIN dbo.Guilds G WITH (NOLOCK) ON (T1.GuildID = G.GuildID)
				   INNER JOIN dbo.GuildMembers GM WITH (NOLOCK) ON (G.GuildID = GM.GuildID AND GM.GuildMemberRoleCode = 0)
				   INNER JOIN dbo.Characters C WITH (NOLOCK) ON (GM.CharacterID = C.CharacterID)
			 WHERE T1.GuildID = @intGuildID;

		END
	END
	ELSE IF @intVersion = 2
	BEGIN
		IF @intGuildID IS NULL
		BEGIN

			-- ?? ?? ??
			SELECT TOP (@inyRanking)
				   RANK() OVER(ORDER BY PT.OpeningPointSum DESC) AS Ranking
				 , PT.GuildID           -- ??ID
				 , G.GuildName          -- ???
				 , C.CharacterID        -- ???ID
				 , C.CharacterName      -- ????
				 , G.GuildLevel         -- ????
				 , (SELECT COUNT(*) FROM dbo.GuildMembers GM WITH (NOLOCK) WHERE GM.GuildID = PT.GuildID) AS JoinMembersCount -- ????
				 , PT.OpeningPointSum   -- ????
				 , G.GuildMaxMemberCount-- ?? ?? ?? ??	
			  FROM dbo.GuildWarOpeningPointGuildRunningTotal PT WITH (NOLOCK)
				   INNER JOIN dbo.Guilds G WITH (NOLOCK) ON (PT.GuildID = G.GuildID)
				   INNER JOIN dbo.GuildMembers GM WITH (NOLOCK) ON (G.GuildID = GM.GuildID AND GM.GuildMemberRoleCode = 0)
				   INNER JOIN dbo.Characters C WITH (NOLOCK) ON (GM.CharacterID = C.CharacterID);

		END
		ELSE
		BEGIN

			-- ?? ?? ??
			;WITH T1 AS (
				SELECT RANK() OVER(ORDER BY OpeningPointSum DESC) AS Ranking, GuildID, OpeningPointSum
				  FROM dbo.GuildWarOpeningPointGuildRunningTotal WITH (NOLOCK)
			)
			SELECT T1.Ranking
				 , T1.GuildID           -- ??ID
				 , G.GuildName          -- ???
				 , C.CharacterID        -- ???ID
				 , C.CharacterName      -- ????
				 , G.GuildLevel         -- ????
				 , (SELECT COUNT(*) FROM dbo.GuildMembers GM WITH (NOLOCK) WHERE GM.GuildID = T1.GuildID) AS JoinMembersCount -- ????
				 , T1.OpeningPointSum   -- ????
				 , G.GuildMaxMemberCount-- ?? ?? ?? ??	
			  FROM T1
				   INNER JOIN dbo.Guilds G WITH (NOLOCK) ON (T1.GuildID = G.GuildID)
				   INNER JOIN dbo.GuildMembers GM WITH (NOLOCK) ON (G.GuildID = GM.GuildID AND GM.GuildMemberRoleCode = 0)
				   INNER JOIN dbo.Characters C WITH (NOLOCK) ON (GM.CharacterID = C.CharacterID)
			 WHERE T1.GuildID = @intGuildID;

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

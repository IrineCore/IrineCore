USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListGuildWarOpeningPointTotal]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 8
author : ???
e-mail : wgkim@eyedentitygames
created date : 2011-03-11

description :
    ??? ?? ??? ??? ?? ??
result set :
    Ranking	int
    GuildWarMissionTypeCode	tinyint
    CharacterID	bigint
    CharacterName	nvarchar(30)
    GuildID	int
    GuildName	nvarchar(50)
    OpeningPointSum	int
return value :
    0 = ??? ????.
    1 = ????? Commit? ? ?? ?????. ????? Rollback???.
    100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
    8 = ?? ??
**/

CREATE PROCEDURE [dbo].[P_GetListGuildWarOpeningPointTotal]
    @inbCharacterID bigint = NULL --// ??? ID
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

	/**_# ??? ID ? ?? ???? ? ??? 1?? ?????. */
    IF @inbCharacterID IS NULL
    BEGIN

        SELECT PT.Ranking           -- ??
             , PT.GuildWarMissionTypeCode -- 0=??, 1=??, 2=??, 3=??, 4=??, 5=??
             , C.CharacterID        -- ???ID
             , C.CharacterName      -- ????
             , PT.GuildID           -- ??ID
             , G.GuildName          -- ???
             , PT.OpeningPointSum   -- ????
			 , ROW_NUMBER() OVER(PARTITION BY PT.GuildWarMissionTypeCode ORDER BY C.CharacterName) AS SortNumber
		  INTO #Temp_Result
          FROM dbo.GuildWarOpeningPointTotal PT WITH (NOLOCK)
               INNER JOIN dbo.Characters C WITH (NOLOCK) ON (PT.CharacterID = C.CharacterID)
               INNER JOIN dbo.Guilds G WITH (NOLOCK) ON (PT.GuildID = G.GuildID)
         WHERE PT.Ranking = 1
         ORDER BY PT.GuildWarMissionTypeCode;

		/**_## ?? 1?? ???? ???? ?????? ???? ?? ?? ???? 1?? ?????. */
		SELECT Ranking
             , GuildWarMissionTypeCode
             , CharacterID
             , CharacterName
             , GuildID
             , GuildName
             , OpeningPointSum
		  FROM #Temp_Result
		 WHERE SortNumber = 1;

		DROP TABLE #Temp_Result;

    END
	/**_# ??? ID ? ?? ???? ?? ???? ??? ??? ?????. */
    ELSE
    BEGIN

        SELECT PT.Ranking           -- ??
             , PT.GuildWarMissionTypeCode -- 0=??, 1=??, 2=??, 3=??, 4=??, 5=??
             , C.CharacterID        -- ???ID
             , C.CharacterName      -- ????
             , PT.GuildID           -- ??ID
             , G.GuildName          -- ???
             , PT.OpeningPointSum   -- ????
          FROM dbo.GuildWarOpeningPointTotal PT WITH (NOLOCK)
               INNER JOIN dbo.Characters C WITH (NOLOCK) ON (PT.CharacterID = C.CharacterID)
               INNER JOIN dbo.Guilds G WITH (NOLOCK) ON (PT.GuildID = G.GuildID)
         WHERE PT.CharacterID = @inbCharacterID
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

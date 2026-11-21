USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetGuildWarOpeningPointRecord]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 9
author : ???
e-mail : wgkim@eyedentitygames
created date : 2011-03-08

description :
    ??? ?? ??? ??
result set :
    @intCharacterOpeningPoint int OUTPUT
    @intGuildOpeningPoint int OUTPUT
    @intBlueTeamPoint int OUTPUT
    @intRedTeamPoint int OUTPUT
return value :
    0 = ??? ????.
    1 = ????? Commit? ? ?? ?????. ????? Rollback???.
    100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
    9 = ?? ??
**/

CREATE PROCEDURE [dbo].[P_GetGuildWarOpeningPointRecord]
      @chrType char(1) --// C=??? ?? ??? ??, G=?? ????? ??, T=? ????? ??
    , @inbID bigint = null --// @chrType=C ?? ??? ID, @chrType=G ?? ?? ID, @chrType=T ?? null
    , @intCharacterOpeningPoint int OUTPUT --// ?? ?? ???
    , @intGuildOpeningPoint int OUTPUT --// ?? ?? ???
    , @intBlueTeamPoint int OUTPUT --// ?? ?? ??? (TeamColorCode = 1)
    , @intRedTeamPoint int OUTPUT --// ?? ?? ??? (TeamColorCode = 2)
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

    /**_# @bitType=C ?? ??? ?? ???? ?????. */
    IF @chrType = 'C'
    BEGIN

	    SELECT @intCharacterOpeningPoint = SUM(OpeningPoint)
             , @intGuildOpeningPoint = 0
             , @intBlueTeamPoint = 0
             , @intRedTeamPoint = 0
	    FROM dbo.VIX_GuildWarOpeningPointForCharacter WITH (NOEXPAND, NOLOCK)
	    WHERE CharacterID = @inbID;

    END
    /**_# @bitType=G ?? ?? ?? ???? ?????. */
    ELSE IF @chrType = 'G'
    BEGIN

	    SELECT @intCharacterOpeningPoint = 0
             , @intGuildOpeningPoint = SUM(OpeningPoint)
             , @intBlueTeamPoint = 0
             , @intRedTeamPoint = 0
	    FROM dbo.VIX_GuildWarOpeningPointForGuild WITH (NOEXPAND, NOLOCK)
	    WHERE GuildID = CONVERT(INT, @inbID);

    END
    /**_# @bitType=T ?? ? ?? ???? ?????. */
    ELSE IF @chrType = 'T'
    BEGIN

        SET @intBlueTeamPoint = 0;
        SET @intRedTeamPoint = 0;

	    SELECT @intCharacterOpeningPoint = 0
             , @intGuildOpeningPoint = 0
             , @intBlueTeamPoint = @intBlueTeamPoint + CASE WHEN TeamColorCode = 1 THEN OpeningPoint ELSE 0 END
             , @intRedTeamPoint = @intRedTeamPoint + CASE WHEN TeamColorCode = 2 THEN OpeningPoint ELSE 0 END
          FROM dbo.VIX_GuildWarOpeningPointForTeam WITH (NOEXPAND, NOLOCK);

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

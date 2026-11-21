USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModGuildByGuildWarApply]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 6
author : ???
e-mail : wgkim@eyedentitygames
created date : 2011-01-06

description :
    ??? ?? ?? / (??) ??? ?? ??? ?? ? ?? ??
result set : 
	TeamColorCode   tinyint
return value :
    0 = ??? ????.
    1 = ????? Commit? ? ?? ?????. ????? Rollback???.
    100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
    6 = ?? ??
**/

CREATE PROCEDURE [dbo].[P_ModGuildByGuildWarApply]
	  @insGuildWarScheduleID smallint --// ??? ??
	, @intGuildID int --// ??ID
    , @bitInputType tinyint = 0 --// ??? ???? (0=????, 1=?????)
    , @inyInputTeamColorCode tinyint = NULL --// 1=??,2=?? (????? ? Color) (????(@bitInputType=0)? ???? ?? ??)
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
		, @intMemberCount1 int
		, @intMemberCount2 int
		, @inyTeamColorCode tinyint;

SET @intMemberCount1 = 0;
SET @intMemberCount2 = 0;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

    /**_# ???? ???? ?? */
    IF @bitInputType = 0
    BEGIN

	    /**_## ??/?? ??? ???? ??? ?? ?? Color? ?????. */
	    SELECT @intMemberCount1 = @intMemberCount1 + CASE WHEN TeamColorCode = 1 THEN MemberCount ELSE 0 END
		     , @intMemberCount2 = @intMemberCount2 + CASE WHEN TeamColorCode = 2 THEN MemberCount ELSE 0 END
	    FROM dbo.VIX_GuildMemberCountByTeamColor WITH (NOEXPAND, NOLOCK)
	    WHERE GuildWarScheduleID = @insGuildWarScheduleID;
    	
	    IF (@intMemberCount1 = 0) or (@intMemberCount1 <= @intMemberCount2)
		    SET @inyTeamColorCode = 1;
	    ELSE
		    SET @inyTeamColorCode = 2;
    	
	    /**_## ?? ???? ??? ??? ? Color? ?????. */
	    UPDATE dbo.Guilds
	    SET TeamColorCode		= @inyTeamColorCode
	      , GuildWarScheduleID	= @insGuildWarScheduleID
	    WHERE GuildID = @intGuildID;
    	
        /**_## ?? ?? ? Color ? ?????. */
	    SELECT @inyTeamColorCode;

    END
    /**_# ????? ???? ?? */
    ELSE IF @bitInputType = 1
    BEGIN

	    /**_## ? Color ? NULL ? ???? ? Color ? ?????. */
        IF @inyInputTeamColorCode IS NOT NULL
        BEGIN

            UPDATE dbo.Guilds
               SET TeamColorCode = @inyInputTeamColorCode
             WHERE GuildID = @intGuildID;

            UPDATE dbo.GuildWarOpeningPointRecord
               SET TeamColorCode = @inyInputTeamColorCode
             WHERE GuildID = @intGuildID;

        END
	    /**_## ? Color ? NULL ??? ? Color ? ???? ?? ??? ??? 0 ?? ???? ???. (??? ?? ??) */
        ELSE
        BEGIN

            UPDATE dbo.Guilds
               SET TeamColorCode = NULL
	             , GuildWarScheduleID = NULL
             WHERE GuildID = @intGuildID
               AND GuildWarScheduleID = @insGuildWarScheduleID;

            UPDATE dbo.GuildWarOpeningPointRecord
               SET OpeningPoint = 0
             WHERE GuildID = @intGuildID;

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

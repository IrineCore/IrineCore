USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddGuildWarPopularityVote]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : wgkim@eyedentitygames
created date : 2011-03-03

description :
    ??? ?? ?? ??
return value :
    0 = ??? ????.
    1 = ????? Commit? ? ?? ?????. ????? Rollback???.
    100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
    103299 = ?? ??? ?????.
history :
    4 = ?? ??
**/

CREATE PROCEDURE [dbo].[P_AddGuildWarPopularityVote]
	  @inbCharacterID bigint --// ??? ID
	, @intGuildID int --// ?? ?? ID
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

    /**_# ?? ??? ?????. */
    IF EXISTS (SELECT * FROM dbo.GuildWarPopularityVote WITH (NOLOCK) WHERE CharacterID = @inbCharacterID)
    BEGIN
        /***_# ?? ??? ??? ??? ??? ??????. */
        SET @intReturnValue = 103299;
        RETURN @intReturnValue;
    END
    ELSE
    BEGIN
        /***_# ?? ??? ?? ???? INSERT ???. */
        INSERT dbo.GuildWarPopularityVote (CharacterID, GuildID, RegistrationDate)
	    VALUES (@inbCharacterID, @intGuildID, GETDATE());
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

USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DelGuildJoinCharacters]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2011-11-21
description : ?? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103317 = ?? ???? ??? ????.
**/

CREATE PROCEDURE [dbo].[P_DelGuildJoinCharacters]
	  @intGuildID int --// ?? ??? ?? ID
	, @inbCharacterID bigint --//  ?? ??? CharacterID
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
	/**_# [GuildJoinCharacters] ??? ?? ??? Delete???.*/
	DELETE dbo.GuildJoinCharacters 
	FROM dbo.GuildJoinCharacters WITH (INDEX(PK_GuildJoinCharacters), FORCESEEK)
	WHERE GuildID = @intGuildID AND CharacterID = @inbCharacterID
	OPTION (KEEPFIXED PLAN);
	
	/**_## ?? ?? ?? ????? ???? ??? 103317? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103317;
		GOTO ErrorHandler;
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

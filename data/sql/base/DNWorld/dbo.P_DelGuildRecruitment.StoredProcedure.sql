USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DelGuildRecruitment]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 9
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2011-11-18
description : ??? ?? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history
1~2 = ???? by ??? 2011-11-18
8 = ??? ???? ?? ??  by ??? 2012-05-10
9 = GuildRecruitments ? GuildRecruitmentOptions ??? ??     by ??? at 2013-06-25
**/

CREATE PROCEDURE [dbo].[P_DelGuildRecruitment]
	  @intGuildID int --// ??ID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;

DECLARE @dt2RegisterDate datetime2(0);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SET @dt2RegisterDate = GETDATE();
	
	BEGIN TRANSACTION;
	
	/**_# [GuildRecruitment]? Delete???.*/
	DELETE dbo.GuildRecruitments 
	FROM dbo.GuildRecruitments WITH (INDEX(PK_GuildRecruitments), FORCESEEK)
	WHERE GuildID = @intGuildID
	OPTION (KEEPFIXED PLAN);

	/**_# [GuildJoinCharacters]? Delete???. (?? ??? ??? ?????.)*/
	DELETE dbo.GuildJoinCharacters
	FROM dbo.GuildJoinCharacters WITH (INDEX(PK_GuildJoinCharacters), FORCESEEK)
	WHERE GuildID = @intGuildID
	OPTION (KEEPFIXED PLAN);

	COMMIT TRANSACTION;

	/**_# ?? ??? ?????.*/
	INSERT dbo.GuildHistory (GuildID, GuildHistoryCode, CharacterID, CharacterLevel, RegisterDate)
	VALUES (@intGuildID, 13, NULL, NULL, @dt2RegisterDate)
	OPTION (KEEPFIXED PLAN);
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

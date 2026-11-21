USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModGuildName]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-10-20
description : ????? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103234 = ?? ???? ?? ?????.
103238 = ??? ???? ????.
history :
2 = (50729) ??? ?? ???? ?? ?? by ??? at 2012-01-30 
3 = ?? ?? ???? ?? ? ?? ?? at 2013-06-25
**/

CREATE PROCEDURE [dbo].[P_ModGuildName]
	  @intGuildID int --// ?? ID
	, @nvcGuildName nvarchar(50) --// ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @nvcOldGuildName nvarchar(50);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# [Guilds] ???? UPDATE ???.*/
	UPDATE dbo.Guilds
	SET @nvcOldGuildName = GuildName
		, GuildName = @nvcGuildName
	WHERE GuildID = @intGuildID;

	/**_## ??? ???? ??? 103238? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103238;
		GOTO ErrorHandler;
	END

	/**_# [GuildHistory] ???? INSERT ???.*/
	INSERT dbo.GuildHistory (GuildID, GuildHistoryCode, RegisterDate, GuildNameBefore, GuildNameAfter)
	VALUES (@intGuildID, 11, GETDATE(), @nvcOldGuildName, @nvcGuildName);

	/**_# [GuildRecruitments] ???? ?? ??? UPDATE???.*/
	UPDATE dbo.GuildRecruitments
	SET GuildName = @nvcGuildName
	WHERE GuildID = @intGuildID;

END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 2601 AND ERROR_MESSAGE() LIKE N'%IX_UNF_Guilds_GuildName%'
		SET @intReturnValue = 103234;

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

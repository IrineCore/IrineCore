USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_SetGuildMemberRole]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-09-20
description : ??? ?? ??? ????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103238 = ??? ???? ????.
**/

CREATE PROCEDURE [dbo].[P_SetGuildMemberRole]
	  @intGuildID int --// ?? ID
	, @inyGuildMemberRoleCode tinyint --// 1=?? ??, 2=?? ???, 3=?? ???, 4=?? ???
	, @intDailyTakeItemLimit int --// ?? ?? ?? ??? ?? ?? (??? ?)
	, @intDailyTakeCoinLimit int --// ?? ?? ?? ???? ?? ?? (??? ?)
	, @intPrivilege int --// ?? & ?? ?? ??
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
	/**_# [GuildMemberRoles] ???? UPSERT???.*/
	/**_## ??? ???? ??? 103238? ?????.*/
	UPDATE dbo.GuildMemberRoles
	SET Privilege = @intPrivilege
		, DailyTakeItemLimit = @intDailyTakeItemLimit
		, DailyTakeCoinLimit = @intDailyTakeCoinLimit
	WHERE GuildID = @intGuildID AND GuildMemberRoleCode = @inyGuildMemberRoleCode;

	IF @@ROWCOUNT = 0
		INSERT dbo.GuildMemberRoles (GuildID, GuildMemberRoleCode, Privilege, DailyTakeItemLimit, DailyTakeCoinLimit)
		VALUES (@intGuildID, @inyGuildMemberRoleCode, @intPrivilege, @intDailyTakeItemLimit, @intDailyTakeCoinLimit);
END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 547 AND ERROR_MESSAGE() LIKE N'%FK_Guilds_GuildMemberRoles_GuildID%'
		SET @intReturnValue = 103238;

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

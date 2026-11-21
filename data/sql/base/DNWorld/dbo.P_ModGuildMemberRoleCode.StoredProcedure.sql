USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModGuildMemberRoleCode]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-09-27
description : ??? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103241 = ???? ???? ?? ??????.
103243 = @inyGuildMemberRoleCode ?? ?? ????
history :
3 = ?? ??? GuildMemberRoleCode ???? ?? ?? ?? at 2010-10-19
**/

CREATE PROCEDURE [dbo].[P_ModGuildMemberRoleCode]
	  @intGuildID int --// ?? ID
	, @inbCharacterID bigint --// ??? ID
	, @inyCharacterLevel tinyint --// ??? ??
	, @inyGuildMemberRoleCode tinyint --// 1=?? ??, 2=?? ???, 3=?? ???, 4=?? ???
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
	/**_# ???? ?? ???? ??? ? ????. input param. ?? ??? ??? 103243? ?????.*/
	IF @inyGuildMemberRoleCode = 0
	BEGIN
		SET @intReturnValue = 103243;
		GOTO ErrorHandler;
	END

	/**_# [GuildMembers] ???? UPDATE???.*/
	UPDATE dbo.GuildMembers
	SET GuildMemberRoleCode = @inyGuildMemberRoleCode
	WHERE GuildID = @intGuildID AND CharacterID = @inbCharacterID AND GuildMemberRoleCode > 0;

	/**## ??? ???? ??? 103241? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103241;
		GOTO ErrorHandler;
	END

	/**_# ?? ??? ?????.*/
	INSERT dbo.GuildHistory (GuildID, GuildHistoryCode, CharacterID, CharacterLevel, GuildMemberRoleCode, RegisterDate)
	VALUES (@intGuildID, 6, @inbCharacterID, @inyCharacterLevel, @inyGuildMemberRoleCode, GETDATE());
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

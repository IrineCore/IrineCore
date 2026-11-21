USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ChangeGuildMaster]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-09-28
description : ?? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103245 = ? ?? ??? ??? ??? ???? ????.
103246 = ?? ?? ??? ??? ??? ???? ????.
**/

CREATE PROCEDURE [dbo].[P_ChangeGuildMaster]
	  @intGuildID int --// ?? ID
	, @inbNewGuildMasterCharacterID bigint --// ??? ?? ???? ??? ID
	, @inbOldGuildMasterCharacterID bigint --// ?? ?? ???? ??? ID
	, @inyOldGuildMasterGuildMemberRoleCode tinyint --// ?? ?? ???? ? ??. 1=?? ??, 2=?? ???, 3=?? ???, 4=?? ???
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dt2Now datetime2(0) = GETDATE();

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	BEGIN TRANSACTION;

	/**_# ? ?? ???? ??? ?? ???? UPDATE ???.*/
	UPDATE dbo.GuildMembers
	SET GuildMemberRoleCode = 0
	WHERE GuildID = @intGuildID AND CharacterID = @inbNewGuildMasterCharacterID AND GuildMemberRoleCode > 0;

	/**_## UPDATE? ???? ??? 103245? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103245;
		GOTO ErrorHandler;
	END

	/**_# ?? ?? ???? ??? ?????.*/
	UPDATE dbo.GuildMembers
	SET GuildMemberRoleCode = @inyOldGuildMasterGuildMemberRoleCode
	WHERE GuildID = @intGuildID AND CharacterID = @inbOldGuildMasterCharacterID AND GuildMemberRoleCode = 0;

	/**_## UPDATE? ???? ??? 103246? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103246;
		GOTO ErrorHandler;
	END

	COMMIT TRANSACTION;

	/**_# ?? ??? ?????.*/
	INSERT dbo.GuildHistory (GuildID, GuildHistoryCode, CharacterID, CharacterLevel, RegisterDate, GuildMemberRoleCode)
	SELECT @intGuildID, 6, @inbNewGuildMasterCharacterID, CharacterLevel, @dt2Now, 0
	FROM dbo.CharacterStatus WITH (NOLOCK)
	WHERE CharacterID = @inbNewGuildMasterCharacterID
    UNION ALL
	SELECT @intGuildID, 6, @inbOldGuildMasterCharacterID, CharacterLevel, @dt2Now, @inyOldGuildMasterGuildMemberRoleCode
	FROM dbo.CharacterStatus WITH (NOLOCK)
	WHERE CharacterID = @inbOldGuildMasterCharacterID;
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

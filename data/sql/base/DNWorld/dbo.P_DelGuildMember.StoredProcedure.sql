USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DelGuildMember]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 7
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-09-27
description : ???? ?????. (?? ?? ??)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103241 = ???? ???? ?? ??????.
history :
3 = ?? ?? ???? ????? ?? by ??? at 2011-12-23
7 = ???? ?? ?? by ??? at 2013-03-20
**/

CREATE PROCEDURE [dbo].[P_DelGuildMember]
	  @intGuildID int --// ?? ID
	, @inbCharacterID bigint --// ??? ID
	, @inyCharacterLevel tinyint --// ??? ??
	, @bitBanishFlag bit --// ?? ?? ??. 0=?? ??, 1=?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	  , @vchGuildItemMailIDs varchar(max)
	  , @intTotalMailCount int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# [GuildMembers] ????? DELETE???.*/
	DELETE dbo.GuildMembers
	WHERE GuildID = @intGuildID AND CharacterID = @inbCharacterID AND GuildMemberRoleCode > 0;

	/**_## ??? ???? ??? 103241? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103241;
		GOTO ErrorHandler;
	END
	
	/**_# ?? ?? ???(???)? DELETE???.*/
	EXEC dbo.P_DelEffectItemForGuild @inbCharacterID = @inbCharacterID, @intGuildID = @intGuildID;

	/**_# ????? ?? ??? ??? DELETE ???.*/
	IF EXISTS (SELECT TOP 1 MailID FROM dbo.Mails WITH (NOLOCK) WHERE ReceiverCharacterID = @inbCharacterID AND MailTypeCode = 19 AND DeleteFlag = 0)
	BEGIN
		SET @vchGuildItemMailIDs = '';

		SELECT @vchGuildItemMailIDs = @vchGuildItemMailIDs + CAST(MailID AS varchar(10)) + ','
		FROM dbo.Mails WITH (NOLOCK)
		WHERE ReceiverCharacterID = @inbCharacterID AND MailTypeCode = 19 AND DeleteFlag = 0;

		SET @vchGuildItemMailIDs = LEFT(@vchGuildItemMailIDs, LEN(@vchGuildItemMailIDs) - 1);

		EXEC @intReturnValue = dbo.P_DelMail
			  @inbReceiverCharacterID = @inbCharacterID
			, @vchMailIDs = @vchGuildItemMailIDs
			, @intTotalMailCount = @intTotalMailCount OUTPUT;
	END

	/**_# ?? ?? ??? ??? DELETE ???.*/
	DELETE dbo.WeeklyGuildContributionPoint WHERE CharacterID = @inbCharacterID;

	/**_# ?? ??? ?????.*/
	INSERT dbo.GuildHistory (GuildID, GuildHistoryCode, CharacterID, CharacterLevel, RegisterDate)
	VALUES (@intGuildID, CASE @bitBanishFlag WHEN 0 THEN 4 ELSE 5 END, @inbCharacterID, @inyCharacterLevel, GETDATE());
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

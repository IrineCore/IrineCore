USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModGuildMemberSelfIntroduction]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-09-27
description : ??? ????? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103242 = ???? ????.
**/

CREATE PROCEDURE [dbo].[P_ModGuildMemberSelfIntroduction]
	  @intGuildID int --// ?? ID
	, @inbCharacterID bigint --// ??? ID
	, @nvcSelfIntroduction nvarchar(300) --// ?? ??
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
	/**_# [GuildMembers] ???? UPDATE???.*/
	UPDATE dbo.GuildMembers
	SET SelfIntroduction = @nvcSelfIntroduction
	WHERE GuildID = @intGuildID AND CharacterID = @inbCharacterID;

	/**## ??? ???? ??? 103242? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103242;
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

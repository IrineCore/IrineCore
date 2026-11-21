USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_EnterPrivateChannel]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 6
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-07-30
description : ????? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103329 = ?? ????? ???? ????.
103345 = ?? ????? ??? ?????.
103347 = ???? ????? ?? ????.
**/

CREATE PROCEDURE [dbo].[P_EnterPrivateChannel]
	  @inbCharacterID bigint --// ???ID
	, @inbPrivateChannelID bigint --// ????ID
	, @nvcCharacterName nvarchar(50) --// ?????
	, @intServerID int = NULL --// ??ID
	, @intMaxMemberCount int = 100 --// ?? ?? ???
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @intChannelMemberCount int, @intChannelMaxMemberCount int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	IF @intMaxMemberCount <= 0
		SET @intChannelMaxMemberCount = 100;
	ELSE
		SET @intChannelMaxMemberCount = @intMaxMemberCount;

	SELECT @intChannelMemberCount = COUNT(*) FROM dbo.PrivateChannelMembers WITH (NOLOCK, INDEX(PK_PrivateChannelMembers)) WHERE PrivateChannelID = @inbPrivateChannelID;
	
	IF @intChannelMemberCount >= @intChannelMaxMemberCount 
	BEGIN
		SET @intReturnValue = 103347;
		GOTO ErrorHandler;
	END

	/**_# [PrivateChannelMembers] ???? INSERT???.*/
	INSERT dbo.PrivateChannelMembers (CharacterID, PrivateChannelID, CharacterName, ServerID)
	VALUES (@inbCharacterID, @inbPrivateChannelID, @nvcCharacterName, @intServerID);

END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 547 AND ERROR_MESSAGE() LIKE N'%FK_%'
		SET @intReturnValue = 103329;	

	ELSE IF ERROR_NUMBER() = 2627 AND ERROR_MESSAGE() LIKE N'%PK_%'
		SET @intReturnValue = 103345;	

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

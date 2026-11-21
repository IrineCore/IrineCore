USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_JoinPrivateChannel]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 10
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-07-11
description : ????? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103329 = ?? ????? ???? ????.
103347 = ???? ????? ?? ????.
**/

CREATE PROCEDURE [dbo].[P_JoinPrivateChannel]
	  @inbCharacterID bigint --// ???ID	
	, @nvcCharacterName nvarchar(30) --// ?????	
	, @inbPrivateChannelID bigint --// ????ID
	, @intServerID int = NULL--// ??ID
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

	IF NOT EXISTS (SELECT * FROM dbo.PrivateChannels WITH (NOLOCK) WHERE PrivateChannelID = @inbPrivateChannelID)
	BEGIN
		SET @intReturnValue = 103329;
		GOTO ErrorHandler;		
	END
	
	BEGIN TRANSACTION
	
		/**_# [Characters] ???? UPDATE???.*/	
		UPDATE dbo.Characters
		SET PrivateChannelID = @inbPrivateChannelID
		WHERE CharacterID = @inbCharacterID;

		SELECT @intChannelMemberCount = COUNT(*) FROM dbo.PrivateChannelMembers WITH (NOLOCK, INDEX(PK_PrivateChannelMembers)) WHERE PrivateChannelID = @inbPrivateChannelID;
		
		IF @intChannelMemberCount >= @intChannelMaxMemberCount 
		BEGIN
			SET @intReturnValue = 103347;
			GOTO ErrorHandler;
		END

		/**_# [PrivateChannelMembers] ???? INSERT???.*/	
		INSERT dbo.PrivateChannelMembers (CharacterID, PrivateChannelID, CharacterName, ServerID)
		VALUES (@inbCharacterID, @inbPrivateChannelID, @nvcCharacterName, @intServerID);
	
	COMMIT TRANSACTION		
	
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

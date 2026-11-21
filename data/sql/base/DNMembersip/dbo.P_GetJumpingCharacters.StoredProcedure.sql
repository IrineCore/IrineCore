USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetJumpingCharacters]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetJumpingCharacters]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetJumpingCharacters] AS' 
END
GO
/**
version : 2
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2012-05-30

description :
	????? ??
result set :
	WorldID / int
	CharacterID / bigint
	CharacterName / nvarchar(30)
	AccountID / int
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
	103337 = ?? ???? ???? ?? ??? ??? ?? ???? ????.
history :
	1 = ????
	2 = result set ?? (AccountID)     by ??? at 2012-05-30
**/

ALTER PROCEDURE [dbo].[P_GetJumpingCharacters]
	  @intJumpSequence int --// ?? ??
	, @inbCharacterID bigint = NULL --// ???ID
	, @nvcCharacterName nvarchar(30) = NULL --// ????
	, @intWorldID int = NULL --// ??ID (NULL ? ?? ?? ?? ??)
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

	-- 1? ?? ???
	IF @intJumpSequence = 1 AND OBJECT_ID('JumpingCharacters') IS NOT NULL
	BEGIN
		IF @inbCharacterID IS NOT NULL
		BEGIN
			SELECT B.WorldID, B.CharacterID, B.CharacterName, B.AccountID
			  FROM dbo.JumpingCharacters A WITH (NOLOCK)
				   INNER JOIN dbo.Characters B WITH (NOLOCK) ON (A.CharacterID = B.CharacterID)
			 WHERE (A.CharacterID = @inbCharacterID)
			   AND (@intWorldID IS NULL OR B.WorldID = @intWorldID);
		END
		ELSE IF @nvcCharacterName IS NOT NULL
		BEGIN
			SELECT B.WorldID, B.CharacterID, B.CharacterName, B.AccountID
			  FROM dbo.JumpingCharacters A WITH (NOLOCK)
				   INNER JOIN dbo.Characters B WITH (NOLOCK) ON (A.CharacterID = B.CharacterID)
			 WHERE (B.CharacterName = @nvcCharacterName)
			   AND (@intWorldID IS NULL OR B.WorldID = @intWorldID);
		END
		ELSE
		BEGIN
			SELECT B.WorldID, B.CharacterID, B.CharacterName, B.AccountID
			  FROM dbo.JumpingCharacters A WITH (NOLOCK)
				   INNER JOIN dbo.Characters B WITH (NOLOCK) ON (A.CharacterID = B.CharacterID)
			 WHERE (@intWorldID IS NULL OR B.WorldID = @intWorldID);
		END
	END
	-- 2? ?? ???
	ELSE IF @intJumpSequence = 2 AND OBJECT_ID('ReturnJumpingCharacters') IS NOT NULL
	BEGIN
		IF @inbCharacterID IS NOT NULL
		BEGIN
			SELECT B.WorldID, B.CharacterID, B.CharacterName, B.AccountID
			  FROM dbo.ReturnJumpingCharacters A WITH (NOLOCK)
				   INNER JOIN dbo.Characters B WITH (NOLOCK) ON (A.CharacterID = B.CharacterID)
			 WHERE (A.CharacterID = @inbCharacterID)
			   AND (@intWorldID IS NULL OR B.WorldID = @intWorldID);
		END
		ELSE IF @nvcCharacterName IS NOT NULL
		BEGIN
			SELECT B.WorldID, B.CharacterID, B.CharacterName, B.AccountID
			  FROM dbo.ReturnJumpingCharacters A WITH (NOLOCK)
				   INNER JOIN dbo.Characters B WITH (NOLOCK) ON (A.CharacterID = B.CharacterID)
			 WHERE (B.CharacterName = @nvcCharacterName)
			   AND (@intWorldID IS NULL OR B.WorldID = @intWorldID);
		END
		ELSE
		BEGIN
			SELECT B.WorldID, B.CharacterID, B.CharacterName, B.AccountID
			  FROM dbo.ReturnJumpingCharacters A WITH (NOLOCK)
				   INNER JOIN dbo.Characters B WITH (NOLOCK) ON (A.CharacterID = B.CharacterID)
			 WHERE (@intWorldID IS NULL OR B.WorldID = @intWorldID);
		END
	END
	-- 3? ?? ???
	ELSE IF @intJumpSequence = 3 AND OBJECT_ID('WarpCharacters') IS NOT NULL
	BEGIN
		IF @inbCharacterID IS NOT NULL
		BEGIN
			SELECT B.WorldID, B.CharacterID, B.CharacterName, B.AccountID
			  FROM dbo.WarpCharacters A WITH (NOLOCK)
				   INNER JOIN dbo.Characters B WITH (NOLOCK) ON (A.CharacterID = B.CharacterID)
			 WHERE (A.CharacterID = @inbCharacterID)
			   AND (@intWorldID IS NULL OR B.WorldID = @intWorldID);
		END
		ELSE IF @nvcCharacterName IS NOT NULL
		BEGIN
			SELECT B.WorldID, B.CharacterID, B.CharacterName, B.AccountID
			  FROM dbo.WarpCharacters A WITH (NOLOCK)
				   INNER JOIN dbo.Characters B WITH (NOLOCK) ON (A.CharacterID = B.CharacterID)
			 WHERE (B.CharacterName = @nvcCharacterName)
			   AND (@intWorldID IS NULL OR B.WorldID = @intWorldID);
		END
		ELSE
		BEGIN
			SELECT B.WorldID, B.CharacterID, B.CharacterName, B.AccountID
			  FROM dbo.WarpCharacters A WITH (NOLOCK)
				   INNER JOIN dbo.Characters B WITH (NOLOCK) ON (A.CharacterID = B.CharacterID)
			 WHERE (@intWorldID IS NULL OR B.WorldID = @intWorldID);
		END
	END

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103337; -- ?? ???? ???? ?? ??? ??? ?? ???? ????.
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

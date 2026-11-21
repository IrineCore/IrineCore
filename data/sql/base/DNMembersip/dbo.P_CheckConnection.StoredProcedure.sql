USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_CheckConnection]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_CheckConnection]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_CheckConnection] AS' 
END
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-05-03
description : ?? ?? ??? ?????. (input parameter 4? ? 1?? ???? ?????. ???? NULL? ?????.)
result set : WorldID, CurServerID, CertifyingStep
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

ALTER PROCEDURE [dbo].[P_CheckConnection]
	  @intAccountID int = NULL --// Account ID
	, @nvcAccountName nvarchar(50) = NULL --// Account Name
	, @inbCharacterID bigint = NULL --// Character ID
	, @nvcCharacterName nvarchar(30) = NULL --// Character Name
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
	IF @intAccountID IS NULL
	BEGIN
		IF @nvcAccountName IS NOT NULL
			SELECT @intAccountID = AccountID FROM dbo.Accounts WITH (NOLOCK) WHERE AccountName = @nvcAccountName;

		ELSE IF @inbCharacterID IS NOT NULL
			SELECT @intAccountID = AccountID FROM dbo.Characters WITH (NOLOCK) WHERE CharacterID = @inbCharacterID;

		ELSE IF @nvcCharacterName IS NOT NULL
			SELECT @intAccountID = AccountID FROM dbo.Characters WITH (NOLOCK) WHERE CharacterName = @nvcCharacterName AND DeleteFlag = CAST(0 AS bit);
	END

	SELECT WorldID, CurServerID, CertifyingStep
	FROM dbo.DNAuth WITH (NOLOCK)
	WHERE AccountDBID = @intAccountID;
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

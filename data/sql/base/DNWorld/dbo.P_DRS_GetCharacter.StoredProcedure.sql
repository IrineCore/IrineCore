USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_GetCharacter]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : wgkim@eyedentitygames.com

created date : 2012-01-05
description :
	??? ?? ??
	- @inbCharacterID ? @nvcCharacterName ? ? ??? ??? ??
result set : 
	AccountID / int
	AccountName / nvarchar(50)
	CharacterID / bigint
	CharacterName / nvarchar(30)
	WorldID / int
	CharacterLevel / tinyint
	JobCode / tinyint
	Coin / bigint
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
	103102 = ???? ???? ????.
	103326 = input parameter ? ? ? ?????.
history :
	1~3 = ????
**/

CREATE PROCEDURE [dbo].[P_DRS_GetCharacter]
	  @inbCharacterID bigint = NULL --// ??? ID
	, @nvcCharacterName nvarchar(30) = NULL --// ??? ??
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

	/**_# ??? ??? ?? ???. */
	IF @inbCharacterID IS NOT NULL
	BEGIN
		SELECT C.AccountID, C.AccountName, C.CharacterID, C.CharacterName, C.WorldID, CS.CharacterLevel, CS.JobCode, CS.Coin
		  FROM dbo.Characters C WITH (NOLOCK)
			   INNER JOIN dbo.CharacterStatus CS WITH (NOLOCK) ON (C.CharacterID = CS.CharacterID)
		 WHERE C.CharacterID = @inbCharacterID
		   AND C.DeleteFlag = CONVERT(bit, 0);

		IF @@ROWCOUNT = 0
		BEGIN
			SET @intReturnValue = 103102;
			GOTO ErrorHandler;
		END
	END
	ELSE IF @nvcCharacterName IS NOT NULL
	BEGIN
		SELECT C.AccountID, C.AccountName, C.CharacterID, C.CharacterName, C.WorldID, CS.CharacterLevel, CS.JobCode, CS.Coin
		  FROM dbo.Characters C WITH (NOLOCK)
			   INNER JOIN dbo.CharacterStatus CS WITH (NOLOCK) ON (C.CharacterID = CS.CharacterID)
		 WHERE C.CharacterName = @nvcCharacterName
		   AND C.DeleteFlag = CONVERT(bit, 0);

		IF @@ROWCOUNT = 0
		BEGIN
			SET @intReturnValue = 103102;
			GOTO ErrorHandler;
		END
	END
	ELSE
	BEGIN
		SET @intReturnValue = 103326;
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

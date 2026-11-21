USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_ModSecondAuthPassphraseByServer]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_ModSecondAuthPassphraseByServer]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_ModSecondAuthPassphraseByServer] AS' 
END
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2012-09-26
description : 2? ?? ?? ??? ?? ?? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101115 = ??? ???? ????.
101126 = ?? 2? ?? ????? ???? ????.
**/

ALTER PROCEDURE [dbo].[P_ModSecondAuthPassphraseByServer]
	  @intAccountID int --// ??? ?? ID
	, @binOldSecondAuthPassphrase binary(32) --// ?? 2? ?? ?? ?? (?? ???? NULL? ?????.)
	, @binNewSecondAuthPassphrase binary(32) --// ? 2? ?? ?? ??
	, @inySecondAuthLimit tinyint --// 2? ?? ?? ?? ?? ?? ? (per day)
	, @inySecondAuthFailCount tinyint OUTPUT --// 2? ?? ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @binSecondPassphrase binary(32)
	, @inySecondAuthCodeBefore tinyint
	, @inySecondAuthCodeAfter tinyint;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# ?? ??? ?????.*/
	SELECT @binSecondPassphrase = SP.SecondPassphrase
		, @inySecondAuthCodeBefore = A.SecondAuthCode
	FROM dbo.Accounts A WITH (NOLOCK)
		LEFT OUTER JOIN dbo.SecondPassphrases SP WITH (NOLOCK) ON A.AccountID = SP.AccountID
	WHERE A.AccountID = @intAccountID;

	/**_# ??? ???? ??? 101115? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 101115;
		GOTO ErrorHandler;
	END

	/**_# ?? 2? ?? ?? ??? ???? ???, 2? ?? ?? ???? ????? 101126? ?????.*/
	IF @inySecondAuthCodeBefore IN (2, 3) AND @binSecondPassphrase <> @binOldSecondAuthPassphrase
	BEGIN
		UPDATE dbo.Accounts
		SET @inySecondAuthFailCount = SecondAuthFailCount + 1
			, SecondAuthFailCount = CASE WHEN SecondAuthFailCount >= (@inySecondAuthLimit - 1) THEN 0 ELSE SecondAuthFailCount + 1 END
		WHERE AccountID = @intAccountID;

		SET @intReturnValue = 101126;
		GOTO ErrorHandler;
	END

	/**_# 2? ?? ?? ??? ?????.*/
	UPDATE dbo.SecondPassphrases
	SET SecondPassphrase = @binNewSecondAuthPassphrase
	WHERE AccountID = @intAccountID;

	IF @@ROWCOUNT = 0
		INSERT dbo.SecondPassphrases (AccountID, SecondPassphrase)
		VALUES (@intAccountID, @binNewSecondAuthPassphrase);

	UPDATE dbo.Accounts
	SET SecondAuthResetDate = NULL
		, @inySecondAuthFailCount = SecondAuthFailCount = 0
		, @inySecondAuthCodeAfter = SecondAuthCode = CASE SecondAuthCode WHEN 1 THEN 3 ELSE SecondAuthCode END
	WHERE AccountID = @intAccountID;

	IF @inySecondAuthCodeBefore <> @inySecondAuthCodeAfter
		INSERT dbo.SecondAuthLogs (LogDate, AccountID, SecondAuthCode)
		VALUES (GETDATE(), @intAccountID, @inySecondAuthCodeAfter);

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

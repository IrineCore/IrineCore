USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_ValidateSecondAuthPassphraseByServer]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_ValidateSecondAuthPassphraseByServer]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_ValidateSecondAuthPassphraseByServer] AS' 
END
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2012-09-26
description : 2? ?? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101115 = ??? ???? ????.
101125 = 2? ?? ?? ?? ??? ??? ?????.
101127 = 2? ?? ????? ???? ????.
**/

ALTER PROCEDURE [dbo].[P_ValidateSecondAuthPassphraseByServer]
	  @intAccountID int --// ??? ?? ID
	, @binSecondPassphrase binary(32) --// 2? ?? ?? ??
	, @inySecondAuthLimit tinyint --// 2? ?? ?? ?? ?? ?? ? (per day)
	, @inySecondAuthFailCount tinyint OUTPUT --// 2? ?? ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @binSecondPassphraseDB binary(32)
	, @intBeforeAccountID int = 0;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SET @inySecondAuthFailCount = 0;

	SELECT @binSecondPassphraseDB = SP.SecondPassphrase
		, @inySecondAuthFailCount = A.SecondAuthFailCount
	FROM dbo.Accounts A WITH (NOLOCK)
		LEFT OUTER JOIN dbo.SecondPassphrases SP WITH (NOLOCK) ON A.AccountID = SP.AccountID
	WHERE A.AccountID = @intAccountID;

	/**_# ??? ???? ??? 101115? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 101115;
		GOTO ErrorHandler;
	END

	/**_# 2? ?? ?? ??? ???? ???, 2? ?? ?? ???? ????? 101127? ?????.*/
	IF @binSecondPassphraseDB IS NOT NULL AND @binSecondPassphraseDB <> @binSecondPassphrase
	BEGIN
		UPDATE dbo.Accounts
		SET @inySecondAuthFailCount = SecondAuthFailCount += 1
		WHERE AccountID = @intAccountID;

		/**_## 2? ?? ?? ?? ??? ??? ???? 101125? ?????.*/
		IF @inySecondAuthFailCount >= @inySecondAuthLimit
		BEGIN
			UPDATE dbo.Accounts
			SET SecondAuthFailCount = 0
			WHERE AccountID = @intAccountID;

			SET @intReturnValue = 101125;
			GOTO ErrorHandler;
		END

		SET @intReturnValue = 101127;
		GOTO ErrorHandler;
	END

	/**_# 2? ?? ?? ??? ??????.*/
	IF @inySecondAuthFailCount > 0
		UPDATE dbo.Accounts
		SET SecondAuthFailCount = 0
		WHERE AccountID = @intAccountID;

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

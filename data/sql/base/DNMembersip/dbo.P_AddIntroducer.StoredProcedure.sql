USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_AddIntroducer]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_AddIntroducer]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_AddIntroducer] AS' 
END
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-09-01
description : ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101173 = @nvcAccountName ? ???? ?? ?????.
101174 = @nvcIntroducedAccountName ? ???? ?? ?????.
101175 = ?? ????? ???? ????.
**/

ALTER PROCEDURE [dbo].[P_AddIntroducer]
	  @nvcAccountName nvarchar(30) --// ??? ?? ??
	, @nvcIntroducedAccountName nvarchar(30) --// ???? ?? ??? ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @intAccountID int
	, @intIntroducedAccountID int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT @intAccountID = AccountID FROM dbo.Accounts WITH (NOLOCK) WHERE AccountName = @nvcAccountName;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 101173;
		GOTO ErrorHandler;
	END

	SELECT @intIntroducedAccountID = AccountID FROM dbo.Accounts WITH (NOLOCK) WHERE AccountName = @nvcIntroducedAccountName;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 101174;
		GOTO ErrorHandler;
	END

	/**_# [Introducers] ???? INSERT???.*/
	INSERT dbo.Introducers (AccountID, IntroducedAccountID, RegisterDate)
	VALUES (@intAccountID, @intIntroducedAccountID, GETDATE());
END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 2627 AND ERROR_MESSAGE() LIKE N'%PK_Introducers%'
		SET @intReturnValue = 101175;

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

USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModAccountName]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2012-10-12
description : AccountName? ?????. (??? ?? ??)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_ModAccountName]
	  @nvcOldAccountName nvarchar(50) --// ?? ??? ?? ??
	, @nvcNewAccountName nvarchar(50) --// ? ??? ?? ??
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
    SET @nvcOldAccountName = @nvcOldAccountName + N'@nx'

	UPDATE dbo.Characters
	SET AccountName = @nvcNewAccountName
	FROM dbo.Characters WITH (INDEX(IX_NN_Characters_AccountName), FORCESEEK)
	WHERE AccountName = @nvcOldAccountName
	OPTION (KEEPFIXED PLAN);
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

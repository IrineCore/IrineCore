USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_ValidateAccountNames]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_ValidateAccountNames]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_ValidateAccountNames] AS' 
END
GO
/**
version : 3
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-05-03
description : ?? ??? ???? ?? - ?? ??? ?? - ???. (??? ?? 1,000? ?)
result set : AccountName
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

ALTER PROCEDURE [dbo].[P_ValidateAccountNames]
	  @nvcAccountNames  nvarchar(max) --// ,? ?????? AccountName ??? (?? 1,000?)
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @tblAccountNames table (AccountName nvarchar(50) NOT NULL PRIMARY KEY);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	INSERT @tblAccountNames (AccountName)
	SELECT string
	FROM dbo.TF_NSplit(@nvcAccountNames, ',')
	WHERE string > ''
	GROUP BY string
	ORDER BY string;

	SELECT T.AccountName
	FROM @tblAccountNames T
	WHERE NOT EXISTS (
		SELECT * FROM dbo.Accounts WITH (NOLOCK) WHERE AccountName = T.AccountName
	)
	ORDER BY AccountName;
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

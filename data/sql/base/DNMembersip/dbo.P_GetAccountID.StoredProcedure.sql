USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetAccountID]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetAccountID]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetAccountID] AS' 
END
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-04-14
description : AccountName?? AccountID? ?? ?????.
result set : seq/int, AccountName/nvarchar, AccountID/bigint
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

ALTER PROCEDURE [dbo].[P_GetAccountID]
	  @nvcAccountNames nvarchar(max) --// ,? ?????? ?? ?? ???
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
	SELECT T.seq, T.string AS AccountName, ACC.AccountID
	FROM dbo.TF_NSplit(@nvcAccountNames, N',') T
		LEFT OUTER JOIN dbo.Accounts ACC WITH (NOLOCK) ON T.string = ACC.AccountName
	ORDER BY T.seq;
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

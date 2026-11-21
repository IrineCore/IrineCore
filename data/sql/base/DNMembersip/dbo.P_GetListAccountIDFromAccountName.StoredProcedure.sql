USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListAccountIDFromAccountName]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetListAccountIDFromAccountName]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetListAccountIDFromAccountName] AS' 
END
GO
/**
version : 2
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-01-03
description : ??ID? ???? ????? ?????.
result set : AccountID/int, AccountName/nvarchar
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

ALTER PROCEDURE [dbo].[P_GetListAccountIDFromAccountName]
	  @xmlAccountNames xml --// <root><Characters AccountName="kimbab"/></root>
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @intDoc int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	EXEC sp_xml_preparedocument @intDoc OUTPUT, @xmlAccountNames;

	WITH S (AccountName) AS (
		SELECT AccountName
		FROM OPENXML (@intDoc, N'/root/Characters')
		WITH (
			AccountName nvarchar(50) '@AccountName'
		)
	)
	SELECT AC.AccountID, S.AccountName
	FROM S
		INNER JOIN dbo.Accounts AC WITH (NOLOCK, INDEX(IX_UN_Accounts_AccountName), FORCESEEK) ON S.AccountName = AC.AccountName;	

	EXEC sp_xml_removedocument @intDoc;
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

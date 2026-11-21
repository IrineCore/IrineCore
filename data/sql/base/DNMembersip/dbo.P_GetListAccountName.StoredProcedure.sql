USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListAccountName]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetListAccountName]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetListAccountName] AS' 
END
GO
/**
version : 5
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-10-04
description : AccountID? AccountName? ?????.
result set : AccountID/int, AccountName/nvarchar
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
4 = TF_Split(), TF_NSplit() ??? ????? ?? by ??? at 2011-11-02
5 = Option / Index Hint ?? ? Join ?? ??     by ??? at 2012-03-16
**/

ALTER PROCEDURE [dbo].[P_GetListAccountName]
	  @vchAccountIDs varchar(max) --// ,? ?????? AccountID ???
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
	SELECT AC.AccountID, AC.AccountName
	FROM dbo.TF_Split(@vchAccountIDs, ',') F
		INNER JOIN dbo.Accounts AC WITH (NOLOCK, FORCESEEK, INDEX(PK_Accounts)) ON AC.AccountID = F.string
	ORDER BY F.seq
	OPTION (FORCE ORDER, LOOP JOIN, KEEPFIXED PLAN);
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

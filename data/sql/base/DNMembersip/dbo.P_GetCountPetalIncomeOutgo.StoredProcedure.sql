USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetCountPetalIncomeOutgo]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetCountPetalIncomeOutgo]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetCountPetalIncomeOutgo] AS' 
END
GO
/**
version : 3
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-07-21
description : ?? ?/?? ??? ??????.
result set : RecordCount
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

ALTER PROCEDURE [dbo].[P_GetCountPetalIncomeOutgo]
	  @nvcAccountName nvarchar(50) --// ??? ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @intAccountID int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT @intAccountID = AccountID FROM dbo.Accounts WITH (NOLOCK) WHERE AccountName = @nvcAccountName;

	SELECT (
		SELECT COUNT(*) FROM (
			SELECT OutgoDate
			FROM dbo.PetalIncome A WITH (NOLOCK)
				INNER JOIN dbo.PetalOutgo B WITH (NOLOCK) ON A.PetalIncomeID = B.PetalIncomeID
			WHERE A.AccountID = @intAccountID
			GROUP BY OutgoDate
		) S
	) + (
		SELECT COUNT(*)
		FROM dbo.PetalIncome WITH (NOLOCK)
		WHERE AccountID = @intAccountID
	) AS RecordCount;	
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

USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListAccountWithSameJoinIP]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-11-03
description : ?? IP???? ?? ? ??? ?? ??? ?????.
result set : JoinIP/varchar, AccountName/nvarchar, RegisterDate/datetime2(0)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListAccountWithSameJoinIP]
	  @dt2BasisDate datetime2(0) --// ?? ??
	, @intCount int --// ?? IP?? ??? ?? ?? @intCount ??? ??? ?????.
    , @bitExcludeRestraintAccount bit = NULL --// ???? ?? ?? (0=??, 1=???)
WITH EXECUTE AS 'SPExecutor'
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
	IF @bitExcludeRestraintAccount = 1 
	WITH A AS (
		SELECT JoinIP
		FROM dbo.DimAccounts
		WHERE RegisterDate >= @dt2BasisDate AND RegisterDate < DATEADD(DAY, 1, @dt2BasisDate)
		GROUP BY JoinIP
		HAVING COUNT(*) >= @intCount
	)
	SELECT dbo.FN_IPNumber2IP(B.JoinIP) AS JoinIP, B.AccountName, B.RegisterDate
	FROM A
		INNER JOIN dbo.DimAccounts B ON A.JoinIP = B.JoinIP
		LEFT OUTER JOIN dbo.SS_RestraintedAccounts R WITH (NOLOCK) ON B.AccountID = R.AccountID
	WHERE B.RegisterDate >= @dt2BasisDate AND B.RegisterDate < DATEADD(DAY, 1, @dt2BasisDate)
		 AND R.AccountID IS NULL		
	ORDER BY B.JoinIP;
	ELSE
	WITH A AS (
		SELECT JoinIP
		FROM dbo.DimAccounts
		WHERE RegisterDate >= @dt2BasisDate AND RegisterDate < DATEADD(DAY, 1, @dt2BasisDate)
		GROUP BY JoinIP
		HAVING COUNT(*) >= @intCount
	)
	SELECT dbo.FN_IPNumber2IP(B.JoinIP) AS JoinIP, B.AccountName, B.RegisterDate
	FROM A
		INNER JOIN dbo.DimAccounts B ON A.JoinIP = B.JoinIP
	WHERE B.RegisterDate >= @dt2BasisDate AND B.RegisterDate < DATEADD(DAY, 1, @dt2BasisDate)
	ORDER BY B.JoinIP;
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

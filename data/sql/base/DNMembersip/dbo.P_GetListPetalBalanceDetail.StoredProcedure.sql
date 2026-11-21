USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListPetalBalanceDetail]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetListPetalBalanceDetail]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetListPetalBalanceDetail] AS' 
END
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-11-24
description : ?? ?? ?? ??? ?????.
result set : PetalIncomeID, PetalIncomeCode, PetalAmount, PetalBalance, IncomeDate
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

ALTER PROCEDURE [dbo].[P_GetListPetalBalanceDetail]
	  @intAccountID int --// ??? ?? ID
	, @intPetalBalance int OUTPUT --// ?? ??
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
	EXEC dbo.P_GetPetalBalance
		  @intAccountID = @intAccountID
		, @intPetalBalance = @intPetalBalance OUTPUT;

	SELECT A.PetalIncomeID, A.PetalIncomeCode, A.PetalAmount, A.PetalAmount - ISNULL(B.PetalAmount, 0) AS PetalBalance, A.IncomeDate
	FROM dbo.PetalIncome A WITH (NOLOCK)
		LEFT OUTER JOIN dbo.VIX_PetalOutgoTotal B WITH (NOEXPAND, NOLOCK) ON A.PetalIncomeID = B.PetalIncomeID
	WHERE A.PetalAmount > ISNULL(B.PetalAmount, 0) AND A.AccountID = @intAccountID
	ORDER BY A.PetalIncomeID;
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

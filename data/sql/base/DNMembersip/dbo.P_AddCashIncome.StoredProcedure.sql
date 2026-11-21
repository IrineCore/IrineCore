USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_AddCashIncome]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_AddCashIncome]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_AddCashIncome] AS' 
END
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2012-07-11
description : ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101197 = ??? ???? ????.
**/

ALTER PROCEDURE [dbo].[P_AddCashIncome]
	  @nvcAccountName nvarchar(50) --// ??? ?? ??
	, @inyCashIncomeCode tinyint --// 1=PG? ??, 2=??? ??
	, @inyPGCode tinyint --// 1=????
	, @nvcPGKey nvarchar(128) --// PG? ?? ?
	, @intCashAmount int --// ?? ??
	, @intCashIncomeID int = NULL OUTPUT --// ?? ?? ID
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
	INSERT dbo.CashIncome (AccountID, CashIncomeCode, PGCode, PGKey, CashAmount, IncomeDate)
	SELECT AccountID, @inyCashIncomeCode, @inyPGCode, @nvcPGKey, @intCashAmount, GETDATE()
	FROM dbo.Accounts WITH (NOLOCK, INDEX(IX_UN_Accounts_AccountName), FORCESEEK)
	WHERE AccountName = @nvcAccountName
	OPTION (KEEPFIXED PLAN);

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 101197;
		GOTO ErrorHandler;
	END

	SET @intCashIncomeID = SCOPE_IDENTITY();
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

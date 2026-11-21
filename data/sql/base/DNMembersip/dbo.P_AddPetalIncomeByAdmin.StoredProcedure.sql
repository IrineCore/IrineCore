USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_AddPetalIncomeByAdmin]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_AddPetalIncomeByAdmin]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_AddPetalIncomeByAdmin] AS' 
END
GO
/**
version : 4
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-02-19
description : ??? ?????. ??!!! ?? ???? ?? ??? ???? ????. ?? ????? ??? ?? ?????.
result set : AccountID, AccountName
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101163 = @inyPetalIncomeCode ?? @intPetalAmount ?? ??? ????.
101164 = ?? ?? ??? ??? ??? ????. ???? ????? ?????.
history :
4 = TF_Split(), TF_NSplit() ??? ????? ?? by ??? at 2011-11-02
**/

ALTER PROCEDURE [dbo].[P_AddPetalIncomeByAdmin]
	  @vchAccountIDs varchar(max) = NULL --// ,? ?????? ??? ?? ID (?? 500?) / AccountName?? ???? ?? NULL ?????.
	, @nvcAccountNames nvarchar(max) = NULL --// ,? ?????? ??? ?? ?? (?? 500?) / AccountID? ???? ?? NULL ?????.
	, @intAuditLogID int --// GSM? AuditLogID
	, @inyPetalIncomeCode tinyint --// 2=??? ??, 4=???
	, @intPetalAmount int --// ??? ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @dt2Now datetime2(0) = GETDATE();
DECLARE @tblAccounts table (AccountName nvarchar(50) NULL, AccountID int NULL);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	IF @intPetalAmount <= 0 OR @inyPetalIncomeCode NOT IN (2, 4)
	BEGIN
		SET @intReturnValue = 101163;
		GOTO ErrorHandler;
	END

	IF @nvcAccountNames IS NOT NULL
		INSERT @tblAccounts (AccountName, AccountID)
		SELECT F.string, ACC.AccountID
		FROM dbo.TF_NSplit(@nvcAccountNames, N',') F
			LEFT OUTER JOIN dbo.Accounts ACC WITH (NOLOCK) ON F.string = ACC.AccountName;

	ELSE
		INSERT @tblAccounts (AccountName, AccountID)
		SELECT ACC.AccountName, CAST(F.string AS int)
		FROM dbo.TF_Split(@vchAccountIDs, ',') F
			LEFT OUTER JOIN dbo.Accounts ACC WITH (NOLOCK) ON CAST(F.string AS int) = ACC.AccountID;

	SELECT AccountID, AccountName FROM @tblAccounts WHERE AccountID IS NULL OR AccountName IS NULL;

	IF @@ROWCOUNT > 0
	BEGIN
		SET @intReturnValue = 101164;
		GOTO ErrorHandler;
	END

	/**_# [PetalIncome] ???? INSERT???.*/
	INSERT dbo.PetalIncome (AccountID, PetalIncomeCode, PurchaseOrderID, AuditLogID, PetalAmount, IncomeDate)
	SELECT AccountID, @inyPetalIncomeCode, NULL, @intAuditLogID, @intPetalAmount, @dt2Now
	FROM @tblAccounts
	ORDER BY AccountID;
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

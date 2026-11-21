USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetPagePetalIncomeOutgo]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetPagePetalIncomeOutgo]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetPagePetalIncomeOutgo] AS' 
END
GO
/**
version : 3
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-07-21
description : ?? ?/?? ??? ??????.
result set : InOutType, IncomePetalAmount, PetalIncomeCode, IncomePurchaseOrderID, AuditLogID, OutgoPetalAmount, PetalOutgoCode, OutgoPurchaseOrderID, OccurDate
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101146 = ?? ?/?? ??? ???? ????.
**/

ALTER PROCEDURE [dbo].[P_GetPagePetalIncomeOutgo]
	  @intPageNumber int --// ??? ??.
	, @intPageSize int --// ??? ???.
	, @nvcAccountName nvarchar(50) --// ??? ?? ??
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

	;WITH PTO AS (
		SELECT MIN(B.PetalOutgoID) AS PetalOutgoID, SUM(B.PetalAmount) AS OutgoPetalAmount
		FROM dbo.PetalIncome A WITH (NOLOCK)
			INNER JOIN dbo.PetalOutgo B WITH (NOLOCK) ON A.PetalIncomeID = B.PetalIncomeID
		WHERE A.AccountID = @intAccountID
		GROUP BY OutgoDate
	), PT (InOutType, IncomePetalAmount, PetalIncomeCode, IncomePurchaseOrderID, AuditLogID, OutgoPetalAmount, PetalOutgoCode, OutgoPurchaseOrderID, OccurDate) AS (
		SELECT 1, PetalAmount AS IncomePetalAmount, PetalIncomeCode, PurchaseOrderID, AuditLogID, NULL, NULL, NULL, IncomeDate
		FROM dbo.PetalIncome WITH (NOLOCK)
		WHERE AccountID = @intAccountID
		UNION ALL
		SELECT 2, NULL, NULL, NULL, NULL, PTO.OutgoPetalAmount, O.PetalOutgoCode, O.PurchaseOrderID, O.OutgoDate
		FROM PTO
			INNER JOIN dbo.PetalOutgo O ON PTO.PetalOutgoID = O.PetalOutgoID
	), PG AS (
		SELECT ROW_NUMBER() OVER(ORDER BY OccurDate) AS RowNumber
			, InOutType, IncomePetalAmount, PetalIncomeCode, IncomePurchaseOrderID, AuditLogID, OutgoPetalAmount, PetalOutgoCode, OutgoPurchaseOrderID, OccurDate
		FROM PT
	)
	SELECT InOutType, IncomePetalAmount, PetalIncomeCode, IncomePurchaseOrderID, AuditLogID, OutgoPetalAmount, PetalOutgoCode, OutgoPurchaseOrderID, OccurDate
	FROM PG
	WHERE RowNumber BETWEEN (((@intPageNumber - 1) * @intPageSize) + 1) AND (@intPageNumber * @intPageSize)
	ORDER BY RowNumber
	OPTION (MAXDOP 1);

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 101146;
		GOTO ErrorHandler;
	END
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

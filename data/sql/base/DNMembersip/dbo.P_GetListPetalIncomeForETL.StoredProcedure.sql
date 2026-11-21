USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListPetalIncomeForETL]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetListPetalIncomeForETL]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetListPetalIncomeForETL] AS' 
END
GO
/**
version : 3
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-03-07
description : ?? ?? ??? ?????.
result set : PetalIncomeID, AccountID, PetalIncomeCode, PurchaseOrderID, AuditLogID, PetalAmount, IncomeDate
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

ALTER PROCEDURE [dbo].[P_GetListPetalIncomeForETL]
	  @dtmBasisDate datetime --// ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dt2BasisDate datetime2(0) = CAST(@dtmBasisDate AS date);
/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	;WITH A AS (
		SELECT PetalIncomeID
		FROM dbo.PetalIncome WITH (NOLOCK)
		WHERE IncomeDate >= @dt2BasisDate AND IncomeDate < DATEADD(DAY, 1, @dt2BasisDate)
	)
	SELECT PC.PetalIncomeID, PC.AccountID, PC.PetalIncomeCode, PC.PurchaseOrderID, PC.AuditLogID, PC.PetalAmount, PC.IncomeDate
	FROM dbo.PetalIncome PC WITH (NOLOCK, FORCESEEK) INNER JOIN A ON PC.PetalIncomeID = A.PetalIncomeID;			
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

USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListPetalOutgoForETL]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetListPetalOutgoForETL]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetListPetalOutgoForETL] AS' 
END
GO
/**
version : 4
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-03-07
description : ?? ?? ??? ?????.
result set : PetalOutgoID, PetalIncomeID, PetalOutgoCode, PurchaseOrderID, PetalAmount, OutgoDate
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
3 = PetalOutgoFKey ??     by ??? at 2012-02-20
4 = ??? ??? ID ?? ?? ?? ?? ??(JOIN?? ??) by ??? at 2013-03-22
**/

ALTER PROCEDURE [dbo].[P_GetListPetalOutgoForETL]
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
		SELECT PetalOutgoID
		FROM dbo.PetalOutgo WITH (NOLOCK)
		WHERE OutgoDate >= @dt2BasisDate AND OutgoDate < DATEADD(DAY, 1, @dt2BasisDate)
	)
	SELECT PO.PetalOutgoID, PO.PetalIncomeID, PO.PetalOutgoCode, PO.PurchaseOrderID, PO.PetalAmount, PO.OutgoDate, PO.PetalOutgoFKey
	FROM dbo.PetalOutgo PO WITH (NOLOCK, FORCESEEK) INNER JOIN A ON PO.PetalOutgoID = A.PetalOutgoID;	
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

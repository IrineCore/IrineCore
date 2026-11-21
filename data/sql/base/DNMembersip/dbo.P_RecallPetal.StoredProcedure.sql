USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_RecallPetal]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_RecallPetal]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_RecallPetal] AS' 
END
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-11-24
description : ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

ALTER PROCEDURE [dbo].[P_RecallPetal]
	@inbPetalIncomeID bigint --// ??? ?? ?? ID
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
	INSERT dbo.PetalOutgo (PetalIncomeID, PetalOutgoCode, PurchaseOrderID, PetalAmount, OutgoDate)
	SELECT A.PetalIncomeID, 3, NULL, A.PetalAmount - ISNULL(B.PetalAmount, 0), GETDATE()
	FROM dbo.PetalIncome A WITH (UPDLOCK)
		LEFT OUTER JOIN dbo.VIX_PetalOutgoTotal B WITH (NOEXPAND, UPDLOCK) ON A.PetalIncomeID = B.PetalIncomeID
	WHERE A.PetalIncomeID = @inbPetalIncomeID AND A.PetalAmount > ISNULL(B.PetalAmount, 0);
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

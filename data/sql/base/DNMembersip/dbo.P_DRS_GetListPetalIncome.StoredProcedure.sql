USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_GetListPetalIncome]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_DRS_GetListPetalIncome]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_DRS_GetListPetalIncome] AS' 
END
GO
/**
version : 5
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2012-02-22

description :
	?? ?? ?? ??
result set :
	PetalAmount / int
	IncomeDate / datetime2(0)
	PetalIncomeCode / tinyint
	PetalIncomeFKey / bigint
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

ALTER PROCEDURE [dbo].[P_DRS_GetListPetalIncome]
	  @inyPetalIncomeCode tinyint = NULL --// 8=(DOORS) ?????, 9=(DOORS) ???????, 10=(DOORS) ???????, 11=(DOORS) ???????
	, @inbPetalIncomeFKey bigint = NULL --// PetalIncomeCode ? ??? : 8=(DOORS) ContractID, 9=(DOORS) AuctionID, 10=(DOORS) BiddingID, 11=(DOORS) ContractID
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

	IF @inbPetalIncomeFKey IS NOT NULL
	BEGIN
		SELECT AccountID, PetalAmount, IncomeDate, PetalIncomeCode, PetalIncomeFKey
		  FROM dbo.PetalIncome WITH (NOLOCK, INDEX(IX_NNF_PetalIncome_PetalIncomeFKey_PetalIncomeCode))
		 WHERE (PetalIncomeCode IN (8, 9, 10, 11))
		   AND (PetalIncomeFKey = @inbPetalIncomeFKey)
		   AND (@inyPetalIncomeCode IS NULL OR PetalIncomeCode = @inyPetalIncomeCode);
	END
	ELSE
	BEGIN
		SELECT AccountID, PetalAmount, IncomeDate, PetalIncomeCode, PetalIncomeFKey
		  FROM dbo.PetalIncome WITH (NOLOCK, INDEX(IX_NNF_PetalIncome_PetalIncomeFKey_PetalIncomeCode))
		 WHERE (PetalIncomeCode IN (8, 9, 10, 11))
		   AND (PetalIncomeFKey IS NOT NULL)
		   AND (@inyPetalIncomeCode IS NULL OR PetalIncomeCode = @inyPetalIncomeCode);
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

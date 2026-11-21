USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_AddPetalIncome]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_DRS_AddPetalIncome]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_DRS_AddPetalIncome] AS' 
END
GO
/**
version : 4
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2012-02-20

description :
	?? ??
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
	101115 = ??? ???? ????.
**/

ALTER PROCEDURE [dbo].[P_DRS_AddPetalIncome]
	  @intAccountID int --// ??? ?? ID
	, @intPetalAmount int --// ??? ?? ??
	, @inyPetalIncomeCode tinyint --// 8=(DOORS) ?????, 9=(DOORS) ???????, 10=(DOORS) ???????, 11=(DOORS) ???????
	, @inbPetalIncomeFKey bigint --// PetalIncomeCode ? ??? : 8=(DOORS) ContractID, 9=(DOORS) AuctionID, 10=(DOORS) BiddingID, 11=(DOORS) ContractID
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

	IF @intPetalAmount > 0
	BEGIN
		/**_# [PetalIncome] ???? INSERT???.*/
		INSERT dbo.PetalIncome (AccountID, PetalAmount, IncomeDate, PetalIncomeCode, PetalIncomeFKey)
		SELECT AccountID, @intPetalAmount, GETDATE(), @inyPetalIncomeCode, @inbPetalIncomeFKey
		  FROM dbo.Accounts WITH (NOLOCK)
		 WHERE AccountID = @intAccountID;

		IF @@ROWCOUNT = 0
		BEGIN
			SET @intReturnValue = 101115; -- ??? ???? ????.
			GOTO ErrorHandler;
		END
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

USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_AddPetalIncomeForReward]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_AddPetalIncomeForReward]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_AddPetalIncomeForReward] AS' 
END
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-11-22
description : ?? ??? ?? ? ???? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101115 = ??? ???? ????.
101186 = ?? ?? ??? ??? ?? ?????.
101187 = @inbPurchaseOrderDetailID ?? ???? ????.
history :
2 = (50729) ??? ?? ???? ?? ?? by ??? at 2012-01-30 
**/

ALTER PROCEDURE [dbo].[P_AddPetalIncomeForReward]
	  @inbCharacterID bigint --// ??? ID
	, @inbPurchaseOrderDetailID bigint --// ?? ?? DetailID
	, @intPetalAmount int --// ??? ?? ??
	, @intPetalBalance int OUTPUT --// ?? ? ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @intAccountID int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# ???? AccountID? ?????.*/
	SELECT @intAccountID = AccountID
	FROM dbo.Characters WITH (NOLOCK)
	WHERE CharacterID = @inbCharacterID;

	/**_## ??? ???? ??? 101115? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 101115;
		GOTO ErrorHandler;
	END

	/**_# [PetalIncome] ???? INSERT???.*/
	/**_# ???? ?? PurchaseOrderDetailID?? 101187? ?????.*/
	INSERT dbo.PetalIncome (AccountID, PetalIncomeCode, PurchaseOrderDetailID, AuditLogID, PetalAmount, IncomeDate)
	VALUES (@intAccountID, 1, @inbPurchaseOrderDetailID, NULL, @intPetalAmount, GETDATE());

	/**_# ??? ?? ??? ?????.*/
	EXEC @intReturnValue = dbo.P_GetPetalBalance
		  @intAccountID = @intAccountID
		, @intPetalBalance = @intPetalBalance OUTPUT;
END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 547 AND ERROR_MESSAGE() LIKE N'%FK_PurchaseOrderDetails_PetalIncome_PurchaseOrderDetailID%'
		SET @intReturnValue = 101187;

	ELSE IF ERROR_NUMBER() = 2601 AND ERROR_MESSAGE() LIKE N'%IX_UNF_PetalIncome_PurchaseOrderDetailID%'
		SET @intReturnValue = 101186;

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

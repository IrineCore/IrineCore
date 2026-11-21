USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetPetalBalance]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetPetalBalance]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetPetalBalance] AS' 
END
GO
/**
version : 3
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-02-19
description : ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

ALTER PROCEDURE [dbo].[P_GetPetalBalance]
	  @intAccountID int --// ??? ?? ID
	, @intPetalBalance int OUTPUT --// ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @intIncomeAmount int = 0
	, @intOutgoAmount int = 0;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT @intIncomeAmount = PetalAmount
	FROM dbo.VIX_PetalIncome WITH (NOEXPAND, NOLOCK)
	WHERE AccountID = @intAccountID;

	IF @@ROWCOUNT = 0
		SET @intPetalBalance = 0;

	ELSE
	BEGIN
		SELECT @intOutgoAmount = PetalAmount
		FROM dbo.VIX_PetalOutgo WITH (NOEXPAND, NOLOCK)
		WHERE AccountID = @intAccountID;

		SET @intPetalBalance = @intIncomeAmount - @intOutgoAmount;
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

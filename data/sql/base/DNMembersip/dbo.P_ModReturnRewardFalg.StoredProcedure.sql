USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_ModReturnRewardFalg]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_ModReturnRewardFalg]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_ModReturnRewardFalg] AS' 
END
GO
/**
version : 2
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-12-04
description : ??? ?? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101115 = ??? ???? ????.
101219 = ??? ?? ???? ?? ?? ?????.
**/

ALTER PROCEDURE [dbo].[P_ModReturnRewardFalg]
	  @intAccountID int --// ??? ?? ID	
	, @bitReturnRewardFlag bit --// 0=????? ?? ??, 1=????? ??	
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
      , @bitCurrentFlag bit;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# [Accounts] ???? UPDATE???.*/
	IF @bitReturnRewardFlag = CAST(1 AS bit) 
	BEGIN
		SELECT @bitCurrentFlag = ReturnRewardFlag 
		FROM dbo.Accounts WITH (NOLOCK) 
		WHERE AccountID = @intAccountID;

		IF @bitCurrentFlag = @bitReturnRewardFlag
		BEGIN
			SET @intReturnValue = 101219;
			GOTO ErrorHandler;
		END

		UPDATE dbo.Accounts 
		   SET ReturnRewardFlag = @bitReturnRewardFlag
		 WHERE AccountID = @intAccountID;
	END
	ELSE
		UPDATE dbo.Accounts 
		   SET ReturnRewardFlag = @bitReturnRewardFlag
		 WHERE AccountID = @intAccountID;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 101115;
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

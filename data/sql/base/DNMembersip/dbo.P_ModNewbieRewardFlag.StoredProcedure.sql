USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_ModNewbieRewardFlag]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_ModNewbieRewardFlag]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_ModNewbieRewardFlag] AS' 
END
GO
/**
version : 2
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-11-21
description : ???? ?? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101115 = ??? ???? ????.
101200 = ?? ?? ?? ??? ?? ?????.
**/

ALTER PROCEDURE [dbo].[P_ModNewbieRewardFlag]
	  @intAccountID int --// ??? ?? ID	
	, @bitNewbieRewardFlag bit --// 0=?????? ?? ??, 1=?????? ??	
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	  , @dt2Now datetime2(0) = GETDATE()
	  , @bitCurrentFlag bit;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# [Accounts] ???? UPDATE???.*/
	IF @bitNewbieRewardFlag = CAST(1 AS bit) 
	BEGIN
		SELECT @bitCurrentFlag = NewbieRewardFlag FROM dbo.Accounts WITH (NOLOCK) WHERE AccountID = @intAccountID;

		IF @bitCurrentFlag = @bitNewbieRewardFlag
		BEGIN
			SET @intReturnValue = 101200;
			GOTO ErrorHandler;
		END

		UPDATE dbo.Accounts 
		   SET NewbieRewardFlag = @bitNewbieRewardFlag, NewbieRewardDate = @dt2Now
		 WHERE AccountID = @intAccountID;
	END
	ELSE
		UPDATE dbo.Accounts 
		   SET NewbieRewardFlag = @bitNewbieRewardFlag, NewbieRewardDate = NULL
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

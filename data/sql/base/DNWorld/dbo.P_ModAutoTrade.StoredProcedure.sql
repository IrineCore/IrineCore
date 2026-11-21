USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModAutoTrade]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-08-19
description : ?? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103314 = STOP ???? ??? ? ????.
**/

CREATE PROCEDURE [dbo].[P_ModAutoTrade]
	  @intItemID int --// ??? ID
	, @intMaxUnitPrice int --// ?? ??
	, @inbRemainBudget bigint --// ?? ??
	, @bitEnableFlag bit --// 0=disable, 1=enable
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
	/**_# [AutoTrade] ???? UPDATE???.*/
	UPDATE dbo.AutoTrade
	SET MaxUnitPrice = @intMaxUnitPrice, RemainBudget = @inbRemainBudget, EnableFlag = @bitEnableFlag
	WHERE ItemID = @intItemID AND (EnableFlag = 0 OR @bitEnableFlag = 0);

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103314;
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

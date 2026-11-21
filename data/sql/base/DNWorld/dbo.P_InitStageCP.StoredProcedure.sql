USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_InitStageCP]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-12-24
description : ???? CP? ??????.(???)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_InitStageCP]
	  @inyTypeCode tinyint --//  0-All, 1-Legend, 2-Monthly, 3-Personal
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
	IF @inyTypeCode = 0
	BEGIN
		TRUNCATE TABLE dbo.StageClearBest;
		TRUNCATE TABLE dbo.StageClearMonthlyBest;
		TRUNCATE TABLE dbo.StageClearPersonalBest;
	END
	ELSE IF @inyTypeCode = 1
		TRUNCATE TABLE dbo.StageClearBest;
	ELSE IF @inyTypeCode = 2
		TRUNCATE TABLE dbo.StageClearMonthlyBest;
	ELSE IF @inyTypeCode = 3
		TRUNCATE TABLE dbo.StageClearPersonalBest
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

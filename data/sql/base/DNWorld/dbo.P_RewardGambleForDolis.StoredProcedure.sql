USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_RewardGambleForDolis]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-01-24
description : ?? ????? ????? ???..
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_RewardGambleForDolis]
	  @inyGambleFeeCode tinyint --// 1=??, 2=??
	, @dt2GambelStartDate datetime2(0) --// ??????
	, @dt2GambelEndDate datetime2(0) --// ??????
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
	/**_# [GambleIDs] ?? ????? ?????.*/
	UPDATE dbo.GambleIDs
	SET RewardFlag = 1
	WHERE GambelStartDate BETWEEN @dt2GambelStartDate AND @dt2GambelEndDate
      AND GambleFeeCode = @inyGambleFeeCode;	

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

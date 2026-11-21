USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddGamble]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-01-23
description : ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_AddGamble]
	  @inyGambleFeeCode tinyint --// 1=??, 2=??
	, @intEntranceFee int --// ???
	, @inbGambleID bigint = NULL OUTPUT --// ??ID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	  , @dt2Now datetime2(0) = GETDATE();

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# [GambleIDs] ???? INSERT???.*/
	INSERT dbo.GambleIDs (GambleFeeCode, EntranceFee, GambelStartDate)
	VALUES (@inyGambleFeeCode, @intEntranceFee, @dt2Now);

	SET @inbGambleID = SCOPE_IDENTITY();
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

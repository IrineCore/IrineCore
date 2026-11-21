USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddAutoTrade]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-08-19
description : ?? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103313 = ?? ??? ??????.
**/

CREATE PROCEDURE [dbo].[P_AddAutoTrade]
	  @intItemID int --// ??? ID
	, @intMaxUnitPrice int --// ?? ??
	, @inbRemainBudget bigint --// ?? ??
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
	/**_# [AutoTrade] ???? INSERT???.*/
	INSERT dbo.AutoTrade (ItemID, MaxUnitPrice, RemainBudget, EnableFlag)
	VALUES (@intItemID, @intMaxUnitPrice, @inbRemainBudget, 0);
END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 2627 AND ERROR_MESSAGE() LIKE N'%PK_AutoTrade%'
		SET @intReturnValue = 103313;

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

USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_ModKeySettingOption]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_ModKeySettingOption]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_ModKeySettingOption] AS' 
END
GO
/**
version : 6
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-14
description : ? ?? ?? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101115 = ??? ???? ????.
history :
2 = KeySettingOption ??? binary(41)?? binary(43)?? ?? at 2010-07-08
3 = KeySettingOption ??? binary(43)?? binary(44)?? ?? at 2010-07-08
4 = KeySettingOption ??? binary(44)?? binary(60)?? ?? at 2010-08-16
5 = KeySettingOption ??? binary(60)?? binary(70)?? ?? by ??? at 2012-09-03
6 = KeySettingOption ??? binary(70)?? binary(80)?? ?? by ??? at 2013-02-04
**/

ALTER PROCEDURE [dbo].[P_ModKeySettingOption]
	  @intAccountID int --// ??? ?? ID
	, @binKeySettingOption binary(80) --// ? ?? ??
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
	/**_# [Accounts] ???? SELECT???.*/
	UPDATE dbo.Accounts
	SET KeySettingOption = @binKeySettingOption
	WHERE AccountID = @intAccountID;

	/**_# ??? ??? ???? ??? 101115? ?????.*/
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

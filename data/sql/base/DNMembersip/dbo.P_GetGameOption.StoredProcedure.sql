USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetGameOption]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetGameOption]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetGameOption] AS' 
END
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-06
description : ?? ?? ?? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101115 = ??? ???? ????.
**/

ALTER PROCEDURE [dbo].[P_GetGameOption]
	  @intAccountID int --// ??? ?? ID
	, @binGameOption binary(20) OUTPUT --// ?? ?? ?? (????)
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
	SELECT @binGameOption = GameOption
	FROM dbo.Accounts WITH (NOLOCK)
	WHERE AccountID = @intAccountID;

	/**_# ??? ???? ??? 101115? ?????.*/
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

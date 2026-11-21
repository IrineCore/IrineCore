USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_DelBlockedIPException]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_DelBlockedIPException]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_DelBlockedIPException] AS' 
END
GO
/**
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-04
Description : ?? ?? IP ??? ?????.
return value :
0 = There is no error.
1 = The transaction is in an uncommittable state. Rolling back transaction.
100 = System error occurred.
101106 = ?? ?? IP ??? ???? ????.
**/

ALTER PROCEDURE [dbo].[P_DelBlockedIPException]
	  @vchStartIP varchar(15) --// ?? IP ??.
	, @vchEndIP varchar(15) --// ? IP ??.
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE
	@intReturnValue int,
	@intStartIP int,
	@intEndIP int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END;

BEGIN TRY
	SET @intStartIP = dbo.FN_IP2IPNumber(@vchStartIP);
	SET @intEndIP = dbo.FN_IP2IPNumber(@vchEndIP);

	/**_# [BlockedIPExceptions] ????? ?????.*/
	DELETE dbo.BlockedIPExceptions
	WHERE StartIP = (SELECT @intStartIP) AND EndIP = (SELECT @intEndIP);

	/**_## ??? ?? ??? 101106? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 101106;
		GOTO ErrorHandler;
	END;
END TRY
BEGIN CATCH
	GOTO ErrorHandler;
END CATCH

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

IF @intReturnValue IS NULL OR @intReturnValue = 0
	EXEC @intReturnValue = dbo.P_AddErrorLog;

RETURN @intReturnValue;
GO

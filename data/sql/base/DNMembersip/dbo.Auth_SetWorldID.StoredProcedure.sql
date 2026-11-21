USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[Auth_SetWorldID]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[Auth_SetWorldID]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[Auth_SetWorldID] AS' 
END
GO
/**
version : 4
author : ???
e-mail : b4nfter@eyedentitygames.com
created date : 2010-01-08
description : ?? (??? ???? ?? ?? ???? ??) / ?? ????? ?? (Default)
return value :
history :
10 = Try..Catch.. ?? ??     by ??? at 2011-12-05
**/

ALTER PROCEDURE [dbo].[Auth_SetWorldID]
	@pAccountDBID int			--// ?? DBID
	,@pWorldID tinyint			--// ?? ID
AS
SET NOCOUNT ON
SET XACT_ABORT ON;

DECLARE @intReturnValue int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	-- ?? ????? ??? ????	
	IF NOT EXISTS(SELECT * FROM dbo.DNAuth WITH (NOLOCK) WHERE AccountDBID = @pAccountDBID)
	BEGIN
		INSERT dbo.DNAuth (AccountDBID) 
		VALUES (@pAccountDBID);
		
		IF @@ROWCOUNT = 0
		BEGIN
			SET @intReturnValue = 1;
			GOTO ErrorHandler;
		END
	END

	UPDATE dbo.DNAuth 
	SET WorldID = @pWorldID 
	WHERE AccountDBID = @pAccountDBID;
	
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 1;
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

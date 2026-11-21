USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[Auth_ResetAuthServer]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[Auth_ResetAuthServer]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[Auth_ResetAuthServer] AS' 
END
GO
/**
version : 9
author : ???
e-mail : b4nfter@eyedentitygames.com
created date : 2010-01-08
description : ???? ??? (?? ??)
return value :
history :
5 = version 3?? ?? at 2010-08-12
6 = Try..Catch.. ?? ??     by ??? at 2011-12-
7 = ?? Update? Loop ? ?? by ??? at 2012-09-25
8~9 = Update? Delete? ?? by ??? at 2012-09-26
**/

ALTER PROCEDURE [dbo].[Auth_ResetAuthServer]
	 @pServerID			INT			--// ?? ????
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

	SELECT AccountDBID
	INTO #tempAccountDBID
	FROM dbo.DNAuth WITH (NOLOCK)
	WHERE CurServerID = @pServerID;

	WHILE 1=1
	BEGIN		
		DELETE dbo.DNAuth
		WHERE AccountDBID IN (
			SELECT TOP 100 AccountDBID
			FROM #tempAccountDBID			
			ORDER BY AccountDBID
		);

		IF @@ROWCOUNT = 0
			break;		

		DELETE #tempAccountDBID
		WHERE AccountDBID IN (
			SELECT TOP 100 AccountDBID
			FROM #tempAccountDBID			
			ORDER BY AccountDBID
		);

		IF @@ROWCOUNT = 0
			break;		
	END

	IF (@@ERROR <> 0)
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

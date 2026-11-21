USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[Auth_ResetAuth]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[Auth_ResetAuth]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[Auth_ResetAuth] AS' 
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
6 = version 4? ?? at 2010-08-12
7 = ?? ??? at 2011-04-18
8~9 = Try..Catch.. ?? ??     by ??? at 2011-12-01
**/

ALTER PROCEDURE [dbo].[Auth_ResetAuth]
	  @pAccountDBID		int			--// ?? DBID
	, @pSessionID		int			--// ?? ID
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

DECLARE @aSessionID int;

BEGIN TRY
	-- ??? ??? ???? ????? ?? ???? ???? ?? ???? ?? ????? ??
	BEGIN TRANSACTION;

	UPDATE dbo.DNAuth
	SET @aSessionID = [SessionID]
		, CertifyingKey = 0
		, CertifyingStep = 0
		, CurServerID = 0
		, AccessTime = '1900-01-01'
		, WorldID = 0
		, SessionID = 0
		, CharacterDBID = 0
		, AccountName = N''
		, CharacterName = N''
		, AccountLevel = 0
		, IsAdult = 0
		, Age = 0
		, DailyCreateCount = 0
	WHERE AccountDBID = @pAccountDBID;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 1305;		-- ?????? ???? ??
		GOTO ErrorHandler;
	END

	IF @pSessionID <> @aSessionID
	BEGIN
		SET @intReturnValue = 1310;		-- ?? ID ? ???? ??
		GOTO ErrorHandler;
	END

	COMMIT TRANSACTION;
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

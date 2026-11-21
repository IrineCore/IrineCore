USE [dnmembership]
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE PROCEDURE [dbo].[Auth_ResetAuthByAccountWorld]
	  @pAccountDBID int
	, @pWorldID tinyint
	, @pSessionID int = 0
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @aSessionID int;

IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
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
	WHERE AccountDBID = @pAccountDBID
	  AND (@pWorldID = 0 OR WorldID = @pWorldID);

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 1305;
		GOTO ErrorHandler;
	END

	IF @pSessionID <> 0 AND @pSessionID <> @aSessionID
	BEGIN
		SET @intReturnValue = 1310;
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

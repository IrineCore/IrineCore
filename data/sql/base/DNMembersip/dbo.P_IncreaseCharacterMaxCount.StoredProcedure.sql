USE [dnmembership]
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE PROCEDURE [dbo].[P_IncreaseCharacterMaxCount]
	  @intAccountID int
	, @inyAddCount tinyint
	, @inyLimitCount tinyint
	, @inyCharacterMaxCount tinyint OUTPUT
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	  , @inyCurrentMax tinyint;

IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	BEGIN TRANSACTION;

	SELECT @inyCurrentMax = CharacterMaxCount
	FROM dbo.Accounts WITH (UPDLOCK)
	WHERE AccountID = @intAccountID;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 101115;
		GOTO ErrorHandler;
	END

	IF (@inyCurrentMax + @inyAddCount) > @inyLimitCount
	BEGIN
		SET @inyCharacterMaxCount = @inyCurrentMax;
		SET @intReturnValue = 101116;
		GOTO ErrorHandler;
	END

	SET @inyCharacterMaxCount = @inyCurrentMax + @inyAddCount;

	UPDATE dbo.Accounts
	SET CharacterMaxCount = @inyCharacterMaxCount
	WHERE AccountID = @intAccountID;

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

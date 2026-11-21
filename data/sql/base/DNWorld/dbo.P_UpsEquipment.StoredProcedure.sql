USE [dnworld]
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE PROCEDURE [dbo].[P_UpsEquipment]
	  @inbCharacterID bigint
	, @inyItemLocationCode tinyint
	, @inyItemLocationIndex tinyint
	, @inbItemSerial bigint
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;

IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	IF @inbItemSerial > 0
	BEGIN
		UPDATE dbo.MaterializedItems
		SET ItemLocationCode = @inyItemLocationCode
		  , ItemLocationIndex = @inyItemLocationIndex
		WHERE OwnerCharacterID = @inbCharacterID
		  AND ItemSerial = @inbItemSerial;
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

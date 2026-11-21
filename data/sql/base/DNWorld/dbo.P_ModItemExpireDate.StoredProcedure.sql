USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModItemExpireDate]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-05-24
description : ???? ????? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103151 = ???? ???? ????.
**/

CREATE PROCEDURE [dbo].[P_ModItemExpireDate]
	  @inbItemSerial bigint --// ??? ?? ??
	, @intItemLifespan int --// ??? ??? ?? (?? : ?)
	, @inbOwnerCharacterID bigint --// ???? ??? ??? ID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dt2Now datetime2(0) = GETDATE();

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	UPDATE dbo.MaterializedItems
	SET ItemLifespan += @intItemLifespan
		, ItemExpireDate =
			CASE
				WHEN ItemExpireDate < @dt2Now THEN DATEADD(MINUTE, @intItemLifespan, @dt2Now)
				ELSE DATEADD(MINUTE, @intItemLifespan, ItemExpireDate)
			END
	WHERE ItemSerial = @inbItemSerial AND OwnerCharacterID = @inbOwnerCharacterID;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103151;
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

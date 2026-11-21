USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ExtendItemExpireDate]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-03-19
description : ?? ??? ???? ?? ??? ?? ?????. ??!! ?? ???? ? GSM? ?? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
2 = Tuning. ??? ???? ???? ?? ???     by 20110601.wgkim
**/

CREATE PROCEDURE [dbo].[P_ExtendItemExpireDate]
	  @intExtendedLifespan int --// ??? ??? ?? (?? : ?)
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @dtmCurrentDateTime datetime;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	BEGIN TRANSACTION;

	SET @dtmCurrentDateTime = GETDATE();

	INSERT dbo.ItemLifespanExtendLogs (ExtendedLifespan, LogDate) VALUES (@intExtendedLifespan, @dtmCurrentDateTime)

	UPDATE dbo.MaterializedItems
	   SET ItemExpireDate = DATEADD(MINUTE, @intExtendedLifespan, ItemExpireDate)
	 WHERE EternityFlag = CAST(0 AS bit)
	   AND ItemExpireDate >= CONVERT(smalldatetime, DATEADD(MINUTE, (-1) * @intExtendedLifespan, @dtmCurrentDateTime));

	UPDATE dbo.EffectItems
	   SET EffectItemExpireDate = DATEADD(MINUTE, @intExtendedLifespan, EffectItemExpireDate)
	 WHERE EternityFlag = CAST(0 AS bit)
	   AND EffectItemExpireDate >= CONVERT(smalldatetime, DATEADD(MINUTE, (-1) * @intExtendedLifespan, @dtmCurrentDateTime));

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

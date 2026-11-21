USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModPeriodicReset]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-06-22
description : ?? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
2 = @intPeriodicResetValue ? ?? ?? (int ? bigint)     by 20110701.wgkim
**/

CREATE PROCEDURE [dbo].[P_ModPeriodicReset]
	  @inbCharacterID bigint --// ??? ID
	, @inyPeriodicResetCode tinyint --// 1=??? ???
	, @intPeriodicResetValue bigint --// ?? ?
	, @dtmServerTime datetime --// ?? ??
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
	/**_# [PeriodicReset] ???? UPSERT???.*/
	UPDATE dbo.PeriodicReset
	SET PeriodicResetValue = @intPeriodicResetValue
		, LastModifyDate = CAST(CONVERT(char(16), @dtmServerTime, 121) AS smalldatetime)
	WHERE CharacterID = @inbCharacterID AND PeriodicResetCode = @inyPeriodicResetCode;

	IF @@ROWCOUNT = 0
		INSERT dbo.PeriodicReset (CharacterID, PeriodicResetCode, PeriodicResetValue, LastModifyDate)
		VALUES (@inbCharacterID, @inyPeriodicResetCode, @intPeriodicResetValue, CAST(CONVERT(char(16), @dtmServerTime, 121) AS smalldatetime));
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

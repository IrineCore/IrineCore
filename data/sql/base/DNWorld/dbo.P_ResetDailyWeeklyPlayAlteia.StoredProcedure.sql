USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ResetDailyWeeklyPlayAlteia]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-11-20
description : ??, ?? ???? ?????? ?? ???.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_ResetDailyWeeklyPlayAlteia]
	  @inbCharacterID bigint --// ??? ID
	, @inySetTypeCode tinyint --//1=??????? ? ????, 2=???????
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
	IF @inySetTypeCode = 1
	BEGIN
		UPDATE dbo.PlayAlteia
		SET DailyPlayCount = 0
		WHERE CharacterID = @inbCharacterID;

		DELETE dbo.AlteiaSendTickets
		WHERE CharacterID = @inbCharacterID OR ReceiveCharacterID = @inbCharacterID;
	END
	ELSE IF @inySetTypeCode = 2
		UPDATE dbo.PlayAlteia
		SET WeeklyPlayCount = 0
		WHERE CharacterID = @inbCharacterID;
		
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

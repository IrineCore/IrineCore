USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddNestClearRecord]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-08-16
description : ??? ??? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_AddNestClearRecord]
	  @intMapID int --// ?ID
	, @inbCharacterID bigint --// ???ID
	, @inbPartyID bigint --// ??ID
	, @intClearTime int --// ?????	
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @dtmClearDate datetime;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SET @dtmClearDate = GETDATE();

	/**_# [NestClearRecord] ???? INSERT???.*/
	INSERT dbo.NestClearRecord (MapID, CharacterID, PartyID, ClearTime, ClearDate)
	VALUES (@intMapID, @inbCharacterID, @inbPartyID, @intClearTime, @dtmClearDate);
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

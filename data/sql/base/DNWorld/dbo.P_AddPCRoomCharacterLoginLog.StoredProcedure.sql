USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddPCRoomCharacterLoginLog]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-05-30
description : ??? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_AddPCRoomCharacterLoginLog]
	  @inbCharacterID bigint --// ??? ID
	, @vchIP varchar(15) --// IP ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
      , @dt2LoginDate datetime2(0) = GETDATE()
      , @intLogTableSuffix int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SET @intLogTableSuffix = dbo.FN_LogTableSuffix(@dt2LoginDate);

	/**_# [PCRoomCharacterLoginLogs0] ???? INSERT???.*/
	IF @intLogTableSuffix = 0
		INSERT dbo.PCRoomCharacterLoginLogs0 (LoginDate, CharacterID, LoginIP)
		VALUES (@dt2LoginDate, @inbCharacterID, dbo.FN_IP2IPNumber(@vchIP));

	ELSE IF @intLogTableSuffix = 1
		INSERT dbo.PCRoomCharacterLoginLogs1 (LoginDate, CharacterID, LoginIP)
		VALUES (@dt2LoginDate, @inbCharacterID, dbo.FN_IP2IPNumber(@vchIP));

	ELSE
		INSERT dbo.PCRoomCharacterLoginLogs2 (LoginDate, CharacterID, LoginIP)
		VALUES (@dt2LoginDate, @inbCharacterID, dbo.FN_IP2IPNumber(@vchIP));
END TRY
BEGIN CATCH
	IF XACT_STATE() <> 0
		ROLLBACK TRANSACTION;

	IF @intReturnValue IS NULL OR @intReturnValue = 0
		EXEC @intReturnValue = dbo.P_AddErrorLog @intPROCID = @@PROCID;

	ELSE
		GOTO ErrorHandler;

	RETURN @intReturnValue;
END CATCH;

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

RETURN @intReturnValue;
GO

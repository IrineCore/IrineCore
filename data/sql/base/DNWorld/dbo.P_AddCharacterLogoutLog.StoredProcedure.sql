USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddCharacterLogoutLog]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 6
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-02-12

description :
	??? ?? ???? ??? ?????.
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
	4 = ???? ?? ??? ??? ?? ??? ??     by 20111019.wgkim
	5 = 4? ?? ?? ??     by 20111019.wgkim
	6 = PC??? ?? ??     by ??? at 2013-05-29
**/

CREATE PROCEDURE [dbo].[P_AddCharacterLogoutLog]
	  @dt2LogoutDate datetime2(0) --// ??? ?? ?? ??
	, @inbCharacterID bigint --// ??? ID
	, @dt2LoginDate datetime2(0) --// ??? ??? ??
	, @intLoginIP int --// ??? IP ??
	, @intSessionID int --// ?? ID
	, @bitPCRoomFlag bit = NULL --// 0=??, 1=PC?
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @intLogTableSuffix int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	IF @dt2LoginDate >= @dt2LogoutDate
		RETURN 0;

	SET @intLogTableSuffix = dbo.FN_LogTableSuffix(@dt2LogoutDate);

	/**_# [CharacterLogoutLogs] ???? INSERT???.*/
	IF @intLogTableSuffix = 0
		INSERT dbo.CharacterLogoutLogs0 (LogoutDate, CharacterID, LoginDate, LoginIP, SessionID, PCRoomFlag)
		VALUES (@dt2LogoutDate, @inbCharacterID, @dt2LoginDate, @intLoginIP, @intSessionID, ISNULL(@bitPCRoomFlag, 0));

	ELSE IF @intLogTableSuffix = 1
		INSERT dbo.CharacterLogoutLogs1 (LogoutDate, CharacterID, LoginDate, LoginIP, SessionID, PCRoomFlag)
		VALUES (@dt2LogoutDate, @inbCharacterID, @dt2LoginDate, @intLoginIP, @intSessionID, ISNULL(@bitPCRoomFlag, 0));

	ELSE
		INSERT dbo.CharacterLogoutLogs2 (LogoutDate, CharacterID, LoginDate, LoginIP, SessionID, PCRoomFlag)
		VALUES (@dt2LogoutDate, @inbCharacterID, @dt2LoginDate, @intLoginIP, @intSessionID, ISNULL(@bitPCRoomFlag, 0));

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

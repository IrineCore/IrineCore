USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListPCRoomCharacterLogoutLog]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-05-30
description : ??? ???? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListPCRoomCharacterLogoutLog]
	@dtmBasisDate datetime --// ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @dt2BasisDate datetime2(0), @intLogTableSuffix int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SET @dt2BasisDate = DATEADD(DAY, DATEDIFF(DAY, 0, @dtmBasisDate), 0);
	SET @intLogTableSuffix = dbo.FN_LogTableSuffix(@dt2BasisDate);

	/**_# [PCRoomCharacterLogoutLogs0] ???? INSERT???.*/
	IF @intLogTableSuffix = 0
		SELECT LogoutDate, CharacterID, LoginDate, LoginIP
		FROM dbo.PCRoomCharacterLogoutLogs0 WITH (NOLOCK)
		WHERE LogoutDate >= @dt2BasisDate AND LogoutDate < DATEADD(DAY, 1, @dt2BasisDate);

	ELSE IF @intLogTableSuffix = 1
		SELECT LogoutDate, CharacterID, LoginDate, LoginIP
		FROM dbo.PCRoomCharacterLogoutLogs1 WITH (NOLOCK)
		WHERE LogoutDate >= @dt2BasisDate AND LogoutDate < DATEADD(DAY, 1, @dt2BasisDate);

	ELSE
		SELECT LogoutDate, CharacterID, LoginDate, LoginIP
		FROM dbo.PCRoomCharacterLogoutLogs2 WITH (NOLOCK)
		WHERE LogoutDate >= @dt2BasisDate AND LogoutDate < DATEADD(DAY, 1, @dt2BasisDate);
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

USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListCharacterLoginLog]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-12-23
description : ??? ??? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListCharacterLoginLog]
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

	IF @intLogTableSuffix = 0
		SELECT LoginDate, CharacterID, LoginIP, PCRoomFlag
		FROM dbo.CharacterLoginLogs0 WITH (NOLOCK)
		WHERE LoginDate >= @dt2BasisDate AND LoginDate < DATEADD(DAY, 1, @dt2BasisDate)
		OPTION (MAXDOP 1);

	ELSE IF @intLogTableSuffix = 1
		SELECT LoginDate, CharacterID, LoginIP, PCRoomFlag
		FROM dbo.CharacterLoginLogs1 WITH (NOLOCK)
		WHERE LoginDate >= @dt2BasisDate AND LoginDate < DATEADD(DAY, 1, @dt2BasisDate)
		OPTION (MAXDOP 1);

	ELSE
		SELECT LoginDate, CharacterID, LoginIP, PCRoomFlag
		FROM dbo.CharacterLoginLogs2 WITH (NOLOCK)
		WHERE LoginDate >= @dt2BasisDate AND LoginDate < DATEADD(DAY, 1, @dt2BasisDate)
		OPTION (MAXDOP 1);

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

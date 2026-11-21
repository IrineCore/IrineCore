USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetAbuseMonitor]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 6
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-05-16
description : ???? ??? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetAbuseMonitor]
	  @inbCharacterID bigint --// ??? ID
	, @intAbuseCount1 int OUTPUT --// ???
	, @intAbuseCount2 int OUTPUT --// ???
	, @intPlayRestraint int = 0 OUTPUT --// PlayRestraint
	, @intResetRestraint int = 0 OUTPUT --// ResetRestraint
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @intObjectID int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT @intAbuseCount1 = AbuseCount1
		, @intAbuseCount2 = AbuseCount2
		, @intPlayRestraint = PlayRestraint
		, @intResetRestraint = ResetRestraint
	FROM dbo.AbuseMonitor WITH (NOLOCK)
	WHERE CharacterID = @inbCharacterID;

	IF @@ROWCOUNT = 0
	BEGIN
		SELECT @intAbuseCount1 = 0, @intAbuseCount2 = 0;

		EXEC dbo.P_GetRestraintConstant
			  @intPlayRestraint = @intPlayRestraint OUTPUT
			, @intResetRestraint = @intResetRestraint OUTPUT;
	END

	ELSE IF @intResetRestraint + @intPlayRestraint IS NULL
	BEGIN
		EXEC dbo.P_GetRestraintConstant
			  @intPlayRestraint = @intPlayRestraint OUTPUT
			, @intResetRestraint = @intResetRestraint OUTPUT;
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

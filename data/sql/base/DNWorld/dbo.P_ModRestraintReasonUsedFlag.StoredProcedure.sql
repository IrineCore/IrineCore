USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModRestraintReasonUsedFlag]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-07
description : ?? ??? ?????? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
102105 = ?? ??? ???? ????.
102106 = ?? UsedFlag? 1? ?????? ????.
**/

CREATE PROCEDURE [dbo].[P_ModRestraintReasonUsedFlag]
	  @intRestraintReasonID int --// ?? ?? ID
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @bitUsedFlag bit;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# [RestraintReasons] ???? UPDATE???.*/
	UPDATE dbo.RestraintReasons SET @bitUsedFlag = UsedFlag, UsedFlag = 1 WHERE RestraintReasonID = @intRestraintReasonID;

	/**_# ?? ??? ???? ??? 102105? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 102105;
		GOTO ErrorHandler;
	END

	/**_# ?? UsedFlag ? 1? ???? ?? ??? 102106? ?????.*/
	IF @bitUsedFlag = 1
	BEGIN
		SET @intReturnValue = 102106;
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

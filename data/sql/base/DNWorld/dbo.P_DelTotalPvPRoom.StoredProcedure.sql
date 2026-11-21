USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DelTotalPvPRoom]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-09-12
description : ?? ?? PvP?? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103408 = ??? ???? PVP?? ????
**/

CREATE PROCEDURE [dbo].[P_DelTotalPvPRoom]
	  @intTotalPvPRoomID int
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
	/**_# [TotalPvPRoomMembers] ???? DELETE???.*/
	DELETE dbo.TotalPvPRoomMembers
	WHERE TotalPvPRoomID = @intTotalPvPRoomID;

	/**_# [TotalPvPRooms] ???? DELETE???.*/
	DELETE dbo.TotalPvPRooms
	WHERE TotalPvPRoomID = @intTotalPvPRoomID;
	
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103408
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

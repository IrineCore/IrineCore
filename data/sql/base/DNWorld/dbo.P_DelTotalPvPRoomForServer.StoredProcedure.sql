USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DelTotalPvPRoomForServer]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-09-26
description : ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_DelTotalPvPRoomForServer]
	  @intServerID int --// ??ID
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
	FROM dbo.TotalPvPRooms A WITH(NOLOCK)
		INNER JOIN dbo.TotalPvPRoomMembers B WITH(NOLOCK) ON A.TotalPvPRoomID = B.TotalPvPRoomID
	WHERE A.ServerID = @intServerID;

	/**_# [TotalPvPRooms] ???? DELETE???.*/
	DELETE dbo.TotalPvPRooms
	WHERE ServerID = @intServerID;

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

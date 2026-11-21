USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddPVPEndLog]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-04-06
description : PVP ?? ??? ?????. (PVP ? ??)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
3, 4 = @inbRoomID1 => @inbRoomID1?? ?? ??
**/

CREATE PROCEDURE [dbo].[P_AddPVPEndLog]
	  @inbRoomID1 bigint --// ??? ? ID 1
	, @intRoomID2 int --// ??? ? ID 2
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @inbPVPStartID bigint
	, @dt2Now datetime2(0) = GETDATE()
	, @intLogTableSuffix int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# [PVPStartIDs] ???? UPDATE???.*/
	;WITH C1 AS (
		SELECT TOP (1) PVPStartID
		FROM dbo.PVPStartIDs WITH (NOLOCK)
		WHERE RoomID1 = @inbRoomID1 AND RoomID2 = @intRoomID2
		ORDER BY PVPStartID DESC
	)
	UPDATE dbo.PVPStartIDs
	SET PVPEndDate = GETDATE()
	FROM dbo.PVPStartIDs L
		INNER JOIN C1 ON L.PVPStartID = C1.PVPStartID;
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

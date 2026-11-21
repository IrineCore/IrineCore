USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddNestGateEndLog]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-08-17
description : ??? ?? ????? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_AddNestGateEndLog]
	  @inbRoomID bigint --// ??? ? ID
    , @inyEndPartySize tinyint --// ?? ?? ???
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
      , @inbNestGateStartID bigint
      , @dt2Now datetime2(0) = GETDATE();

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT TOP (1) @inbNestGateStartID = NestGateStartID
	FROM dbo.NestGateStartIDs WITH (NOLOCK, INDEX(IX_NN_NestGateStartIDs_RoomID), FORCESEEK)
	WHERE RoomID = @inbRoomID
	ORDER BY NestGateStartID DESC
	OPTION(KEEPFIXED PLAN);
	
	/**_# [NestGateStartIDs] ???? UPDATE???.*/
	UPDATE dbo.NestGateStartIDs
	SET NestGateEndDate = @dt2Now
      , EndPartySize = @inyEndPartySize
	WHERE NestGateStartID = @inbNestGateStartID;

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

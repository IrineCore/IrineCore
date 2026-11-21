USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_UsePVPLadderPoint]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-02-10
description : PVP ?? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103297 = PVP ?? ???? ?????.
history :
2 = input param. @inyPVPLadderCode ?? at 2011-02-15
3 = ??? ?? ?? at 2011-03-21
4 = ??? ??? ?? ?? at 2011-03-29
**/

CREATE PROCEDURE [dbo].[P_UsePVPLadderPoint]
	  @inbCharacterID bigint --// ??? ID
	, @intPVPLadderPoint int --// ??? PVP ?? ???
	, @intMapID int --// ? ID
	, @vchIP varchar(15) --// IP ??
	, @intPVPLadderPointAfter int OUTPUT --// ??? ?? PVP ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @dt2Now datetime2(0) = GETDATE(), @intLogTableSuffix int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	BEGIN TRANSACTION;

	UPDATE dbo.CharacterStatus
	SET @intPVPLadderPointAfter = PVPLadderPoint -= @intPVPLadderPoint
	WHERE CharacterID = @inbCharacterID;

	IF @intPVPLadderPointAfter IS NULL OR @intPVPLadderPointAfter < 0
	BEGIN
		SET @intReturnValue = 103297;
		GOTO ErrorHandler;
	END

	COMMIT TRANSACTION;

	SET @intLogTableSuffix = dbo.FN_LogTableSuffix(@dt2Now);

	IF @intLogTableSuffix = 0
		INSERT dbo.PointLogs0 (LogDate, CharacterID, PointCode, PointBefore, PointAfter, MapID, IP)
		VALUES (@dt2Now, @inbCharacterID, 1, @intPVPLadderPointAfter - @intPVPLadderPoint, @intPVPLadderPointAfter, @intMapID, dbo.FN_IP2IPNumber(@vchIP));

	ELSE IF @intLogTableSuffix = 1
		INSERT dbo.PointLogs1 (LogDate, CharacterID, PointCode, PointBefore, PointAfter, MapID, IP)
		VALUES (@dt2Now, @inbCharacterID, 1, @intPVPLadderPointAfter - @intPVPLadderPoint, @intPVPLadderPointAfter, @intMapID, dbo.FN_IP2IPNumber(@vchIP));

	ELSE
		INSERT dbo.PointLogs2 (LogDate, CharacterID, PointCode, PointBefore, PointAfter, MapID, IP)
		VALUES (@dt2Now, @inbCharacterID, 1, @intPVPLadderPointAfter - @intPVPLadderPoint, @intPVPLadderPointAfter, @intMapID, dbo.FN_IP2IPNumber(@vchIP));
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

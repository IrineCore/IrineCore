USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddPVPStartLog]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 8
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-04-06
description : PVP ?? ??? ?????. (PVP ? ??)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
3 = @inbRoomID1 => @inbRoomID1?? ?? ??
4 = @inyPVPModeDetailCode ?? ??
5 = @inyPVPModeCode ?? ??
6 = PVPModeCode ?? ?? at 2011-02-15
8 = ???, ???? ????? ?? by ??? at 2012-04-13
**/

CREATE PROCEDURE [dbo].[P_AddPVPStartLog]
	  @inbRoomID1 bigint --// ??? ? ID 1
	, @intRoomID2 int --// ??? ? ID 2
	, @inyMaxPlayerCount tinyint --// ?? ???? ?
	, @inyPVPModeCode tinyint --// 1=???, 2=???, 3=??, 4=???, 5=??, 6=1:1 PVP??, 7=2:2 PVP??, 8=3:3 PVP??, 9=4:4 PVP??
	, @inyPVPModeDetailCode tinyint --// PVP ??? ??? ? ?? ?? ??
	, @bitBreakIntoFlag bit --// 0=?? ??, 1=?? ??
	, @bitItemUseFlag bit --// 0=??? ??, 1=??? ??
	, @bitSecreteFlag bit --// 0=??, 1=???
	, @intMapID int --// ? ID
	, @inbRoomOpenCharacterID bigint = NULL --// ? ?? CharacterID
	, @inbCheifCharacterID bigint = NULL --// ?? CharacterID
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
	/**_# [PVPStartIDs] ???? INSERT???.*/
	INSERT dbo.PVPStartIDs (RoomID1, RoomID2, PVPStartDate, PVPEndDate)
	VALUES (@inbRoomID1, @intRoomID2, @dt2Now, NULL);

	SET @inbPVPStartID = SCOPE_IDENTITY();

	SET	@intLogTableSuffix = dbo.FN_LogTableSuffix(@dt2Now);

	/**_# [PVPStartLogs] ???? INSERT???.*/
	IF @intLogTableSuffix = 0
		INSERT dbo.PVPStartLogs0 (PVPStartID, MaxPlayerCount, PVPModeCode, PVPModeDetailCode, BreakIntoFlag, ItemUseFlag, SecreteFlag, MapID, RoomOpenCharacterID, CheifCharacterID)
		VALUES (@inbPVPStartID, @inyMaxPlayerCount, @inyPVPModeCode, @inyPVPModeDetailCode, @bitBreakIntoFlag, @bitItemUseFlag, @bitSecreteFlag, @intMapID, @inbRoomOpenCharacterID, @inbCheifCharacterID);

	ELSE IF @intLogTableSuffix = 1
		INSERT dbo.PVPStartLogs1 (PVPStartID, MaxPlayerCount, PVPModeCode, PVPModeDetailCode, BreakIntoFlag, ItemUseFlag, SecreteFlag, MapID, RoomOpenCharacterID, CheifCharacterID)
		VALUES (@inbPVPStartID, @inyMaxPlayerCount, @inyPVPModeCode, @inyPVPModeDetailCode, @bitBreakIntoFlag, @bitItemUseFlag, @bitSecreteFlag, @intMapID, @inbRoomOpenCharacterID, @inbCheifCharacterID);

	ELSE
		INSERT dbo.PVPStartLogs2 (PVPStartID, MaxPlayerCount, PVPModeCode, PVPModeDetailCode, BreakIntoFlag, ItemUseFlag, SecreteFlag, MapID, RoomOpenCharacterID, CheifCharacterID)
		VALUES (@inbPVPStartID, @inyMaxPlayerCount, @inyPVPModeCode, @inyPVPModeDetailCode, @bitBreakIntoFlag, @bitItemUseFlag, @bitSecreteFlag, @intMapID, @inbRoomOpenCharacterID, @inbCheifCharacterID);

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

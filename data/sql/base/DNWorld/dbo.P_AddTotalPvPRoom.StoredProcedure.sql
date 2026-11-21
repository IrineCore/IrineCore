USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddTotalPvPRoom]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 6
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-09-12
description : ?? ?? PVP Room? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_AddTotalPvPRoom]
	  @inyPvPModeCode tinyint --// 1=???, 2=???, 3=??, 4=???, 5=??, 6=1:1 PVP??, 7=2:2 PVP??, 8=3:3 PVP??, 9=4:4 PVP??, 10=???, 11=???, 12=???
	, @intWorldID int --// ?? ID
	, @intServerID int --// ?? ID
	, @intRoomID int --// ??? ? ID
	, @nvcTotalPvPRoomName nvarchar(50) --// ???? PVP ? ??
	, @inyMaxPlayerCount tinyint --// ?? ?? ?? ?
	, @intMapID int --// ? ID
	, @bitBreakIntoFlag bit --// 0=?? ??, 1=?? ??
	, @bitItemPossibleFlag bit --// 0=??? ????, 1=??? ????
	, @bitReviseFlag bit --// 0=?? ??, 1=?? ??
	, @bitObserverFlag bit --// 0=?? ??, 1=?? ??
	, @bitRandomFlag bit --// 0=????, 1=????
	, @inyMinLevel tinyint --// ?? ??
	, @inyMaxLevel tinyint --// ?? ??
	, @intPassWordValue int --// ????
	, @inyChannelTypeCode tinyint --//?? ??
	, @intPlayTime int --// PVP ??? ??(???)
	, @inyWinConditionCode tinyint --// ????(??? ??)
	, @intTotalPvPRoomID int = NULL OUTPUT --// ???? PVP? ID 
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
	/**_# [TotalPvPRooms] ???? INSERT???.*/
	INSERT dbo.TotalPvPRooms (PvPModeCode, WorldID, ServerID, RoomID, TotalPvPRoomName, MaxPlayerCount, MapID, BreakIntoFlag, ItemPossibleFlag, ReviseFlag, ObserverFlag, RandomFlag, MinLevel, MaxLevel, PassWordValue, ChannelTypeCode, PlayTime, WinConditionCode)
	VALUES (@inyPvPModeCode, @intWorldID, @intServerID, @intRoomID, @nvcTotalPvPRoomName, @inyMaxPlayerCount, @intMapID, @bitBreakIntoFlag, @bitItemPossibleFlag, @bitReviseFlag, @bitObserverFlag, @bitRandomFlag, @inyMinLevel, @inyMaxLevel, @intPassWordValue, @inyChannelTypeCode, @intPlayTime, @inyWinConditionCode);

	SET @intTotalPvPRoomID = SCOPE_IDENTITY();
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

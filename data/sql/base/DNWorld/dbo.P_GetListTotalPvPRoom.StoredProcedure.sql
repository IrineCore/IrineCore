USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListTotalPvPRoom]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 7
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-09-12
description : ???? PVP?? ?????.
result set :
	TotalPvPRoomID/int
	WorldID/int
	ServerID/int
	RoomID/bigint
	PvPModeCode/tinyint
	TotalPvPRoomName/nvarchar(50)
	MaxPlayerCount/tinyint
	MapID/int
	BreakIntoFlag/bit
	ItemPossibleFlag/bit
	ReviseFlag/bit
	ObserverFlag/bit
	RandomFlag/bit
	MinLevel/tinyint
	MaxLevel/tinyint
	PvPRoomMemberCount/int
	PassWordValue/int
	ChannelTypeCode/tinyint
	PlayTime/int
	WinConditionCode/tinyint
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListTotalPvPRoom]
	  @intWorldID int--//??ID
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
	SELECT A.TotalPvPRoomID
		 , A.WorldID
		 , A.ServerID
	     , A.RoomID
		 , A.PvPModeCode
		 , A.TotalPvPRoomName
		 , A.MaxPlayerCount
		 , A.MapID
		 , A.BreakIntoFlag
		 , A.ItemPossibleFlag
		 , A.ReviseFlag
		 , A.ObserverFlag
		 , A.RandomFlag
		 , A.MinLevel
		 , A.MaxLevel
		 , ISNULL(B.PvPRoomMemberCount, 0) AS PvPRoomMemberCount
		 , A.PassWordValue
	     , A.ChannelTypeCode
		 , A.PlayTime
		 , A.WinConditionCode
	FROM dbo.TotalPvPRooms A WITH(NOLOCK)
		LEFT OUTER JOIN dbo.VIX_TotalPvPRoomMemberCount B WITH(NOEXPAND, NOLOCK) ON A.TotalPvPRoomID = B.TotalPvPRoomID
	WHERE A.WorldID = @intWorldID

END TRY
BEGIN CATCH
END CATCH;

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

IF @intReturnValue IS NULL OR @intReturnValue = 0
	EXEC @intReturnValue = dbo.P_AddErrorLog;

RETURN @intReturnValue;
GO

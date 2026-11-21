USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModFatigueForGSM]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 6
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-03-16
description : ???? ?????. ?, ??? ??? ???? ???? ????? ???? ?? ??? ?? ????? ?? User? Kick?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
3 = ??? ??? ??
4 = VIP ??? ?? at 2010-06-11
5 = ??? ?? ?? at 2011-03-18
6 = ??? ??? ?? ?? at 2011-03-29
**/

CREATE PROCEDURE [dbo].[P_ModFatigueForGSM]
	  @inbCharacterID bigint --// ??? ID
	, @sdtLastFatigueDate smalldatetime = NULL --// ?? ??? ??? ??. NULL? ???? ??? UPDATE?? ????.
	, @insFatigue smallint = NULL  --// ???. NULL? ???? ??? UPDATE?? ????.
	, @insPCRoomFatigue smallint = NULL  --// PC? ???. NULL? ???? ??? UPDATE?? ????.
	, @insWeeklyFatigue smallint = NULL  --// ?? ???. NULL? ???? ??? UPDATE?? ????.
	, @insEventFatigue smallint = NULL --// ??? ???. NULL? ???? ??? UPDATE?? ????.
	, @insVIPFatigue smallint = NULL --// VIP ???. NULL? ???? ??? UPDATE?? ????.
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dt2Now datetime2(0) = GETDATE()
	, @insFatigueBefore smallint
	, @insPCRoomFatigueBefore smallint
	, @insWeeklyFatigueBefore smallint
	, @insEventFatigueBefore smallint
	, @insVIPFatigueBefore smallint
	, @insFatigueValue smallint;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	UPDATE dbo.CharacterStatus
	SET LastFatigueDate = COALESCE(@sdtLastFatigueDate, LastFatigueDate)
		, @insFatigueBefore = Fatigue
		, @insPCRoomFatigueBefore = PCRoomFatigue
		, @insWeeklyFatigueBefore = WeeklyFatigue
		, @insEventFatigueBefore = EventFatigue
		, @insVIPFatigueBefore = VIPFatigue
		, Fatigue = COALESCE(@insFatigue, Fatigue)
		, PCRoomFatigue = COALESCE(@insPCRoomFatigue, PCRoomFatigue)
		, WeeklyFatigue = COALESCE(@insWeeklyFatigue, WeeklyFatigue)
		, EventFatigue = COALESCE(@insEventFatigue, EventFatigue)
		, VIPFatigue = COALESCE(@insVIPFatigue, VIPFatigue)
		, EventFatigueGainDate = CASE WHEN @insEventFatigue IS NULL THEN EventFatigueGainDate ELSE GETDATE() END
	WHERE CharacterID = @inbCharacterID;

	/**_# [P_AddFatigueLog]? ???? ??? ??? ?????.*/
	IF @insFatigue > 0
	BEGIN
		SET @insFatigueValue = @insFatigue - @insFatigueBefore;

		EXEC @intReturnValue = dbo.P_AddFatigueLog
			  @dt2LogDate = @dt2Now
			, @inbCharacterID = @inbCharacterID
			, @inyFatigueTypeCode = 2
			, @insCurrentFatigue = @insFatigue
			, @insChangedFatigue = @insFatigueValue
			, @intChannelID = NULL
			, @intMapID = NULL;
	END

	IF @insPCRoomFatigue > 0
	BEGIN
		SET @insFatigueValue = @insPCRoomFatigue - @insPCRoomFatigueBefore;

		EXEC @intReturnValue = dbo.P_AddFatigueLog
			  @dt2LogDate = @dt2Now
			, @inbCharacterID = @inbCharacterID
			, @inyFatigueTypeCode = 1
			, @insCurrentFatigue = @insPCRoomFatigue
			, @insChangedFatigue = @insFatigueValue
			, @intChannelID = NULL
			, @intMapID = NULL;
	END

	IF @insWeeklyFatigue > 0
	BEGIN
		SET @insFatigueValue = @insWeeklyFatigue - @insWeeklyFatigueBefore;

		EXEC @intReturnValue = dbo.P_AddFatigueLog
			  @dt2LogDate = @dt2Now
			, @inbCharacterID = @inbCharacterID
			, @inyFatigueTypeCode = 3
			, @insCurrentFatigue = @insWeeklyFatigue
			, @insChangedFatigue = @insFatigueValue
			, @intChannelID = NULL
			, @intMapID = NULL;
	END

	IF @insEventFatigue > 0
	BEGIN
		SET @insFatigueValue = @insEventFatigue - @insEventFatigueBefore;

		EXEC @intReturnValue = dbo.P_AddFatigueLog
			  @dt2LogDate = @dt2Now
			, @inbCharacterID = @inbCharacterID
			, @inyFatigueTypeCode = 4
			, @insCurrentFatigue = @insEventFatigue
			, @insChangedFatigue = @insFatigueValue
			, @intChannelID = NULL
			, @intMapID = NULL;
	END

	IF @insVIPFatigue > 0
	BEGIN
		SET @insFatigueValue = @insVIPFatigue - @insVIPFatigueBefore;

		EXEC @intReturnValue = dbo.P_AddFatigueLog
			  @dt2LogDate = @dt2Now
			, @inbCharacterID = @inbCharacterID
			, @inyFatigueTypeCode = 5
			, @insCurrentFatigue = @insVIPFatigue
			, @insChangedFatigue = @insFatigueValue
			, @intChannelID = NULL
			, @intMapID = NULL;
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

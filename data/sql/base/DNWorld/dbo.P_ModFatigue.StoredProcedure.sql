USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModFatigue]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 18
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-13
description : ??? ?? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
14 = VIP ??? ?? at 2010-06-11
16 = VIP ???? ?? ?? ?? at 2010-07-13
17 = ??? ?? ?? at 2011-03-18
18 = ??? ??? ?? ?? at 2011-03-29
**/

CREATE PROCEDURE [dbo].[P_ModFatigue]
	  @inbCharacterID bigint --// ??? ID
	, @inyFatigueTypeCode tinyint --// 1=PC? ???, 2=?? ???, 3=?? ???, 4=??? ???, 5=VIP ???
	, @insFatigueValue smallint --// ?? ??? ?
	, @intChannelID int --// ???? ??? ?? ID
	, @intMapID int --// ???? ??? ? ID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @insCurrentFatigue smallint
	, @dtmNow datetime = GETDATE()
	, @insPCRoomFatigueBefore smallint
	, @insFatigueBefore smallint
	, @insWeeklyFatigueBefore smallint
	, @insEventFatigueBefore smallint
	, @insVIPFatigueBefore smallint
	, @insPCRoomFatigue smallint
	, @insFatigue smallint
	, @insWeeklyFatigue smallint
	, @insEventFatigue smallint
	, @insVIPFatigue smallint;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END
    --NEXONZ BEGIN
    SET @insFatigueValue = 30000 --MAX 32767
    --NEXONZ END

BEGIN TRY
	/**_# [CharacterStatus] ???? UPDATE???.*/
	UPDATE dbo.CharacterStatus
	SET @insPCRoomFatigueBefore = PCRoomFatigue
		, @insFatigueBefore = Fatigue
		, @insWeeklyFatigueBefore = WeeklyFatigue
		, @insEventFatigueBefore = EventFatigue
		, @insVIPFatigueBefore = VIPFatigue
		, @insPCRoomFatigue = PCRoomFatigue = 
			CASE @inyFatigueTypeCode
				WHEN 1 THEN @insFatigueValue
				ELSE PCRoomFatigue
			END
		, @insFatigue = Fatigue = 
			CASE @inyFatigueTypeCode
				WHEN 2 THEN @insFatigueValue
				ELSE Fatigue
			END
		, @insWeeklyFatigue = WeeklyFatigue = 
			CASE @inyFatigueTypeCode
				WHEN 3 THEN @insFatigueValue
				ELSE WeeklyFatigue
			END
		, @insEventFatigue = EventFatigue = 
			CASE @inyFatigueTypeCode
				WHEN 4 THEN @insFatigueValue
				ELSE EventFatigue
			END
		, @insVIPFatigue = VIPFatigue = 
			CASE @inyFatigueTypeCode
				WHEN 5 THEN @insFatigueValue
				ELSE VIPFatigue
			END
		, LastFatigueDate = 
			CASE
				WHEN @insFatigueValue >=
					CASE @inyFatigueTypeCode
						WHEN 1 THEN PCRoomFatigue
						WHEN 2 THEN Fatigue
						WHEN 3 THEN WeeklyFatigue
						WHEN 5 THEN VIPFatigue
					END THEN @dtmNow
				ELSE LastFatigueDate
			END
	WHERE CharacterID = @inbCharacterID;

	SET @insCurrentFatigue = 
		CASE @inyFatigueTypeCode
			WHEN 1 THEN @insPCRoomFatigue
			WHEN 2 THEN @insFatigue
			WHEN 3 THEN @insWeeklyFatigue
			WHEN 4 THEN @insEventFatigue
			WHEN 5 THEN @insVIPFatigue
		END;

	/**_# [P_AddFatigueLog]? ???? ??? ??? ?????.*/
	SET @insFatigueValue = 
		CASE @inyFatigueTypeCode
			WHEN 1 THEN @insPCRoomFatigue - @insPCRoomFatigueBefore
			WHEN 2 THEN @insFatigue - @insFatigueBefore
			WHEN 3 THEN @insWeeklyFatigue - @insWeeklyFatigueBefore
			WHEN 4 THEN @insEventFatigue - @insEventFatigueBefore
			WHEN 5 THEN @insVIPFatigue - @insVIPFatigueBefore
		END;

	EXEC @intReturnValue = dbo.P_AddFatigueLog
		  @dt2LogDate = @dtmNow
		, @inbCharacterID = @inbCharacterID
		, @inyFatigueTypeCode = @inyFatigueTypeCode
		, @insCurrentFatigue = @insCurrentFatigue
		, @insChangedFatigue = @insFatigueValue
		, @intChannelID = @intChannelID
		, @intMapID = @intMapID;

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

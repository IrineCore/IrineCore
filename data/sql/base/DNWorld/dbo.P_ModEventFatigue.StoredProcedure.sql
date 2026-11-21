USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModEventFatigue]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-07-13
description : ??? ??? ?? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
4 = ??? ?? ?? at 2011-03-18
5 = ??? ??? ?? ?? at 2011-03-29
**/

CREATE PROCEDURE [dbo].[P_ModEventFatigue]
	  @inbCharacterID bigint --// ??? ID
	, @insEventFatigue smallint --// ?? ??? ??? ?
	, @bitEventFatigueInitFlag bit --// ??? ??? ??? ??. 0=??? ??, 1=???
	, @inyInitTime tinyint --// ??? ???? ????? ??. 0 ~ 23
	, @intChannelID int --// ???? ??? ?? ID
	, @intMapID int --// ???? ??? ? ID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dtmNow datetime = GETDATE()
	, @insChangedFatigue smallint
	, @sdtFatigueInitDate smalldatetime;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SET @sdtFatigueInitDate = DATEADD(HOUR, @inyInitTime, CAST(CAST(@dtmNow AS date) AS smalldatetime));
	SET @sdtFatigueInitDate = CASE WHEN @sdtFatigueInitDate <= @dtmNow THEN DATEADD(DAY, 1, @sdtFatigueInitDate) ELSE @sdtFatigueInitDate END;

	/**_# [CharacterStatus] ???? UPDATE???.*/
	UPDATE dbo.CharacterStatus
	SET @insChangedFatigue = @insEventFatigue - EventFatigue
		, EventFatigue = @insEventFatigue
	WHERE CharacterID = @inbCharacterID AND (@bitEventFatigueInitFlag = 0 OR (@bitEventFatigueInitFlag = 1 AND EventFatigueGainDate NOT BETWEEN DATEADD(DAY, -1, @sdtFatigueInitDate) AND @sdtFatigueInitDate));

	IF @@ROWCOUNT > 0 AND @insChangedFatigue <> 0

	/**_# [P_AddFatigueLog]? ???? ??? ??? ?????.*/
	EXEC @intReturnValue = dbo.P_AddFatigueLog
		  @dt2LogDate = @dtmNow
		, @inbCharacterID = @inbCharacterID
		, @inyFatigueTypeCode = 4
		, @insCurrentFatigue = @insEventFatigue
		, @insChangedFatigue = @insChangedFatigue
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

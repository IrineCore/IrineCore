USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddStageRewardLog]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-04-06
description : ???? ?? ?? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
5 = input param. @intRewardFailReason ?? at 2010-12-23
**/

CREATE PROCEDURE [dbo].[P_AddStageRewardLog]
	  @inbRoomID bigint --// ??? ? ID
	, @inbCharacterID bigint --// ??? ID
	, @inyRewardBoxCode tinyint --// 1=??, 2=?, 3=?, 4=?
	, @inbRewardItemSerial bigint --// ?? ?? ??? ??? (?? ???? NULL)
	, @intRewardItemID int --// ?? ?? ??? ID
	, @intRewardFailReason int = NULL --// ?? ?? ?? (???? 0)
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @inbStageStartID bigint
	, @dt2StageStartDate datetime2(0)
	, @intLogTableSuffix int
	, @dt2Now datetime2(0) = GETDATE();

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT TOP (1) @inbStageStartID = StageStartID, @dt2StageStartDate = StageStartDate
	FROM dbo.StageStartIDs WITH (NOLOCK)
	WHERE RoomID = @inbRoomID
	ORDER BY StageStartID DESC;

	IF @@ROWCOUNT = 0 OR (DATEDIFF(DAY, @dt2StageStartDate, @dt2Now) > 0 AND DATEDIFF(MINUTE, @dt2StageStartDate, @dt2Now) > 180)
		RETURN 0;

	SET @intLogTableSuffix = dbo.FN_LogTableSuffix(@dt2StageStartDate);

	/**_# [StageRewardLogs] ???? INSERT???.*/
	IF @intLogTableSuffix = 0
		INSERT dbo.StageRewardLogs0 (StageStartID, CharacterID, RewardBoxCode, RewardItemSerial, RewardItemID, RewardFailReason)
		VALUES (@inbStageStartID, @inbCharacterID, @inyRewardBoxCode, @inbRewardItemSerial, @intRewardItemID, @intRewardFailReason);

	ELSE IF @intLogTableSuffix = 1
		INSERT dbo.StageRewardLogs1 (StageStartID, CharacterID, RewardBoxCode, RewardItemSerial, RewardItemID, RewardFailReason)
		VALUES (@inbStageStartID, @inbCharacterID, @inyRewardBoxCode, @inbRewardItemSerial, @intRewardItemID, @intRewardFailReason);

	ELSE
		INSERT dbo.StageRewardLogs2 (StageStartID, CharacterID, RewardBoxCode, RewardItemSerial, RewardItemID, RewardFailReason)
		VALUES (@inbStageStartID, @inbCharacterID, @inyRewardBoxCode, @inbRewardItemSerial, @intRewardItemID, @intRewardFailReason);

END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 2601 AND ERROR_MESSAGE() LIKE N'%IX/_UN/_StageRewardLogs%' ESCAPE(N'/')
		RETURN 0;

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

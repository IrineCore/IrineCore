USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddEventFatigue]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 9
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-05-17
description : ??? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
2 = ??? ??? ???? ?? ?? ?? at 2010-10-25
5 = ??? ?? ?? ??? AM 9:00?? ?? at 2011-10-10
7 = TF_Split(), TF_NSplit() ??? ????? ?? by ??? at 2011-11-01
**/

CREATE PROCEDURE [dbo].[P_AddEventFatigue]
	  @inyTargetType tinyint --// 1=?? ??, 2=??? ??
	, @insFatigue smallint --// ??? ???
	, @vchCharacterIDs varchar(max) --// ,? ?????? ??? ID ???. ?? 500? / ?? ?? ?? ? NULL ?????. 
	, @nvcCharacterNames nvarchar(max) --// ,? ?????? ??? ?? ???. ?? 500? / ?? ?? ?? ? NULL ?????.
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @dt2Now datetime2(0), @inbCharacterID bigint = 0, @intLogTableSuffix int;

DECLARE @tblTemp table (CharacterID bigint NOT NULL);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	IF @inyTargetType = 1
	BEGIN
		WHILE 1 = 1
		BEGIN
			DELETE @tblTemp;

			SET @dt2Now = GETDATE();
			SET @intLogTableSuffix = dbo.FN_LogTableSuffix(@dt2Now);

			WITH C AS (
				SELECT TOP (500) CHS.CharacterID
				FROM dbo.Characters CH WITH (NOLOCK)
					INNER JOIN dbo.CharacterStatus CHS WITH (NOLOCK) ON CH.CharacterID = CHS.CharacterID
				WHERE CHS.CharacterID > @inbCharacterID
					AND CHS.EventFatigueGainDate < DATEADD(HOUR, 9, CAST(CAST(@dt2Now AS date) AS datetime2(0)))
					AND CH.DeleteFlag = CAST(0 AS bit)
				ORDER BY CHS.CharacterID
			)
			UPDATE dbo.CharacterStatus
			SET @inbCharacterID = C.CharacterID
				, EventFatigue = @insFatigue
				, EventFatigueGainDate = @dt2Now
			OUTPUT inserted.CharacterID INTO @tblTemp (CharacterID)
			FROM dbo.CharacterStatus CHS
				INNER JOIN C ON CHS.CharacterID = C.CharacterID;

			IF @@ROWCOUNT = 0
				BREAK;

			/**_# [FatigueLogs] ???? INSERT???.*/
			IF @intLogTableSuffix = 0
				INSERT dbo.FatigueLogs0 (LogDate, CharacterID, FatigueTypeCode, CurrentFatigue, ChangedFatigue, ChannelID, MapID)
				SELECT @dt2Now, CharacterID, 4, @insFatigue, @insFatigue, NULL, NULL
				FROM @tblTemp;

			ELSE IF @intLogTableSuffix = 1
				INSERT dbo.FatigueLogs1 (LogDate, CharacterID, FatigueTypeCode, CurrentFatigue, ChangedFatigue, ChannelID, MapID)
				SELECT @dt2Now, CharacterID, 4, @insFatigue, @insFatigue, NULL, NULL
				FROM @tblTemp;

			ELSE
				INSERT dbo.FatigueLogs2 (LogDate, CharacterID, FatigueTypeCode, CurrentFatigue, ChangedFatigue, ChannelID, MapID)
				SELECT @dt2Now, CharacterID, 4, @insFatigue, @insFatigue, NULL, NULL
				FROM @tblTemp;
		END
	END

	ELSE IF @inyTargetType = 2
	BEGIN
		SET @dt2Now = GETDATE();
		SET @intLogTableSuffix = dbo.FN_LogTableSuffix(@dt2Now);

		IF @vchCharacterIDs IS NOT NULL
			INSERT @tblTemp (CharacterID)
			SELECT DISTINCT CH.CharacterID
			FROM dbo.TF_Split(@vchCharacterIDs, ',') F
				INNER JOIN dbo.Characters CH WITH (NOLOCK) ON CH.CharacterID = CAST(F.string AS bigint)
				INNER JOIN dbo.CharacterStatus CHS WITH (NOLOCK) ON CH.CharacterID = CHS.CharacterID
			WHERE CHS.EventFatigueGainDate < DATEADD(HOUR, 9, CAST(CAST(@dt2Now AS date) AS datetime2(0))) AND CH.DeleteFlag = CAST(0 AS bit);

		ELSE
			INSERT @tblTemp (CharacterID)
			SELECT DISTINCT CH.CharacterID
			FROM dbo.Characters CH WITH (NOLOCK)
				INNER JOIN dbo.CharacterStatus CHS WITH (NOLOCK) ON CH.CharacterID = CHS.CharacterID
				INNER JOIN dbo.TF_NSplit(@vchCharacterIDs, N',') F ON CH.CharacterName = CAST(F.string AS nvarchar(30))
			WHERE CHS.EventFatigueGainDate < DATEADD(HOUR, 9, CAST(CAST(@dt2Now AS date) AS datetime2(0))) AND CH.DeleteFlag = CAST(0 AS bit);

		UPDATE dbo.CharacterStatus
		SET EventFatigue = @insFatigue
			, EventFatigueGainDate = @dt2Now
		FROM dbo.CharacterStatus CHS
			INNER JOIN @tblTemp C ON CHS.CharacterID = C.CharacterID;

		/**_# [FatigueLogs] ???? INSERT???.*/
		IF @intLogTableSuffix = 0
			INSERT dbo.FatigueLogs0 (LogDate, CharacterID, FatigueTypeCode, CurrentFatigue, ChangedFatigue, ChannelID, MapID)
			SELECT @dt2Now, CharacterID, 4, @insFatigue, @insFatigue, NULL, NULL
			FROM @tblTemp;

		ELSE IF @intLogTableSuffix = 1
			INSERT dbo.FatigueLogs1 (LogDate, CharacterID, FatigueTypeCode, CurrentFatigue, ChangedFatigue, ChannelID, MapID)
			SELECT @dt2Now, CharacterID, 4, @insFatigue, @insFatigue, NULL, NULL
			FROM @tblTemp;

		ELSE
			INSERT dbo.FatigueLogs2 (LogDate, CharacterID, FatigueTypeCode, CurrentFatigue, ChangedFatigue, ChannelID, MapID)
			SELECT @dt2Now, CharacterID, 4, @insFatigue, @insFatigue, NULL, NULL
			FROM @tblTemp;
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

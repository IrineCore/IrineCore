USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_SetDailyWeeklyMission]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 9
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-15
description : ??, ????? ? ???.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103102 = ???? ???? ????.
103126 = @vchMissionIDs ? ??
history :
3 = @inyMissionTypeCode ?? ?? (3=??? ??)
4 = TF_Split(), TF_NSplit() ??? ????? ?? by ??? at 2011-11-02
5 = MissionTypeCode ?? by ??? at 2012-04-06
6 = MissionTypeCode ???? ?? by ??? at 2012-12-11
8 = ???? ?? ??(?? ??? ?? ??ID? ??) by ??? at 2013-06-20
**/

CREATE PROCEDURE [dbo].[P_SetDailyWeeklyMission]
	  @inbCharacterID bigint --// ??? ID
	, @inyMissionTypeCode tinyint --// 1=????, 2=????, 3=?????, 4=?????, 5=???? ??, 6=???????, 7=??????, 8=????, 9=????
	, @vchMissionIDs varchar(8000) --// ,? ???? ?? ?? ID ???
	, @bitLongtermMission bit = NULL --// (optional) ???? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @tblMissionIDs table (MissionIndex tinyint NOT NULL PRIMARY KEY, MissionID int NOT NULL);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# @vchMissionIDs? ??? ???.*/
	INSERT @tblMissionIDs (MissionIndex, MissionID)
	SELECT seq - 1, CAST(string AS int)
	FROM dbo.TF_Split(@vchMissionIDs, ',')
	WHERE string > ''
	OPTION (KEEPFIXED PLAN);

	/**_# @vchMissionIDs ?? ??? ??? 103126? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103126;
		GOTO ErrorHandler;
	END

	/**_# [AssignMissions]???? UPSERT???.*/
	UPDATE dbo.AssignMissions
	SET AssignDate = GETDATE()
	WHERE CharacterID = @inbCharacterID AND MissionTypeCode = @inyMissionTypeCode;

	IF @@ROWCOUNT = 0
	BEGIN
		INSERT dbo.AssignMissions (CharacterID, MissionTypeCode, AssignDate)
		VALUES (@inbCharacterID, @inyMissionTypeCode, GETDATE())
	END

	/**_# [AssignMissionDetails]???? MERGE???.*/
	IF @bitLongtermMission = CAST (1 AS bit)
		WITH C1 AS (
			SELECT CharacterID, MissionTypeCode, MissionIndex, MissionID, MissionCounter, AchieveFlag
			FROM dbo.AssignMissionDetails
			WHERE CharacterID = @inbCharacterID AND MissionTypeCode = @inyMissionTypeCode
		)
		MERGE C1 AS T
		USING @tblMissionIDs AS S
		ON T.MissionIndex = S.MissionIndex
		WHEN MATCHED THEN
			UPDATE SET MissionID = S.MissionID, MissionCounter = CASE WHEN T.MissionID = S.MissionID THEN MissionCounter ELSE 0 END, AchieveFlag = CASE WHEN T.MissionID = S.MissionID THEN AchieveFlag ELSE 0 END
		WHEN NOT MATCHED BY TARGET THEN
			INSERT (CharacterID, MissionTypeCode, MissionIndex, MissionID, MissionCounter, AchieveFlag) VALUES (@inbCharacterID, @inyMissionTypeCode, S.MissionIndex, S.MissionID, 0, 0)
		WHEN NOT MATCHED BY SOURCE THEN
			DELETE;

	ELSE
		WITH C1 AS (
			SELECT CharacterID, MissionTypeCode, MissionIndex, MissionID, MissionCounter, AchieveFlag
			FROM dbo.AssignMissionDetails
			WHERE CharacterID = @inbCharacterID AND MissionTypeCode = @inyMissionTypeCode
		)
		MERGE C1 AS T
		USING @tblMissionIDs AS S
		ON T.MissionIndex = S.MissionIndex
		WHEN MATCHED THEN
			UPDATE SET MissionID = S.MissionID, MissionCounter = 0, AchieveFlag = 0
		WHEN NOT MATCHED BY TARGET THEN
			INSERT (CharacterID, MissionTypeCode, MissionIndex, MissionID, MissionCounter, AchieveFlag) VALUES (@inbCharacterID, @inyMissionTypeCode, S.MissionIndex, S.MissionID, 0, 0)
		WHEN NOT MATCHED BY SOURCE THEN
			DELETE;

END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 547 AND ERROR_MESSAGE() LIKE N'%FK_Characters_AssignMissions_CharacterID%'
		SET @intReturnValue = 103102;

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

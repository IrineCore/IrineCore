USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModGuildEXP]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 20
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2011-11-24
description : ?? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history
11~12 = ???? ??? ?? by ??? at 2012-01-03
18 = ?? ??? ?? ?????? ?? ??, ???? ???? ????? ?? by ??? at 2013-03-07
19 = ?? ?? ???  by ???  at 2013-05-23
20 = ?? ?? ???? ?? ?? ?? ?? at 2013-06-25
**/

CREATE PROCEDURE [dbo].[P_ModGuildEXP]
	  @intGuildID int --// ?? ID
	, @inyGuildPointType tinyint --//?? ??? ?? 1=???? ???, 2=?? ???, 3=?? ???(?????? ?? ?? ???? ??)
	, @intGuildPoint int --// ?? ??? ??? 
	, @intTodayMaxGuildPoint int --//?? ????? ???
    , @intGuildLevel int --//?? ? ?? ?? 
	, @intTotalGuildEXP int OUTPUT --// ?? ? ??? ? ?? ???
	, @intTodayGuildPoint int OUTPUT --// ?? ? ??? ?? ?? ???
	, @inbCharacterID bigint = NULL --// ?? ??? ?? ???ID
	, @bitIsMaxLevel bit = 0 --// ?? ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @intChangedGuildLevel int;
DECLARE @dt2Now datetime2(0) = GETDATE()
DECLARE @dt2Today datetime2(0) = CAST(@dt2Now AS date);
DECLARE @dt2StagePointLastModDate datetime2(0);
DECLARE @intDailyStagePoint int;
DECLARE @dt2MissionPointLastModDate datetime2(0);
DECLARE @intDailyMissionPoint int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	IF @inyGuildPointType = 1 -- ???? ???
	BEGIN
		IF @bitIsMaxLevel = 0
		BEGIN
			UPDATE dbo.Guilds	
			SET @intTodayGuildPoint = DailyStagePoint = 
					CASE
						WHEN StagePointLastModDate >= @dt2Today THEN
							CASE
								WHEN (DailyStagePoint + @intGuildPoint) >= @intTodayMaxGuildPoint THEN @intTodayMaxGuildPoint
								ELSE (DailyStagePoint + @intGuildPoint)
							END
						ELSE @intGuildPoint
					END
			  , @dt2StagePointLastModDate = StagePointLastModDate
			  , @intDailyStagePoint = DailyStagePoint
			  , @intTotalGuildEXP = TotalGuildEXP += @intTodayGuildPoint - CASE WHEN StagePointLastModDate >= @dt2Today THEN DailyStagePoint ELSE 0 END
			  , StagePointLastModDate = @dt2Now
			  , @intChangedGuildLevel = @intGuildLevel - GuildLevel
			  , GuildLevel = CASE WHEN @intChangedGuildLevel > 0 THEN CAST(@intGuildLevel AS tinyint) ELSE GuildLevel END 
			FROM dbo.Guilds WITH (INDEX(PK_Guilds), FORCESEEK)
			WHERE GuildID = @intGuildID
			OPTION (KEEPFIXED PLAN);
		END	
		ELSE
		BEGIN
			UPDATE dbo.Guilds	
			SET @intTodayGuildPoint = DailyStagePoint = 
					CASE
						WHEN StagePointLastModDate >= @dt2Today THEN
							CASE
								WHEN (DailyStagePoint + @intGuildPoint) >= @intTodayMaxGuildPoint THEN @intTodayMaxGuildPoint
								ELSE (DailyStagePoint + @intGuildPoint)
							END
						ELSE @intGuildPoint
					END
			  , @dt2StagePointLastModDate = StagePointLastModDate
			  , @intDailyStagePoint = DailyStagePoint
			  , @intTotalGuildEXP = TotalGuildEXP
			  , StagePointLastModDate = @dt2Now
			  , @intChangedGuildLevel = @intGuildLevel - GuildLevel
			  , GuildLevel = CASE WHEN @intChangedGuildLevel > 0 THEN CAST(@intGuildLevel AS tinyint) ELSE GuildLevel END 
			FROM dbo.Guilds WITH (INDEX(PK_Guilds), FORCESEEK)
			WHERE GuildID = @intGuildID
			OPTION (KEEPFIXED PLAN);
		END

		IF @inbCharacterID IS NOT NULL
		BEGIN
			UPDATE dbo.GuildMembers 
			SET GuildContributionPoint += @intTodayGuildPoint - CASE WHEN @dt2StagePointLastModDate >= @dt2Today THEN @intDailyStagePoint ELSE 0 END			
			WHERE CharacterID = @inbCharacterID;

			UPDATE dbo.WeeklyGuildContributionPoint 
			SET WeeklyGuildContributionPoint += @intTodayGuildPoint - CASE WHEN @dt2StagePointLastModDate >= @dt2Today THEN @intDailyStagePoint ELSE 0 END			
			WHERE CharacterID = @inbCharacterID;

			IF @@ROWCOUNT = 0
				INSERT dbo.WeeklyGuildContributionPoint (CharacterID, WeeklyGuildContributionPoint, GuildID)
				VALUES (@inbCharacterID, @intTodayGuildPoint - CASE WHEN @dt2StagePointLastModDate >= @dt2Today THEN @intDailyStagePoint ELSE 0 END, @intGuildID);

		END
	END
	ELSE IF @inyGuildPointType = 2 -- ?? ???
	BEGIN
		IF @bitIsMaxLevel = 0
		BEGIN
			UPDATE dbo.Guilds	
			SET @intTodayGuildPoint = DailyMissionPoint = 
					CASE
						WHEN MissionPointLastModDate >= @dt2Today THEN
							CASE 
								WHEN (DailyMissionPoint + @intGuildPoint) >= @intTodayMaxGuildPoint THEN @intTodayMaxGuildPoint 
								ELSE (DailyMissionPoint + @intGuildPoint) 
							END
						ELSE @intGuildPoint
					END
			  , @dt2MissionPointLastModDate = MissionPointLastModDate
			  , @intDailyMissionPoint = DailyMissionPoint
			  , @intTotalGuildEXP = TotalGuildEXP += @intTodayGuildPoint - CASE WHEN MissionPointLastModDate >= @dt2Today THEN DailyMissionPoint ELSE 0 END
			  , MissionPointLastModDate = @dt2Now
			  , @intChangedGuildLevel = @intGuildLevel - GuildLevel
			  , GuildLevel = CASE WHEN @intChangedGuildLevel > 0 THEN CAST(@intGuildLevel AS tinyint) ELSE GuildLevel END 
			FROM dbo.Guilds WITH (INDEX(PK_Guilds), FORCESEEK)
			WHERE GuildID = @intGuildID
			OPTION (KEEPFIXED PLAN);
		END
		ELSE
		BEGIN		
			UPDATE dbo.Guilds	
			SET @intTodayGuildPoint = DailyMissionPoint = 
					CASE
						WHEN MissionPointLastModDate >= @dt2Today THEN
							CASE 
								WHEN (DailyMissionPoint + @intGuildPoint) >= @intTodayMaxGuildPoint THEN @intTodayMaxGuildPoint 
								ELSE (DailyMissionPoint + @intGuildPoint) 
							END
						ELSE @intGuildPoint
					END
			  , @dt2MissionPointLastModDate = MissionPointLastModDate
			  , @intDailyMissionPoint = DailyMissionPoint
			  , @intTotalGuildEXP = TotalGuildEXP
			  , MissionPointLastModDate = @dt2Now
			  , @intChangedGuildLevel = @intGuildLevel - GuildLevel
			  , GuildLevel = CASE WHEN @intChangedGuildLevel > 0 THEN CAST(@intGuildLevel AS tinyint) ELSE GuildLevel END 
			FROM dbo.Guilds WITH (INDEX(PK_Guilds), FORCESEEK)
			WHERE GuildID = @intGuildID
			OPTION (KEEPFIXED PLAN);
		END
		
		IF @inbCharacterID IS NOT NULL
		BEGIN
			UPDATE dbo.GuildMembers 
			SET GuildContributionPoint += @intTodayGuildPoint - CASE WHEN @dt2MissionPointLastModDate >= @dt2Today THEN @intDailyMissionPoint ELSE 0 END			
			WHERE CharacterID = @inbCharacterID;

			UPDATE dbo.WeeklyGuildContributionPoint 
			SET WeeklyGuildContributionPoint += @intTodayGuildPoint - CASE WHEN @dt2MissionPointLastModDate >= @dt2Today THEN @intDailyMissionPoint ELSE 0 END			
			WHERE CharacterID = @inbCharacterID;

			IF @@ROWCOUNT = 0
				INSERT dbo.WeeklyGuildContributionPoint (CharacterID, WeeklyGuildContributionPoint, GuildID)
				VALUES (@inbCharacterID, @intTodayGuildPoint - CASE WHEN @dt2MissionPointLastModDate >= @dt2Today THEN @intDailyMissionPoint ELSE 0 END, @intGuildID);

		END
				
	END
	ELSE IF @inyGuildPointType = 3 -- ?? ???
	BEGIN	
		IF @bitIsMaxLevel = 0
		BEGIN
			UPDATE dbo.Guilds	
			SET @intTotalGuildEXP = TotalGuildEXP += @intGuildPoint
			  , @intTodayGuildPoint = @intGuildPoint
			  , @intChangedGuildLevel = @intGuildLevel - GuildLevel
			  , GuildLevel = CASE WHEN @intChangedGuildLevel > 0 THEN CAST(@intGuildLevel AS tinyint) ELSE GuildLevel END 
			WHERE GuildID = @intGuildID;
		END
		ELSE
		BEGIN
			UPDATE dbo.Guilds	
			SET @intTotalGuildEXP = TotalGuildEXP
			  , @intTodayGuildPoint = @intGuildPoint
			  , @intChangedGuildLevel = @intGuildLevel - GuildLevel
			  , GuildLevel = CASE WHEN @intChangedGuildLevel > 0 THEN CAST(@intGuildLevel AS tinyint) ELSE GuildLevel END 
			WHERE GuildID = @intGuildID;
		END

		IF @inbCharacterID IS NOT NULL
		BEGIN
			UPDATE dbo.GuildMembers 
			SET GuildContributionPoint += @intGuildPoint		
			WHERE CharacterID = @inbCharacterID;
			UPDATE dbo.WeeklyGuildContributionPoint 
			SET WeeklyGuildContributionPoint += @intGuildPoint		
			WHERE CharacterID = @inbCharacterID;

			IF @@ROWCOUNT = 0
				INSERT dbo.WeeklyGuildContributionPoint (CharacterID, WeeklyGuildContributionPoint, GuildID)
				VALUES (@inbCharacterID, @intGuildPoint, @intGuildID);

		END				
	END
	/**_# [GuildHistory] ???? INSERT???.*/
	IF @intChangedGuildLevel > 0 
	BEGIN
		INSERT dbo.GuildHistory (GuildID, GuildHistoryCode, RegisterDate, GuildLevel)
		VALUES (@intGuildID, 2, @dt2Now, CAST(@intGuildLevel AS tinyint));	

		UPDATE dbo.GuildRecruitments
		SET GuildLevel = CAST(@intGuildLevel AS tinyint)
		WHERE GuildID = @intGuildID;
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

USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_RewardAlteiaRanking]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 12
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-11-09
description : ??????? ?? ??? ???.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_RewardAlteiaRanking]
	  @dt2BasisDate datetime2(0) = NULL
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	  , @dtsSendDate smalldatetime = CASE WHEN @dt2BasisDate IS NULL THEN GETDATE() ELSE CAST(@dt2BasisDate as smalldatetime) END
      , @i int = 1
	  , @j int
      , @intRanking int
      , @inbCharacterID bigint
	  , @intGuildID int
      , @intMailScheduleID int
	  , @dt2AlteiaStartDate datetime2(0)
	  , @dt2AlteiaEndDate datetime2(0);

DECLARE @tblGoldKeyRanker TABLE(
	Ranking int,
	CharacterID bigint
)
DECLARE @tblPlayTimeRanker TABLE(
	Ranking int,
	CharacterID bigint
)
DECLARE @tblGuildRanker TABLE(
	Ranking int,
	GuildID int
)

DECLARE @tblReward table
(
	RewardCode tinyint, Ranking int, ItemID int, SenderID varchar(10), SubjectID varchar(10), ContentID varchar(10)
)

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT @dt2AlteiaStartDate = AlteiaStartDate, @dt2AlteiaEndDate = DATEADD(SECOND, -1, CAST(CAST(DATEADD(DAY, 1, AlteiaEndDate) AS date) as datetime2(0)))
	FROM dbo.AlteiaSchedules WITH(NOLOCK)

	IF @dtsSendDate BETWEEN @dt2AlteiaStartDate AND @dt2AlteiaEndDate
	BEGIN
		/**_# ?????? ??? ??? ??? INSERT ???.*/
		INSERT @tblReward(RewardCode, Ranking, ItemID , SenderID , SubjectID , ContentID)
		SELECT 1, 1, 805307647, 1000009487, 1000056271, 1000056284 UNION ALL
		SELECT 1, 1, 805307657, NULL, NULL, NULL UNION ALL
		SELECT 1, 2, 805307648, 1000009487, 1000056272, 1000056285 UNION ALL
		SELECT 1, 2, 805307658, NULL, NULL, NULL UNION ALL
		SELECT 1, 3, 805307649, 1000009487, 1000056273, 1000056286 UNION ALL
		SELECT 1, 3, 805307659, NULL, NULL, NULL UNION ALL
		SELECT 1, 4, 805307650, 1000009487, 1000056274, 1000056287 UNION ALL
		SELECT 1, 4, 805307660, NULL, NULL, NULL UNION ALL
		SELECT 1, 5, 805307651, 1000009487, 1000056275, 1000056288 UNION ALL
		SELECT 1, 5, 805307661, NULL, NULL, NULL UNION ALL
		SELECT 1, 6, 805307652, 1000009487, 1000056276, 1000056289 UNION ALL
		SELECT 1, 6, 805307662, NULL, NULL, NULL UNION ALL
		SELECT 1, 7, 805307653, 1000009487, 1000056277, 1000056290 UNION ALL
		SELECT 1, 7, 805307663, NULL, NULL, NULL UNION ALL
		SELECT 1, 8, 805307654, 1000009487, 1000056278, 1000056291 UNION ALL
		SELECT 1, 8, 805307664, NULL, NULL, NULL UNION ALL
		SELECT 1, 9, 805307655, 1000009487, 1000056279, 1000056292 UNION ALL
		SELECT 1, 9, 805307665, NULL, NULL, NULL UNION ALL
		SELECT 1, 10, 805307656, 1000009487, 1000056280, 1000056293 UNION ALL
		SELECT 1, 10, 805307666, NULL, NULL, NULL UNION ALL
		SELECT 2, 1, 805307647, 1000009487, 1000056271, 1000056294 UNION ALL
		SELECT 2, 1, 805307657, NULL, NULL, NULL UNION ALL
		SELECT 2, 2, 805307648, 1000009487, 1000056272, 1000056295 UNION ALL
		SELECT 2, 2, 805307658, NULL, NULL, NULL UNION ALL
		SELECT 2, 3, 805307649, 1000009487, 1000056273, 1000056296 UNION ALL
		SELECT 2, 3, 805307659, NULL, NULL, NULL UNION ALL
		SELECT 2, 4, 805307650, 1000009487, 1000056274, 1000056297 UNION ALL
		SELECT 2, 4, 805307660, NULL, NULL, NULL UNION ALL
		SELECT 2, 5, 805307651, 1000009487, 1000056275, 1000056298 UNION ALL
		SELECT 2, 5, 805307661, NULL, NULL, NULL UNION ALL
		SELECT 2, 6, 805307652, 1000009487, 1000056276, 1000056299 UNION ALL
		SELECT 2, 6, 805307662, NULL, NULL, NULL UNION ALL
		SELECT 2, 7, 805307653, 1000009487, 1000056277, 1000056300 UNION ALL
		SELECT 2, 7, 805307663, NULL, NULL, NULL UNION ALL
		SELECT 2, 8, 805307654, 1000009487, 1000056278, 1000056301 UNION ALL
		SELECT 2, 8, 805307664, NULL, NULL, NULL UNION ALL
		SELECT 2, 9, 805307655, 1000009487, 1000056279, 1000056302 UNION ALL
		SELECT 2, 9, 805307665, NULL, NULL, NULL UNION ALL
		SELECT 2, 10, 805307656, 1000009487, 1000056280, 1000056303 UNION ALL
		SELECT 2, 10, 805307666, NULL, NULL, NULL UNION ALL
		SELECT 3, 1, 805307585, 1000009487, 1000056268, 1000056281 UNION ALL
		SELECT 3, 1, 805307761, NULL, NULL, NULL UNION ALL
		SELECT 3, 2, 805307586, 1000009487, 1000056269, 1000056282 UNION ALL
		SELECT 3, 2, 805307762, NULL, NULL, NULL UNION ALL
		SELECT 3, 3, 805307587, 1000009487, 1000056270, 1000056283 UNION ALL
		SELECT 3, 3, 805307763, NULL, NULL, NULL;

		INSERT @tblGoldKeyRanker (Ranking, CharacterID)
		SELECT TOP 10 ROW_NUMBER() OVER(ORDER BY AcquireGoldKeyCount DESC, GoldKeyRegisterDate) AS Ranking ,CharacterID
		FROM dbo.PlayAlteia WITH(NOLOCK);

		INSERT @tblPlayTimeRanker (Ranking, CharacterID)
		SELECT TOP 10 ROW_NUMBER() OVER(ORDER BY PlayTime, PlayTimeRegisterDate) AS Ranking, CharacterID
		FROM dbo.PlayAlteia WITH(NOLOCK);	

		/**_# ?? ???? ?? ???? ?????.(? ???? ????? ???? ??? ??)*/
		WHILE @i <= 10
		BEGIN
			SELECT @intRanking = Ranking, @inbCharacterID = CharacterID
			FROM @tblGoldKeyRanker
			WHERE Ranking = @i;

			IF @@ROWCOUNT > 0
			BEGIN 
				INSERT dbo.MailSchedules (SendDate, SenderName, [Subject], Content, AttachCoin, MailScheduleExpireDate, MailTypeCode, NewFlag, DeleteFlag)
				SELECT @dtsSendDate, SenderID, SubjectID, ContentID, 0, DATEADD(DAY, 7, @dtsSendDate), 1, 1, 0
				FROM @tblReward
				WHERE RewardCode = 1
				  AND Ranking = @intRanking 
				  AND SubjectID IS NOT NULL;

				SET @intMailScheduleID = SCOPE_IDENTITY();

				INSERT dbo.AttachItemsForMailSchedule(MailScheduleID, ItemID, ItemCount, ItemDurability, RandomSeed, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, EternityFlag, ItemLifespan)
				SELECT @intMailScheduleID, ItemID, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0
				FROM @tblReward
				WHERE RewardCode = 1
				  AND Ranking = @intRanking;

				INSERT dbo.MailSchedulesTarget(MailScheduleID, CharacterID, DeleteFlag)
				SELECT @intMailScheduleID, @inbCharacterID, 0;
			END
			-----------------------------------------------------------------------------------------
			SELECT @intRanking = Ranking, @inbCharacterID = CharacterID
			FROM @tblPlayTimeRanker
			WHERE Ranking = @i;

			IF @@ROWCOUNT > 0
			BEGIN 
				INSERT dbo.MailSchedules (SendDate, SenderName, [Subject], Content, AttachCoin, MailScheduleExpireDate, MailTypeCode, NewFlag, DeleteFlag)
				SELECT @dtsSendDate, SenderID, SubjectID, ContentID, 0, DATEADD(DAY, 7, @dtsSendDate), 1, 1, 0
				FROM @tblReward
				WHERE RewardCode = 2
				  AND Ranking = @intRanking 
				  AND SubjectID IS NOT NULL;

				SET @intMailScheduleID = SCOPE_IDENTITY();

				INSERT dbo.AttachItemsForMailSchedule(MailScheduleID, ItemID, ItemCount, ItemDurability, RandomSeed, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, EternityFlag, ItemLifespan)
				SELECT @intMailScheduleID, ItemID, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0
				FROM @tblReward
				WHERE RewardCode = 2
				  AND Ranking = @intRanking;

				INSERT dbo.MailSchedulesTarget(MailScheduleID, CharacterID, DeleteFlag)
				SELECT @intMailScheduleID, @inbCharacterID, 0;
			END
			SET @i += 1;
		END
		
		/**_# ??? ??? ?????.*/
		INSERT dbo.RewardAlteiaRanker(RewardDate, RewardCode, Ranking, CharacterID)
		SELECT @dtsSendDate, 1, Ranking, CharacterID
		FROM @tblGoldKeyRanker
		UNION ALL
		SELECT @dtsSendDate, 2, Ranking, CharacterID
		FROM @tblPlayTimeRanker;

		/**_# ?????? ??? ???.*/
		TRUNCATE TABLE dbo.PlayAlteia;

		/**_# ??? ??? ???? ????? ???.*/
		IF CAST(@dt2AlteiaEndDate AS date) = CAST(@dtsSendDate AS date)
		BEGIN
			INSERT @tblGuildRanker (Ranking, GuildID)
			SELECT TOP 3 ROW_NUMBER() OVER(ORDER BY AcquireGoldKeyCount DESC, UpdateDate) AS Ranking, GuildID
			FROM dbo.AlteiaGuildRank WITH(NOLOCK);	

			SET @i = 1

			WHILE @i <= 3
			BEGIN
				SELECT @intRanking = Ranking, @intGuildID = GuildID
				FROM @tblGuildRanker
				WHERE Ranking = @i;

				IF @@ROWCOUNT > 0
				BEGIN 
					INSERT dbo.MailSchedules (SendDate, SenderName, [Subject], Content, AttachCoin, MailScheduleExpireDate, MailTypeCode, NewFlag, DeleteFlag)
					SELECT @dtsSendDate, SenderID, SubjectID, ContentID, 0, DATEADD(DAY, 7, @dtsSendDate), 1, 1, 0
					FROM @tblReward
					WHERE RewardCode = 3
					  AND Ranking = @intRanking 
					  AND SubjectID IS NOT NULL;

					SET @intMailScheduleID = SCOPE_IDENTITY();

					INSERT dbo.AttachItemsForMailSchedule(MailScheduleID, ItemID, ItemCount, ItemDurability, RandomSeed, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, EternityFlag, ItemLifespan)
					SELECT @intMailScheduleID, ItemID, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0
					FROM @tblReward
					WHERE RewardCode = 3
					  AND Ranking = @intRanking;

					INSERT dbo.MailSchedulesTarget(MailScheduleID, CharacterID, DeleteFlag)
					SELECT @intMailScheduleID, CharacterID, 0
					FROM dbo.GuildMembers WITH(NOLOCK)
					WHERE GuildID = @intGuildID;
				END

				SET @i += 1;
			END

			INSERT dbo.RewardAlteiaRanker(RewardDate, RewardCode, Ranking, GuildID)
			SELECT @dtsSendDate, 3, Ranking, GuildID
			FROM @tblGuildRanker;
			
			/**_# ?? ??? ??? ???.*/
			TRUNCATE TABLE dbo.AlteiaGuildRank;
		END
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

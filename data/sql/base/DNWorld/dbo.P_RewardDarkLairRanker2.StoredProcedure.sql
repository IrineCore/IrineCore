USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_RewardDarkLairRanker2]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 16
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-11-19
description : ??? ????? ? ????? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
9 = ?? ??? ? ??? ???? ??? ?? ? ??? ?? by ??? at 2013-01-07
11~13 = DB????? ?? ???, ?????? ???? ????? ?? by ??? at 2013-02-25
**/

CREATE PROCEDURE [dbo].[P_RewardDarkLairRanker2]
	@dt2BasisDate datetime2(0) = NULL
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	  , @dtsSendDate smalldatetime = CASE WHEN @dt2BasisDate IS NULL THEN GETDATE() ELSE CAST(@dt2BasisDate AS smalldatetime) END
      , @i int = 1
      , @intRanking int
      , @inbCharacterID bigint
      , @intMailScheduleID int
	  , @stmt nvarchar(max) = N'';


DECLARE @tblReward TABLE
(
	Ranking int, ItemID int, SenderID varchar(10), SubjectID varchar(10), ContentID varchar(10)
)

DECLARE @tblRewardTarget TABLE
(
	Seq int identity(1, 1), Ranking int, PartyUserCount int, CharacterID bigint
)
DECLARE @tblRewardTarget2 TABLE
(
	Seq int identity(1, 1), Ranking int, PartyUserCount int, CharacterID bigint
)

DECLARE @tblTableIndexInfo TABLE
(
	name sysname NULL,
	type tinyint NULL,
	is_primary_key bit NULL,
	is_unique bit NULL,
	TableName sysname NULL, 
	index_key nvarchar(100) NULL,
	included_columns nvarchar(100) NULL,
	filter_definition nvarchar(200) NULL
)


/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# ?????? ??? ??? ??? INSERT ???.*/
	INSERT @tblReward(Ranking, ItemID , SenderID , SubjectID , ContentID)
	SELECT 1, 268442067, 1000009487, 1000050071, 1000050081 UNION ALL
	SELECT 1, 536872085, NULL, NULL, NULL UNION ALL
	SELECT 2, 268442068, 1000009487, 1000050072, 1000050081 UNION ALL
	SELECT 2, 536872086, NULL, NULL, NULL UNION ALL
	SELECT 3, 268442069, 1000009487, 1000050073, 1000050081 UNION ALL
	SELECT 3, 536872087, NULL, NULL, NULL UNION ALL
	SELECT 4, 268442070, 1000009487, 1000050074, 1000050081 UNION ALL
	SELECT 5, 268442071, 1000009487, 1000050075, 1000050081 UNION ALL
	SELECT 6, 268442072, 1000009487, 1000050076, 1000050081 UNION ALL
	SELECT 7, 268442073, 1000009487, 1000050077, 1000050081 UNION ALL
	SELECT 8, 268442074, 1000009487, 1000050078, 1000050081 UNION ALL
	SELECT 9, 268442075, 1000009487, 1000050079, 1000050081 UNION ALL
	SELECT 10, 268442076, 1000009487, 1000050080, 1000050081;

	/**_# ???? ??? ? ?? 10??? ??? ?? ?????? Insert???.*/
	WITH CTE AS	
	(
		SELECT RANK() OVER (PARTITION BY PartyUserCount ORDER BY PlayRound DESC, PlayTime, DarkLairRankingID) AS Ranking, PartyUserCount, DarkLairRankingID
		FROM dbo.DarkLairRanking WITH (NOLOCK, INDEX(IX_NNIF_DarkLairRanking_MapID_PartyUserCount_PlayRound_PlayTime), FORCESEEK)
		WHERE MapID = 1000012601 AND BestFlag = CAST(1 AS bit) AND BlockFlag = CAST(0 AS bit)
	)
	INSERT @tblRewardTarget (Ranking, PartyUserCount, CharacterID)
	SELECT A.Ranking, A.PartyUserCount, B.CharacterID
	FROM CTE A 
		INNER JOIN dbo.DarkLairPartyMembers B WITH(NOLOCK) ON A.DarkLairRankingID = B.DarkLairRankingID
	WHERE A.Ranking <= 10
	OPTION (KEEPFIXED PLAN);

	/**_# 1??? 10??? ???? ChallengeDarkLairRewardCharacters? ?????. */
	INSERT dbo.ChallengeDarkLairRewardCharacters (BasisDate, Ranking, PartyMemberCount, CharacterID, RewardFlag)
	SELECT DATEADD(MONTH, -1, @dtsSendDate), Ranking, PartyUserCount, CharacterID, 0
	FROM @tblRewardTarget;

	/**_# ?? ??? ? ??? ???? ??? ?? ? ??? ??(?? ?? ??? ?? ? ??)*/
	WITH CTE AS(
		SELECT ROW_NUMBER() OVER(PARTITION BY PartyUserCount, CharacterID ORDER BY Ranking) AS DuplicationSeq
		FROM @tblRewardTarget
	)
	DELETE CTE	
	WHERE DuplicationSeq > 1;

	INSERT @tblRewardTarget2 (Ranking, PartyUserCount, CharacterID)
	SELECT Ranking, PartyUserCount, CharacterID
	FROM @tblRewardTarget;

	/**_# ?? ???? ?? ???? ?????.(? ???? ????? ???? ??? ??)*/
	WHILE 1=1
	BEGIN
		SELECT @intRanking = Ranking, @inbCharacterID = CharacterID
		FROM @tblRewardTarget2
		WHERE Seq = @i;
		
		IF @@ROWCOUNT = 0
			BREAK;
		INSERT dbo.MailSchedules (SendDate, SenderName, [Subject], Content, AttachCoin, MailScheduleExpireDate, MailTypeCode, NewFlag, DeleteFlag)
		SELECT @dtsSendDate, SenderID, SubjectID, ContentID, 0, DATEADD(DAY, 7, @dtsSendDate), 1, 1, 0
		FROM @tblReward
		WHERE Ranking = @intRanking 
          AND SubjectID IS NOT NULL;

		SET @intMailScheduleID = SCOPE_IDENTITY();
		INSERT dbo.AttachItemsForMailSchedule(MailScheduleID, ItemID, ItemCount, ItemDurability, RandomSeed, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, EternityFlag, ItemLifespan)
		SELECT @intMailScheduleID, ItemID, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0
		FROM @tblReward
		WHERE Ranking = @intRanking;

		INSERT dbo.MailSchedulesTarget(MailScheduleID, CharacterID, DeleteFlag)
		SELECT @intMailScheduleID, @inbCharacterID, 0

		SET @i += 1;
	END

	IF OBJECT_ID('DarkLairRankingTemp') IS NOT NULL
		DROP TABLE dbo.DarkLairRankingTemp;

	IF OBJECT_ID('DarkLairPartyMembersTemp') IS NOT NULL
		DROP TABLE dbo.DarkLairPartyMembersTemp;

	IF OBJECT_ID('DarkLairPersonalBestTemp') IS NOT NULL
		DROP TABLE dbo.DarkLairPersonalBestTemp;
	
	SELECT DarkLairRankingID, MapID, PartyUserCount, PlayRound, PlayTime, PartyName, RegisterDate, BestFlag, 1 AS BlockFlag
	INTO dbo.DarkLairRankingTemp
	FROM dbo.DarkLairRanking WITH(NOLOCK)
	WHERE RegisterDate BETWEEN CAST(@dtsSendDate AS date) AND @dtsSendDate;

	SELECT A.DarkLairRankingID, CharacterID, CharacterLevel, JobCode, CompleteFlag
	INTO dbo.DarkLairPartyMembersTemp
	FROM dbo.DarkLairRankingTemp A WITH(NOLOCK)
		INNER JOIN dbo.DarkLairPartyMembers B WITH(NOLOCK) ON A.DarkLairRankingID = B.DarkLairRankingID

	SELECT CharacterID, B.MapID, B.PartyUserCount, A.DarkLairRankingID 
	INTO dbo.DarkLairPersonalBestTemp
	FROM dbo.DarkLairRankingTemp A WITH(NOLOCK)
		INNER JOIN dbo.DarkLairPersonalBest B WITH(NOLOCK) ON A.DarkLairRankingID = B.DarkLairRankingID
	
	INSERT @tblTableIndexInfo (name, type, is_primary_key, is_unique, TableName, index_key, included_columns, filter_definition)
	SELECT i.name, i.type, i.is_primary_key, i.is_unique, OBJECT_NAME(o.object_id) AS TableName
	,'index_keys' = 
			(SELECT 
				   STUFF((SELECT ',' + CASE WHEN ii.is_descending_key = 1 THEN ic.name + ' DESC' ELSE ic.name END 
						  FROM sys.index_columns ii WITH(NOLOCK)
							INNER JOIN sys.columns ic WITH(NOLOCK) ON ii.column_id = ic.column_id AND ii.object_id = ic.object_id
						  WHERE ii.index_id = i.index_id 
							AND ii.object_id = i.object_id
							AND ii.is_included_column = 0
						  ORDER BY ii.key_ordinal ASC 
						  FOR XML PATH('') 
						  ) ,1,1,'')
			)
	,'included_columns' = 
			(SELECT 
				   STUFF((SELECT ',' + ic.name
						  FROM sys.index_columns ii WITH(NOLOCK) 
							INNER JOIN sys.columns ic WITH(NOLOCK) ON ii.column_id = ic.column_id AND ii.object_id = ic.object_id
						  WHERE ii.index_id = i.index_id 
							AND ii.object_id = i.object_id
							AND ii.is_included_column = 1
						  FOR XML PATH('') 
						  ) ,1,1,'')
			)
	,'filter_definition' = i.filter_definition
	FROM sys.indexes i WITH(NOLOCK)
		INNER JOIN sys.objects o WITH(NOLOCK) ON o.object_id = i.object_id AND o.type = 'U'
	WHERE o.name IN ('DarkLairRanking', 'DarkLairPartyMembers', 'DarkLairPersonalBest')

	SELECT @stmt += 'ALTER TABLE dbo.' + TableName + ' ADD CONSTRAINT ' + name + ' PRIMARY KEY (' + index_key + ')' + CHAR(13) + CHAR(10)
	FROM @tblTableIndexInfo
	WHERE is_primary_key = 1;
	SELECT @stmt += 'CREATE ' + CASE WHEN is_unique = 1 THEN 'UNIQUE ' ELSE '' END + CASE WHEN type = 1 THEN 'CLUSTERD' WHEN type = 2 THEN 'NONCLUSTERED' END + ' INDEX ' + name + ' ON dbo.' + TableName + '(' + index_key + ')' + CASE WHEN included_columns IS NOT NULL THEN ' INCLUDE (' + included_columns + ')' ELSE '' END + CASE WHEN filter_definition IS NOT NULL THEN ' WHERE ' + filter_definition ELSE '' END + CHAR(13) + CHAR(10)
	FROM @tblTableIndexInfo
	WHERE is_primary_key = 0;

	SELECT @stmt += 'ALTER TABLE dbo.' + o.name + ' ADD CONSTRAINT ' + ck.name + ' CHECK ' + ck.definition + CHAR(13) + CHAR(10)
	FROM sys.check_constraints ck WITH(NOLOCK) 
		INNER JOIN sys.objects o WITH(NOLOCK) ON ck.parent_object_id = o.object_id
		INNER JOIN sys.columns c WITH(NOLOCK) ON c.object_id = o.object_id and ck.parent_column_id = c.column_id
	WHERE o.name IN ('DarkLairRanking', 'DarkLairPartyMembers', 'DarkLairPersonalBest')

	SELECT @stmt += 'ALTER TABLE dbo.' + o.name + ' ADD CONSTRAINT ' + df.name + ' DEFAULT ' + df.definition + ' FOR ' + c.name  + CHAR(13) + CHAR(10)
	FROM sys.default_constraints df WITH(NOLOCK)
		INNER JOIN sys.objects o WITH(NOLOCK) ON o.object_id = df.parent_object_id
		INNER JOIN sys.columns c WITH(NOLOCK) ON c.object_id = o.object_id and df.parent_column_id = c.column_id
	WHERE o.name IN ('DarkLairRanking', 'DarkLairPartyMembers', 'DarkLairPersonalBest')

	SELECT @stmt += 'ALTER TABLE dbo.' + OBJECT_NAME(FK.parent_object_id) + ' ADD CONSTRAINT ' + FK.name + ' FOREIGN KEY (' + S.columnNames + ') REFERENCES dbo.' + OBJECT_NAME(FK.referenced_object_id) + ' (' + S.columnNames + ')' + CHAR(13) + CHAR(10)
	  FROM sys.foreign_keys FK
		INNER JOIN (
		  SELECT FKC2.constraint_object_id, STUFF((
			SELECT N',' + COL.name
			FROM sys.foreign_key_columns FKC
			  INNER JOIN sys.columns COL ON FKC.parent_object_id = COL.[object_id] AND FKC.parent_column_id = COL.column_id
			WHERE FKC.constraint_object_id = FKC2.constraint_object_id
			ORDER BY COL.column_id
			FOR XML PATH('')
		  ), 1, 1, N'') AS columnNames
		  FROM sys.foreign_key_columns FKC2
		  GROUP BY FKC2.constraint_object_id
		) S ON FK.[object_id] = S.constraint_object_id
	WHERE FK.parent_object_id IN (object_id('DarkLairRanking'), object_id('DarkLairPartyMembers'), object_id('DarkLairPersonalBest'));

	DROP TABLE dbo.DarkLairPartyMembers;
	DROP TABLE dbo.DarkLairPersonalBest;
	DROP TABLE dbo.DarkLairRanking;

	EXEC sp_rename N'DarkLairRankingTemp', N'DarkLairRanking'
	EXEC sp_rename N'DarkLairPartyMembersTemp', N'DarkLairPartyMembers'
	EXEC sp_rename N'DarkLairPersonalBestTemp', N'DarkLairPersonalBest'

	EXEC (@stmt);

	/**_# ??? ?? ?? ??? ?????. */
	UPDATE dbo.ChallengeDarkLairRewardCharacters
	SET RewardFlag = 1
	FROM dbo.ChallengeDarkLairRewardCharacters A
		INNER JOIN @tblRewardTarget2 B ON A.Ranking = B.Ranking AND A.PartyMemberCount = B.PartyUserCount AND A.CharacterID = B.CharacterID
	WHERE A.BasisDate = CAST(DATEADD(MONTH, -1, @dtsSendDate) AS date);

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

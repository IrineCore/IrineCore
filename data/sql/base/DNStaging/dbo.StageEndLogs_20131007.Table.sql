USE [dnstaging]
GO
/****** Object:  Table [dbo].[StageEndLogs_20131007]    Script Date: 11/14/2026 9:00:07 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[StageEndLogs_20131007](
	[StageStartDate] [datetime2](0) NOT NULL,
	[StageEndDate] [datetime2](0) NULL,
	[StageStartID] [bigint] NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[JobCode] [tinyint] NOT NULL,
	[CharacterLevel] [tinyint] NOT NULL,
	[ClearFlag] [bit] NOT NULL,
	[ClearGradeCode] [tinyint] NULL,
	[MaxComboCount] [int] NULL,
	[BossKillCount] [tinyint] NULL,
	[RewardCharacterExp] [int] NULL,
	[BonusCharacterExp] [int] NULL,
	[ClearPoint] [int] NULL,
	[BonusClearPoint] [int] NULL,
	[ClearTime] [int] NULL,
	[RewardBoxCode] [tinyint] NULL,
	[RewardItemSerial] [bigint] NULL,
	[RewardItemID] [int] NULL,
	[PartyStartID] [bigint] NULL,
	[PartySize] [tinyint] NOT NULL,
	[MapID] [int] NOT NULL,
	[DifficultyCode] [tinyint] NOT NULL,
	[WorldID] [int] NULL,
	[RewardFailReason] [int] NULL,
	[ClearRound] [tinyint] NULL,
	[TeacherPupilFlag] [bit] NULL,
	[NestFatigue] [smallint] NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_StageEndLogs_20131007_StageStartDate]    Script Date: 11/14/2026 9:00:07 AM ******/
CREATE CLUSTERED INDEX [IX_NC_StageEndLogs_20131007_StageStartDate] ON [dbo].[StageEndLogs_20131007]
(
	[StageStartDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_StageEndLogs_20131007_CharacterID]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_StageEndLogs_20131007_CharacterID] ON [dbo].[StageEndLogs_20131007]
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_StageEndLogs_20131007_RewardItemSerial]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_StageEndLogs_20131007_RewardItemSerial] ON [dbo].[StageEndLogs_20131007]
(
	[RewardItemSerial] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_StageEndLogs_20131007_StageStartID]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_StageEndLogs_20131007_StageStartID] ON [dbo].[StageEndLogs_20131007]
(
	[StageStartID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
ALTER TABLE [dbo].[StageEndLogs_20131007]  WITH CHECK ADD  CONSTRAINT [CK_StageEndLogs_20131007_StageStartDate] CHECK  (([StageStartDate]>='2013-10-07' AND [StageStartDate]<'2013-10-08'))
GO
ALTER TABLE [dbo].[StageEndLogs_20131007] CHECK CONSTRAINT [CK_StageEndLogs_20131007_StageStartDate]
GO

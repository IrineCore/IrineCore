USE [dnstaging]
GO
/****** Object:  Table [dbo].[StageEndLogs]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[StageEndLogs](
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

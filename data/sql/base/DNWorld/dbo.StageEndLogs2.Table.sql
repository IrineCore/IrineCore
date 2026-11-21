USE [dnworld]
GO
/****** Object:  Table [dbo].[StageEndLogs2]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[StageEndLogs2](
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
	[ClearRound] [tinyint] NULL,
	[TeacherPupilFlag] [bit] NULL,
	[NestFatigue] [smallint] NULL
) ON [DataFG1]
GO
/****** Object:  Index [IX_NN_StageEndLogs2_StageStartID_CharacterID]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_StageEndLogs2_StageStartID_CharacterID] ON [dbo].[StageEndLogs2]
(
	[StageStartID] ASC,
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO

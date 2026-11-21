USE [dnworld]
GO
/****** Object:  Table [dbo].[NestDeathLogs2]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[NestDeathLogs2](
	[LogDate] [datetime2](0) NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[MosterCode] [int] NOT NULL,
	[MonsterSkillCode] [int] NOT NULL,
	[StageStartID] [bigint] NULL,
	[CharacterLevel] [tinyint] NULL,
	[JobCode] [tinyint] NULL
) ON [DataFG1]
GO
/****** Object:  Index [IX_NC_NestDeathLogs2_LogDate]    Script Date: 11/14/2026 8:54:16 AM ******/
CREATE CLUSTERED INDEX [IX_NC_NestDeathLogs2_LogDate] ON [dbo].[NestDeathLogs2]
(
	[LogDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
GO

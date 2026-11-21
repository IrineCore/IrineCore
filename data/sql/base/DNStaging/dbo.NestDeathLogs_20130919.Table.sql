USE [dnstaging]
GO
/****** Object:  Table [dbo].[NestDeathLogs_20130919]    Script Date: 11/14/2026 9:00:08 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[NestDeathLogs_20130919](
	[LogDate] [datetime2](0) NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[CharacterLevel] [tinyint] NOT NULL,
	[JobCode] [tinyint] NOT NULL,
	[MonsterSkillCode] [int] NOT NULL,
	[MonsterCode] [int] NOT NULL,
	[StageStartID] [bigint] NULL,
	[MapID] [int] NOT NULL,
	[WorldID] [int] NOT NULL,
	[PartySize] [tinyint] NOT NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_NestDeathLogs_20130919_LogDate]    Script Date: 11/14/2026 9:00:08 AM ******/
CREATE CLUSTERED INDEX [IX_NC_NestDeathLogs_20130919_LogDate] ON [dbo].[NestDeathLogs_20130919]
(
	[LogDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_NestDeathLogs_20130919_CharacterID]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_NestDeathLogs_20130919_CharacterID] ON [dbo].[NestDeathLogs_20130919]
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
ALTER TABLE [dbo].[NestDeathLogs_20130919]  WITH CHECK ADD  CONSTRAINT [CK_NestDeathLogs_20130919_LogDate] CHECK  (([LogDate]>='2013-09-19' AND [LogDate]<'2013-09-20'))
GO
ALTER TABLE [dbo].[NestDeathLogs_20130919] CHECK CONSTRAINT [CK_NestDeathLogs_20130919_LogDate]
GO

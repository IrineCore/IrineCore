USE [dnstaging]
GO
/****** Object:  Table [dbo].[SkillPointLogs_20130914]    Script Date: 11/14/2026 9:00:07 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[SkillPointLogs_20130914](
	[LogDate] [datetime2](0) NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[SkillPointChangeCode] [tinyint] NOT NULL,
	[SkillPointBefore] [smallint] NOT NULL,
	[SkillPointAfter] [smallint] NOT NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_SkillPointLogs_20130914_LogDate]    Script Date: 11/14/2026 9:00:07 AM ******/
CREATE CLUSTERED INDEX [IX_NC_SkillPointLogs_20130914_LogDate] ON [dbo].[SkillPointLogs_20130914]
(
	[LogDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_SkillPointLogs_20130914_CharacterID]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_SkillPointLogs_20130914_CharacterID] ON [dbo].[SkillPointLogs_20130914]
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
ALTER TABLE [dbo].[SkillPointLogs_20130914]  WITH CHECK ADD  CONSTRAINT [CK_SkillPointLogs_20130914_LogDate] CHECK  (([LogDate]>='2013-09-14' AND [LogDate]<'2013-09-15'))
GO
ALTER TABLE [dbo].[SkillPointLogs_20130914] CHECK CONSTRAINT [CK_SkillPointLogs_20130914_LogDate]
GO

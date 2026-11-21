USE [dnstaging]
GO
/****** Object:  Table [dbo].[SkillLogs_20130916]    Script Date: 11/14/2026 9:00:07 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[SkillLogs_20130916](
	[LogDate] [datetime2](0) NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[SkillChangeCode] [tinyint] NOT NULL,
	[SkillID] [int] NOT NULL,
	[SkillLevelBefore] [tinyint] NOT NULL,
	[SkillLevelAfter] [tinyint] NOT NULL,
	[ChannelID] [int] NULL,
	[MapID] [int] NULL,
	[SkillCode] [tinyint] NOT NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_SkillLogs_20130916_LogDate]    Script Date: 11/14/2026 9:00:07 AM ******/
CREATE CLUSTERED INDEX [IX_NC_SkillLogs_20130916_LogDate] ON [dbo].[SkillLogs_20130916]
(
	[LogDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_SkillLogs_20130916_CharacterID]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_SkillLogs_20130916_CharacterID] ON [dbo].[SkillLogs_20130916]
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
ALTER TABLE [dbo].[SkillLogs_20130916]  WITH CHECK ADD  CONSTRAINT [CK_SkillLogs_20130916_LogDate] CHECK  (([LogDate]>='2013-09-16' AND [LogDate]<'2013-09-17'))
GO
ALTER TABLE [dbo].[SkillLogs_20130916] CHECK CONSTRAINT [CK_SkillLogs_20130916_LogDate]
GO

USE [dnworld]
GO
/****** Object:  Table [dbo].[SkillLogs2]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[SkillLogs2](
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
/****** Object:  Index [IX_NC_SkillLogs2_LogDate]    Script Date: 11/14/2026 8:54:16 AM ******/
CREATE CLUSTERED INDEX [IX_NC_SkillLogs2_LogDate] ON [dbo].[SkillLogs2]
(
	[LogDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
ALTER TABLE [dbo].[SkillLogs2] ADD  CONSTRAINT [DF_SkillLogs2_SkillCode]  DEFAULT ((1)) FOR [SkillCode]
GO

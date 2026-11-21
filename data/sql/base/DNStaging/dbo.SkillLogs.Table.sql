USE [dnstaging]
GO
/****** Object:  Table [dbo].[SkillLogs]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[SkillLogs](
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

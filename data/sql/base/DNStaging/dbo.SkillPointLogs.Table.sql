USE [dnstaging]
GO
/****** Object:  Table [dbo].[SkillPointLogs]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[SkillPointLogs](
	[LogDate] [datetime2](0) NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[SkillPointChangeCode] [tinyint] NOT NULL,
	[SkillPointBefore] [smallint] NOT NULL,
	[SkillPointAfter] [smallint] NOT NULL
) ON [PRIMARY]
GO

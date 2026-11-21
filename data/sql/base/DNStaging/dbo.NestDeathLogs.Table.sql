USE [dnstaging]
GO
/****** Object:  Table [dbo].[NestDeathLogs]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[NestDeathLogs](
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

USE [dnstaging]
GO
/****** Object:  Table [dbo].[CharactersBuffer]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[CharactersBuffer](
	[CharacterID] [bigint] NOT NULL,
	[CharacterClassCode] [tinyint] NOT NULL,
	[CharacterIndex] [tinyint] NOT NULL,
	[VillageFirstVisitFlag] [bit] NOT NULL,
	[SkillResetFlag] [bit] NOT NULL,
	[CharacterLevel] [tinyint] NOT NULL,
	[CharacterExp] [int] NOT NULL,
	[MissionScore] [int] NOT NULL,
	[JobCode] [tinyint] NOT NULL,
	[Coin] [bigint] NOT NULL,
	[WarehouseCoin] [bigint] NOT NULL,
	[SkillPoint] [smallint] NOT NULL,
	[LastLoginDate] [datetime2](0) NOT NULL
) ON [PRIMARY]
GO

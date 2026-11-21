USE [dnstaging]
GO
/****** Object:  Table [dbo].[WeeklyRank_PVPKillDeath]    Script Date: 11/14/2026 9:00:07 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[WeeklyRank_PVPKillDeath](
	[CharacterID] [bigint] NOT NULL,
	[WorldID] [int] NOT NULL,
	[CharacterLevel] [tinyint] NOT NULL,
	[JobCode] [tinyint] NOT NULL,
	[PVPLevel] [tinyint] NOT NULL,
	[KillCount] [int] NOT NULL,
	[DeathCount] [int] NOT NULL,
	[vsWarriorKOWin] [int] NOT NULL,
	[vsWarriorKOLose] [int] NOT NULL,
	[vsArcherKOWin] [int] NOT NULL,
	[vsArcherKOLose] [int] NOT NULL,
	[vsSorceressKOWin] [int] NOT NULL,
	[vsSorceressKOLose] [int] NOT NULL,
	[vsClericKOWin] [int] NOT NULL,
	[vsClericKOLose] [int] NOT NULL,
	[TotalKillRank] [int] NOT NULL,
	[WorldKillRank] [int] NOT NULL,
	[WorldCharacterClassKillRank] [int] NOT NULL,
	[TotalDeathRank] [int] NOT NULL,
	[WorldDeathRank] [int] NOT NULL,
	[WorldCharacterClassDeathRank] [int] NOT NULL,
	[CharacterClassKillRank] [int] NULL,
	[CharacterClassDeathRank] [int] NULL,
	[vsAcademicKOWin] [int] NOT NULL,
	[vsAcademicKOLose] [int] NOT NULL,
	[vsKaliKOWin] [int] NOT NULL,
	[vsKaliKOLose] [int] NOT NULL,
	[vsAssasinKOWin] [int] NULL,
	[vsAssasinKOLose] [int] NULL,
 CONSTRAINT [PK_WeeklyRank_PVPKillDeath] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO

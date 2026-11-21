USE [dnstaging]
GO
/****** Object:  Table [dbo].[SS_PVPScores]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[SS_PVPScores](
	[CharacterID] [bigint] NOT NULL,
	[PVPWin] [int] NOT NULL,
	[PVPLose] [int] NOT NULL,
	[PVPDraw] [int] NOT NULL,
	[PVPGiveUp] [int] NOT NULL,
	[PVPLevel] [tinyint] NOT NULL,
	[PVPExp] [int] NOT NULL,
	[PVPPlayTime] [int] NOT NULL,
	[PVPKillPoint] [bigint] NOT NULL,
	[PVPAssisPoint] [bigint] NOT NULL,
	[PVPSupportPoint] [bigint] NOT NULL,
	[vsWarriorKOWin] [int] NOT NULL,
	[vsWarriorKOLose] [int] NOT NULL,
	[vsArcherKOWin] [int] NOT NULL,
	[vsArcherKOLose] [int] NOT NULL,
	[vsSorceressKOWin] [int] NOT NULL,
	[vsSorceressKOLose] [int] NOT NULL,
	[vsClericKOWin] [int] NOT NULL,
	[vsClericKOLose] [int] NOT NULL,
	[vsAcademicKOWin] [int] NOT NULL,
	[vsAcademicKOLose] [int] NOT NULL,
	[vsKaliKOWin] [int] NOT NULL,
	[vsKaliKOLose] [int] NOT NULL,
	[vsAssasinKOWin] [int] NULL,
	[vsAssasinKOLose] [int] NULL,
 CONSTRAINT [PK_SS_PVPScores] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO

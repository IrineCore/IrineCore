USE [dnworld]
GO
/****** Object:  Table [dbo].[PVPScores]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PVPScores](
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
	[vsWarriorKOWin] [int] NULL,
	[vsWarriorKOLose] [int] NULL,
	[vsArcherKOWin] [int] NULL,
	[vsArcherKOLose] [int] NULL,
	[vsSorceressKOWin] [int] NULL,
	[vsSorceressKOLose] [int] NULL,
	[vsClericKOWin] [int] NULL,
	[vsClericKOLose] [int] NULL,
	[vsAcademicKOWin] [int] NULL,
	[vsAcademicKOLose] [int] NULL,
 CONSTRAINT [PK_PVPScores] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO

USE [dnstaging]
GO
/****** Object:  Table [dbo].[PVPEndLogs]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PVPEndLogs](
	[PVPStartDate] [datetime2](0) NOT NULL,
	[PVPEndDate] [datetime2](0) NULL,
	[PVPStartID] [bigint] NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[TeamCode] [tinyint] NOT NULL,
	[BreakIntoFlag] [bit] NOT NULL,
	[PVPResultCode] [tinyint] NOT NULL,
	[PVPPlayTime] [int] NOT NULL,
	[PVPKillPoint] [int] NOT NULL,
	[PVPAssistPoint] [int] NOT NULL,
	[PVPSupportPoint] [int] NOT NULL,
	[vsWarriorKOWin] [int] NOT NULL,
	[vsWarriorKOLose] [int] NOT NULL,
	[vsArcherKOWin] [int] NOT NULL,
	[vsArcherKOLose] [int] NOT NULL,
	[vsSorceressKOWin] [int] NOT NULL,
	[vsSorceressKOLose] [int] NOT NULL,
	[vsClericKOWin] [int] NOT NULL,
	[vsClericKOLose] [int] NOT NULL,
	[WorldID] [int] NULL,
	[vsAcademicKOWin] [int] NOT NULL,
	[vsAcademicKOLose] [int] NOT NULL,
	[vsKaliKOWin] [int] NOT NULL,
	[vsKaliKOLose] [int] NOT NULL,
	[OccupationCount] [int] NULL,
	[OccupationTimeLimitWinFlag] [bit] NULL,
	[vsAssasinKOWin] [int] NOT NULL,
	[vsAssasinKOLose] [int] NOT NULL
) ON [PRIMARY]
GO

USE [dnworld]
GO
/****** Object:  Table [dbo].[PVPEndLogs0]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [dbo].[PVPEndLogs0](
	[PVPStartID] [bigint] NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[TeamCode] [tinyint] NOT NULL,
	[BreakIntoFlag] [bit] NOT NULL,
	[PVPResultCode] [tinyint] NOT NULL,
	[PVPPlayTime] [int] NOT NULL,
	[PVPKillPoint] [int] NOT NULL,
	[PVPAssistPoint] [int] NOT NULL,
	[PVPSupportPoint] [int] NOT NULL,
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
	[CharacterClassWinLose] [varchar](max) COLLATE Chinese_PRC_CI_AS NULL,
	[OccupationCount] [int] NULL,
	[OccupationTimeLimitWinFlag] [bit] NULL
) ON [DataFG1] TEXTIMAGE_ON [DataFG1]
GO
SET ANSI_PADDING OFF
GO
/****** Object:  Index [IX_UN_PVPEndLogs0_PVPStartID_CharacterID]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE UNIQUE NONCLUSTERED INDEX [IX_UN_PVPEndLogs0_PVPStartID_CharacterID] ON [dbo].[PVPEndLogs0]
(
	[PVPStartID] ASC,
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO

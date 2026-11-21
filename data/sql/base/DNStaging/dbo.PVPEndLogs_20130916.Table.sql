USE [dnstaging]
GO
/****** Object:  Table [dbo].[PVPEndLogs_20130916]    Script Date: 11/14/2026 9:00:08 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PVPEndLogs_20130916](
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
/****** Object:  Index [IX_NC_PVPEndLogs_20130916_PVPStartDate]    Script Date: 11/14/2026 9:00:08 AM ******/
CREATE CLUSTERED INDEX [IX_NC_PVPEndLogs_20130916_PVPStartDate] ON [dbo].[PVPEndLogs_20130916]
(
	[PVPStartDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_PVPEndLogs_20130916_CharacterID]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_PVPEndLogs_20130916_CharacterID] ON [dbo].[PVPEndLogs_20130916]
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_PVPEndLogs_20130916_PVPStartID]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_PVPEndLogs_20130916_PVPStartID] ON [dbo].[PVPEndLogs_20130916]
(
	[PVPStartID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
ALTER TABLE [dbo].[PVPEndLogs_20130916]  WITH CHECK ADD  CONSTRAINT [CK_PVPEndLogs_20130916_PVPStartDate] CHECK  (([PVPStartDate]>='2013-09-16' AND [PVPStartDate]<'2013-09-17'))
GO
ALTER TABLE [dbo].[PVPEndLogs_20130916] CHECK CONSTRAINT [CK_PVPEndLogs_20130916_PVPStartDate]
GO

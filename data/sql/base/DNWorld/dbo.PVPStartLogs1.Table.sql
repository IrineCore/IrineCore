USE [dnworld]
GO
/****** Object:  Table [dbo].[PVPStartLogs1]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PVPStartLogs1](
	[PVPStartID] [bigint] NOT NULL,
	[MaxPlayerCount] [tinyint] NOT NULL,
	[PVPModeCode] [tinyint] NOT NULL,
	[PVPModeDetailCode] [tinyint] NOT NULL,
	[BreakIntoFlag] [bit] NOT NULL,
	[ItemUseFlag] [bit] NOT NULL,
	[SecreteFlag] [bit] NOT NULL,
	[MapID] [int] NOT NULL,
	[RoomOpenCharacterID] [bigint] NULL,
	[CheifCharacterID] [bigint] NULL
) ON [DataFG1]
GO
/****** Object:  Index [IX_UN_PVPStartLogs1_PVPStartID]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE UNIQUE NONCLUSTERED INDEX [IX_UN_PVPStartLogs1_PVPStartID] ON [dbo].[PVPStartLogs1]
(
	[PVPStartID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO

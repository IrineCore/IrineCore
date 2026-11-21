USE [dnstaging]
GO
/****** Object:  Table [dbo].[PVPLogs]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PVPLogs](
	[PVPStartDate] [datetime2](0) NOT NULL,
	[PVPEndDate] [datetime2](0) NULL,
	[PVPStartID] [bigint] NOT NULL,
	[MaxPlayerCount] [tinyint] NOT NULL,
	[PVPModeCode] [tinyint] NOT NULL,
	[PVPModeDetailCode] [tinyint] NOT NULL,
	[BreakIntoFlag] [bit] NOT NULL,
	[ItemUseFlag] [bit] NOT NULL,
	[SecreteFlag] [bit] NOT NULL,
	[MapID] [int] NOT NULL,
	[WorldID] [int] NULL,
	[RoomOpenCharacterID] [bigint] NULL,
	[CheifCharacterID] [bigint] NULL
) ON [PRIMARY]
GO

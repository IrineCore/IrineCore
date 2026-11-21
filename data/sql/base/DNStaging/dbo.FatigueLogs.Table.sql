USE [dnstaging]
GO
/****** Object:  Table [dbo].[FatigueLogs]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[FatigueLogs](
	[LogDate] [datetime2](0) NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[FatigueTypeCode] [tinyint] NOT NULL,
	[CurrentFatigue] [smallint] NOT NULL,
	[ChangedFatigue] [smallint] NOT NULL,
	[ChannelID] [int] NULL,
	[MapID] [int] NULL
) ON [PRIMARY]
GO

USE [dnstaging]
GO
/****** Object:  Table [dbo].[ItemAttributeLogs]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ItemAttributeLogs](
	[LogDate] [datetime2](0) NOT NULL,
	[WorldID] [int] NOT NULL,
	[ItemSerial] [bigint] NOT NULL,
	[ItemDurabilityBefore] [smallint] NOT NULL,
	[ItemDurabilityAfter] [smallint] NOT NULL,
	[ItemLevelBefore] [tinyint] NOT NULL,
	[ItemLevelAfter] [tinyint] NOT NULL,
	[ItemPotentialBefore] [tinyint] NOT NULL,
	[ItemPotentialAfter] [tinyint] NOT NULL,
	[SoulBoundFlagBefore] [bit] NOT NULL,
	[SoulBoundFlagAfter] [bit] NOT NULL,
	[SealCountBefore] [tinyint] NOT NULL,
	[SealCountAfter] [tinyint] NOT NULL,
	[ChannelID] [int] NULL,
	[MapID] [int] NULL,
	[IP] [int] NULL
) ON [PRIMARY]
GO

USE [dnstaging]
GO
/****** Object:  Table [dbo].[ItemAttributeLogs_20130918]    Script Date: 11/14/2026 9:00:08 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ItemAttributeLogs_20130918](
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
/****** Object:  Index [IX_NC_ItemAttributeLogs_20130918_LogDate]    Script Date: 11/14/2026 9:00:08 AM ******/
CREATE CLUSTERED INDEX [IX_NC_ItemAttributeLogs_20130918_LogDate] ON [dbo].[ItemAttributeLogs_20130918]
(
	[LogDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_ItemAttributeLogs_20130918_ItemSerial]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_ItemAttributeLogs_20130918_ItemSerial] ON [dbo].[ItemAttributeLogs_20130918]
(
	[ItemSerial] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
ALTER TABLE [dbo].[ItemAttributeLogs_20130918]  WITH CHECK ADD  CONSTRAINT [CK_ItemAttributeLogs_20130918_LogDate] CHECK  (([LogDate]>='2013-09-18' AND [LogDate]<'2013-09-19'))
GO
ALTER TABLE [dbo].[ItemAttributeLogs_20130918] CHECK CONSTRAINT [CK_ItemAttributeLogs_20130918_LogDate]
GO

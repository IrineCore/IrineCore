USE [dnstaging]
GO
/****** Object:  Table [dbo].[ItemLogs_20130920]    Script Date: 11/14/2026 9:00:08 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ItemLogs_20130920](
	[LogDate] [datetime2](0) NOT NULL,
	[ItemSerial] [bigint] NOT NULL,
	[ItemID] [int] NOT NULL,
	[ItemLogCode] [tinyint] NOT NULL,
	[OwnerCharacterID] [bigint] NULL,
	[OwnerCharacterIDBefore] [bigint] NULL,
	[ItemCount] [smallint] NOT NULL,
	[ItemCountBefore] [smallint] NOT NULL,
	[ItemLogFKey1] [bigint] NULL,
	[ItemLogFKey2] [bigint] NULL,
	[OwnershipStartDate] [datetime2](0) NULL,
	[IP] [int] NULL,
	[WorldID] [int] NOT NULL,
	[ItemLevel] [tinyint] NULL,
	[ItemPotential] [tinyint] NULL,
	[SealCount] [tinyint] NULL,
	[SoulBoundFlag] [bit] NULL,
	[ItemOption] [tinyint] NULL,
	[ItemLevelBefore] [tinyint] NULL,
	[ItemPotentialMoveCount] [tinyint] NULL,
	[DragonJewelType] [tinyint] NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_ItemLogs_20130920_LogDate]    Script Date: 11/14/2026 9:00:08 AM ******/
CREATE CLUSTERED INDEX [IX_NC_ItemLogs_20130920_LogDate] ON [dbo].[ItemLogs_20130920]
(
	[LogDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_ItemLogs_20130920_IP]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_ItemLogs_20130920_IP] ON [dbo].[ItemLogs_20130920]
(
	[IP] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_ItemLogs_20130920_ItemID]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_ItemLogs_20130920_ItemID] ON [dbo].[ItemLogs_20130920]
(
	[ItemID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_ItemLogs_20130920_ItemLogCode_ItemLogFKey1]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_ItemLogs_20130920_ItemLogCode_ItemLogFKey1] ON [dbo].[ItemLogs_20130920]
(
	[ItemLogCode] ASC,
	[ItemLogFKey1] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_ItemLogs_20130920_ItemSerial]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_ItemLogs_20130920_ItemSerial] ON [dbo].[ItemLogs_20130920]
(
	[ItemSerial] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_ItemLogs_20130920_OwnerCharacterID]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_ItemLogs_20130920_OwnerCharacterID] ON [dbo].[ItemLogs_20130920]
(
	[OwnerCharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_ItemLogs_20130920_OwnerCharacterIDBefore]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_ItemLogs_20130920_OwnerCharacterIDBefore] ON [dbo].[ItemLogs_20130920]
(
	[OwnerCharacterIDBefore] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
ALTER TABLE [dbo].[ItemLogs_20130920]  WITH CHECK ADD  CONSTRAINT [CK_ItemLogs_20130920_LogDate] CHECK  (([LogDate]>='2013-09-20' AND [LogDate]<'2013-09-21'))
GO
ALTER TABLE [dbo].[ItemLogs_20130920] CHECK CONSTRAINT [CK_ItemLogs_20130920_LogDate]
GO

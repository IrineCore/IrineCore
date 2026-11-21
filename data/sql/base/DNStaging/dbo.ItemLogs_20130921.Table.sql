USE [dnstaging]
GO
/****** Object:  Table [dbo].[ItemLogs_20130921]    Script Date: 11/14/2026 9:00:08 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ItemLogs_20130921](
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
/****** Object:  Index [IX_NC_ItemLogs_20130921_LogDate]    Script Date: 11/14/2026 9:00:08 AM ******/
CREATE CLUSTERED INDEX [IX_NC_ItemLogs_20130921_LogDate] ON [dbo].[ItemLogs_20130921]
(
	[LogDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_ItemLogs_20130921_IP]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_ItemLogs_20130921_IP] ON [dbo].[ItemLogs_20130921]
(
	[IP] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_ItemLogs_20130921_ItemID]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_ItemLogs_20130921_ItemID] ON [dbo].[ItemLogs_20130921]
(
	[ItemID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_ItemLogs_20130921_ItemLogCode_ItemLogFKey1]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_ItemLogs_20130921_ItemLogCode_ItemLogFKey1] ON [dbo].[ItemLogs_20130921]
(
	[ItemLogCode] ASC,
	[ItemLogFKey1] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_ItemLogs_20130921_ItemSerial]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_ItemLogs_20130921_ItemSerial] ON [dbo].[ItemLogs_20130921]
(
	[ItemSerial] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_ItemLogs_20130921_OwnerCharacterID]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_ItemLogs_20130921_OwnerCharacterID] ON [dbo].[ItemLogs_20130921]
(
	[OwnerCharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_ItemLogs_20130921_OwnerCharacterIDBefore]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_ItemLogs_20130921_OwnerCharacterIDBefore] ON [dbo].[ItemLogs_20130921]
(
	[OwnerCharacterIDBefore] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
ALTER TABLE [dbo].[ItemLogs_20130921]  WITH CHECK ADD  CONSTRAINT [CK_ItemLogs_20130921_LogDate] CHECK  (([LogDate]>='2013-09-21' AND [LogDate]<'2013-09-22'))
GO
ALTER TABLE [dbo].[ItemLogs_20130921] CHECK CONSTRAINT [CK_ItemLogs_20130921_LogDate]
GO

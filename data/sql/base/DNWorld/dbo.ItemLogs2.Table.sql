USE [dnworld]
GO
/****** Object:  Table [dbo].[ItemLogs2]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ItemLogs2](
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
	[ItemLevel] [tinyint] NULL,
	[ItemPotential] [tinyint] NULL,
	[SealCount] [tinyint] NULL,
	[ItemOption] [tinyint] NULL,
	[SoulBoundFlag] [bit] NULL,
	[ItemLevelBefore] [tinyint] NULL,
	[ItemDurability] [smallint] NULL,
	[ItemPotentialMoveCount] [tinyint] NULL,
	[DragonJewelType] [tinyint] NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_ItemLogs2_LogDate]    Script Date: 11/14/2026 8:54:16 AM ******/
CREATE CLUSTERED INDEX [IX_NC_ItemLogs2_LogDate] ON [dbo].[ItemLogs2]
(
	[LogDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_ItemLogs2_OwnerCharacterID_ItemLogCode]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_ItemLogs2_OwnerCharacterID_ItemLogCode] ON [dbo].[ItemLogs2]
(
	[OwnerCharacterID] ASC,
	[ItemLogCode] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO

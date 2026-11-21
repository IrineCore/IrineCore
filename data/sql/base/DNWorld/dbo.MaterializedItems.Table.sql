USE [dnworld]
GO
/****** Object:  Table [dbo].[MaterializedItems]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[MaterializedItems](
	[MaterializedItemID] [bigint] IDENTITY(1,1) NOT NULL,
	[ItemSerial] [bigint] NOT NULL,
	[ItemID] [int] NOT NULL,
	[ItemRemainCount] [smallint] NOT NULL,
	[PayMethodCode] [tinyint] NULL,
	[SenderCharacterID] [bigint] SPARSE  NULL,
	[ItemMaterializeCode] [tinyint] NOT NULL,
	[ItemMaterializeFKey] [bigint] NULL,
	[ItemDurability] [smallint] NOT NULL,
	[RandomSeed] [int] NOT NULL,
	[CoolTime] [int] NOT NULL,
	[ItemLevel] [tinyint] NOT NULL,
	[ItemPotential] [tinyint] NOT NULL,
	[SoulBoundFlag] [bit] NOT NULL,
	[SealCount] [tinyint] NOT NULL,
	[ItemOption] [tinyint] NOT NULL,
	[ItemMaterializeDate] [datetime2](0) NOT NULL,
	[ItemLifespan] [int] NULL,
	[EternityFlag] [bit] NOT NULL,
	[ItemExpireDate] [smalldatetime] NULL,
	[OwnerCharacterID] [bigint] NULL,
	[OwnershipStartDate] [datetime2](0) NULL,
	[ExpireCompleteFlag] [bit] NOT NULL,
	[TradeItemFlag] [bit] NULL,
	[AdditiveItemID] [int] SPARSE  NULL,
	[ItemLocationCode] [tinyint] NULL,
	[ItemLocationIndex] [tinyint] NULL,
	[ItemPotentialMoveCount] [tinyint] SPARSE  NULL,
	[ItemCount] [smallint] NULL,
	[ItemPrice] [bigint] NULL,
	[DragonJewelType] [tinyint] NULL,
 CONSTRAINT [PK_MaterializedItems] PRIMARY KEY CLUSTERED 
(
	[MaterializedItemID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
/****** Object:  Index [IX_NN_MaterializedItems_ItemID]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_MaterializedItems_ItemID] ON [dbo].[MaterializedItems]
(
	[ItemID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_NNI_MaterializedItems_OwnerCharacterID_ItemLocationCode]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NNI_MaterializedItems_OwnerCharacterID_ItemLocationCode] ON [dbo].[MaterializedItems]
(
	[OwnerCharacterID] ASC,
	[ItemLocationCode] ASC
)
INCLUDE([ItemExpireDate],[EternityFlag]) WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_UNF_MaterializedItems_OwnerCharacterID_ItemLocationCode_ItemLocationIndex]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE UNIQUE NONCLUSTERED INDEX [IX_UNF_MaterializedItems_OwnerCharacterID_ItemLocationCode_ItemLocationIndex] ON [dbo].[MaterializedItems]
(
	[OwnerCharacterID] ASC,
	[ItemLocationCode] ASC,
	[ItemLocationIndex] ASC
)
WHERE ([ItemLocationCode] IN ((1), (7)))
WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_UNI_MaterializedItems_ItemSerial]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE UNIQUE NONCLUSTERED INDEX [IX_UNI_MaterializedItems_ItemSerial] ON [dbo].[MaterializedItems]
(
	[ItemSerial] ASC
)
INCLUDE([OwnerCharacterID],[ItemLocationCode],[ItemLocationIndex]) WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
ALTER TABLE [dbo].[MaterializedItems] ADD  CONSTRAINT [DF_MaterializedItems_EternityFlag]  DEFAULT ((1)) FOR [EternityFlag]
GO
ALTER TABLE [dbo].[MaterializedItems] ADD  CONSTRAINT [DF_MaterializedItems_ExpireCompleteFlag]  DEFAULT ((0)) FOR [ExpireCompleteFlag]
GO
ALTER TABLE [dbo].[MaterializedItems]  WITH CHECK ADD  CONSTRAINT [CK_MaterializedItems_ItemLocationCode_ItemLocationIndex] CHECK  (([ItemLocationCode]>(1) OR [ItemLocationCode]=(1) AND ([ItemLocationIndex]>=(0) AND [ItemLocationIndex]<=(12) OR [ItemLocationIndex]>=(253))))
GO
ALTER TABLE [dbo].[MaterializedItems] CHECK CONSTRAINT [CK_MaterializedItems_ItemLocationCode_ItemLocationIndex]
GO
ALTER TABLE [dbo].[MaterializedItems]  WITH CHECK ADD  CONSTRAINT [CK_MaterializedItems_ItemRemainCount] CHECK  (([ItemRemainCount]>=(0)))
GO
ALTER TABLE [dbo].[MaterializedItems] CHECK CONSTRAINT [CK_MaterializedItems_ItemRemainCount]
GO

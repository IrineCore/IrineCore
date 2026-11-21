USE [dnworld]
GO
/****** Object:  Table [dbo].[DragonJewels]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[DragonJewels](
	[DragonJewelID] [int] IDENTITY(1,1) NOT NULL,
	[ItemID] [int] NOT NULL,
	[ItemSerial] [bigint] NOT NULL,
	[MaterializedItemID] [bigint] NOT NULL,
	[ParentItemID] [int] NOT NULL,
	[ParentItemSerial] [bigint] NOT NULL,
	[ParentItemMaterializedItemID] [bigint] NOT NULL,
	[DragonJewelLocationIndex] [tinyint] NOT NULL,
	[ItemPotential] [tinyint] NULL,
	[ItemOption] [tinyint] NULL,
 CONSTRAINT [PK_DragonJewels] PRIMARY KEY CLUSTERED 
(
	[DragonJewelID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
/****** Object:  Index [IX_NN_DragonJewels_ItemSerial]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_DragonJewels_ItemSerial] ON [dbo].[DragonJewels]
(
	[ItemSerial] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_NN_DragonJewels_ParentItemMaterializedItemID]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_DragonJewels_ParentItemMaterializedItemID] ON [dbo].[DragonJewels]
(
	[ParentItemMaterializedItemID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_UNI_DragonJewels_ParentItemSerial_DragonJewelLocationIndex]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE UNIQUE NONCLUSTERED INDEX [IX_UNI_DragonJewels_ParentItemSerial_DragonJewelLocationIndex] ON [dbo].[DragonJewels]
(
	[ParentItemSerial] ASC,
	[DragonJewelLocationIndex] ASC
)
INCLUDE([ItemID],[ItemOption]) WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO

USE [dnworld]
GO
/****** Object:  Table [dbo].[RepurchaseItem]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[RepurchaseItem](
	[RepurchaseID] [int] IDENTITY(1,1) NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[SellDate] [datetime] NOT NULL,
	[SellPrice] [int] NOT NULL,
	[ItemSerial] [bigint] NULL,
	[ItemID] [int] NOT NULL,
	[ItemRemainCount] [smallint] NOT NULL,
	[ItemDurability] [smallint] NOT NULL,
	[RandomSeed] [int] NOT NULL,
	[CoolTime] [int] NOT NULL,
	[ItemLevel] [tinyint] NOT NULL,
	[ItemPotential] [tinyint] NOT NULL,
	[SoulBoundFlag] [bit] NOT NULL,
	[ItemOption] [tinyint] NOT NULL,
	[SealCount] [tinyint] NOT NULL,
	[ItemMaterializeDate] [datetime2](0) NOT NULL,
	[EternityFlag] [bit] NOT NULL,
	[ItemExpireDate] [smalldatetime] NULL,
	[AdditiveItemID] [int] NULL,
	[ItemPotentialMoveCount] [tinyint] SPARSE  NULL,
	[DragonJewelType] [tinyint] NULL
) ON [DataFG1]
GO
/****** Object:  Index [IX_NC_RepurchaseItem_CharacterID_SellDate]    Script Date: 11/14/2026 8:54:16 AM ******/
CREATE CLUSTERED INDEX [IX_NC_RepurchaseItem_CharacterID_SellDate] ON [dbo].[RepurchaseItem]
(
	[CharacterID] ASC,
	[SellDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
GO
/****** Object:  Index [PK_RepurchaseItem]    Script Date: 11/14/2026 8:54:37 AM ******/
ALTER TABLE [dbo].[RepurchaseItem] ADD  CONSTRAINT [PK_RepurchaseItem] PRIMARY KEY NONCLUSTERED 
(
	[RepurchaseID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
